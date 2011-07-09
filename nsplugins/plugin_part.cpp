/*
  Netscape Plugin Loader KPart

  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
                     Stefan Schimanski <1Stein@gmx.de>
  Copyright (c) 2002-2005 George Staikos <staikos@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kparts/browserinterface.h>
#include <kparts/browserextension.h>

#include <tqlabel.h>

#include "nspluginloader.h"
#include "plugin_part.h"


class PluginBrowserExtension : public KParts::BrowserExtension
{
    friend class PluginPart;
public:
    PluginBrowserExtension( KParts::ReadOnlyPart *parent,
                          const char *name = 0L )
        : KParts::BrowserExtension( parent, name ) {}
    ~PluginBrowserExtension() {}

    // ATTENTION: you -CANNOT- add data members here
};


PluginLiveConnectExtension::PluginLiveConnectExtension(PluginPart* part) 
: KParts::LiveConnectExtension(part), _part(part), _retval(0L) {
}

PluginLiveConnectExtension::~PluginLiveConnectExtension() {
}

bool PluginLiveConnectExtension::get(const unsigned long, const TQString &field, Type &type, unsigned long &retobj, TQString &value) {
Q_UNUSED(type);
Q_UNUSED(retobj);
Q_UNUSED(value);
    kdDebug(1432) << "PLUGIN:LiveConnect::get " << field << endl;
    return false;
}

bool PluginLiveConnectExtension::call(const unsigned long, const TQString &func, const TQStringList &args, Type &type, unsigned long &retobjid, TQString &value) {
Q_UNUSED(type);
Q_UNUSED(retobjid);
Q_UNUSED(value);
    kdDebug(1432) << "PLUGIN:LiveConnect::call " << func << " args: " << args << endl;
    return false;
}

bool PluginLiveConnectExtension::put( const unsigned long, const TQString &field, const TQString &value) {
    kdDebug(1432) << "PLUGIN:LiveConnect::put " << field << " " << value << endl;
    if (_retval && field == "__nsplugin") {
        *_retval = value;
        return true;
    } else if (field.lower() == "src") {
        _part->changeSrc(value);
        return true;
    }
    return false;
}

TQString PluginLiveConnectExtension::evalJavaScript( const TQString & script )
{
    kdDebug(1432) << "PLUGIN:LiveConnect::evalJavaScript " << script << endl;
    ArgList args;
    TQString jscode;
    jscode.sprintf("this.__nsplugin=eval(\"%s\")",  TQString(script).tqreplace('\\', "\\\\").tqreplace('"', "\\\"").latin1());
    //kdDebug(1432) << "String is [" << jscode << "]" << endl;
    args.push_back(qMakePair(KParts::LiveConnectExtension::TypeString, jscode));
    TQString nsplugin("Undefined");
    _retval = &nsplugin;
    emit partEvent(0, "eval", args);
    _retval = 0L;
    return nsplugin;
}

extern "C"
{
  /**
   * This function is the 'main' function of this part.  It takes
   * the form 'void *init_lib<library name>()  It always returns a
   * new factory object
   */

   K_EXPORT_COMPONENT_FACTORY(libnsplugin,PluginFactory)
}


NSPluginCallback::NSPluginCallback(PluginPart *part)
  : DCOPObject()
{
    _part = part;
}


void NSPluginCallback::postURL(TQString url, TQString target, TQByteArray data, TQString mime)
{
    _part->postURL( url, target, data, mime );
}


void NSPluginCallback::requestURL(TQString url, TQString target)
{
    _part->requestURL( url, target );
}


void NSPluginCallback::statusMessage( TQString msg )
{
    _part->statusMessage( msg );
}


void NSPluginCallback::evalJavaScript( int id, TQString script )
{
    _part->evalJavaScript( id, script );
}
/**
 * We need one static instance of the factory for our C 'main'
 * function
 */
KInstance *PluginFactory::s_instance = 0L;


PluginFactory::PluginFactory()
{
    kdDebug(1432) << "PluginFactory::PluginFactory" << endl;
    s_instance = 0;

    // preload plugin loader
    _loader = NSPluginLoader::instance();
}


