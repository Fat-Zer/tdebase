/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdlib.h>
#include <kdebug.h>

#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdeglobal.h>

#include "tderandrapp.h"

static const char tderandrtrayVersion[] = "0.5";
static const TDECmdLineOptions options[] =
{
	{ "login", I18N_NOOP("Application is being auto-started at TDE session start"), 0L },
	TDECmdLineLastOption
};

int main(int argc, char **argv)
{
	TDEAboutData aboutData("randr", I18N_NOOP("Resize and Rotate"), tderandrtrayVersion, I18N_NOOP("Resize and Rotate System Tray App"), TDEAboutData::License_GPL, "(c) 2009,2010 Timothy Pearson", 0L, "");
	aboutData.addAuthor("Timothy Pearson",I18N_NOOP("Developer and maintainer"), "kb9vqf@pearsoncomputing.net");
	aboutData.addAuthor("Hamish Rodda",I18N_NOOP("Original developer and maintainer"), "rodda@kde.org");
	aboutData.addCredit("Lubos Lunak",I18N_NOOP("Many fixes"), "l.lunak@suse.cz");
	aboutData.setProductName("tderandr/tderandrtray");
	TDEGlobal::locale()->setMainCatalogue("tderandr");

	TDECmdLineArgs::init(argc,argv,&aboutData);
	TDECmdLineArgs::addCmdLineOptions(options);
	TDEApplication::addCmdLineOptions();

	KRandRApp app;

	return app.exec();
}
