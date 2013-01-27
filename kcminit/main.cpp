/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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

#include <config.h>

#include "main.h"

#include <unistd.h>

#include <tqfile.h>
#include <tqtimer.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <klibloader.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <tdeconfig.h>

#include <X11/Xlib.h>

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List modules that are run at startup"), 0 },
    { "+module", I18N_NOOP("Configuration module to run"), 0 },
    KCmdLineLastOption
};

static int ready[ 2 ];
static bool startup = false;

static void sendReady()
{
  if( ready[ 1 ] == -1 )
    return;
  char c = 0;
  write( ready[ 1 ], &c, 1 );
  close( ready[ 1 ] );
  ready[ 1 ] = -1;
}

static void waitForReady()
{
  char c = 1;
  close( ready[ 1 ] );
  read( ready[ 0 ], &c, 1 );
  close( ready[ 0 ] );
}

bool KCMInit::runModule(const TQString &libName, KService::Ptr service)
{
    KLibLoader *loader = KLibLoader::self();
    KLibrary *lib = loader->library(TQFile::encodeName(libName));
    if (lib) {
	// get the init_ function
	TQString factory = TQString("init_%1").arg(service->init());
	void *init = lib->symbol(factory.utf8());
	if (init) {
	    // initialize the module
	    kdDebug(1208) << "Initializing " << libName << ": " << factory << endl;
	    
	    void (*func)() = (void(*)())init;
	    func();
	    return true;
	}
	loader->unloadLibrary(TQFile::encodeName(libName));
    }
    return false;
}

void KCMInit::runModules( int phase )
{
  // look for X-TDE-Init=... entries
  for(KService::List::Iterator it = list.begin();
      it != list.end();
      ++it) {
      KService::Ptr service = (*it);
      
      TQString library = service->property("X-TDE-Init-Library", TQVariant::String).toString();
      if (library.isEmpty())
        library = service->library();
      
      if (library.isEmpty() || service->init().isEmpty())
	continue; // Skip

      // see ksmserver's README for the description of the phases
      TQVariant vphase = service->property("X-TDE-Init-Phase", TQVariant::Int );
      int libphase = 1;
      if( vphase.isValid() )
          libphase = vphase.toInt();

      if( phase != -1 && libphase != phase )
          continue;

      TQString libName = TQString("kcm_%1").arg(library);

      // try to load the library
      if (! alreadyInitialized.contains( libName.ascii() )) {
	  if (!runModule(libName, service)) {
	      libName = TQString("libkcm_%1").arg(library);
	      if (! alreadyInitialized.contains( libName.ascii() )) {
		  runModule(libName, service);
		  alreadyInitialized.append( libName.ascii() );
	      }
	  } else 
	      alreadyInitialized.append( libName.ascii() );
      }
  }
}

KCMInit::KCMInit( TDECmdLineArgs* args )
: DCOPObject( "kcminit" )
{
  TQCString arg;
  if (args->count() == 1) {
    arg = args->arg(0);
  }

  if (args->isSet("list"))
  {
    list = KService::allInitServices();
    
    for(KService::List::Iterator it = list.begin();
        it != list.end();
        ++it)
    {
      KService::Ptr service = (*it);
      if (service->library().isEmpty() || service->init().isEmpty())
	continue; // Skip
      printf("%s\n", TQFile::encodeName(service->desktopEntryName()).data());
    }
    return;
  }

  if (!arg.isEmpty()) {

    TQString module = TQFile::decodeName(arg);
    if (!module.endsWith(".desktop"))
       module += ".desktop";

    KService::Ptr serv = KService::serviceByStorageId( module );
    if ( !serv || serv->library().isEmpty() ||
	 serv->init().isEmpty()) {
      kdError(1208) << TQString(i18n("Module %1 not found!").arg(module)) << endl;
      return;
    } else
      list.append(serv);

  } else {

    // locate the desktop files
    list = KService::allInitServices();

  }

  if ( !kapp->dcopClient()->isAttached() )
    kapp->dcopClient()->attach();

  // This key has no GUI apparently
  TDEConfig config("kcmdisplayrc", true );
  config.setGroup("X11");
  bool multihead = !config.readBoolEntry( "disableMultihead", false) &&
                    (ScreenCount(tqt_xdisplay()) > 1);
  // Pass env. var to tdeinit.
  TQCString name = "TDE_MULTIHEAD";
  TQCString value = multihead ? "true" : "false";
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << name << value;
  kapp->dcopClient()->send("tdelauncher", "tdelauncher", "setLaunchEnv(TQCString,TQCString)", params);
  setenv( name, value, 1 ); // apply effect also to itself

  if( startup )
  {
     runModules( 0 );
     kapp->dcopClient()->send( "ksplash", "", "upAndRunning(TQString)",  TQString("kcminit"));
     sendReady();
     TQTimer::singleShot( 300 * 1000, tqApp, TQT_SLOT( quit())); // just in case
     tqApp->exec(); // wait for runPhase1() and runPhase2()
  }
  else
     runModules( -1 ); // all phases
}

KCMInit::~KCMInit()
{
  sendReady();
}

void KCMInit::runPhase1()
{
  runModules( 1 );
  emitDCOPSignal( "phase1Done()", TQByteArray());
}

void KCMInit::runPhase2()
{
  runModules( 2 );
  emitDCOPSignal( "phase2Done()", TQByteArray());
  tqApp->exit( 0 );
}

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
  // tdeinit waits for kcminit to finish, but during KDE startup
  // only important kcm's are started very early in the login process,
  // the rest is delayed, so fork and make parent return after the initial phase
  pipe( ready );
  if( fork() != 0 )
  {
      waitForReady();
      return 0;
  }
  close( ready[ 0 ] );

  startup = ( strcmp( argv[ 0 ], "kcminit_startup" ) == 0 ); // started from starttde?

  KLocale::setMainCatalogue("kcontrol");
  TDEAboutData aboutData( "kcminit", I18N_NOOP("KCMInit"),
	"",
	I18N_NOOP("KCMInit - runs startups initialization for Control Modules."));

  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  TDEApplication app;
  app.dcopClient()->registerAs( "kcminit", false );
  KLocale::setMainCatalogue(0);
  KCMInit kcminit( TDECmdLineArgs::parsedArgs());
  return 0;
}

#include "main.moc"
