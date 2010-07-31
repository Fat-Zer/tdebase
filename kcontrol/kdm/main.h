/*
 * main.h
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

#ifndef __kdm_main_h
#define __kdm_main_h

#include <tqtabwidget.h>
#include <tqmap.h>

#include <kcmodule.h>

class KDMAppearanceWidget;
class KDMFontWidget;
class KDMSessionsWidget;
class KDMUsersWidget;
class KDMConvenienceWidget;
class KBackground;

class KDModule : public KCModule
{
  Q_OBJECT

public:

  KDModule(TQWidget *parent, const char *name, const TQStringList &);
  ~KDModule();

  void load();
  void save();
  void defaults();

public slots:

  void slotMinMaxUID(int min, int max);

signals:

  void clearUsers();
  void addUsers(const TQMap<TQString,int> &);
  void delUsers(const TQMap<TQString,int> &);

private:

  QTabWidget		*tab;

  KDMAppearanceWidget	*appearance;
  KBackground		*background;
  KDMFontWidget		*font;
  KDMSessionsWidget	*sessions;
  KDMUsersWidget	*users;
  KDMConvenienceWidget	*convenience;

  TQMap<TQString, QPair<int,TQStringList> >	usermap;
  TQMap<TQString,int>	groupmap;
  int			minshowuid, maxshowuid;
  bool			updateOK;

  void propagateUsers();

};

#endif

