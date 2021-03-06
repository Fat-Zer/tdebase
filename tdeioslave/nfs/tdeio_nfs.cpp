/*  This file is part of the KDE project

    Copyright (C) 2000 Alexander Neundorf <neundorf@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <arpa/inet.h>

// This is needed on Solaris so that rpc.h defines clnttcp_create etc.
#ifndef PORTMAP
#define PORTMAP
#endif
#include <rpc/rpc.h> // for rpc calls

#include <errno.h>
#include <grp.h>
#include <memory.h>
#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <tqfile.h>
#include <tqdir.h>

#include <kdebug.h>
#include <kinstance.h>
#include <tdelocale.h>

#include <tdeio/global.h>
#include <iostream>

#include "nfs_prot.h"
#define fhandle _fhandle
#include "mount.h"
#include "tdeio_nfs.h"

#define MAXHOSTLEN 256

//#define MAXFHAGE 60*15   //15 minutes maximum age for file handles

//this ioslave is for NFS version 2
#define NFSPROG ((u_long)100003)
#define NFSVERS ((u_long)2)

using namespace TDEIO;
using namespace std;

//this is taken from tdelibs/tdecore/fakes.cpp
//#if !defined(HAVE_GETDOMAINNAME)

int x_getdomainname(char *name, size_t len)
{
   struct utsname uts;
   struct hostent *hent;
   int rv = -1;

   if (name == 0L)
      errno = EINVAL;
   else
   {
      name[0] = '\0';
      if (uname(&uts) >= 0)
      {
         if ((hent = gethostbyname(uts.nodename)) != 0L)
         {
            char *p = strchr(hent->h_name, '.');
            if (p != 0L)
            {
               ++p;
               if (strlen(p) > len-1)
                  errno = EINVAL;
               else
               {
                  strcpy(name, p);
                  rv = 0;
               }
            }
         }
      }
   }
   return rv;
}
//#endif


extern "C" { int KDE_EXPORT kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  TDEInstance instance( "tdeio_nfs" );

  if (argc != 4)
  {
     fprintf(stderr, "Usage: tdeio_nfs protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }
  kdDebug(7121) << "NFS: kdemain: starting" << endl;

  NFSProtocol slave(argv[2], argv[3]);
  slave.dispatchLoop();
  return 0;
}

static bool isRoot(const TQString& path)
{
   return (path.isEmpty() || (path=="/"));
}

static bool isAbsoluteLink(const TQString& path)
{
   //hmm, don't know
   if (path.isEmpty()) return TRUE;
   if (path[0]=='/') return TRUE;
   return FALSE;
}

static void createVirtualDirEntry(UDSEntry & entry)
{
   UDSAtom atom;

   atom.m_uds = TDEIO::UDS_FILE_TYPE;
   atom.m_long = S_IFDIR;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_ACCESS;
   atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_USER;
   atom.m_str = "root";
   entry.append( atom );
   atom.m_uds = TDEIO::UDS_GROUP;
   atom.m_str = "root";
   entry.append( atom );

   //a dummy size
   atom.m_uds = TDEIO::UDS_SIZE;
   atom.m_long = 1024;
   entry.append( atom );
}


static void stripTrailingSlash(TQString& path)
{
   //if (path=="/") return;
   if (path=="/") path="";
   else if (path[path.length()-1]=='/') path.truncate(path.length()-1);
}

static void getLastPart(const TQString& path, TQString& lastPart, TQString& rest)
{
   int slashPos=path.findRev("/");
   lastPart=path.mid(slashPos+1);
   rest=path.left(slashPos+1);
}

static TQString removeFirstPart(const TQString& path)
{
   TQString result("");
   if (path.isEmpty()) return result;
   result=path.mid(1);
   int slashPos=result.find("/");
   return result.mid(slashPos+1);
}

NFSFileHandle::NFSFileHandle()
:m_isInvalid(FALSE)
{
   memset(m_handle,'\0',NFS_FHSIZE+1);
//   m_detectTime=time(0);
}

NFSFileHandle::NFSFileHandle(const NFSFileHandle & handle)
:m_isInvalid(FALSE)
{
   m_handle[NFS_FHSIZE]='\0';
   memcpy(m_handle,handle.m_handle,NFS_FHSIZE);
   m_isInvalid=handle.m_isInvalid;
//   m_detectTime=handle.m_detectTime;
}

NFSFileHandle::~NFSFileHandle()
{}

NFSFileHandle& NFSFileHandle::operator= (const NFSFileHandle& src)
{
   memcpy(m_handle,src.m_handle,NFS_FHSIZE);
   m_isInvalid=src.m_isInvalid;
//   m_detectTime=src.m_detectTime;
   return *this;
}

NFSFileHandle& NFSFileHandle::operator= (const char* src)
{
   if (src==0)
   {
      m_isInvalid=TRUE;
      return *this;
   };
   memcpy(m_handle,src,NFS_FHSIZE);
   m_isInvalid=FALSE;
//   m_detectTime=time(0);
   return *this;
}

/*time_t NFSFileHandle::age() const
{
   return (time(0)-m_detectTime);
}*/


NFSProtocol::NFSProtocol (const TQCString &pool, const TQCString &app )
:SlaveBase( "nfs", pool, app )
,m_client(0)
,m_sock(-1)
,m_lastCheck(time(0))
{
   kdDebug(7121)<<"NFS::NFS: -"<<pool<<"-"<<endl;
}

NFSProtocol::~NFSProtocol()
{
   closeConnection();
}

/*This one is currently unused, so it could be removed.
 The intention was to keep handles around, and from time to time
 remove handles which are too old. Alex
 */
/*void NFSProtocol::checkForOldFHs()
{
   kdDebug(7121)<<"checking for fhs older than "<<MAXFHAGE<<endl;
   kdDebug(7121)<<"current items: "<<m_handleCache.count()<<endl;
   NFSFileHandleMap::Iterator it=m_handleCache.begin();
   NFSFileHandleMap::Iterator lastIt=it;
   while (it!=m_handleCache.end())
   {
      kdDebug(7121)<<it.data().age()<<flush;
      if (it.data().age()>MAXFHAGE)
      {
         kdDebug(7121)<<"removing"<<endl;
         m_handleCache.remove(it);
         if (it==lastIt)
         {
            it=m_handleCache.begin();
            lastIt=it;
         }
         else
            it=lastIt;
      }
      lastIt=it;
      it++;
   };
   kdDebug(7121)<<"left items: "<<m_handleCache.count()<<endl;
   m_lastCheck=time(0);
}*/

void NFSProtocol::closeConnection()
{
   close(m_sock);
   m_sock=-1;
   if (m_client==0) return;
   CLNT_DESTROY(m_client);

   m_client=0;
}

bool NFSProtocol::isExportedDir(const TQString& path)
{
   return (m_exportedDirs.find(path.mid(1))!=m_exportedDirs.end());
}

/* This one works recursive.
 It tries to get the file handle out of the file handle cache.
 If this doesn't succeed, it needs to do a nfs rpc call
 in order to obtain one.
 */
