/***************************************************************************
                          sftpProtocol.h  -  description
                             -------------------
    begin                : Sat Jun 30 20:08:47 CDT 2001
    copyright            : (C) 2001 by Lucas Fisher
    email                : ljfisher@purdue.edu
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __tdeio_sftp_h__
#define __tdeio_sftp_h__

#include <tqstring.h>
#include <tqcstring.h>
#include <tqobject.h>

#include <kurl.h>
#include <tdeio/global.h>
#include <tdeio/slavebase.h>
#include <kdebug.h>

#include "process.h"
#include "sftpfileattr.h"
#include "ksshprocess.h"

#define TDEIO_SFTP_DB 7120


class sftpProtocol : public TDEIO::SlaveBase
{

public:
  sftpProtocol(const TQCString &pool_socket, const TQCString &app_socket);
  virtual ~sftpProtocol();
  virtual void setHost(const TQString& h, int port, const TQString& user, const TQString& pass);
  virtual void get(const KURL& url);
  virtual void listDir(const KURL& url) ;
  virtual void mimetype(const KURL& url);
  virtual void stat(const KURL& url);
  virtual void copy(const KURL &src, const KURL &dest, int permissions, bool overwrite);
  virtual void put(const KURL& url, int permissions, bool overwrite, bool resume);
  virtual void closeConnection();
  virtual void slave_status();
  virtual void del(const KURL &url, bool isfile);
  virtual void chmod(const KURL& url, int permissions);
  virtual void symlink(const TQString& target, const KURL& dest, bool overwrite);
  virtual void rename(const KURL& src, const KURL& dest, bool overwrite);
  virtual void mkdir(const KURL&url, int permissions);
  virtual void openConnection();

private: // Private variables
  /** True if ioslave is connected to sftp server. */
  bool mConnected;

  /** Host we are connected to. */
  TQString mHost;

  /** Port we are connected to. */
  int mPort;

  /** Ssh process to which we send the sftp packets. */
  KSshProcess ssh;

  /** Username to use when connecting */
  TQString mUsername;

  /** User's password */
  TQString mPassword;

  /** Message id of the last sftp packet we sent. */
  unsigned int mMsgId;

  /** Type of packet we are expecting to receive next. */
  unsigned char mExpected;

  /** Version of the sftp protocol we are using. */
  int sftpVersion;
  
  struct Status 
  {
    int code;
    TDEIO::filesize_t size;
    TQString text;
  };

private: // private methods
  bool getPacket(TQByteArray& msg);

   /* Type is a sftp packet type found in .sftp.h'.
   * Example: SSH2_FXP_READLINK, SSH2_FXP_RENAME, etc.
   *
   * Returns true if the type is supported by the sftp protocol
   * version negotiated by the client and server (sftpVersion).
   */
  bool isSupportedOperation(int type);
  /** Used to have the server canonicalize any given path name to an absolute path.
      This is useful for converting path names containing ".." components or relative
      pathnames without a leading slash into absolute paths.
      Returns the canonicalized url. */
  int sftpRealPath(const KURL& url, KURL& newUrl);

  /** Send an sftp packet to stdin of the ssh process. */
  bool putPacket(TQByteArray& p);
  /** Process SSH_FXP_STATUS packets. */
  void processStatus(TQ_UINT8, const TQString& message = TQString::null);
  /** Process SSH_FXP_STATUS packes and return the result. */
  Status doProcessStatus(TQ_UINT8, const TQString& message = TQString::null);
  /** Opens a directory handle for url.path. Returns true if succeeds. */
  int sftpOpenDirectory(const KURL& url, TQByteArray& handle);
  /** Closes a directory or file handle. */
  int sftpClose(const TQByteArray& handle);
  /** Send a sftp command to rename a file or directoy. */
  int sftpRename(const KURL& src, const KURL& dest);
  /** Set a files attributes. */
  int sftpSetStat(const KURL& url, const sftpFileAttr& attr);
  /** Sends a sftp command to remove a file or directory. */
  int sftpRemove(const KURL& url, bool isfile);
  /** Creates a symlink named dest to target. */
  int sftpSymLink(const TQString& target, const KURL& dest);
  /** Get directory listings. */
  int sftpReadDir(const TQByteArray& handle, const KURL& url);
  /** Retrieves the destination of a link. */
  int sftpReadLink(const KURL& url, TQString& target);
  /** Stats a file. */
  int sftpStat(const KURL& url, sftpFileAttr& attr);
  /** No descriptions */
  int sftpOpen(const KURL& url, const TQ_UINT32 pflags, const sftpFileAttr& attr, TQByteArray& handle);
  /** No descriptions */
  int sftpRead(const TQByteArray& handle, TDEIO::filesize_t offset, TQ_UINT32 len, TQByteArray& data);
  /** No descriptions */
  int sftpWrite(const TQByteArray& handle, TDEIO::filesize_t offset, const TQByteArray& data);
  
  /** Performs faster upload when the source is a local file... */
  void sftpCopyPut(const KURL& src, const KURL& dest, int mode, bool overwrite);
  /** Performs faster download when the destination is a local file... */
  void sftpCopyGet(const KURL& dest, const KURL& src, int mode, bool overwrite);
  
  /** */
  Status sftpGet( const KURL& src, TDEIO::filesize_t offset = 0, int fd = -1);
  void sftpPut( const KURL& dest, int permissions, bool resume, bool overwrite, int fd = -1);
};
#endif
