/*
 *
 * Copyright (c) 2005 Sandro Giessl <sandro@giessl.com>
 * Copyright (c) 2005 Luciano Montanaro <mikelima@cirulla.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <tqtimer.h>

#include <kdebug.h>
#include <tdeconfig.h>
#include <kdecoration_plugins_p.h>
#include <kdecorationfactory.h>

#include <time.h>
#include <sys/timeb.h>
#include <iostream>


#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>

#include "preview.h"
#include "main.h"

static TDECmdLineOptions options[] =
{
	{ "+decoration", "Decoration library to use, such as twin3_plastik.", 0 },
	{ "+tests", "Which test should be executed ('all', 'repaint', 'caption', 'resize', 'recreation')", 0 },
	{ "+repetitions", "Number of test repetitions.", 0 },
	{ 0, 0, 0 }
};

DecoBenchApplication::DecoBenchApplication(const TQString &library, Tests tests, int count) :
		m_tests(tests),
		m_count(count)
{
	TDEConfig twinConfig("twinrc");
	twinConfig.setGroup("Style");

	plugins = new KDecorationPreviewPlugins( &twinConfig );
	preview = new KDecorationPreview( plugins, 0 );

	if (plugins->loadPlugin(library) )
		kdDebug() << "Decoration library " << library << " loaded..." << endl;
	else
		kdError() << "Error loading decoration library " << library << "!" << endl;

	if (preview->recreateDecoration() )
		kdDebug() << "Decoration created..." << endl;
	else
		kdError() << "Error creating decoration!" << endl;

	preview->show();
}

DecoBenchApplication::~DecoBenchApplication()
{
	delete preview;
	delete plugins;
}

void DecoBenchApplication::executeTest()
{
	clock_t stime = clock();
	timeb astart, aend;
	ftime(&astart);

	if (m_tests == AllTests || m_tests == RepaintTest)
		preview->performRepaintTest(m_count);
	if (m_tests == AllTests || m_tests == CaptionTest)
		preview->performCaptionTest(m_count);
	if (m_tests == AllTests || m_tests == ResizeTest)
		preview->performResizeTest(m_count);
	if (m_tests == AllTests || m_tests == RecreationTest)
		preview->performRecreationTest(m_count);

	clock_t etime = clock();
	ftime(&aend);

	long long time_diff = (aend.time - astart.time)*1000+aend.millitm - astart.millitm;
	kdDebug() << "Total:" << (float(time_diff)/1000) << endl;
	quit();
}

int main(int argc, char** argv)
{
	TQString style = "keramik";
	// TDEApplication app(argc, argv);
	TDEAboutData about("decobenchmark", "DecoBenchmark", "0.1", "twin decoration performance tester...", TDEAboutData::License_LGPL, "(C) 2005 Sandro Giessl");
	TDECmdLineArgs::init(argc, argv, &about);
	TDECmdLineArgs::addCmdLineOptions( options );

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	if (args->count() != 3)
		TDECmdLineArgs::usage("Wrong number of arguments!");

	TQString library = TQString(args->arg(0) );
	TQString t = TQString(args->arg(1) );
	int count = TQString(args->arg(2) ).toInt();

	Tests test;
	if (t == "all")
		test = AllTests;
	else if (t == "repaint")
		test = RepaintTest;
	else if (t == "caption")
		test = CaptionTest;
	else if (t == "resize")
		test = ResizeTest;
	else if (t == "recreation")
		test = RecreationTest;
	else
		TDECmdLineArgs::usage("Specify a valid test!");

	DecoBenchApplication app(library, test, count);

	TQTimer::singleShot(0, &app, TQT_SLOT(executeTest()));
	app.exec();
}
#include "main.moc"

// kate: space-indent off; tab-width 4;
