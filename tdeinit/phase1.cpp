/***************************************************************************
 *   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include <pwd.h>
#include <signal.h>

static const char description[] = I18N_NOOP("TDE Initialization Phase 1");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	int return_code = -1;

	KAboutData about("tdeinit_phase1", I18N_NOOP("tdeinit_phase1"), version, description,
			KAboutData::License_GPL, "(C) 2012 Timothy Pearson", 0, 0, "kb9vqf@pearsoncomputing.net");
	about.addAuthor( "Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net" );
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	
	KApplication app;
	
	KConfig config("twinrc", true);
	config.setGroup( "ThirdPartyWM" );
	TQString wmToLaunch = config.readEntry("WMExecutable", "");
	TQString wmArguments = config.readEntry("WMAdditionalArguments", "");

	// Check for TWIN override environment variable
	const char * twin_env = getenv("TWIN");
	if (twin_env) {
		wmToLaunch = twin_env;
	}

	// Make sure the specified WM exists
	if (KStandardDirs::findExe(wmToLaunch) == TQString::null) {
		wmToLaunch = "";
		
	}

	// Launch the WM!
	if (wmToLaunch == "") {
		return_code = system("kwrapper ksmserver");
	}
	else {
		return_code = system((TQString("kwrapper ksmserver --windowmanager %1 --windowmanageraddargs %2").arg(wmToLaunch).arg(wmArguments)).ascii());
	}

	return return_code;
}
