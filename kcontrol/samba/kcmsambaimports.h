/*
 * kcmsambaimports.h
 *
 * Copyright (c) 2000 Alexander Neundorf <alexander.neundorf@rz.tu-ilmenau.de>
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

#ifndef kcmsambaimports_h_included
#define kcmsambaimports_h_included
 
#include <tqtimer.h>
#include <tqlistview.h>
#include <tdeconfig.h>

class ImportsView: public TQWidget
{
   Q_OBJECT
   public:
      ImportsView(TQWidget *parent, TDEConfig *config=0, const char * name=0);
      virtual ~ImportsView() {};
      void saveSettings() {};
      void loadSettings() {};
private:
   TDEConfig *configFile;
   TQListView list;
   TQTimer timer;
private slots:
   void updateList();
};

#endif // main_included
