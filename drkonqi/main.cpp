/*****************************************************************
 * drkonqi - The TDE Crash Handler
 *
 * Copyright (C) 2000-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************/

#include <config.h>

#include <stdlib.h>
#include <unistd.h>

#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdelocale.h>
#include <dcopclient.h>

#include "krashconf.h"
#include "toplevel.h"

static const char version[] = "1.0";
static const char description[] = I18N_NOOP( "TDE crash handler gives the user feedback if a program crashed" );

static const TDECmdLineOptions options[] =
{
  {"signal <number>", I18N_NOOP("The signal number that was caught"), 0},
  {"appname <name>",  I18N_NOOP("Name of the program"), 0},
  {"apppath <path>",  I18N_NOOP("Path to the executable"), 0},
  {"appversion <version>", I18N_NOOP("The version of the program"), 0},
  {"bugaddress <address>", I18N_NOOP("The bug address to use"), 0},
  {"programname <name>", I18N_NOOP("Translated name of the program"), 0},
  {"pid <pid>", I18N_NOOP("The PID of the program"), 0},
  {"startupid <id>", I18N_NOOP("Startup ID of the program"), 0},
  {"tdeinit", I18N_NOOP("The program was started by tdeinit"), 0},
  {"safer", I18N_NOOP("Disable arbitrary disk access"), 0},
  TDECmdLineLastOption
};

int main( int argc, char* argv[] )
{
  // Drop privs.
  setgid(getgid());
  if (setuid(getuid()) < 0 && geteuid() != getuid())
     exit (255);

  // Make sure that DrKonqi doesn't start DrKonqi when it crashes :-]
  setenv("TDE_DEBUG", "true", 1);
  unsetenv("SESSION_MANAGER");

  TDEAboutData aboutData( "drkonqi",
                        I18N_NOOP("The TDE Crash Handler"),
                        version,
                        description,
                        TDEAboutData::License_BSD,
                        "(C) 2012, The Trinity Desktop Project");
  aboutData.addAuthor("Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net");
  aboutData.addAuthor("Hans Petter Bieker", 0, "bieker@kde.org");

  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication::disableAutoDcopRegistration();

  TDEApplication a;

  KrashConfig krashconf;

  Toplevel w(&krashconf);

  return w.exec();
}
