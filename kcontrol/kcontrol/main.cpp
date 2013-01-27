/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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


/**
 * Howto debug:
 *    start "kcontrol --nofork" in a debugger.
 *
 * If you want to test with command line arguments you need
 * -after you have started kcontrol in the debugger-
 * open another shell and run kcontrol with the desired
 * command line arguments.
 *
 * The command line arguments will be passed to the version of
 * kcontrol in the debugger via DCOP and will cause a call
 * to newInstance().
 */

#include <tqpaintdevicemetrics.h>

#include <kcmdlineargs.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kglobalsettings.h>
#include <tdeconfig.h>
#include <kdebug.h>

#include "main.h"
#include "main.moc"
#include "toplevel.h"
#include "global.h"
#include "moduleIface.h"

#include "version.h"

KControlApp::KControlApp()
  : KUniqueApplication()
  , toplevel(0)
{
  toplevel = new TopLevel();

  setMainWidget(toplevel);
  TDEGlobal::setActiveInstance(this);

  // KUniqueApplication does dcop regitration for us
  ModuleIface *modIface = new ModuleIface(TQT_TQOBJECT(toplevel), "moduleIface");

  connect (modIface, TQT_SIGNAL(helpClicked()), toplevel, TQT_SLOT(slotHelpRequest()));
  connect (modIface, TQT_SIGNAL(handbookClicked()), toplevel, TQT_SLOT(slotHandbookRequest()));

  TQRect desk = TDEGlobalSettings::desktopGeometry(toplevel);
  TDEConfig *config = TDEGlobal::config();
  config->setGroup("General");
  // Initial size is:
  // never bigger than workspace as reported by desk
  // 940x700 on 96 dpi, 12 pt font
  // 800x600 on 72 dpi, 12 pt font
  // --> 368 + 6 x dpiX, 312 + 4 x dpiY
  // Adjusted for font size
  TQPaintDeviceMetrics pdm(toplevel);
  int fontSize = toplevel->fontInfo().pointSize();
  if (fontSize == 0)
    fontSize = (toplevel->fontInfo().pixelSize() * 72) / pdm.logicalDpiX();
  int x = config->readNumEntry(TQString::fromLatin1("InitialWidth %1").arg(desk.width()), 
			       QMIN( desk.width(), 368 + (6*pdm.logicalDpiX()*fontSize)/12 ) );
  int y = config->readNumEntry(TQString::fromLatin1("InitialHeight %1").arg(desk.height()), 
			       QMIN( desk.height(), 312 + (4*pdm.logicalDpiX()*fontSize)/12 ) );
  toplevel->resize(x,y);
}

KControlApp::~KControlApp()
{
  if (toplevel)
    {
      TDEConfig *config = TDEGlobal::config();
      config->setGroup("General");
      TQWidget *desk = TQT_TQWIDGET(TQApplication::desktop());
      config->writeEntry(TQString::fromLatin1("InitialWidth %1").arg(desk->width()), toplevel->width());
      config->writeEntry(TQString::fromLatin1("InitialHeight %1").arg(desk->height()), toplevel->height());
      config->sync();
    }
  delete toplevel;
}

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
  KLocale::setMainCatalogue("kcontrol");
  TDEAboutData aboutKControl( "kcontrol", I18N_NOOP("Trinity Control Center"),
    KCONTROL_VERSION, I18N_NOOP("The Trinity Control Center"), TDEAboutData::License_GPL,
    I18N_NOOP("(c) 1998-2004, The TDE Control Center Developers"));

  TDEAboutData aboutKInfoCenter( "kinfocenter", I18N_NOOP("Trinity Info Center"),
    KCONTROL_VERSION, I18N_NOOP("The Trinity Info Center"), TDEAboutData::License_GPL,
    I18N_NOOP("(c) 1998-2004, The TDE Control Center Developers"));

  TQCString argv_0 = argv[0];
  TDEAboutData *aboutData;
  if (argv_0.right(11) == "kinfocenter")
  {
     aboutData = &aboutKInfoCenter;
     KCGlobal::setIsInfoCenter(true);
     kdDebug(1208) << "Running as KInfoCenter!\n" << endl;
  }
  else
  {
     aboutData = &aboutKControl;
     KCGlobal::setIsInfoCenter(false);
  }

  aboutData->addAuthor("Timothy Pearson", I18N_NOOP("Current Maintainer"), "kb9vqf@pearsoncomputing.net");
  if (argv_0.right(11) == "kinfocenter")
    aboutData->addAuthor("Helge Deller", I18N_NOOP("Previous Maintainer"), "deller@kde.org");
  else
    aboutData->addAuthor("Daniel Molkentin", I18N_NOOP("Previous Maintainer"), "molkentin@kde.org");

  aboutData->addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
  aboutData->addAuthor("Matthias Elter",0, "elter@kde.org");
  aboutData->addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
  aboutData->addAuthor("Waldo Bastian",0, "bastian@kde.org");

  TDECmdLineArgs::init( argc, argv, aboutData );
  KUniqueApplication::addCmdLineOptions();

  KCGlobal::init();

  if (!KControlApp::start()) {
	kdDebug(1208) << "kcontrol is already running!\n" << endl;
	return (0);
  }

  KControlApp app;

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}
