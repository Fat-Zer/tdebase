/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003  <ravi@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>

#include "wndmain.h"

static KCmdLineOptions options[] = {
  { "managed", I18N_NOOP("Execute KSplash in MANAGED mode"),0 },
  { "test", I18N_NOOP("Run in test mode"), 0 },
  { "nofork", I18N_NOOP("Do not fork into the background"), 0 },
  { "theme <argument>", I18N_NOOP("Override theme"), "" },
  { "nodcop", I18N_NOOP("Do not attempt to start DCOP server"),0 },
  { "steps <number>", I18N_NOOP("Number of steps"), "7" },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData about(
    "ksplash",
    I18N_NOOP("KSplash"),
    VERSION,
    I18N_NOOP("Trinity splash screen"),
    KAboutData::License_GPL,
    I18N_NOOP("(c) 2001 - 2003, Flaming Sword Productions\n (c) 2003 KDE developers"),
    "http://www.kde.org");
  about.addAuthor( "Ravikiran Rajagopal", I18N_NOOP("Author and maintainer"), "ravi@ee.eng.ohio-state.edu" );
  about.addAuthor( "Brian Ledbetter", I18N_NOOP("Original author"), "brian@shadowcom.net" );

  TDECmdLineArgs::init(argc, argv, &about);
  TDECmdLineArgs::addCmdLineOptions(options);
  TDECmdLineArgs *arg = TDECmdLineArgs::parsedArgs();

  if ( !( arg->isSet( "dcop" ) ) )
    TDEApplication::disableAutoDcopRegistration();
  else if ( TDEApplication::dcopClient()->attach() )
    TDEApplication::dcopClient()->registerAs( "ksplash", false );

  TDEApplication app;

  KSplash wndMain("ksplash");
  if ( arg->isSet( "steps" ) )
  {
    int steps = QMAX( arg->getOption( "steps" ).toInt(), 0 );
    if ( steps )
      wndMain.setStartupItemCount( steps );
  }

  // The position of this fork() matters, fork too early and you risk the
  // calls to KSplash::programStarted being missed. Now that wndMain has
  // been instantiated it is safe to do this. An earlier version of
  // this program had this fork occuring before the instantiation,
  // and this led to a race condition where if ksplash lost the race it would
  // hang because it would wait for signals that had already been sent
  if( arg->isSet( "fork" ) )
  {
    if (fork())
      exit(0);
  }

  app.setMainWidget(&wndMain);
  app.setTopWidget(&wndMain);
  return(app.exec());
}