NFSFileHandle NFSProtocol::getFileHandle(TQString path)
{
   if (m_client==0) openConnection();

   //I'm not sure if this is useful
   //if ((time(0)-m_lastCheck)>MAXFHAGE) checkForOldFHs();

   stripTrailingSlash(path);
   kdDebug(7121)<<"getting FH for -"<<path<<"-"<<endl;
   //now the path looks like "/root/some/dir" or "" if it was "/"
   NFSFileHandle parentFH;
   //we didn't find it
   if (path.isEmpty())
   {
      kdDebug(7121)<<"path is empty, invalidating the FH"<<endl;
      parentFH.setInvalid();
      return parentFH;
   }
   //check wether we have this filehandle cached
   //the filehandles of the exported root dirs are always in the cache
   if (m_handleCache.find(path)!=m_handleCache.end())
   {
      kdDebug(7121)<<"path is in the cache, returning the FH -"<<m_handleCache[path]<<"-"<<endl;
      return m_handleCache[path];
   }
   TQString rest, lastPart;
   getLastPart(path,lastPart,rest);
   kdDebug(7121)<<"splitting path into rest -"<<rest<<"- and lastPart -"<<lastPart<<"-"<<endl;

   parentFH=getFileHandle(rest);
   //f*ck, it's invalid
   if (parentFH.isInvalid())
   {
      kdDebug(7121)<<"the parent FH is invalid"<<endl;
      return parentFH;
   }
   // do the rpc call
   diropargs dirargs;
   diropres dirres;
   memcpy(dirargs.dir.data,(const char*)parentFH,NFS_FHSIZE);
   TQCString tmpStr=TQFile::encodeName(lastPart);
   dirargs.name=tmpStr.data();

   //cerr<<"calling rpc: FH: -"<<parentFH<<"- with name -"<<dirargs.name<<"-"<<endl;

   int clnt_stat = clnt_call(m_client, NFSPROC_LOOKUP,
                         (xdrproc_t) xdr_diropargs, (char*)&dirargs,
                         (xdrproc_t) xdr_diropres, (char*)&dirres,total_timeout);

   if ((clnt_stat!=RPC_SUCCESS) || (dirres.status!=NFS_OK))
   {
      //we failed
      kdDebug(7121)<<"lookup of filehandle failed"<<endl;
      parentFH.setInvalid();
      return parentFH;
   }
   //everything went fine up to now :-)
   parentFH=dirres.diropres_u.diropres.file.data;
   //kdDebug(7121)<<"filesize: "<<dirres.diropres_u.diropres.attributes.size<<endl;
   m_handleCache.insert(path,parentFH);
   kdDebug(7121)<<"return FH -"<<parentFH<<"-"<<endl;
   return parentFH;
}

/* Open connection connects to the mount daemon on the server side.
 In order to do this it needs authentication and calls auth_unix_create().
 Then it asks the mount daemon for the exported shares. Then it tries
 to mount all these shares. If this succeeded for at least one of them,
 a client for the nfs daemon is created.
 */
void NFSProtocol::openConnection()
{
   kdDebug(7121)<<"NFS::openConnection for -" << m_currentHost.latin1() << "-" << endl;
   if (m_currentHost.isEmpty())
   {
      error(ERR_UNKNOWN_HOST,"");
      return;
   }
   struct sockaddr_in server_addr;
   if (m_currentHost[0] >= '0' && m_currentHost[0] <= '9')
   {
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = inet_addr(m_currentHost.latin1());
   }
   else
   {
      struct hostent *hp=gethostbyname(m_currentHost.latin1());
      if (hp==0)
      {
         error( ERR_UNKNOWN_HOST, m_currentHost.latin1() );
         return;
      }
      server_addr.sin_family = AF_INET;
      memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);
   }

   // create mount deamon client
   closeConnection();
   server_addr.sin_port = 0;
   m_sock = RPC_ANYSOCK;
   m_client=clnttcp_create(&server_addr,MOUNTPROG, MOUNTVERS, &m_sock, 0, 0);
   if (m_client==0)
   {
      server_addr.sin_port = 0;
      m_sock = RPC_ANYSOCK;
      pertry_timeout.tv_sec = 3;
      pertry_timeout.tv_usec = 0;
      m_client = clntudp_create(&server_addr,MOUNTPROG, MOUNTVERS, pertry_timeout, &m_sock);
      if (m_client==0)
      {
         clnt_pcreateerror(const_cast<char *>("mount clntudp_create"));
         error(ERR_COULD_NOT_CONNECT, m_currentHost.latin1());
         return;
      }
   }
   TQCString hostName("localhost");
   char nameBuffer[1024];
   nameBuffer[0] = '\0';
   if (gethostname(nameBuffer, 1024)==0)
   {
      nameBuffer[sizeof(nameBuffer)-1] = '\0';
      hostName=nameBuffer;
      // I have the same problem here as Stefan Westerfeld, that's why I use
      // the getdomainname() from fakes.cpp (renamed to x_getdomainname()), this one works
      // taken from tdelibs/arts/mcopy/mcoputils.cc
      nameBuffer[0] = '\0';
      if (x_getdomainname(nameBuffer, 1024)==0)
      {
         nameBuffer[sizeof(nameBuffer)-1] = '\0';
         /*
          * I don't know why, but on my linux machine, the domainname
          * always ends up being (none), which is certainly no valid
          * domainname
          */
         if(strcmp(nameBuffer,"(none)") != 0) {
            hostName += ".";
            hostName += nameBuffer;
         }
      }
   }
   kdDebug(7121) << "hostname is -" << hostName << "-" << endl;
   m_client->cl_auth = authunix_create(hostName.data(), geteuid(), getegid(), 0, 0);
   total_timeout.tv_sec = 20;
   total_timeout.tv_usec = 0;

   exports exportlist;
   //now do the stuff
   memset(&exportlist, '\0', sizeof(exportlist));

   int clnt_stat = clnt_call(m_client, MOUNTPROC_EXPORT,(xdrproc_t) xdr_void, NULL,
                         (xdrproc_t) xdr_exports, (char*)&exportlist,total_timeout);
   if (!checkForError(clnt_stat, 0, m_currentHost.latin1())) return;

   fhstatus fhStatus;
   bool atLeastOnceSucceeded(FALSE);
   for(; exportlist!=0;exportlist = exportlist->ex_next) {
      kdDebug(7121) << "found export: " << exportlist->ex_dir << endl;

      memset(&fhStatus, 0, sizeof(fhStatus));
      clnt_stat = clnt_call(m_client, MOUNTPROC_MNT,(xdrproc_t) xdr_dirpath, (char*)(&(exportlist->ex_dir)),
                            (xdrproc_t) xdr_fhstatus,(char*) &fhStatus,total_timeout);
      if (fhStatus.fhs_status==0) {
         atLeastOnceSucceeded=TRUE;
         NFSFileHandle fh;
         fh=fhStatus.fhstatus_u.fhs_fhandle;
         TQString fname;
         if ( exportlist->ex_dir[0] == '/' )
            fname = TDEIO::encodeFileName(exportlist->ex_dir + 1);
         else
            fname = TDEIO::encodeFileName(exportlist->ex_dir);
         m_handleCache.insert(TQString("/")+fname,fh);
         m_exportedDirs.append(fname);
         // kdDebug() <<"appending file -"<<fname<<"- with FH: -"<<fhStatus.fhstatus_u.fhs_fhandle<<"-"<<endl;
      }
   }
   if (!atLeastOnceSucceeded)
   {
      closeConnection();
      error( ERR_COULD_NOT_AUTHENTICATE, m_currentHost.latin1());
      return;
   }
   server_addr.sin_port = 0;

   //now create the client for the nfs daemon
   //first get rid of the old one
   closeConnection();
   m_sock = RPC_ANYSOCK;
   m_client = clnttcp_create(&server_addr,NFSPROG,NFSVERS,&m_sock,0,0);
   if (m_client == 0)
   {
      server_addr.sin_port = 0;
      m_sock = RPC_ANYSOCK;
      pertry_timeout.tv_sec = 3;
      pertry_timeout.tv_usec = 0;
      m_client = clntudp_create(&server_addr,NFSPROG, NFSVERS, pertry_timeout, &m_sock);
      if (m_client==0)
      {
         clnt_pcreateerror(const_cast<char *>("NFS clntudp_create"));
         error(ERR_COULD_NOT_CONNECT, m_currentHost.latin1());
         return;
      }
   }
   m_client->cl_auth = authunix_create(hostName.data(),geteuid(),getegid(),0,0);
   connected();
   kdDebug(7121)<<"openConnection succeeded"<<endl;
}

