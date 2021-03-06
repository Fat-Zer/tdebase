/*
 *  main.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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
 *
 */
#include <unistd.h>

#include <tqlabel.h>
#include <tqlayout.h>

#include <dcopclient.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "main.h"
#include "main.moc"

#include "tzone.h"
#include "dtime.h"

typedef KGenericFactory<KclockModule, TQWidget> KlockModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_clock, KlockModuleFactory("kcmkclock"))

KclockModule::KclockModule(TQWidget *parent, const char *name, const TQStringList &)
  : TDECModule(KlockModuleFactory::instance(), parent, name)
{
  TDEAboutData *about =
  new TDEAboutData(I18N_NOOP("kcmclock"), I18N_NOOP("TDE Clock Control Module"),
                  0, 0, TDEAboutData::License_GPL,
                  "(c) 1996 - 2001 Luca Montecchiani");

  about->addAuthor("Luca Montecchiani", I18N_NOOP("Original author"), "m.luca@usa.net");
  about->addAuthor("Paul Campbell", I18N_NOOP("Current Maintainer"), "paul@taniwha.com");
  about->addAuthor("Benjamin Meyer", I18N_NOOP("Added NTP support"), "ben+kcmclock@meyerhome.net");
  setAboutData( about );
  setQuickHelp( i18n("<h1>Date & Time</h1> This control module can be used to set the system date and"
    " time. As these settings do not only affect you as a user, but rather the whole system, you"
    " can only change these settings when you start the Control Center as root. If you do not have"
    " the root password, but feel the system time should be corrected, please contact your system"
    " administrator."));

  TDEGlobal::locale()->insertCatalogue("timezones"); // For time zone translations

  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

  dtime = new Dtime(this);
  layout->addWidget(dtime);
  connect(dtime, TQT_SIGNAL(timeChanged(bool)), this, TQT_SIGNAL(changed(bool)));

  tzone = new Tzone(this);
  layout->addWidget(tzone);
  connect(tzone, TQT_SIGNAL(zoneChanged(bool)), this, TQT_SIGNAL(changed(bool)));

  layout->addStretch();

  if(getuid() == 0)
    setButtons(Help|Apply);
  else
    setButtons(Help);
}

void KclockModule::save()
{
//  The order here is important
#ifdef __OpenBSD__
  tzone->save();
  dtime->save();
#else
  dtime->save();
  tzone->save();
#endif

  // Tell the clock applet about the change so that it can update its timezone
  kapp->dcopClient()->send( "kicker", "ClockApplet", "reconfigure()", TQByteArray() );
}

void KclockModule::load()
{
  dtime->load();
  tzone->load();
}

