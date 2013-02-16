//
//
// "Desktop Options" Tab for KDesktop configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000

#include <config.h>

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqvgroupbox.h>
#include <tqwhatsthis.h>

#include <dcopclient.h>

#include <tdeapplication.h>
#include <kcustommenueditor.h>
#include <kdebug.h>
#include <tdefileitem.h>
#include <tdeglobalsettings.h>
#include <kipc.h>
#include <tdelistview.h>
#include <tdeio/job.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <konq_defaults.h> // include default values directly from libkonq
#include <kurlrequester.h>

#include "rootopts.h"

//-----------------------------------------------------------------------------

DesktopPathConfig::DesktopPathConfig(TQWidget *parent, const char * )
    : TDECModule( parent, "kcmkonq" )
{
  TQLabel * tmpLabel;

#undef RO_LASTROW
#undef RO_LASTCOL
#define RO_LASTROW 4   // 3 paths + last row
#define RO_LASTCOL 2

  int row = 0;
  TQGridLayout *lay = new TQGridLayout(this, RO_LASTROW+1, RO_LASTCOL+1,
      0, KDialog::spacingHint());

  lay->setRowStretch(RO_LASTROW,10); // last line grows

  lay->setColStretch(0,0);
  lay->setColStretch(1,0);
  lay->setColStretch(2,10);


  setQuickHelp( i18n("<h1>Paths</h1>\n"
    "This module allows you to choose where in the filesystem the "
    "files on your desktop should be stored.\n"
    "Use the \"What's This?\" (Shift+F1) to get help on specific options."));

  // Desktop Paths
  row++;
  tmpLabel = new TQLabel(i18n("Des&ktop path:"), this);
  lay->addWidget(tmpLabel, row, 0);
  urDesktop = new KURLRequester(this);
  urDesktop->setMode( KFile::Directory );
  tmpLabel->setBuddy( urDesktop );
  lay->addMultiCellWidget(urDesktop, row, row, 1, RO_LASTCOL);
  connect(urDesktop, TQT_SIGNAL(textChanged(const TQString &)), this, TQT_SLOT(changed()));
  TQString wtstr = i18n("This folder contains all the files"
                       " which you see on your desktop. You can change the location of this"
                       " folder if you want to, and the contents will move automatically"
                       " to the new location as well.");
  TQWhatsThis::add( tmpLabel, wtstr );
  TQWhatsThis::add( urDesktop, wtstr );

  row++;
  tmpLabel = new TQLabel(i18n("A&utostart path:"), this);
  lay->addWidget(tmpLabel, row, 0);
  urAutostart = new KURLRequester(this);
  urAutostart->setMode( KFile::Directory );
  tmpLabel->setBuddy( urAutostart );
  lay->addMultiCellWidget(urAutostart, row, row, 1, RO_LASTCOL);
  connect(urAutostart, TQT_SIGNAL(textChanged(const TQString &)), this, TQT_SLOT(changed()));
  wtstr = i18n("This folder contains applications or"
               " links to applications (shortcuts) that you want to have started"
               " automatically whenever TDE starts. You can change the location of this"
               " folder if you want to, and the contents will move automatically"
               " to the new location as well.");
  TQWhatsThis::add( tmpLabel, wtstr );
  TQWhatsThis::add( urAutostart, wtstr );

  row++;
  tmpLabel = new TQLabel(i18n("D&ocuments path:"), this);
  lay->addWidget(tmpLabel, row, 0);
  urDocument = new KURLRequester(this);
  urDocument->setMode( KFile::Directory );
  tmpLabel->setBuddy( urDocument );
  lay->addMultiCellWidget(urDocument, row, row, 1, RO_LASTCOL);
  connect(urDocument, TQT_SIGNAL(textChanged(const TQString &)), this, TQT_SLOT(changed()));
  wtstr = i18n("This folder will be used by default to "
               "load or save documents from or to.");
  TQWhatsThis::add( tmpLabel, wtstr );
  TQWhatsThis::add( urDocument, wtstr );

  // -- Bottom --
  Q_ASSERT( row == RO_LASTROW-1 ); // if it fails here, check the row++ and RO_LASTROW above

  load();
}