void NFSProtocol::listDir( const KURL& _url)
{
   KURL url(_url);
   TQString path( TQFile::encodeName(url.path()));

   if (path.isEmpty())
   {
      url.setPath("/");
      redirection(url);
      finished();
      return;
   }
   //open the connection
   if (m_client==0) openConnection();
   //it failed
   if (m_client==0) return;
   if (isRoot(path))
   {
      kdDebug(7121)<<"listing root"<<endl;
      totalSize( m_exportedDirs.count());
      //in this case we don't need to do a real listdir
      UDSEntry entry;
      for (TQStringList::Iterator it=m_exportedDirs.begin(); it!=m_exportedDirs.end(); it++)
      {
         UDSAtom atom;
         entry.clear();
         atom.m_uds = TDEIO::UDS_NAME;
         atom.m_str = (*it);
         kdDebug(7121)<<"listing "<<(*it)<<endl;
         entry.append( atom );
         createVirtualDirEntry(entry);
         listEntry( entry, false);
      }
      listEntry( entry, true ); // ready
      finished();
      return;
   }

   TQStringList filesToList;
   kdDebug(7121)<<"getting subdir -"<<path<<"-"<<endl;
   stripTrailingSlash(path);
   NFSFileHandle fh=getFileHandle(path);
   //cerr<<"this is the fh: -"<<fh<<"-"<<endl;
   if (fh.isInvalid())
   {
      error( ERR_DOES_NOT_EXIST, path);
      return;
   }
   readdirargs listargs;
   memset(&listargs,0,sizeof(listargs));
   listargs.count=1024*16;
   memcpy(listargs.dir.data,fh,NFS_FHSIZE);
   readdirres listres;
   do
   {
      memset(&listres,'\0',sizeof(listres));
      int clnt_stat = clnt_call(m_client, NFSPROC_READDIR, (xdrproc_t) xdr_readdirargs, (char*)&listargs,
                                (xdrproc_t) xdr_readdirres, (char*)&listres,total_timeout);
      if (!checkForError(clnt_stat,listres.status,path)) return;
      for (entry *dirEntry=listres.readdirres_u.reply.entries;dirEntry!=0;dirEntry=dirEntry->nextentry)
      {
         if ((TQString(".")!=dirEntry->name) && (TQString("..")!=dirEntry->name))
            filesToList.append(dirEntry->name);
      }
   } while (!listres.readdirres_u.reply.eof);
   totalSize( filesToList.count());

   UDSEntry entry;
   //stat all files in filesToList
   for (TQStringList::Iterator it=filesToList.begin(); it!=filesToList.end(); it++)
   {
      UDSAtom atom;
      diropargs dirargs;
      diropres dirres;
      memcpy(dirargs.dir.data,fh,NFS_FHSIZE);
      TQCString tmpStr=TQFile::encodeName(*it);
      dirargs.name=tmpStr.data();

      kdDebug(7121)<<"calling rpc: FH: -"<<fh<<"- with name -"<<dirargs.name<<"-"<<endl;

      int clnt_stat= clnt_call(m_client, NFSPROC_LOOKUP,
                         (xdrproc_t) xdr_diropargs, (char*)&dirargs,
                         (xdrproc_t) xdr_diropres, (char*)&dirres,total_timeout);
      if (!checkForError(clnt_stat,dirres.status,(*it))) return;

      NFSFileHandle tmpFH;
      tmpFH=dirres.diropres_u.diropres.file.data;
      m_handleCache.insert(path+"/"+(*it),tmpFH);

      entry.clear();

      atom.m_uds = TDEIO::UDS_NAME;
      atom.m_str = (*it);
      entry.append( atom );

      //is it a symlink ?
      if (S_ISLNK(dirres.diropres_u.diropres.attributes.mode))
      {
         kdDebug(7121)<<"it's a symlink !"<<endl;
         //cerr<<"fh: "<<tmpFH<<endl;
         nfs_fh nfsFH;
         memcpy(nfsFH.data,dirres.diropres_u.diropres.file.data,NFS_FHSIZE);
         //get the link dest
         readlinkres readLinkRes;
         char nameBuf[NFS_MAXPATHLEN];
         readLinkRes.readlinkres_u.data=nameBuf;
         int clnt_stat=clnt_call(m_client, NFSPROC_READLINK,
                                 (xdrproc_t) xdr_nfs_fh, (char*)&nfsFH,
                                 (xdrproc_t) xdr_readlinkres, (char*)&readLinkRes,total_timeout);
         if (!checkForError(clnt_stat,readLinkRes.status,(*it))) return;
         kdDebug(7121)<<"link dest is -"<<readLinkRes.readlinkres_u.data<<"-"<<endl;
         TQCString linkDest(readLinkRes.readlinkres_u.data);
         atom.m_uds = TDEIO::UDS_LINK_DEST;
         atom.m_str = linkDest;
         entry.append( atom );

         bool isValid=isValidLink(path,linkDest);
         if (!isValid)
         {
            completeBadLinkUDSEntry(entry,dirres.diropres_u.diropres.attributes);
         }
         else
         {
            if (isAbsoluteLink(linkDest))
            {
               completeAbsoluteLinkUDSEntry(entry,linkDest);
            }
            else
            {
               tmpStr=TQDir::cleanDirPath(path+TQString("/")+TQString(linkDest)).latin1();
               dirargs.name=tmpStr.data();
               tmpFH=getFileHandle(tmpStr);
               memcpy(dirargs.dir.data,tmpFH,NFS_FHSIZE);

               attrstat attrAndStat;

               kdDebug(7121)<<"calling rpc: FH: -"<<fh<<"- with name -"<<dirargs.name<<"-"<<endl;

               clnt_stat = clnt_call(m_client, NFSPROC_GETATTR,
                                         (xdrproc_t) xdr_diropargs, (char*)&dirargs,
                                         (xdrproc_t) xdr_attrstat, (char*)&attrAndStat,total_timeout);
               if (!checkForError(clnt_stat,attrAndStat.status,tmpStr)) return;
               completeUDSEntry(entry,attrAndStat.attrstat_u.attributes);
            }
         }
      }
      else
         completeUDSEntry(entry,dirres.diropres_u.diropres.attributes);
      listEntry( entry, false);
   }
   listEntry( entry, true ); // ready
   finished();
}