PluginFactory::~PluginFactory()
{
   kdDebug(1432) << "PluginFactory::~PluginFactory" << endl;

   _loader->release();

   if ( s_instance ) {
       delete s_instance->aboutData();
       delete s_instance;
   }
   s_instance = 0;
}

KParts::Part * PluginFactory::createPartObject(TQWidget *parentWidget, const char *widgetName,
                                         TQObject *parent, const char *name,
                                         const char *classname, const TQStringList &args)
{
    Q_UNUSED(classname)
    kdDebug(1432) << "PluginFactory::create" << endl;
    KParts::Part *obj = new PluginPart(parentWidget, widgetName, parent, name, args);
    return obj;
}


KInstance *PluginFactory::instance()
{
    kdDebug(1432) << "PluginFactory::instance" << endl;

    if ( !s_instance )
        s_instance = new KInstance( aboutData() );
    return s_instance;
}

KAboutData *PluginFactory::aboutData()
{
    KAboutData *about = new KAboutData("plugin", I18N_NOOP("plugin"), "1.99");
    return about;
}


/**************************************************************************/


PluginPart::PluginPart(TQWidget *parentWidget, const char *widgetName, TQObject *parent,
                       const char *name, const TQStringList &args)
    : KParts::ReadOnlyPart(parent, name), _widget(0), _args(args),
      _destructed(0L)
{
    setInstance(PluginFactory::instance());
    kdDebug(1432) << "PluginPart::PluginPart" << endl;

    // we have to keep the class name of KParts::PluginBrowserExtension
    // to let khtml find it
    _extension = static_cast<PluginBrowserExtension*>(new KParts::BrowserExtension(this));
    _liveconnect = new PluginLiveConnectExtension(this);

    // Only create this if we have no parent since the parent part is
    // responsible for "Save As" then
    if (!parent || !parent->inherits("Part")) {
        new KAction(i18n("&Save As..."), CTRL+Key_S, this, TQT_SLOT(saveAs()), actionCollection(), "saveDocument");
        setXMLFile("nspluginpart.rc");
    }

    // create
    _loader = NSPluginLoader::instance();
    _callback = new NSPluginCallback(this);

    // create a canvas to insert our widget
    _canvas = new PluginCanvasWidget( parentWidget, widgetName );
    //_canvas->setFocusPolicy( TQWidget::ClickFocus );
    _canvas->setFocusPolicy( TQ_WheelFocus );
    _canvas->setBackgroundMode( TQWidget::NoBackground );
    setWidget(_canvas);
    _canvas->show();
    TQObject::connect( _canvas, TQT_SIGNAL(resized(int,int)),
                      this, TQT_SLOT(pluginResized(int,int)) );
}


PluginPart::~PluginPart()
{
    kdDebug(1432) << "PluginPart::~PluginPart" << endl;

    delete _callback;
    _loader->release();
    if (_destructed)
        *_destructed = true;
}


