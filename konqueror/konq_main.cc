/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "konq_main.h"
#include "konq_misc.h"
#include "konq_factory.h"
#include "konq_mainwindow.h"
#include "konq_view.h"
#include "konq_settingsxt.h"
#include "KonquerorIface.h"

#include <tdetempfile.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tdecmdlineargs.h>
#include <dcopclient.h>
#include <kimageio.h>
#include <tqfile.h>

#include <tqwidgetlist.h>

static const TDECmdLineOptions options[] =
{
  { "silent", I18N_NOOP("Start without a default window"), 0 },
  { "preload", I18N_NOOP("Preload for later use"), 0 },
  { "profile <profile>",   I18N_NOOP("Profile to open"), 0 },
  { "profiles", I18N_NOOP("List available profiles"), 0 },
  { "mimetype <mimetype>",   I18N_NOOP("Mimetype to use for this URL (e.g. text/html or inode/directory)"), 0 },
  { "select", I18N_NOOP("For URLs that point to files, opens the directory and selects the file, instead of opening the actual file"), 0 },
  { "+[URL]",   I18N_NOOP("Location to open"), 0 },
  TDECmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
  TDECmdLineArgs::init( argc, argv, KonqFactory::aboutData() );

  TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  TDECmdLineArgs::addTempFileOption();

  KonquerorApplication app;

  app.dcopClient()->registerAs( "konqueror" );

  KonquerorIface *kiface = new KonquerorIface;
  app.dcopClient()->setDefaultObject( kiface->objId() );

  TDEGlobal::locale()->insertCatalogue("libkonq"); // needed for apps using libkonq
  KImageIO::registerFormats();

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  KTempFile crashlog_file(locateLocal("tmp", "konqueror-crash-"), ".log");
  KonqMainWindow::s_crashlog_file = crashlog_file.file();

  if ( kapp->isRestored() )
  {
    int n = 1;
    while ( KonqMainWindow::canBeRestored( n ) )
    {
      TQString className = TDEMainWindow::classNameOfToplevel( n );
      if( className == TQString::fromLatin1( "KonqMainWindow" ))
          (new KonqMainWindow( KURL(), false ) )->restore( n );
      else
          kdWarning() << "Unknown class " << className << " in session saved data!" << endl;
      n++;
    }
  }
  else
  {
     if (args->isSet("profiles"))
     {
       TQStringList profiles = TDEGlobal::dirs()->findAllResources("data", "konqueror/profiles/*", false, true);
       profiles.sort();
       for(TQStringList::ConstIterator it = profiles.begin();
           it != profiles.end(); ++it)
       {
         TQString file = *it;
         file = file.mid(file.findRev('/')+1);
         printf("%s\n", TQFile::encodeName(file).data());
       }

       return 0;
     }
     if (args->isSet("profile"))
     {
       TQString profile = TQString::fromLocal8Bit(args->getOption("profile"));
       TQString profilePath = profile;
       if (profile[0] != '/')
           profilePath = locate( "data", TQString::fromLatin1("konqueror/profiles/")+profile );
       TQString url;
       TQStringList filesToSelect;
       if (args->count() == 1)
           url = TQString::fromLocal8Bit(args->arg(0));
       KURL kurl(url);
       KParts::URLArgs urlargs;
       if (args->isSet("mimetype"))
           urlargs.serviceType = TQString::fromLocal8Bit(args->getOption("mimetype"));
       if (args->isSet("select")) {
           TQString fn = kurl.fileName(false);
           if( !fn.isEmpty() ){
              filesToSelect += fn;
              kurl.setFileName("");
           }
       }
       kdDebug(1202) << "main() -> createBrowserWindowFromProfile servicetype=" << urlargs.serviceType << endl;
       KonqMisc::createBrowserWindowFromProfile( profilePath, profile, kurl, urlargs, false, filesToSelect );
     }
     else
     {
         if (args->count() == 0)
         {
             if (args->isSet("preload"))
             {
                 if( KonqSettings::maxPreloadCount() > 0 )
                 {
                     DCOPRef ref( "kded", "konqy_preloader" );
                     if( !ref.callExt( "registerPreloadedKonqy", DCOPRef::NoEventLoop, 5000,
                         app.dcopClient()->appId(), tqt_xscreen()))
                         return 0; // too many preloaded or failed
		     KonqMainWindow* win = new KonqMainWindow( KURL(), false ); // prepare an empty window too
		     // KonqMainWindow ctor sets always the preloaded flag to false, so create the window before this
                     KonqMainWindow::setPreloadedFlag( true );
		     KonqMainWindow::setPreloadedWindow( win );
                     kdDebug(1202) << "Konqy preloaded :" << app.dcopClient()->appId() << endl;
                 }
                 else
                 {
                     return 0; // no preloading
                 }
             }
             else if (!args->isSet("silent"))
             {
                 // By default try to open in webbrowser mode. People can use "konqueror ." to get a filemanager.
                 TQString profile = "webbrowsing";
                 TQString profilePath = locate( "data", TQString::fromLatin1("konqueror/profiles/")+profile );
                 if ( !profilePath.isEmpty() ) {
                     KonqMisc::createBrowserWindowFromProfile( profilePath, profile );
                 } else {
                     KonqMainWindow *mainWindow = new KonqMainWindow;
                     mainWindow->show();
                 }
             }
             kdDebug(1202) << "main() -> no args" << endl;
         }
         else
         {
             KURL::List urlList;
             KonqMainWindow * mainwin = 0L;
             for ( int i = 0; i < args->count(); i++ )
             {
                 // KonqMisc::konqFilteredURL doesn't cope with local files... A bit of hackery below
                 KURL url = args->url(i);
                 KURL urlToOpen;
                 TQStringList filesToSelect;

                 if (url.isLocalFile() && TQFile::exists(url.path())) // "konqueror index.html"
                     urlToOpen = url;
                 else
                     urlToOpen = KURL( KonqMisc::konqFilteredURL(0L, TQString::fromLocal8Bit(args->arg(i))) ); // "konqueror slashdot.org"

                 if ( !mainwin ) {
                     KParts::URLArgs urlargs;
                     if (args->isSet("mimetype"))
                     {
                         urlargs.serviceType = TQString::fromLocal8Bit(args->getOption("mimetype"));
                         kdDebug(1202) << "main() : setting serviceType to " << urlargs.serviceType << endl;
                     }
                     if (args->isSet("select"))
                     {
                        TQString fn = urlToOpen.fileName(false);
                        if( !fn.isEmpty() ){
                           filesToSelect += fn;
                           urlToOpen.setFileName("");
                        }
                     }
                     const bool tempFile = TDECmdLineArgs::isTempFileSet();
                     mainwin = KonqMisc::createNewWindow( urlToOpen, urlargs, false, filesToSelect, tempFile );
                 } else
                     urlList += urlToOpen;
             }
             if ( mainwin )
                 mainwin->openMultiURL( urlList );
         }
     }
  }
  args->clear();

  app.exec();

  // Delete all KonqMainWindows, so that we don't have
  // any parts loaded when KLibLoader::cleanUp is called.
  // Their deletion was postponed in their event()
  // (and Qt doesn't delete WDestructiveClose widgets on exit anyway :(  )
  while( KonqMainWindow::mainWindowList() != NULL )
  { // the list will be deleted by last KonqMainWindow
      delete KonqMainWindow::mainWindowList()->first();
  }

  delete kiface;

  crashlog_file.unlink();

  return 0;
}