void DesktopPathConfig::load()
{
	load( false );
}

void DesktopPathConfig::load( bool useDefaults )
{
    TDEConfig config("kdeglobals", true, false);
    // Desktop Paths
	config.setReadDefaults( useDefaults );
    config.setGroup("Paths");
    urAutostart->setURL( config.readPathEntry( "Autostart" , TDEGlobalSettings::autostartPath() ));

    TDEConfig xdguserconfig( TQDir::homeDirPath()+"/.config/user-dirs.dirs" );
     
    urDesktop->setURL( xdguserconfig.readPathEntry( "XDG_DESKTOP_DIR" , TQDir::homeDirPath() + "/Desktop" ).remove(  "\"" ));
    urDocument->setURL( xdguserconfig.readPathEntry( "XDG_DOCUMENTS_DIR", TQDir::homeDirPath()).remove(  "\"" ));
    
    emit changed( useDefaults );
}

void DesktopPathConfig::defaults()
{
	load( true );
}

void DesktopPathConfig::save()
{
    TDEConfig *config = TDEGlobal::config();
    TDEConfig *xdgconfig = new TDEConfig( TQDir::homeDirPath()+"/.config/user-dirs.dirs" );
    TDEConfigGroupSaver cgs( config, "Paths" );

    bool pathChanged = false;
    bool autostartMoved = false;

    KURL desktopURL;
    desktopURL.setPath( TDEGlobalSettings::desktopPath() );
    KURL newDesktopURL;
    newDesktopURL.setPath(urDesktop->url());

    KURL autostartURL;
    autostartURL.setPath( TDEGlobalSettings::autostartPath() );
    KURL newAutostartURL;
    newAutostartURL.setPath(urAutostart->url());

    KURL documentURL;
    documentURL.setPath( TDEGlobalSettings::documentPath() );
    KURL newDocumentURL;
    newDocumentURL.setPath(urDocument->url());

    if ( !newDesktopURL.equals( desktopURL, true ) )
    {
        // Test which other paths were inside this one (as it is by default)
        // and for each, test where it should go.
        // * Inside destination -> let them be moved with the desktop (but adjust name if necessary)
        // * Not inside destination -> move first
        // !!!
        kdDebug() << "desktopURL=" << desktopURL.url() << endl;
        TQString urlDesktop = urDesktop->url();
        if ( !urlDesktop.endsWith( "/" ))
            urlDesktop+="/";

        if ( desktopURL.isParentOf( autostartURL ) )
        {
            kdDebug() << "Autostart is on the desktop" << endl;

            // Either the Autostart field wasn't changed (-> need to update it)
            if ( newAutostartURL.equals( autostartURL, true ) )
            {
                // Hack. It could be in a subdir inside desktop. Hmmm... Argl.
                urAutostart->setURL( urlDesktop + "Autostart/" );
                kdDebug() << "Autostart is moved with the desktop" << endl;
                autostartMoved = true;
            }
            // or it has been changed (->need to move it from here)
            else
            {
                KURL futureAutostartURL;
                futureAutostartURL.setPath( urlDesktop + "Autostart/" );
                if ( newAutostartURL.equals( futureAutostartURL, true ) )
                    autostartMoved = true;
                else
                    autostartMoved = moveDir( KURL( TDEGlobalSettings::autostartPath() ), KURL( urAutostart->url() ), i18n("Autostart") );
            }
        }

        if ( moveDir( KURL( TDEGlobalSettings::desktopPath() ), KURL( urlDesktop ), i18n("Desktop") ) )
        {
            xdgconfig->writePathEntry( "XDG_DESKTOP_DIR", '"'+ urlDesktop + '"', true, false );
            pathChanged = true;
        }
    }

    if ( !newAutostartURL.equals( autostartURL, true ) )
    {
        if (!autostartMoved)
            autostartMoved = moveDir( KURL( TDEGlobalSettings::autostartPath() ), KURL( urAutostart->url() ), i18n("Autostart") );
        if (autostartMoved)
        {
            config->writePathEntry( "Autostart", urAutostart->url(), true, true );
            pathChanged = true;
        }
    }

    if ( !newDocumentURL.equals( documentURL, true ) )
    {
        bool pathOk = true;
        TQString path = urDocument->url();
        if (!TQDir(path).exists())
        {
            if (!TDEStandardDirs::makeDir(path))
            {
                KMessageBox::sorry(this, TDEIO::buildErrorString(TDEIO::ERR_COULD_NOT_MKDIR, path));
                urDocument->setURL(documentURL.path());
                pathOk = false;
            }
        }

        if (pathOk)
        {
            xdgconfig->writePathEntry( "XDG_DOCUMENTS_DIR", '"' + path + '"', true, false );
            pathChanged = true;
        }
    }

    config->sync();
    xdgconfig->sync();

    if (pathChanged)
    {
        kdDebug() << "DesktopPathConfig::save sending message SettingsChanged" << endl;
        KIPC::sendMessageAll(KIPC::SettingsChanged, TDEApplication::SETTINGS_PATHS);
    }

    // Tell kdesktop about the new config file
    if ( !kapp->dcopClient()->isAttached() )
       kapp->dcopClient()->attach();
    TQByteArray data;

    int konq_screen_number = TDEApplication::desktop()->primaryScreen();
    TQCString appname;
    if (konq_screen_number == 0)
        appname = "kdesktop";
    else
        appname.sprintf("kdesktop-screen-%d", konq_screen_number);
    kapp->dcopClient()->send( appname, "KDesktopIface", "configure()", data );
}

