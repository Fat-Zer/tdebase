#ifndef KQUERY_H
#define KQUERY_H

#include <time.h>

#include <tqobject.h>
#include <tqregexp.h>
#include <tqptrqueue.h>
#include <tqdir.h>
#include <tqstringlist.h>

#include <kio/job.h>
#include <kurl.h>
#include <kprocess.h>

class KFileItem;

class KQuery : public TQObject
{
  Q_OBJECT

 public:
  KQuery(TQObject *parent = 0, const char * name = 0);
  ~KQuery();

  void setSizeRange( int mode, KIO::filesize_t value1, KIO::filesize_t value2 );
  void setTimeRange( time_t from, time_t to );
  void setRegExp( const TQString &regexp, bool caseSensitive );
  void setRecursive( bool recursive );
  void setPath(const KURL & url );
  void setFileType( int filetype );
  void setMimeType( const TQStringList & mimetype );
  void setContext( const TQString & context, bool casesensitive, 
    bool search_binary, bool useRegexp );
  void setUsername( TQString username );
  void setGroupname( TQString groupname );
	void setMetaInfo(const TQString &metainfo, const TQString &metainfokey);
  void setUseFileIndex(bool);

  void start();
  void kill();
  const KURL& url()              {return m_url;};

 private:
  /* Check if file meets the find's requirements*/
  inline void processQuery(KFileItem*);

 public slots: 
  /* List of files found using slocate */
  void slotListEntries(TQStringList);
 protected slots:
  /* List of files found using KIO */
  void slotListEntries(KIO::Job *, const KIO::UDSEntryList &);
  void slotResult(KIO::Job *);
  void slotCanceled(KIO::Job *);
  void slotreceivedSdtout(KProcess*,char*,int);
  void slotreceivedSdterr(KProcess*,char*,int);
  void slotendProcessLocate(KProcess*);

 signals:
  void addFile(const KFileItem *filename, const TQString& matchingLine);
  void result(int);

 private:
  void checkEntries();

  int m_filetype;
  int m_sizemode;
  KIO::filesize_t m_sizeboundary1;
  KIO::filesize_t m_sizeboundary2;
  KURL m_url;
  time_t m_timeFrom;
  time_t m_timeTo;
  TQRegExp m_regexp;// regexp for file content
  bool m_recursive;
  TQStringList m_mimetype;
  TQString m_context;
  TQString m_username;
  TQString m_groupname;
  TQString m_metainfo;
  TQString m_metainfokey;
  bool m_casesensitive;
  bool m_search_binary;
  bool m_regexpForContent;
  bool m_useLocate;
  char* bufferLocate;
  int bufferLocateLength;
  TQStringList locateList;
  KProcess *processLocate;
  TQPtrList<TQRegExp> m_regexps;// regexps for file name
//  TQValueList<bool> m_regexpsContainsGlobs;  // what should this be good for ? Alex
  KIO::ListJob *job;
  bool m_insideCheckEntries;
  TQPtrQueue<KFileItem> m_fileItems;
  TQRegExp* metaKeyRx;
  int m_result;
  TQStringList ignore_mimetypes;
  TQStringList ooo_mimetypes;     // OpenOffice.org mimetypes
  TQStringList koffice_mimetypes;
};

#endif
