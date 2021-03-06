/*

  This is a standalone application that executes Netscape plugins.


  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
                     Stefan Schimanski <1Stein@gmx.de>

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

#include "nsplugin.h"

#include <dcopclient.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tqptrlist.h>
#include <tqsocketnotifier.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef Bool
#undef Bool
#endif
#include <tdeconfig.h>

#include "qxteventloop.h"
#include "glibevents.h"

/**
 *  Use RLIMIT_DATA on systems that don't define RLIMIT_AS,
 *  such as FreeBSD 4.
 */

#ifndef RLIMIT_AS
#define RLIMIT_AS RLIMIT_DATA
#endif

/**
 * The error handler catches all X errors, writes the error
 * message to the debug log and continues.
 *
 * This is done to prevent abortion of the plugin viewer
 * in case the plugin does some invalid X operation.
 *
 */
static int x_errhandler(Display *dpy, XErrorEvent *error)
{
  char errstr[256];
  XGetErrorText(dpy, error->error_code, errstr, 256);
  kdDebug(1430) << "Detected X Error: " << errstr << endl;
  return 1;
}

/*
 * As the plugin viewer needs to be a motif application, I give in to
 * the "old style" and keep lot's of global vars. :-)
 */

static TQCString g_dcopId;

/**
 * parseCommandLine - get command line parameters
 *
 */
void parseCommandLine(int argc, char *argv[])
{
   for (int i=0; i<argc; i++)
   {
      if (!strcmp(argv[i], "-dcopid") && (i+1 < argc))
      {
         g_dcopId = argv[i+1];
         i++;
      }
   }
}

#if TQT_VERSION < 0x030100

static XtAppContext g_appcon;
static bool g_quit = false;

void quitXt()
{
   g_quit = true;
}


/**
 * socket notifier handling
 *
 */

struct SocketNot
{
  int fd;
  TQObject *obj;
  XtInputId id;
};

TQPtrList<SocketNot> _notifiers[3];

/**
 * socketCallback - send event to the socket notifier
 *
 */
void socketCallback(void *client_data, int* /*source*/, XtInputId* /*id*/)
{
  kdDebug(1430) << "-> socketCallback( client_data=" << client_data << " )" << endl;

  TQEvent event( TQEvent::SockAct );
  SocketNot *socknot = (SocketNot *)client_data;
  kdDebug(1430) << "obj=" << (void*)socknot->obj << endl;
  TQApplication::sendEvent( socknot->obj, &event );

  kdDebug(1430) << "<- socketCallback" << endl;
}


/**
 * qt_set_socket_handler - redefined internal qt function to register sockets
 * The linker looks in the main binary first and finds this implementation before
 * the original one in Qt. I hope this works with every dynamic library loader on any OS.
 *
 */
extern bool qt_set_socket_handler( int, int, TQObject *, bool );
bool qt_set_socket_handler( int sockfd, int type, TQObject *obj, bool enable )
{
  if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
      tqWarning( "TQSocketNotifier: Internal error" );
#endif
      return FALSE;
  }

  XtPointer inpMask = 0;

  switch (type) {
  case TQSocketNotifier::Read:      inpMask = (XtPointer)XtInputReadMask; break;
  case TQSocketNotifier::Write:     inpMask = (XtPointer)XtInputWriteMask; break;
  case TQSocketNotifier::Exception: inpMask = (XtPointer)XtInputExceptMask; break;
  default: return FALSE;
  }

  if (enable) {
      SocketNot *sn = new SocketNot;
      sn->obj = obj;
      sn->fd = sockfd;

      if( _notifiers[type].isEmpty() ) {
          _notifiers[type].insert( 0, sn );
      } else {
          SocketNot *p = _notifiers[type].first();
          while ( p && p->fd > sockfd )
              p = _notifiers[type].next();

#if defined(CHECK_STATE)
          if ( p && p->fd==sockfd ) {
              static const char *t[] = { "read", "write", "exception" };
              tqWarning( "TQSocketNotifier: Multiple socket notifiers for "
                        "same socket %d and type %s", sockfd, t[type] );
          }
#endif
          if ( p )
              _notifiers[type].insert( _notifiers[type].at(), sn );
          else
              _notifiers[type].append( sn );
      }

      sn->id = XtAppAddInput( g_appcon, sockfd, inpMask, socketCallback, sn );

  } else {

      SocketNot *sn = _notifiers[type].first();
      while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
          sn = _notifiers[type].next();
      if ( !sn )				// not found
          return FALSE;

      XtRemoveInput( sn->id );
      _notifiers[type].remove();
  }

  return TRUE;
}
#endif


int main(int argc, char** argv)
{
    // nspluginviewer is a helper app, it shouldn't do session management at all
   setenv( "SESSION_MANAGER", "", 1 );

   // trap X errors
   kdDebug(1430) << "1 - XSetErrorHandler" << endl;
   XSetErrorHandler(x_errhandler);
   setvbuf( stderr, NULL, _IONBF, 0 );

   kdDebug(1430) << "2 - parseCommandLine" << endl;
   parseCommandLine(argc, argv);

   kdDebug(1430) << "3 - create QXtEventLoop" << endl;
   QXtEventLoop integrator( "nspluginviewer" );
   parseCommandLine(argc, argv);
   TDELocale::setMainCatalogue("nsplugin");

   kdDebug(1430) << "4 - create TDEApplication" << endl;
   TDEApplication app( argc,  argv, "nspluginviewer", true, true, true );
   GlibEvents glibevents;

   {
      TDEConfig cfg("kcmnspluginrc", true);
      cfg.setGroup("Misc");
      int v = KCLAMP(cfg.readNumEntry("Nice Level", 0), 0, 19);
      if (v > 0) {
         nice(v);
      }
      v = cfg.readNumEntry("Max Memory", 0);
      if (v > 0) {
         rlimit rl;
         memset(&rl, 0, sizeof(rl));
         if (0 == getrlimit(RLIMIT_AS, &rl)) {
            rl.rlim_cur = kMin(v, int(rl.rlim_max));
            setrlimit(RLIMIT_AS, &rl);
         }
      }
   }

   // initialize the dcop client
   kdDebug(1430) << "5 - app.dcopClient" << endl;
   DCOPClient *dcop = app.dcopClient();
   if (!dcop->attach())
   {
      KMessageBox::error(NULL,
                            i18n("There was an error connecting to the Desktop "
                                 "communications server. Please make sure that "
                                 "the 'dcopserver' process has been started, and "
                                 "then try again."),
                            i18n("Error Connecting to DCOP Server"));
      exit(1);
   }

   kdDebug(1430) << "6 - dcop->registerAs" << endl;
   if (g_dcopId != 0)
      g_dcopId = dcop->registerAs( g_dcopId, false );
   else
      g_dcopId = dcop->registerAs("nspluginviewer");

   dcop->setNotifications(true);

   // create dcop interface
   kdDebug(1430) << "7 - new NSPluginViewer" << endl;
   NSPluginViewer *viewer = new NSPluginViewer( "viewer", 0 );

   // start main loop
#if TQT_VERSION < 0x030100
   kdDebug(1430) << "8 - XtAppProcessEvent" << endl;
   while (!g_quit)
     XtAppProcessEvent( g_appcon, XtIMAll);
#else
   kdDebug(1430) << "8 - app.exec()" << endl;
   app.exec();
#endif

   // delete viewer
   delete viewer;
}