void NFSProtocol::stat( const KURL & url)
{
   TQString path( TQFile::encodeName(url.path()));
   stripTrailingSlash(path);
   kdDebug(7121)<<"NFS::stat for -"<<path<<"-"<<endl;
   TQString tmpPath=path;
   if ((tmpPath.length()>1) && (tmpPath[0]=='/')) tmpPath=tmpPath.mid(1);
   // We can't stat root, but we know it's a dir
   if (isRoot(path) || isExportedDir(path))
   {
      UDSEntry entry;
      UDSAtom atom;

      atom.m_uds = TDEIO::UDS_NAME;
      atom.m_str = path;
      entry.append( atom );
      createVirtualDirEntry(entry);
      // no size
      statEntry( entry );
      finished();
      kdDebug(7121)<<"succeeded"<<endl;
      return;
   }

   NFSFileHandle fh=getFileHandle(path);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,path);
      return;
   }

   diropargs dirargs;
   attrstat attrAndStat;
   memcpy(dirargs.dir.data,fh,NFS_FHSIZE);
   TQCString tmpStr=TQFile::encodeName(path);
   dirargs.name=tmpStr.data();

   kdDebug(7121)<<"calling rpc: FH: -"<<fh<<"- with name -"<<dirargs.name<<"-"<<endl;

   int clnt_stat = clnt_call(m_client, NFSPROC_GETATTR,
                         (xdrproc_t) xdr_diropargs, (char*)&dirargs,
                         (xdrproc_t) xdr_attrstat, (char*)&attrAndStat,total_timeout);
   if (!checkForError(clnt_stat,attrAndStat.status,path)) return;
   UDSEntry entry;
   entry.clear();

   UDSAtom atom;

   TQString fileName, parentDir;
   getLastPart(path, fileName, parentDir);
   stripTrailingSlash(parentDir);

   atom.m_uds = TDEIO::UDS_NAME;
   atom.m_str = fileName;
   entry.append( atom );

   //is it a symlink ?
   if (S_ISLNK(attrAndStat.attrstat_u.attributes.mode))
   {
      kdDebug(7121)<<"it's a symlink !"<<endl;
      nfs_fh nfsFH;
      memcpy(nfsFH.data,fh,NFS_FHSIZE);
      //get the link dest
      readlinkres readLinkRes;
      char nameBuf[NFS_MAXPATHLEN];
      readLinkRes.readlinkres_u.data=nameBuf;

      int clnt_stat=clnt_call(m_client, NFSPROC_READLINK,
                              (xdrproc_t) xdr_nfs_fh, (char*)&nfsFH,
                              (xdrproc_t) xdr_readlinkres, (char*)&readLinkRes,total_timeout);
      if (!checkForError(clnt_stat,readLinkRes.status,path)) return;
      kdDebug(7121)<<"link dest is -"<<readLinkRes.readlinkres_u.data<<"-"<<endl;
      TQCString linkDest(readLinkRes.readlinkres_u.data);
      atom.m_uds = TDEIO::UDS_LINK_DEST;
      atom.m_str = linkDest;
      entry.append( atom );

      bool isValid=isValidLink(parentDir,linkDest);
      if (!isValid)
      {
         completeBadLinkUDSEntry(entry,attrAndStat.attrstat_u.attributes);
      }
      else
      {
         if (isAbsoluteLink(linkDest))
         {
            completeAbsoluteLinkUDSEntry(entry,linkDest);
         }
         else
         {

            tmpStr=TQDir::cleanDirPath(parentDir+TQString("/")+TQString(linkDest)).latin1();
            diropargs dirargs;
            dirargs.name=tmpStr.data();
            NFSFileHandle tmpFH;
            tmpFH=getFileHandle(tmpStr);
            memcpy(dirargs.dir.data,tmpFH,NFS_FHSIZE);

            kdDebug(7121)<<"calling rpc: FH: -"<<fh<<"- with name -"<<dirargs.name<<"-"<<endl;
            clnt_stat = clnt_call(m_client, NFSPROC_GETATTR,
                                  (xdrproc_t) xdr_diropargs, (char*)&dirargs,
                                  (xdrproc_t) xdr_attrstat, (char*)&attrAndStat,total_timeout);
            if (!checkForError(clnt_stat,attrAndStat.status,tmpStr)) return;
            completeUDSEntry(entry,attrAndStat.attrstat_u.attributes);
         }
      }
   }
   else
      completeUDSEntry(entry,attrAndStat.attrstat_u.attributes);
   statEntry( entry );
   finished();
}

void NFSProtocol::completeAbsoluteLinkUDSEntry(UDSEntry& entry, const TQCString& path)
{
   //taken from file.cc
   struct stat buff;
   if ( ::stat( path.data(), &buff ) == -1 ) return;

   UDSAtom atom;
	atom.m_uds = TDEIO::UDS_FILE_TYPE;
	atom.m_long = buff.st_mode & S_IFMT; // extract file type
	entry.append( atom );

	atom.m_uds = TDEIO::UDS_ACCESS;
	atom.m_long = buff.st_mode & 07777; // extract permissions
	entry.append( atom );

	atom.m_uds = TDEIO::UDS_SIZE;
	atom.m_long = buff.st_size;
	entry.append( atom );

   atom.m_uds = TDEIO::UDS_MODIFICATION_TIME;
   atom.m_long = buff.st_mtime;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_USER;
   uid_t uid = buff.st_uid;
   TQString *temp = m_usercache.find( uid );

   if ( !temp )
   {
      struct passwd *user = getpwuid( uid );
      if ( user )
      {
         m_usercache.insert( uid, new TQString(TQString::fromLatin1(user->pw_name)) );
         atom.m_str = user->pw_name;
      }
      else
         atom.m_str = "???";
   }
   else
      atom.m_str = *temp;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_GROUP;
   gid_t gid = buff.st_gid;
   temp = m_groupcache.find( gid );
   if ( !temp )
   {
      struct group *grp = getgrgid( gid );
      if ( grp )
      {
         m_groupcache.insert( gid, new TQString(TQString::fromLatin1(grp->gr_name)) );
         atom.m_str = grp->gr_name;
      }
      else
         atom.m_str = "???";
   }
   else
      atom.m_str = *temp;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_ACCESS_TIME;
   atom.m_long = buff.st_atime;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_CREATION_TIME;
   atom.m_long = buff.st_ctime;
   entry.append( atom );
}

