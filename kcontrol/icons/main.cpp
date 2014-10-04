/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <tqlayout.h>

#include <kgenericfactory.h>
#include <tdeaboutdata.h>

#include "icons.h"
#include "iconthemes.h"
#include "main.h"

/**** DLL Interface ****/
typedef KGenericFactory<IconModule, TQWidget> IconsFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_icons, IconsFactory("kcmicons") )

/**** IconModule ****/

IconModule::IconModule(TQWidget *parent, const char *name, const TQStringList &)
  : TDECModule(IconsFactory::instance(), parent, name)
{
  TQVBoxLayout *layout = new TQVBoxLayout(this);
  tab = new TQTabWidget(this);
  layout->addWidget(tab);

  tab1 = new IconThemesConfig(this, "themes");
  tab->addTab(tab1, i18n("&Theme"));
  connect(tab1, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

  tab2 = new TDEIconConfig(this, "effects");
  tab->addTab(tab2, i18n("Ad&vanced"));
  connect(tab2, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

  TDEAboutData* about = new TDEAboutData("kcmicons", I18N_NOOP("Icons"), "3.0",
	      I18N_NOOP("Icons Control Panel Module"),
	      TDEAboutData::License_GPL,
 	      I18N_NOOP("(c) 2000-2003 Geert Jansen"), 0, 0);
  about->addAuthor("Geert Jansen", 0, "jansen@kde.org");
  about->addAuthor("Antonio Larrosa Jimenez", 0, "larrosa@kde.org");
  about->addCredit("Torsten Rahn", 0, "torsten@kde.org");
  setAboutData( about );
}


void IconModule::load()
{
  tab1->load();
  tab2->load();
}


void IconModule::save()
{
  tab1->save();
  tab2->save();
}


void IconModule::defaults()
{
  tab1->defaults();
  tab2->defaults();
}


void IconModule::moduleChanged(bool state)
{
  emit changed(state);
}

TQString IconModule::quickHelp() const
{
  return i18n("<h1>Icons</h1>"
    "This module allows you to choose the icons for your desktop.<p>"
    "To choose an icon theme, click on its name and apply your choice by pressing the \"Apply\" button below. If you do not want to apply your choice you can press the \"Reset\" button to discard your changes.</p>"
    "<p>By pressing the \"Install New Theme\" button you can install your new icon theme by writing its location in the box or browsing to the location."
    " Press the \"OK\" button to finish the installation.</p>"
    "<p>The \"Remove Theme\" button will only be activated if you select a theme that you installed using this module."
    " You are not able to remove globally installed themes here.</p>"
    "<p>You can also specify effects that should be applied to the icons.</p>");
}

TQString IconModule::handbookSection() const
{
  int index = tab->currentPageIndex();
  if (index == 0) {
    //return "icon-theme";
    return TQString::null;
  }
  else if (index == 1) {
    return "icons-use";
  }
  else {
    return TQString::null;
  }
}



#include "main.moc"
