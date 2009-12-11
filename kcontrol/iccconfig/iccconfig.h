/**
 * iccconfig.h
 *
 * Copyright (c) 2009 Timothy Pearson <kb9vqf@pearsoncomputing.net>
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

#ifndef _KCM_ICCCONFIG_H
#define _KCM_ICCCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopobject.h>

#include <kcmodule.h>

#include "iccconfigbase.h"

class KConfig;
class KPopupMenu;
class KListViewItem;

class KICCConfig : public KCModule, public DCOPObject
{
  K_DCOP
    Q_OBJECT


public:
  //KICCConfig(QWidget *parent = 0L, const char *name = 0L);
  KICCConfig(QWidget *parent, const char *name, const QStringList &);
  virtual ~KICCConfig();

  ICCConfigBase *base;

  void load();
  void load( bool useDefaults);
  void save();
  void defaults();

  int buttons();
  QString quickHelp() const;

 k_dcop:

private:

  KConfig *config;
  bool _ok;
  KPopupMenu * _popUpKardChooser;


};

#endif