void NFSProtocol::completeBadLinkUDSEntry(UDSEntry& entry, fattr& attributes)
{
   // It is a link pointing to nowhere
   completeUDSEntry(entry,attributes);

   UDSAtom atom;
   atom.m_uds = TDEIO::UDS_FILE_TYPE;
   atom.m_long = S_IFMT - 1;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_ACCESS;
   atom.m_long = S_IRWXU | S_IRWXG | S_IRWXO;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_SIZE;
   atom.m_long = 0L;
   entry.append( atom );
}

void NFSProtocol::completeUDSEntry(UDSEntry& entry, fattr& attributes)
{
   UDSAtom atom;

   atom.m_uds = TDEIO::UDS_SIZE;
   atom.m_long = attributes.size;
   entry.append(atom);

   atom.m_uds = TDEIO::UDS_MODIFICATION_TIME;
   atom.m_long = attributes.mtime.seconds;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_ACCESS_TIME;
   atom.m_long = attributes.atime.seconds;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_CREATION_TIME;
   atom.m_long = attributes.ctime.seconds;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_ACCESS;
   atom.m_long = (attributes.mode & 07777);
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_FILE_TYPE;
   atom.m_long =attributes.mode & S_IFMT; // extract file type
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_USER;
   uid_t uid = attributes.uid;
   TQString *temp = m_usercache.find( uid );
   if ( !temp )
   {
      struct passwd *user = getpwuid( uid );
      if ( user )
      {
         m_usercache.insert( uid, new TQString(user->pw_name) );
         atom.m_str = user->pw_name;
      }
      else
         atom.m_str = "???";
   }
   else
      atom.m_str = *temp;
   entry.append( atom );

   atom.m_uds = TDEIO::UDS_GROUP;
   gid_t gid = attributes.gid;
   temp = m_groupcache.find( gid );
   if ( !temp )
   {
      struct group *grp = getgrgid( gid );
      if ( grp )
      {
         m_groupcache.insert( gid, new TQString(grp->gr_name) );
         atom.m_str = grp->gr_name;
      }
      else
         atom.m_str = "???";
   }
   else
      atom.m_str = *temp;
   entry.append( atom );

/*   TDEIO::UDSEntry::ConstIterator it = entry.begin();
   for( ; it != entry.end(); it++ ) {
      switch ((*it).m_uds) {
      case TDEIO::UDS_FILE_TYPE:
         kdDebug(7121) << "File Type : " << (mode_t)((*it).m_long) << endl;
         break;
      case TDEIO::UDS_ACCESS:
         kdDebug(7121) << "Access permissions : " << (mode_t)((*it).m_long) << endl;
         break;
      case TDEIO::UDS_USER:
         kdDebug(7121) << "User : " << ((*it).m_str.ascii() ) << endl;
         break;
      case TDEIO::UDS_GROUP:
         kdDebug(7121) << "Group : " << ((*it).m_str.ascii() ) << endl;
         break;
      case TDEIO::UDS_NAME:
         kdDebug(7121) << "Name : " << ((*it).m_str.ascii() ) << endl;
         //m_strText = decodeFileName( (*it).m_str );
         break;
      case TDEIO::UDS_URL:
         kdDebug(7121) << "URL : " << ((*it).m_str.ascii() ) << endl;
         break;
      case TDEIO::UDS_MIME_TYPE:
         kdDebug(7121) << "MimeType : " << ((*it).m_str.ascii() ) << endl;
         break;
      case TDEIO::UDS_LINK_DEST:
         kdDebug(7121) << "LinkDest : " << ((*it).m_str.ascii() ) << endl;
         break;
      }
   }*/
}

void NFSProtocol::setHost(const TQString& host, int /*port*/, const TQString& /*user*/, const TQString& /*pass*/)
{
   kdDebug(7121)<<"setHost: -"<<host<<"-"<<endl;
   if (host.isEmpty())
   {
      error(ERR_UNKNOWN_HOST,"");
      return;
   }
   if (host==m_currentHost) return;
   m_currentHost=host;
   m_handleCache.clear();
   m_exportedDirs.clear();
   closeConnection();
}

void NFSProtocol::mkdir( const KURL& url, int permissions )
{
   kdDebug(7121)<<"mkdir"<<endl;
   TQString thePath( TQFile::encodeName(url.path()));
   stripTrailingSlash(thePath);
   TQString dirName, parentDir;
   getLastPart(thePath, dirName, parentDir);
   stripTrailingSlash(parentDir);
   kdDebug(7121)<<"path: -"<<thePath<<"- dir: -"<<dirName<<"- parentDir: -"<<parentDir<<"-"<<endl;
   if (isRoot(parentDir))
   {
      error(ERR_WRITE_ACCESS_DENIED,thePath);
      return;
   }
   NFSFileHandle fh=getFileHandle(parentDir);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,thePath);
      return;
   }

   createargs createArgs;
   memcpy(createArgs.where.dir.data,fh,NFS_FHSIZE);
   TQCString tmpName=TQFile::encodeName(dirName);
   createArgs.where.name=tmpName.data();
   if (permissions==-1) createArgs.attributes.mode=0755;
   else createArgs.attributes.mode=permissions;

   diropres dirres;

   int clnt_stat = clnt_call(m_client, NFSPROC_MKDIR,
                         (xdrproc_t) xdr_createargs, (char*)&createArgs,
                         (xdrproc_t) xdr_diropres, (char*)&dirres,total_timeout);
   if (!checkForError(clnt_stat,dirres.status,thePath)) return;
   finished();
}

bool NFSProtocol::checkForError(int clientStat, int nfsStat, const TQString& text)
{
   if (clientStat!=RPC_SUCCESS)
   {
      kdDebug(7121)<<"rpc error: "<<clientStat<<endl;
      //does this mapping make sense ?
      error(ERR_CONNECTION_BROKEN,i18n("An RPC error occurred."));
      return FALSE;
   }
   if (nfsStat!=NFS_OK)
   {
      kdDebug(7121)<<"nfs error: "<<nfsStat<<endl;
      switch (nfsStat)
      {
      case NFSERR_PERM:
         error(ERR_ACCESS_DENIED,text);
         break;
      case NFSERR_NOENT:
         error(ERR_DOES_NOT_EXIST,text);
         break;
      //does this mapping make sense ?
      case NFSERR_IO:
         error(ERR_INTERNAL_SERVER,text);
         break;
      //does this mapping make sense ?
      case NFSERR_NXIO:
         error(ERR_DOES_NOT_EXIST,text);
         break;
      case NFSERR_ACCES:
         error(ERR_ACCESS_DENIED,text);
         break;
      case NFSERR_EXIST:
         error(ERR_FILE_ALREADY_EXIST,text);
         break;
      //does this mapping make sense ?
      case NFSERR_NODEV:
         error(ERR_DOES_NOT_EXIST,text);
         break;
      case NFSERR_NOTDIR:
         error(ERR_IS_FILE,text);
         break;
      case NFSERR_ISDIR:
         error(ERR_IS_DIRECTORY,text);
         break;
      //does this mapping make sense ?
      case NFSERR_FBIG:
         error(ERR_INTERNAL_SERVER,text);
         break;
      //does this mapping make sense ?
      case NFSERR_NOSPC:
         error(ERR_INTERNAL_SERVER,i18n("No space left on device"));
         break;
      case NFSERR_ROFS:
         error(ERR_COULD_NOT_WRITE,i18n("Read only file system"));
         break;
      case NFSERR_NAMETOOLONG:
         error(ERR_INTERNAL_SERVER,i18n("Filename too long"));
         break;
      case NFSERR_NOTEMPTY:
         error(ERR_COULD_NOT_RMDIR,text);
         break;
      //does this mapping make sense ?
      case NFSERR_DQUOT:
         error(ERR_INTERNAL_SERVER,i18n("Disk quota exceeded"));
         break;
      case NFSERR_STALE:
         error(ERR_DOES_NOT_EXIST,text);
         break;
      default:
         error(ERR_UNKNOWN,text);
         break;
      }
      return FALSE;
   }
   return TRUE;
}

