#include "konq_aboutpage.h"

#include <tqtextcodec.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kurifilter.h>
#include <ktrader.h>
#include <kconfig.h>

#include <assert.h>
#include <tqfile.h>
#include <tqdir.h>

K_EXPORT_COMPONENT_FACTORY( konq_aboutpage, KonqAboutPageFactory )

KInstance *KonqAboutPageFactory::s_instance = 0;
TQString *KonqAboutPageFactory::s_launch_html = 0;
TQString *KonqAboutPageFactory::s_intro_html = 0;
TQString *KonqAboutPageFactory::s_specs_html = 0;
TQString *KonqAboutPageFactory::s_tips_html = 0;
TQString *KonqAboutPageFactory::s_plugins_html = 0;

KonqAboutPageFactory::KonqAboutPageFactory( TQObject *parent, const char *name )
    : KParts::Factory( parent, name )
{
    s_instance = new KInstance( "konqaboutpage" );
}

KonqAboutPageFactory::~KonqAboutPageFactory()
{
    delete s_instance;
    s_instance = 0;
    delete s_launch_html;
    s_launch_html = 0;
    delete s_intro_html;
    s_intro_html = 0;
    delete s_specs_html;
    s_specs_html = 0;
    delete s_tips_html;
    s_tips_html = 0;
    delete s_plugins_html;
    s_plugins_html = 0;
}

KParts::Part *KonqAboutPageFactory::createPartObject( TQWidget *tqparentWidget, const char *widgetName,
                                                      TQObject *parent, const char *name,
                                                      const char *, const TQStringList & )
{
    //KonqFrame *frame = tqt_dynamic_cast<KonqFrame *>( tqparentWidget );
    //if ( !frame ) return 0;

    return new KonqAboutPage( //frame->childView()->mainWindow(),
                              tqparentWidget, widgetName, parent, name );
}

TQString KonqAboutPageFactory::loadFile( const TQString& file )
{
    TQString res;
    if ( file.isEmpty() )
	return res;

    TQFile f( file );

    if ( !f.open( IO_ReadOnly ) )
	return res;

    TQTextStream t( &f );

    res = t.read();

    // otherwise all embedded objects are referenced as about:/...
    TQString basehref = TQString::tqfromLatin1("<BASE HREF=\"file:") +
		       file.left( file.tqfindRev( '/' )) +
		       TQString::tqfromLatin1("/\">\n");
    TQRegExp reg("<head>");
    reg.setCaseSensitive(FALSE);
    res.tqreplace(reg, "<head>\n\t" + basehref);
    return res;
}

