/* 	
    info_svr4.cpp

    UNIX System V Release 4 specific Information about the Hardware.
    Appropriate for SCO OpenServer and UnixWare.
    Written 20-Feb-99 by Ronald Joe Record (rr@sco.com)
    Initially based on info_sgi.cpp
*/

#define INFO_CPU_AVAILABLE
#define INFO_IRQ_AVAILABLE
#define INFO_DMA_AVAILABLE
#define INFO_PCI_AVAILABLE
#define INFO_IOPORTS_AVAILABLE
#define INFO_SOUND_AVAILABLE
#define INFO_DEVICES_AVAILABLE
#define INFO_SCSI_AVAILABLE
#define INFO_PARTITIONS_AVAILABLE
#define INFO_XSERVER_AVAILABLE

#define INFO_DEV_SNDSTAT "/dev/sndstat"

#include <sys/systeminfo.h>

/*  all following functions should return true, when the Information 
    was filled into the lBox-Widget.
    returning false indicates, that information was not available.
*/

bool GetInfo_ReadfromFile( TQListView *lBox, char *Name, char splitchar  )
{
  TQString str;
  char buf[512];

  TQFile *file = new TQFile(Name);
  TQListViewItem* olditem = 0;

  if(!file->open(IO_ReadOnly)) {
    delete file; 
    return false;
  }
  
  while (file->readLine(buf,sizeof(buf)-1) > 0) {
      if (strlen(buf)) {
          char *p=buf;
          if (splitchar!=0)    /* remove leading spaces between ':' and the following text */
              while (*p) {
                  if (*p==splitchar) {
                      *p++ = ' ';
                      while (*p==' ') ++p;
                      *(--p) = splitchar;
                      ++p;
                  }
                  else ++p;
              }
          
          TQString s1 = TQString::fromLocal8Bit(buf);
          TQString s2 = s1.mid(s1.tqfind(splitchar)+1);
          
          s1.truncate(s1.tqfind(splitchar));
          if(!(s1.isEmpty() || s2.isEmpty()))
              olditem = new TQListViewItem(lBox, olditem, s1, s2);
      }
  }
  file->close();
  
  delete file;
  return true;
}

bool GetInfo_CPU( TQListView *lBox )
{
      char buf[256];

      sysinfo(SI_ARCHITECTURE, buf, sizeof(buf));
      new TQListViewItem(lBox, TQString::fromLocal8Bit(buf));
      return true;
}


bool GetInfo_IRQ( TQListView * )
{
	return false;
}

bool GetInfo_DMA( TQListView * )
{
	return false;
}

bool GetInfo_PCI( TQListView *lBox )
{
      char buf[256];

      sysinfo(SI_BUSTYPES, buf, sizeof(buf));
      new TQListViewItem(lBox, TQString::fromLocal8Bit(buf));
      return true;
}

bool GetInfo_IO_Ports( TQListView * )
{
	return false;
}

bool GetInfo_Sound( TQListView *lBox )
{
  if ( GetInfo_ReadfromFile( lBox, INFO_DEV_SNDSTAT, 0 ))
    return true;
  else
    return false;
}

bool GetInfo_Devices( TQListView * )
{
    return false;
}

bool GetInfo_SCSI( TQListView * )
{
    return false;
}

bool GetInfo_Partitions( TQListView * )
{
	return false;
}

bool GetInfo_XServer_and_Video( TQListView *lBox )
{
	return GetInfo_XServer_Generic( lBox );
}