bool PluginPart::openURL(const KURL &url)
{
    closeURL();
    kdDebug(1432) << "-> PluginPart::openURL" << endl;

    m_url = url;
    TQString surl = url.url();
    TQString smime = _extension->urlArgs().serviceType;
    bool reload = _extension->urlArgs().reload;
    bool embed = false;
    bool post = _extension->urlArgs().doPost();

    // handle arguments
    TQStringList argn, argv;

    TQStringList::Iterator it = _args.begin();
    for ( ; it != _args.end(); ) {

        int equalPos = (*it).tqfind("=");
        if (equalPos>0) {

            TQString name = (*it).left(equalPos).upper();
            TQString value = (*it).mid(equalPos+1);
            if (value[0] == '"' && value[value.length()-1] == '"')
                value = value.mid(1, value.length()-2);

            kdDebug(1432) << "name=" << name << " value=" << value << endl;

            if (!name.isEmpty()) {
                // hack to pass view mode from khtml
                if ( name=="__KHTML__PLUGINEMBED" ) {
                    embed = true;
                    kdDebug(1432) << "__KHTML__PLUGINEMBED found" << endl;
                } else {
                    argn << name;
                    argv << value;
                }
            }
        }

        it++;
    }

    if (surl.isEmpty()) {
        kdDebug(1432) << "<- PluginPart::openURL - false (no url passed to nsplugin)" << endl;
        return false;
    }

    // status messages
    emit setWindowCaption( url.prettyURL() );
    emit setStatusBarText( i18n("Loading Netscape plugin for %1").arg(url.prettyURL()) );

    // create plugin widget
    NSPluginInstance *inst = _loader->newInstance(_canvas, surl, smime, embed, argn, argv, kapp->dcopClient()->appId(), _callback->objId(), reload, post, _extension->urlArgs().postData);

    if ( inst ) {
        _widget = inst;
    } else {
        TQLabel *label = new TQLabel( i18n("Unable to load Netscape plugin for %1").arg(url.url()), _canvas );
        label->tqsetAlignment( AlignCenter | WordBreak );
        _widget = label;
    }

    _widget->resize(_canvas->width(), _canvas->height());
    _widget->show();

    kdDebug(1432) << "<- PluginPart::openURL = " << (inst!=0) << endl;
    return inst != 0L;
}


bool PluginPart::closeURL()
{
    kdDebug(1432) << "PluginPart::closeURL" << endl;
    delete _widget;
    _widget = 0;
    return true;
}


void PluginPart::reloadPage()
{
    kdDebug(1432) << "PluginPart::reloadPage()" << endl;
    _extension->browserInterface()->callMethod("goHistory(int)", 0);
}

void PluginPart::postURL(const TQString& url, const TQString& target, const TQByteArray& data, const TQString& mime)
{
    kdDebug(1432) << "PluginPart::postURL( url=" << url
                  << ", target=" << target << endl;

    KURL new_url(this->url(), url);
    KParts::URLArgs args;
    args.setDoPost(true);
    args.frameName = target;
    args.postData = data;
    args.setContentType(mime);

    emit _extension->openURLRequest(new_url, args);
}

void PluginPart::requestURL(const TQString& url, const TQString& target)
{
    kdDebug(1432) << "PluginPart::requestURL( url=" << url
                  << ", target=" << target << endl;

    KURL new_url(this->url(), url);
    KParts::URLArgs args;
    args.frameName = target;
    args.setDoPost(false);

    emit _extension->openURLRequest(new_url, args);
}

void PluginPart::evalJavaScript(int id, const TQString & script)
{
    kdDebug(1432) <<"evalJavascript: before widget check"<<endl;
    if (_widget) {
        bool destructed = false;
        _destructed = &destructed;
	kdDebug(1432) <<"evalJavascript: there is a widget" <<endl;	
        TQString rc = _liveconnect->evalJavaScript(script);
        if (destructed)
            return;
        _destructed = 0L;
        kdDebug(1432) << "Liveconnect: script [" << script << "] evaluated to [" << rc << "]" << endl;
        NSPluginInstance *ni = dynamic_cast<NSPluginInstance*>(_widget.operator->());
        if (ni)
            ni->javascriptResult(id, rc);
    }
}

void PluginPart::statusMessage(TQString msg)
{
    kdDebug(1422) << "PluginPart::statusMessage " << msg << endl;
    emit setStatusBarText(msg);
}


void PluginPart::pluginResized(int w, int h)
{
    kdDebug(1432) << "PluginPart::pluginResized()" << endl;

    if (_widget) {
        _widget->resize(w, h);
    }
}


void PluginPart::changeSrc(const TQString& url) {
    closeURL();
    openURL(KURL( url ));
}


void PluginPart::saveAs() {
    KURL savefile = KFileDialog::getSaveURL(TQString::null, TQString::null, _widget);
    KIO::NetAccess::copy(m_url, savefile, _widget);
}


void PluginCanvasWidget::resizeEvent(TQResizeEvent *ev)
{
    TQWidget::resizeEvent(ev);
    emit resized(width(), height());
}


#include "plugin_part.moc"