bool DesktopPathConfig::moveDir( const KURL & src, const KURL & dest, const TQString & type )
{
    if (!src.isLocalFile() || !dest.isLocalFile())
        return true;
    m_ok = true;
    // Ask for confirmation before moving the files
    if ( KMessageBox::questionYesNo( this, i18n("The path for '%1' has been changed;\ndo you want the files to be moved from '%2' to '%3'?").
                              arg(type).arg(src.path()).arg(dest.path()), i18n("Confirmation Required"),i18n("Move"),KStdGuiItem::cancel() )
            == KMessageBox::Yes )
    {
        bool destExists = TQFile::exists(dest.path());
        if (destExists)
        {
            m_copyToDest = dest;
            m_copyFromSrc = src;
            TDEIO::ListJob* job = TDEIO::listDir( src );
            connect( job, TQT_SIGNAL( entries( TDEIO::Job *, const TDEIO::UDSEntryList& ) ),
                     this, TQT_SLOT( slotEntries( TDEIO::Job *, const TDEIO::UDSEntryList& ) ) );
            tqApp->enter_loop();

            if (m_ok)
            {
                TDEIO::del( src );
            }
        }
        else
        {
            TDEIO::Job * job = TDEIO::move( src, dest );
            connect( job, TQT_SIGNAL( result( TDEIO::Job * ) ), this, TQT_SLOT( slotResult( TDEIO::Job * ) ) );
            // wait for job
            tqApp->enter_loop();
        }
    }
    kdDebug() << "DesktopPathConfig::slotResult returning " << m_ok << endl;
    return m_ok;
}

void DesktopPathConfig::slotEntries( TDEIO::Job * job, const TDEIO::UDSEntryList& list)
{
    if (job->error())
    {
        job->showErrorDialog(this);
        return;
    }

    TDEIO::UDSEntryListConstIterator it = list.begin();
    TDEIO::UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it)
    {
        KFileItem file(*it, m_copyFromSrc, true, true);
        if (file.url() == m_copyFromSrc || file.url().fileName() == "..")
        {
            continue;
        }

        TDEIO::Job * moveJob = TDEIO::move( file.url(), m_copyToDest );
        connect( moveJob, TQT_SIGNAL( result( TDEIO::Job * ) ), this, TQT_SLOT( slotResult( TDEIO::Job * ) ) );
        tqApp->enter_loop();
    }
    tqApp->exit_loop();
}

void DesktopPathConfig::slotResult( TDEIO::Job * job )
{
    if (job->error())
    {
        if ( job->error() != TDEIO::ERR_DOES_NOT_EXIST )
            m_ok = false;
        // If the source doesn't exist, no wonder we couldn't move the dir.
        // In that case, trust the user and set the new setting in any case.

        job->showErrorDialog(this);
    }
    tqApp->exit_loop();
}

#include "rootopts.moc"
