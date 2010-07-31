#ifndef __info_h__
#define __info_h__

#include <tqobject.h>

#include <kio/slavebase.h>

class KProcess;

class InfoProtocol : public KIO::SlaveBase
{
public:

    InfoProtocol( const TQCString &pool, const TQCString &app );
    virtual ~InfoProtocol();

    virtual void get( const KURL& url );
    virtual void stat( const KURL& url );
    virtual void mimetype( const KURL& url );

protected:

    void decodeURL( const KURL &url );
    void decodePath( TQString path );

private:

    TQString   m_page;
    TQString   m_node;

    TQString   m_perl;
    TQString   m_infoScript;
    TQString   m_infoConf;
};

#endif // __info_h__