void NFSProtocol::del( const KURL& url, bool isfile)
{
   TQString thePath( TQFile::encodeName(url.path()));
   stripTrailingSlash(thePath);

   TQString fileName, parentDir;
   getLastPart(thePath, fileName, parentDir);
   stripTrailingSlash(parentDir);
   kdDebug(7121)<<"del(): path: -"<<thePath<<"- file -"<<fileName<<"- parentDir: -"<<parentDir<<"-"<<endl;
   if (isRoot(parentDir))
   {
      error(ERR_ACCESS_DENIED,thePath);
      return;
   }

   NFSFileHandle fh=getFileHandle(parentDir);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,thePath);
      return;
   }

   if (isfile)
   {
      kdDebug(7121)<<"Deleting file "<<thePath<<endl;
      diropargs dirOpArgs;
      memcpy(dirOpArgs.dir.data,fh,NFS_FHSIZE);
      TQCString tmpName=TQFile::encodeName(fileName);
      dirOpArgs.name=tmpName.data();

      nfsstat nfsStat;

      int clnt_stat = clnt_call(m_client, NFSPROC_REMOVE,
                            (xdrproc_t) xdr_diropargs, (char*)&dirOpArgs,
                            (xdrproc_t) xdr_nfsstat, (char*)&nfsStat,total_timeout);
      if (!checkForError(clnt_stat,nfsStat,thePath)) return;
      kdDebug(7121)<<"removing "<<thePath<<" from cache"<<endl;
      m_handleCache.remove(m_handleCache.find(thePath));
      finished();
   }
   else
   {
      kdDebug(7121)<<"Deleting directory "<<thePath<<endl;
      diropargs dirOpArgs;
      memcpy(dirOpArgs.dir.data,fh,NFS_FHSIZE);
      TQCString tmpName=TQFile::encodeName(fileName);
      dirOpArgs.name=tmpName.data();

      nfsstat nfsStat;

      int clnt_stat = clnt_call(m_client, NFSPROC_RMDIR,
                            (xdrproc_t) xdr_diropargs, (char*)&dirOpArgs,
                            (xdrproc_t) xdr_nfsstat, (char*)&nfsStat,total_timeout);
      if (!checkForError(clnt_stat,nfsStat,thePath)) return;
      kdDebug(7121)<<"removing "<<thePath<<" from cache"<<endl;
      m_handleCache.remove(m_handleCache.find(thePath));
      finished();
   }
}

void NFSProtocol::chmod( const KURL& url, int permissions )
{
   TQString thePath( TQFile::encodeName(url.path()));
   stripTrailingSlash(thePath);
   kdDebug( 7121 ) <<  "chmod -"<< thePath << "-"<<endl;
   if (isRoot(thePath) || isExportedDir(thePath))
   {
      error(ERR_ACCESS_DENIED,thePath);
      return;
   }

   NFSFileHandle fh=getFileHandle(thePath);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,thePath);
      return;
   }

   sattrargs sAttrArgs;
   memcpy(sAttrArgs.file.data,fh,NFS_FHSIZE);
   sAttrArgs.attributes.uid=(unsigned int)-1;
   sAttrArgs.attributes.gid=(unsigned int)-1;
   sAttrArgs.attributes.size=(unsigned int)-1;
   sAttrArgs.attributes.atime.seconds=(unsigned int)-1;
   sAttrArgs.attributes.atime.useconds=(unsigned int)-1;
   sAttrArgs.attributes.mtime.seconds=(unsigned int)-1;
   sAttrArgs.attributes.mtime.useconds=(unsigned int)-1;

   sAttrArgs.attributes.mode=permissions;

   nfsstat nfsStat;

   int clnt_stat = clnt_call(m_client, NFSPROC_SETATTR,
                         (xdrproc_t) xdr_sattrargs, (char*)&sAttrArgs,
                         (xdrproc_t) xdr_nfsstat, (char*)&nfsStat,total_timeout);
   if (!checkForError(clnt_stat,nfsStat,thePath)) return;

   finished();
}

void NFSProtocol::get( const KURL& url )
{
   TQString thePath( TQFile::encodeName(url.path()));
   kdDebug(7121)<<"get() -"<<thePath<<"-"<<endl;
   NFSFileHandle fh=getFileHandle(thePath);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,thePath);
      return;
   }
   readargs readArgs;
   memcpy(readArgs.file.data,fh,NFS_FHSIZE);
   readArgs.offset=0;
   readArgs.count=NFS_MAXDATA;
   readArgs.totalcount=NFS_MAXDATA;
   readres readRes;
   int offset(0);
   char buf[NFS_MAXDATA];
   readRes.readres_u.reply.data.data_val=buf;

   TQByteArray array;
   do
   {
      int clnt_stat = clnt_call(m_client, NFSPROC_READ,
                            (xdrproc_t) xdr_readargs, (char*)&readArgs,
                            (xdrproc_t) xdr_readres, (char*)&readRes,total_timeout);
      if (!checkForError(clnt_stat,readRes.status,thePath)) return;
      if (readArgs.offset==0)
         totalSize(readRes.readres_u.reply.attributes.size);

      offset=readRes.readres_u.reply.data.data_len;
      //kdDebug(7121)<<"read "<<offset<<" bytes"<<endl;
      readArgs.offset+=offset;
      if (offset>0)
      {
         array.setRawData(readRes.readres_u.reply.data.data_val, offset);
         data( array );
         array.resetRawData(readRes.readres_u.reply.data.data_val, offset);

         processedSize(readArgs.offset);
      }

   } while (offset>0);
   data( TQByteArray() );
   finished();
}