TQString KonqAboutPageFactory::launch()
{
  // FIXME: only regenerate launch page if kuriikwsfilterrc changed.
	/*
  if ( s_launch_html )
    return *s_launch_html;
	*/

  TQString res = loadFile( locate( "data", "konqueror/about/launch.html" ));
  if ( res.isEmpty() )
    return res;

  KIconLoader *iconloader = KGlobal::iconLoader();
  int iconSize = iconloader->currentSize(KIcon::Desktop);
  TQString home_icon_path = iconloader->iconPath("kfm_home", KIcon::Desktop );
  TQString storage_icon_path = iconloader->iconPath("system", KIcon::Desktop );
  TQString remote_icon_path = iconloader->iconPath("network", KIcon::Desktop );
  TQString wastebin_icon_path = iconloader->iconPath("trashcan_full", KIcon::Desktop );
  TQString applications_icon_path = iconloader->iconPath("kmenu", KIcon::Desktop );
  TQString settings_icon_path = iconloader->iconPath("kcontrol", KIcon::Desktop );
  TQString help_icon_path = iconloader->iconPath("khelpcenter", KIcon::Desktop );
  TQString home_folder = TQDir::homeDirPath();
  TQString continue_icon_path = TQApplication::reverseLayout()?iconloader->iconPath("1leftarrow", KIcon::Small ):iconloader->iconPath("1rightarrow", KIcon::Small );

  res = res.tqarg( locate( "data", "kdeui/about/kde_infopage.css" ) );
  if ( kapp->reverseLayout() )
    res = res.tqarg( "@import \"%1\";" ).tqarg( locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
  else
    res = res.tqarg( "" );

  // Try to split page in three. If it succeeds, insert the default search into the middle part.
  TQStringList parts = TQStringList::split( "<!--search bar splitter-->", res );
  if ( parts.count() == 3 ) {
    KConfig config( "kuriikwsfilterrc", true /*read-only*/, false /*no KDE globals*/ );
    config.setGroup( "General" );
    TQString name = config.readEntry("DefaultSearchEngine");
    KService::Ptr service =
        KService::serviceByDesktopPath(TQString("searchproviders/%1.desktop").tqarg(name));
    if ( service ) {
      TQString searchBar = parts[1];
      searchBar = searchBar
          .tqarg( iconSize ).tqarg( iconSize )
          .tqarg( service->name() )
          .tqarg( service->property("Keys").toStringList()[0] )
          ;
      res = parts[0] + searchBar + parts[2];
    }
    else res = parts[0] + parts[2];
  }

  res = res.tqarg( i18n("Conquer your Desktop!") )
      .tqarg( i18n( "Konqueror" ) )
      .tqarg( i18n("Conquer your Desktop!") )
      .tqarg( i18n("Konqueror is your file manager, web browser and universal document viewer.") )
      .tqarg( i18n( "Starting Points" ) )
      .tqarg( i18n( "Introduction" ) )
      .tqarg( i18n( "Tips" ) )
      .tqarg( i18n( "Specifications" ) )
      .tqarg( home_folder )
      .tqarg( home_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( home_folder )
      .tqarg( i18n( "Home Folder" ) )
      .tqarg( i18n( "Your personal files" ) )
      .tqarg( storage_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( i18n( "Storage Media" ) )
      .tqarg( i18n( "Disks and removable media" ) )
      .tqarg( remote_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( i18n( "Network Folders" ) )
      .tqarg( i18n( "Shared files and folders" ) )
      .tqarg( wastebin_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( i18n( "Trash" ) )
      .tqarg( i18n( "Browse and restore the trash" ) )
      .tqarg( applications_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( i18n( "Applications" ) )
      .tqarg( i18n( "Installed programs" ) )
      .tqarg( help_icon_path )
      .tqarg(iconSize).tqarg(iconSize)
      .tqarg( i18n( "About Kubuntu" ) )
      .tqarg( i18n( "<a href=\"help:/kubuntu/\">Kubuntu Documentation</a>" ) )
      .tqarg( continue_icon_path )
      .tqarg( KIcon::SizeSmall ).tqarg( KIcon::SizeSmall )
      .tqarg( i18n( "Next: An Introduction to Konqueror" ) )
      ;
  i18n("Search the Web");//i18n for possible future use

  s_launch_html = new TQString( res );

  return res;
}

TQString KonqAboutPageFactory::intro()
{
    if ( s_intro_html )
        return *s_intro_html;

    TQString res = loadFile( locate( "data", "konqueror/about/intro.html" ));
    if ( res.isEmpty() )
	return res;

    KIconLoader *iconloader = KGlobal::iconLoader();
    TQString back_icon_path = TQApplication::reverseLayout()?iconloader->iconPath("forward", KIcon::Small ):iconloader->iconPath("back", KIcon::Small );
    TQString gohome_icon_path = iconloader->iconPath("gohome", KIcon::Small );
    TQString continue_icon_path = TQApplication::reverseLayout()?iconloader->iconPath("1leftarrow", KIcon::Small ):iconloader->iconPath("1rightarrow", KIcon::Small );

    res = res.tqarg( locate( "data", "kdeui/about/kde_infopage.css" ) );
    if ( kapp->reverseLayout() )
	res = res.tqarg( "@import \"%1\";" ).tqarg( locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
    else
	res = res.tqarg( "" );

    res = res.tqarg( i18n("Conquer your Desktop!") )
	.tqarg( i18n( "Konqueror" ) )
	.tqarg( i18n( "Conquer your Desktop!") )
	.tqarg( i18n( "Konqueror is your file manager, web browser and universal document viewer.") )
	.tqarg( i18n( "Starting Points" ) )
	.tqarg( i18n( "Introduction" ) )
          .tqarg( i18n( "Tips" ) )
          .tqarg( i18n( "Specifications" ) )
          .tqarg( i18n( "Konqueror makes working with and managing your files easy. You can browse "
                      "both local and networked folders while enjoying advanced features "
                      "such as the powerful sidebar and file previews."
		      ) )
          .tqarg( i18n( "Konqueror is also a full featured and easy to use web browser which you "
                      "can  use to explore the Internet. "
                      "Enter the address (e.g. <a href=\"http://www.kde.org\">http://www.kde.org</A>) "
                      "of a web page you would like to visit in the location bar and press Enter, "
                      "or choose an entry from the Bookmarks menu.") )
          .tqarg( i18n( "To return to the previous "
		      "location, press the back button  <img width='16' height='16' src=\"%1\"> "
                      "in the toolbar. ").tqarg( back_icon_path ) )
          .tqarg( i18n( "To quickly go to your Home folder press the "
                      " home button <img width='16' height='16' src=\"%1\">." ).tqarg(gohome_icon_path) )
          .tqarg( i18n( "For more detailed documentation on Konqueror click <a href=\"%1\">here</a>." )
                      .tqarg("exec:/khelpcenter") )
          .tqarg( i18n( "<em>Tuning Tip:</em> If you want the Konqueror web browser to start faster,"
			" you can turn off this information screen by clicking <a href=\"%1\">here</a>. You can re-enable it"
			" by choosing the Help -> Konqueror Introduction menu option, and then pressing "
			"Settings -> Save View Profile \"Web Browsing\".").tqarg("config:/disable_overview") )
	  .tqarg( "<img width='16' height='16' src=\"%1\">" ).tqarg( continue_icon_path )
	  .tqarg( i18n( "Next: Tips &amp; Tricks" ) )
	;


    s_intro_html = new TQString( res );

    return res;
}

TQString KonqAboutPageFactory::specs()
{
    if ( s_specs_html )
        return *s_specs_html;

    KIconLoader *iconloader = KGlobal::iconLoader();
    TQString res = loadFile( locate( "data", "konqueror/about/specs.html" ));
    TQString continue_icon_path = TQApplication::reverseLayout()?iconloader->iconPath("1leftarrow", KIcon::Small ):iconloader->iconPath("1rightarrow", KIcon::Small );
    if ( res.isEmpty() )
	return res;

    res = res.tqarg( locate( "data", "kdeui/about/kde_infopage.css" ) );
    if ( kapp->reverseLayout() )
	res = res.tqarg( "@import \"%1\";" ).tqarg( locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
    else
	res = res.tqarg( "" );

    res = res.tqarg( i18n("Conquer your Desktop!") )
	.tqarg( i18n( "Konqueror" ) )
	.tqarg( i18n("Conquer your Desktop!") )
	.tqarg( i18n("Konqueror is your file manager, web browser and universal document viewer.") )
	.tqarg( i18n( "Starting Points" ) )
	.tqarg( i18n( "Introduction" ) )
	.tqarg( i18n( "Tips" ) )
	.tqarg( i18n( "Specifications" ) )
          .tqarg( i18n("Specifications") )
          .tqarg( i18n("Konqueror is designed to embrace and support Internet standards. "
		     "The aim is to fully implement the officially sanctioned standards "
		     "from organizations such as the W3 and OASIS, while also adding "
		     "extra support for other common usability features that arise as "
		     "de facto standards across the Internet. Along with this support, "
		     "for such functions as favicons, Internet Keywords, and <A HREF=\"%1\">XBEL bookmarks</A>, "
                     "Konqueror also implements:").tqarg("http://pyxml.sourceforge.net/topics/xbel/") )
          .tqarg( i18n("Web Browsing") )
          .tqarg( i18n("Supported standards") )
          .tqarg( i18n("Additional requirements*") )
          .tqarg( i18n("<A HREF=\"%1\">DOM</A> (Level 1, partially Level 2) based "
                     "<A HREF=\"%2\">HTML 4.01</A>").tqarg("http://www.w3.org/DOM").tqarg("http://www.w3.org/TR/html4/") )
          .tqarg( i18n("built-in") )
          .tqarg( i18n("<A HREF=\"%1\">Cascading Style Sheets</A> (CSS 1, partially CSS 2)").tqarg("http://www.w3.org/Style/CSS/") )
          .tqarg( i18n("built-in") )
          .tqarg( i18n("<A HREF=\"%1\">ECMA-262</A> Edition 3 (roughly equals JavaScript 1.5)").tqarg("http://www.ecma.ch/ecma1/STAND/ECMA-262.HTM") )
          .tqarg( i18n("JavaScript disabled (globally). Enable JavaScript <A HREF=\"%1\">here</A>.").tqarg("exec:/kcmshell khtml_java_js") )
          .tqarg( i18n("JavaScript enabled (globally). Configure JavaScript <A HREF=\\\"%1\\\">here</A>.").tqarg("exec:/kcmshell khtml_java_js") ) // leave the double backslashes here, they are necessary for javascript !
          .tqarg( i18n("Secure <A HREF=\"%1\">Java</A><SUP>&reg;</SUP> support").tqarg("http://java.sun.com") )
          .tqarg( i18n("JDK 1.2.0 (Java 2) compatible VM (<A HREF=\"%1\">Blackdown</A>, <A HREF=\"%2\">IBM</A> or <A HREF=\"%3\">Sun</A>)")
                      .tqarg("http://www.blackdown.org").tqarg("http://www.ibm.com").tqarg("http://java.sun.com") )
          .tqarg( i18n("Enable Java (globally) <A HREF=\"%1\">here</A>.").tqarg("exec:/kcmshell khtml_java_js") ) // TODO Maybe test if Java is enabled ?
          .tqarg( i18n("Netscape Communicator<SUP>&reg;</SUP> <A HREF=\"%4\">plugins</A> (for viewing <A HREF=\"%1\">Flash<SUP>&reg;</SUP></A>, <A HREF=\"%2\">Real<SUP>&reg;</SUP></A>Audio, <A HREF=\"%3\">Real<SUP>&reg;</SUP></A>Video, etc.)")
                      .tqarg("http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash")
                      .tqarg("http://www.real.com").tqarg("http://www.real.com")
                      .tqarg("about:plugins") )
          .tqarg( i18n("built-in") )
          .tqarg( i18n("Secure Sockets Layer") )
          .tqarg( i18n("(TLS/SSL v2/3) for secure communications up to 168bit") )
          .tqarg( i18n("OpenSSL") )
          .tqarg( i18n("Bidirectional 16bit tqunicode support") )
          .tqarg( i18n("built-in") )
          .tqarg( i18n("AutoCompletion for forms") )
          .tqarg( i18n("built-in") )
          .tqarg( i18n("G E N E R A L") )
          .tqarg( i18n("Feature") )
          .tqarg( i18n("Details") )
          .tqarg( i18n("Image formats") )
          .tqarg( i18n("Transfer protocols") )
          .tqarg( i18n("HTTP 1.1 (including gzip/bzip2 compression)") )
          .tqarg( i18n("FTP") )
          .tqarg( i18n("and <A HREF=\"%1\">many more...</A>").tqarg("exec:/kcmshell ioslaveinfo") )
          .tqarg( i18n("URL-Completion") )
          .tqarg( i18n("Manual"))
	  .tqarg( i18n("Popup"))
	  .tqarg( i18n("(Short-) Automatic"))
	  .tqarg( "<img width='16' height='16' src=\"%1\">" ).tqarg( continue_icon_path )
	  .tqarg( i18n("<a href=\"%1\">Return to Starting Points</a>").tqarg("launch.html") )

          ;

    s_specs_html = new TQString( res );

    return res;
}

TQString KonqAboutPageFactory::tips()
{
    if ( s_tips_html )
        return *s_tips_html;

    TQString res = loadFile( locate( "data", "konqueror/about/tips.html" ));
    if ( res.isEmpty() )
	return res;

    KIconLoader *iconloader = KGlobal::iconLoader();
    TQString viewmag_icon_path =
	    iconloader->iconPath("viewmag", KIcon::Small );
    TQString history_icon_path =
	    iconloader->iconPath("history", KIcon::Small );
    TQString openterm_icon_path =
	    iconloader->iconPath("openterm", KIcon::Small );
    TQString locationbar_erase_rtl_icon_path =
	    iconloader->iconPath("clear_left", KIcon::Small );
    TQString locationbar_erase_icon_path =
	    iconloader->iconPath("locationbar_erase", KIcon::Small );
    TQString window_fullscreen_icon_path =
	    iconloader->iconPath("window_fullscreen", KIcon::Small );
    TQString view_left_right_icon_path =
	    iconloader->iconPath("view_left_right", KIcon::Small );
    TQString continue_icon_path = TQApplication::reverseLayout()?iconloader->iconPath("1leftarrow", KIcon::Small ):iconloader->iconPath("1rightarrow", KIcon::Small );

    res = res.tqarg( locate( "data", "kdeui/about/kde_infopage.css" ) );
    if ( kapp->reverseLayout() )
	res = res.tqarg( "@import \"%1\";" ).tqarg( locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
    else
	res = res.tqarg( "" );

    res = res.tqarg( i18n("Conquer your Desktop!") )
	.tqarg( i18n( "Konqueror" ) )
	.tqarg( i18n("Conquer your Desktop!") )
	.tqarg( i18n("Konqueror is your file manager, web browser and universal document viewer.") )
	.tqarg( i18n( "Starting Points" ) )
	.tqarg( i18n( "Introduction" ) )
	.tqarg( i18n( "Tips" ) )
	.tqarg( i18n( "Specifications" ) )
	.tqarg( i18n( "Tips &amp; Tricks" ) )
	  .tqarg( i18n( "Use Internet-Keywords and Web-Shortcuts: by typing \"gg: KDE\" one can search the Internet, "
		      "using Google, for the search phrase \"KDE\". There are a lot of "
		      "Web-Shortcuts predefined to make searching for software or looking "
		      "up certain words in an encyclopedia a breeze. You can even "
                      "<a href=\"%1\">create your own</a> Web-Shortcuts." ).tqarg("exec:/kcmshell ebrowsing") )
	  .tqarg( i18n( "Use the magnifier button <img width='16' height='16' src=\"%1\"> in the"
		      " toolbar to increase the font size on your web page.").tqarg(viewmag_icon_path) )
	  .tqarg( i18n( "When you want to paste a new address into the Location toolbar you might want to "
		      "clear the current entry by pressing the black arrow with the white cross "
		      "<img width='16' height='16' src=\"%1\"> in the toolbar.")
              .tqarg(TQApplication::reverseLayout() ? locationbar_erase_rtl_icon_path : locationbar_erase_icon_path))
	  .tqarg( i18n( "To create a link on your desktop pointing to the current page, "
		      "simply drag the \"Location\" label that is to the left of the Location toolbar, drop it on to "
		      "the desktop, and choose \"Link\"." ) )
	  .tqarg( i18n( "You can also find <img width='16' height='16' src=\"%1\"> \"Full-Screen Mode\" "
		      "in the Settings menu. This feature is very useful for \"Talk\" "
		      "sessions.").tqarg(window_fullscreen_icon_path) )
	  .tqarg( i18n( "Divide et impera (lat. \"Divide and conquer\") - by splitting a window "
		      "into two parts (e.g. Window -> <img width='16' height='16' src=\"%1\"> Split View "
		      "Left/Right) you can make Konqueror appear the way you like. You"
		      " can even load some example view-profiles (e.g. Midnight Commander)"
		      ", or create your own ones." ).tqarg(view_left_right_icon_path))
	  .tqarg( i18n( "Use the <a href=\"%1\">user-agent</a> feature if the website you are visiting "
                      "asks you to use a different browser "
		      "(and do not forget to send a complaint to the webmaster!)" ).tqarg("exec:/kcmshell useragent") )
	  .tqarg( i18n( "The <img width='16' height='16' src=\"%1\"> History in your SideBar ensures "
		      "that you can keep track of the pages you have visited recently.").tqarg(history_icon_path) )
	  .tqarg( i18n( "Use a caching <a href=\"%1\">proxy</a> to speed up your"
		      " Internet connection.").tqarg("exec:/kcmshell proxy") )
	  .tqarg( i18n( "Advanced users will appreciate the Konsole which you can embed into "
		      "Konqueror (Window -> <img width='16' height='16' SRC=\"%1\"> Show "
 		      "Terminal Emulator).").tqarg(openterm_icon_path))
	  .tqarg( i18n( "Thanks to <a href=\"%1\">DCOP</a> you can have full control over Konqueror using a script."
).tqarg("exec:/kdcop") )
	  .tqarg( i18n( "<img width='16' height='16' src=\"%1\">" ).tqarg( continue_icon_path ) )
	  .tqarg( i18n( "Next: Specifications" ) )
          ;


    s_tips_html = new TQString( res );

    return res;
}


TQString KonqAboutPageFactory::plugins()
{
    if ( s_plugins_html )
        return *s_plugins_html;

    TQString res = loadFile( locate( "data", kapp->reverseLayout() ? "konqueror/about/plugins_rtl.html" : "konqueror/about/plugins.html" ))
                  .tqarg(i18n("Installed Plugins"))
                  .tqarg(i18n("<td>Plugin</td><td>Description</td><td>File</td><td>Types</td>"))
                  .tqarg(i18n("Installed"))
                  .tqarg(i18n("<td>Mime Type</td><td>Description</td><td>Suffixes</td><td>Plugin</td>"));
    if ( res.isEmpty() )
	return res;

    s_plugins_html = new TQString( res );

    return res;
}


KonqAboutPage::KonqAboutPage( //KonqMainWindow *
                              TQWidget *tqparentWidget, const char *widgetName,
                              TQObject *parent, const char *name )
    : KHTMLPart( tqparentWidget, widgetName, parent, name, BrowserViewGUI )
{
    //m_mainWindow = mainWindow;
    TQTextCodec* codec = KGlobal::locale()->codecForEncoding();
    if (codec)
	setCharset(codec->name(), true);
    else
	setCharset("iso-8859-1", true);
    // about:blah isn't a kioslave -> disable View source
    KAction * act = actionCollection()->action("viewDocumentSource");
    if ( act )
      act->setEnabled( false );
}

KonqAboutPage::~KonqAboutPage()
{
}

bool KonqAboutPage::openURL( const KURL &u )
{
	kdDebug(1202) << "now in KonqAboutPage::openURL( \"" << u.url() << "\" )" << endl;
	if ( u.url() == "about:plugins" )
		serve( KonqAboutPageFactory::plugins(), "plugins" );
	else if ( !u.query().isEmpty() ) {
		TQMap< TQString, TQString > queryItems = u.queryItems( 0 );
		TQMap< TQString, TQString >::ConstIterator query = queryItems.begin();
		TQString newUrl;
		if (query.key() == "strigi") {
		  newUrl = KURIFilter::self()->filteredURI( query.key() + ":?q=" + query.data() );
		} else {
		  newUrl = KURIFilter::self()->filteredURI( query.key() + ":" + query.data() );
		}
		kdDebug(1202) << "scheduleRedirection( 0, \"" << newUrl << "\" )" << endl;
		scheduleRedirection( 0, newUrl );
	}
	else serve( KonqAboutPageFactory::launch(), "konqueror" );
	return true;
}

bool KonqAboutPage::openFile()
{
    return true;
}

void KonqAboutPage::saveState( TQDataStream &stream )
{
    stream << m_htmlDoc;
    stream << m_what;
}

void KonqAboutPage::restoreState( TQDataStream &stream )
{
    stream >> m_htmlDoc;
    stream >> m_what;
    serve( m_htmlDoc, m_what );
}

void KonqAboutPage::serve( const TQString& html, const TQString& what )
{
    m_what = what;
    begin( KURL( TQString("about:%1").tqarg(what) ) );
    write( html );
    end();
    m_htmlDoc = html;
}

void KonqAboutPage::urlSelected( const TQString &url, int button, int state, const TQString &target, KParts::URLArgs _args )
{
    KURL u( url );
    if ( u.protocol() == "exec" )
    {
        TQStringList args = TQStringList::split( TQChar( ' ' ), url.mid( 6 ) );
        TQString executable = args[ 0 ];
        args.remove( args.begin() );
        KApplication::kdeinitExec( executable, args );
        return;
    }

    if ( url == TQString::tqfromLatin1("launch.html") )
    {
        emit browserExtension()->openURLNotify();
	serve( KonqAboutPageFactory::launch(), "konqueror" );
        return;
    }
    else if ( url == TQString::tqfromLatin1("intro.html") )
    {
        emit browserExtension()->openURLNotify();
        serve( KonqAboutPageFactory::intro(), "konqueror" );
        return;
    }
    else if ( url == TQString::tqfromLatin1("specs.html") )
    {
        emit browserExtension()->openURLNotify();
	serve( KonqAboutPageFactory::specs(), "konqueror" );
        return;
    }
    else if ( url == TQString::tqfromLatin1("tips.html") )
    {
        emit browserExtension()->openURLNotify();
        serve( KonqAboutPageFactory::tips(), "konqueror" );
        return;
    }

    else if ( url == TQString::tqfromLatin1("config:/disable_overview") )
    {
	if ( KMessageBox::questionYesNo( widget(),
					 i18n("Do you want to disable showing "
					      "the introduction in the webbrowsing profile?"),
					 i18n("Faster Startup?"),i18n("Disable"),i18n("Keep") )
	     == KMessageBox::Yes )
	{
	    TQString profile = locateLocal("data", "konqueror/profiles/webbrowsing");
	    KSaveFile file( profile );
	    if ( file.status() == 0 ) {
		TQCString content = "[Profile]\n"
			           "Name=Web-Browser";
		fputs( content.data(), file.fstream() );
		file.close();
	    }
	}
	return;
    }

    KHTMLPart::urlSelected( url, button, state, target, _args );
}

#include "konq_aboutpage.moc"
