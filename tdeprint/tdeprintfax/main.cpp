/*
 *   tdeprintfax - a interface to fax-packages
 *   Copyright (C) 2001  Michael Goffioul
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "tdeprintfax.h"

#include <tqfile.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdelocale.h>
#include <tdeapplication.h>

TQString debugFlag;
int oneShotFlag = false;

static const char description[] =
	I18N_NOOP("A small fax utility to be used with tdeprint.");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static TDECmdLineOptions options[] =
{
  { "phone ", I18N_NOOP("Phone number to fax to"), 0 },
  { "immediate", I18N_NOOP("Send fax immediately"), 0 },
  { "batch", I18N_NOOP("Exit after sending"), 0 },
  { "+[file]", I18N_NOOP("File to fax (added to the file list)"), 0 },
   TDECmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  TDEAboutData aboutData( "tdeprintfax", I18N_NOOP("TDEPrintFax"),
    "1.0", description, TDEAboutData::License_GPL,
    "(c), 2001 Michael Goffioul", 0, 0);
  aboutData.addAuthor("Michael Goffioul",0, "tdeprint@swing.be");
  TDECmdLineArgs::init( argc, argv, &aboutData );
  TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  TDEApplication::addCmdLineOptions();

  TDEApplication a;
  TDECmdLineArgs	*args = TDECmdLineArgs::parsedArgs();

  KdeprintFax	*w = new KdeprintFax;
  a.setMainWidget(w);
  w->show();
  for (int i=0;i<args->count();i++)
  	w->addURL(args->url(i));

	TQString phone = args->getOption( "phone" );
	if( !phone.isEmpty() ) {
		w->setPhone( phone );
	}

	if( args->isSet( "immediate" ) ) {
		w->sendFax( args->isSet( "batch" ) );
	}

  args->clear();
  return a.exec();
}
