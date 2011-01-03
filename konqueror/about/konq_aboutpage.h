#ifndef __konq_aboutpage_h__
#define __konq_aboutpage_h__

#include <kparts/factory.h>
#include <khtml_part.h>

class KHTMLPart;
class KInstance;

class KonqAboutPageFactory : public KParts::Factory
{
public:
    KonqAboutPageFactory( TQObject *parent = 0, const char *name = 0 );
    virtual ~KonqAboutPageFactory();

    virtual KParts::Part *createPartObject( TQWidget *tqparentWidget, const char *widgetName,
                                            TQObject *parent, const char *name,
                                            const char *classname, const TQStringList &args );

    static KInstance *instance() { return s_instance; }

    static TQString launch();
    static TQString intro();
    static TQString specs();
    static TQString tips();
    static TQString plugins();

private:
    static TQString loadFile( const TQString& file );

    static KInstance *s_instance;
    static TQString *s_launch_html, *s_intro_html, *s_specs_html, *s_tips_html, *s_plugins_html;
};

class KonqAboutPage : public KHTMLPart
{
    Q_OBJECT
public:
    KonqAboutPage( /*KonqMainWindow *mainWindow,*/
                   TQWidget *tqparentWidget, const char *widgetName,
                   TQObject *parent, const char *name );
    ~KonqAboutPage();

    virtual bool openURL( const KURL &url );

    virtual bool openFile();

    virtual void saveState( TQDataStream &stream );
    virtual void restoreState( TQDataStream &stream );

protected:
    virtual void urlSelected( const TQString &url, int button, int state, const TQString &target, KParts::URLArgs args = KParts::URLArgs() );

private:
    void serve( const TQString&, const TQString& );

    KHTMLPart *m_doc;
    //KonqMainWindow *m_mainWindow;
    TQString m_htmlDoc;
    TQString m_what;
};

#endif