//TODO the partial putting thing is not yet implemented
void NFSProtocol::put( const KURL& url, int _mode, bool _overwrite, bool /*_resume*/ )
{
    TQString destPath( TQFile::encodeName(url.path()));
    kdDebug( 7121 ) << "Put -" << destPath <<"-"<<endl;
    /*TQString dest_part( dest_orig );
    dest_part += ".part";*/

    stripTrailingSlash(destPath);
    TQString parentDir, fileName;
    getLastPart(destPath,fileName, parentDir);
    if (isRoot(parentDir))
    {
       error(ERR_WRITE_ACCESS_DENIED,destPath);
       return;
    }

    NFSFileHandle destFH;
    destFH=getFileHandle(destPath);
    kdDebug(7121)<<"file handle for -"<<destPath<<"- is "<<destFH<<endl;

    //the file exists and we don't want to overwrite
    if ((!_overwrite) && (!destFH.isInvalid()))
    {
       error(ERR_FILE_ALREADY_EXIST,destPath);
       return;
    }
    //TODO: is this correct ?
    //we have to "create" the file anyway, no matter if it already
    //exists or not
    //if we don't create it new, written text will be, hmm, "inserted"
    //in the existing file, i.e. a file could not become smaller, since
    //write only overwrites or extends, but doesn't remove stuff from a file (aleXXX)

    kdDebug(7121)<<"creating the file -"<<fileName<<"-"<<endl;
    NFSFileHandle parentFH;
    parentFH=getFileHandle(parentDir);
    //cerr<<"fh for parent dir: "<<parentFH<<endl;
    //the directory doesn't exist
    if (parentFH.isInvalid())
    {
       kdDebug(7121)<<"parent directory -"<<parentDir<<"- does not exist"<<endl;
       error(ERR_DOES_NOT_EXIST,parentDir);
       return;
    }
    createargs createArgs;
    memcpy(createArgs.where.dir.data,(const char*)parentFH,NFS_FHSIZE);
    TQCString tmpName=TQFile::encodeName(fileName);
    createArgs.where.name=tmpName.data();

    //the mode is apparently ignored if the file already exists
    if (_mode==-1) createArgs.attributes.mode=0644;
    else createArgs.attributes.mode=_mode;
    createArgs.attributes.uid=geteuid();
    createArgs.attributes.gid=getegid();
    //this is required, otherwise we are not able to write shorter files
    createArgs.attributes.size=0;
    //hmm, do we need something here ? I don't think so
    createArgs.attributes.atime.seconds=(unsigned int)-1;
    createArgs.attributes.atime.useconds=(unsigned int)-1;
    createArgs.attributes.mtime.seconds=(unsigned int)-1;
    createArgs.attributes.mtime.useconds=(unsigned int)-1;

    diropres dirOpRes;
    int clnt_stat = clnt_call(m_client, NFSPROC_CREATE,
                              (xdrproc_t) xdr_createargs, (char*)&createArgs,
                              (xdrproc_t) xdr_diropres, (char*)&dirOpRes,total_timeout);
    if (!checkForError(clnt_stat,dirOpRes.status,fileName)) return;
    //we created the file successfully
    //destFH=getFileHandle(destPath);
    destFH=dirOpRes.diropres_u.diropres.file.data;
    kdDebug(7121)<<"file -"<<fileName<<"- in dir -"<<parentDir<<"- created successfully"<<endl;
    //cerr<<"with fh "<<destFH<<endl;

    //now we can put
    int result;
    // Loop until we got 0 (end of data)
    writeargs writeArgs;
    memcpy(writeArgs.file.data,(const char*)destFH,NFS_FHSIZE);
    writeArgs.beginoffset=0;
    writeArgs.totalcount=0;
    writeArgs.offset=0;
    attrstat attrStat;
    int bytesWritten(0);
    kdDebug(7121)<<"starting to put"<<endl;
    do
    {
       TQByteArray buffer;
       dataReq(); // Request for data
       result = readData( buffer );
       //kdDebug(7121)<<"received "<<result<<" bytes for putting"<<endl;
       char * data=buffer.data();
       int bytesToWrite=buffer.size();
       int writeNow(0);
       if (result > 0)
       {
          do
          {
             if (bytesToWrite>NFS_MAXDATA)
             {
                writeNow=NFS_MAXDATA;
             }
             else
             {
                writeNow=bytesToWrite;
             };
             writeArgs.data.data_val=data;
             writeArgs.data.data_len=writeNow;

             int clnt_stat = clnt_call(m_client, NFSPROC_WRITE,
                                       (xdrproc_t) xdr_writeargs, (char*)&writeArgs,
                                       (xdrproc_t) xdr_attrstat, (char*)&attrStat,total_timeout);
             //kdDebug(7121)<<"written"<<endl;
             if (!checkForError(clnt_stat,attrStat.status,fileName)) return;
             bytesWritten+=writeNow;
             writeArgs.offset=bytesWritten;

             //adjust the pointer
             data=data+writeNow;
             //decrease the rest
             bytesToWrite-=writeNow;
          } while (bytesToWrite>0);
       }
    } while ( result > 0 );
    finished();
}

void NFSProtocol::rename( const KURL &src, const KURL &dest, bool _overwrite )
{
   TQString srcPath( TQFile::encodeName(src.path()));
   TQString destPath( TQFile::encodeName(dest.path()));
   stripTrailingSlash(srcPath);
   stripTrailingSlash(destPath);
   kdDebug(7121)<<"renaming -"<<srcPath<<"- to -"<<destPath<<"-"<<endl;

   if (isRoot(srcPath) || isExportedDir(srcPath))
   {
      error(ERR_CANNOT_RENAME,srcPath);
      return;
   }

   if (!_overwrite)
   {
      NFSFileHandle testFH;
      testFH=getFileHandle(destPath);
      if (!testFH.isInvalid())
      {
         error(ERR_FILE_ALREADY_EXIST,destPath);
         return;
      }
   }

   TQString srcFileName, srcParentDir, destFileName, destParentDir;

   getLastPart(srcPath, srcFileName, srcParentDir);
   NFSFileHandle srcFH=getFileHandle(srcParentDir);
   if (srcFH.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,srcParentDir);
      return;
   }
   renameargs renameArgs;
   memcpy(renameArgs.from.dir.data,srcFH,NFS_FHSIZE);
   TQCString tmpName=TQFile::encodeName(srcFileName);
   renameArgs.from.name=tmpName.data();

   getLastPart(destPath, destFileName, destParentDir);
   NFSFileHandle destFH=getFileHandle(destParentDir);
   if (destFH.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,destParentDir);
      return;
   }
   memcpy(renameArgs.to.dir.data,destFH,NFS_FHSIZE);
   TQCString tmpName2=TQFile::encodeName(destFileName);
   renameArgs.to.name=tmpName2.data();
   nfsstat nfsStat;

   int clnt_stat = clnt_call(m_client, NFSPROC_RENAME,
                             (xdrproc_t) xdr_renameargs, (char*)&renameArgs,
                             (xdrproc_t) xdr_nfsstat, (char*)&nfsStat,total_timeout);
   if (!checkForError(clnt_stat,nfsStat,destPath)) return;
   finished();
}

void NFSProtocol::copy( const KURL &src, const KURL &dest, int _mode, bool _overwrite )
{
   //prepare the source
   TQString thePath( TQFile::encodeName(src.path()));
   stripTrailingSlash(thePath);
   kdDebug( 7121 ) << "Copy to -" << thePath <<"-"<<endl;
   NFSFileHandle fh=getFileHandle(thePath);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,thePath);
      return;
   };

   //create the destination
   TQString destPath( TQFile::encodeName(dest.path()));
   stripTrailingSlash(destPath);
   TQString parentDir, fileName;
   getLastPart(destPath,fileName, parentDir);
   if (isRoot(parentDir))
   {
      error(ERR_ACCESS_DENIED,destPath);
      return;
   }
   NFSFileHandle destFH;
   destFH=getFileHandle(destPath);
   kdDebug(7121)<<"file handle for -"<<destPath<<"- is "<<destFH<<endl;

   //the file exists and we don't want to overwrite
   if ((!_overwrite) && (!destFH.isInvalid()))
   {
      error(ERR_FILE_ALREADY_EXIST,destPath);
      return;
   }
   //TODO: is this correct ?
   //we have to "create" the file anyway, no matter if it already
   //exists or not
   //if we don't create it new, written text will be, hmm, "inserted"
   //in the existing file, i.e. a file could not become smaller, since
   //write only overwrites or extends, but doesn't remove stuff from a file

   kdDebug(7121)<<"creating the file -"<<fileName<<"-"<<endl;
   NFSFileHandle parentFH;
   parentFH=getFileHandle(parentDir);
   //the directory doesn't exist
   if (parentFH.isInvalid())
   {
      kdDebug(7121)<<"parent directory -"<<parentDir<<"- does not exist"<<endl;
      error(ERR_DOES_NOT_EXIST,parentDir);
      return;
   };
   createargs createArgs;
   memcpy(createArgs.where.dir.data,(const char*)parentFH,NFS_FHSIZE);
   TQCString tmpName=TQFile::encodeName(fileName);
   createArgs.where.name=tmpName.data();
   if (_mode==-1) createArgs.attributes.mode=0644;
   else createArgs.attributes.mode=_mode;
   createArgs.attributes.uid=geteuid();
   createArgs.attributes.gid=getegid();
   createArgs.attributes.size=0;
   createArgs.attributes.atime.seconds=(unsigned int)-1;
   createArgs.attributes.atime.useconds=(unsigned int)-1;
   createArgs.attributes.mtime.seconds=(unsigned int)-1;
   createArgs.attributes.mtime.useconds=(unsigned int)-1;

   diropres dirOpRes;
   int clnt_stat = clnt_call(m_client, NFSPROC_CREATE,
                             (xdrproc_t) xdr_createargs, (char*)&createArgs,
                             (xdrproc_t) xdr_diropres, (char*)&dirOpRes,total_timeout);
   if (!checkForError(clnt_stat,dirOpRes.status,destPath)) return;
   //we created the file successfully
   destFH=dirOpRes.diropres_u.diropres.file.data;
   kdDebug(7121)<<"file -"<<fileName<<"- in dir -"<<parentDir<<"- created successfully"<<endl;

   char buf[NFS_MAXDATA];
   writeargs writeArgs;
   memcpy(writeArgs.file.data,(const char*)destFH,NFS_FHSIZE);
   writeArgs.beginoffset=0;
   writeArgs.totalcount=0;
   writeArgs.offset=0;
   writeArgs.data.data_val=buf;
   attrstat attrStat;

   readargs readArgs;
   memcpy(readArgs.file.data,fh,NFS_FHSIZE);
   readArgs.offset=0;
   readArgs.count=NFS_MAXDATA;
   readArgs.totalcount=NFS_MAXDATA;
   readres readRes;
   readRes.readres_u.reply.data.data_val=buf;

   int bytesRead(0);
   do
   {
      //first read
      int clnt_stat = clnt_call(m_client, NFSPROC_READ,
                                (xdrproc_t) xdr_readargs, (char*)&readArgs,
                                (xdrproc_t) xdr_readres, (char*)&readRes,total_timeout);
      if (!checkForError(clnt_stat,readRes.status,thePath)) return;
      if (readArgs.offset==0)
         totalSize(readRes.readres_u.reply.attributes.size);

      bytesRead=readRes.readres_u.reply.data.data_len;
      //kdDebug(7121)<<"read "<<bytesRead<<" bytes"<<endl;
      //then write
      if (bytesRead>0)
      {
         readArgs.offset+=bytesRead;

         writeArgs.data.data_len=bytesRead;

         clnt_stat = clnt_call(m_client, NFSPROC_WRITE,
                               (xdrproc_t) xdr_writeargs, (char*)&writeArgs,
                               (xdrproc_t) xdr_attrstat, (char*)&attrStat,total_timeout);
         //kdDebug(7121)<<"written"<<endl;
         if (!checkForError(clnt_stat,attrStat.status,destPath)) return;
         writeArgs.offset+=bytesRead;
      }
   } while (bytesRead>0);

   finished();
}

//TODO why isn't this even called ?
void NFSProtocol::symlink( const TQString &target, const KURL &dest, bool )
{
   kdDebug(7121)<<"symlinking "<<endl;
   TQString destPath=dest.path();
   stripTrailingSlash(destPath);

   TQString parentDir, fileName;
   getLastPart(destPath,fileName, parentDir);
   kdDebug(7121)<<"symlinking "<<parentDir<<" "<<fileName<<" to "<<target<<endl;
   NFSFileHandle fh=getFileHandle(parentDir);
   if (fh.isInvalid())
   {
      error(ERR_DOES_NOT_EXIST,parentDir);
      return;
   }
   if (isRoot(parentDir))
   {
      error(ERR_ACCESS_DENIED,destPath);
      return;
   }

   kdDebug(7121)<<"tach"<<endl;
   TQCString tmpStr=target.latin1();
   symlinkargs symLinkArgs;
   symLinkArgs.to=tmpStr.data();
   memcpy(symLinkArgs.from.dir.data,(const char*)fh,NFS_FHSIZE);
   TQCString tmpStr2=TQFile::encodeName(destPath);
   symLinkArgs.from.name=tmpStr2.data();

   nfsstat nfsStat;
   int clnt_stat = clnt_call(m_client, NFSPROC_SYMLINK,
                             (xdrproc_t) xdr_symlinkargs, (char*)&symLinkArgs,
                             (xdrproc_t) xdr_nfsstat, (char*)&nfsStat,total_timeout);
   if (!checkForError(clnt_stat,nfsStat,destPath)) return;

   finished();

}

bool NFSProtocol::isValidLink(const TQString& parentDir, const TQString& linkDest)
{
   kdDebug(7121)<<"isValidLink: parent: "<<parentDir<<" link: "<<linkDest<<endl;
   if (linkDest.isEmpty()) return FALSE;
   if (isAbsoluteLink(linkDest))
   {
      kdDebug(7121)<<"is an absolute link"<<endl;
      return TQFile::exists(linkDest);
   }
   else
   {
      kdDebug(7121)<<"is a relative link"<<endl;
      TQString absDest=parentDir+"/"+linkDest;
      kdDebug(7121)<<"pointing abs to "<<absDest<<endl;
      absDest=removeFirstPart(absDest);
      kdDebug(7121)<<"removed first part "<<absDest<<endl;
      absDest=TQDir::cleanDirPath(absDest);
      kdDebug(7121)<<"simplified to "<<absDest<<endl;
      if (absDest.find("../")==0)
         return FALSE;

      kdDebug(7121)<<"is inside the nfs tree"<<endl;
      absDest=parentDir+"/"+linkDest;
      absDest=TQDir::cleanDirPath(absDest);
      kdDebug(7121)<<"getting file handle of "<<absDest<<endl;
      NFSFileHandle fh=getFileHandle(absDest);
      return (!fh.isInvalid());
   }
   return FALSE;
}

