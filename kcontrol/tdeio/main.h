/*
 * main.cpp for lisa,reslisa,tdeio_lan and tdeio_rlan kcm module
 *
 *  Copyright (C) 2000 Alexander Neundorf <neundorf@kde.org>
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

#ifndef MYMAIN_H
#define MYMAIN_H

#include <tdecmodule.h>
#include <tdeglobal.h>

class TQTabWidget;

class LanBrowser : public TDECModule
{
   Q_OBJECT
   public:
      LanBrowser(TQWidget *parent=0);
      virtual void load();
      virtual void save();

      virtual TQString handbookDocPath() const;
      virtual TQString handbookSection() const;

   private:
      TQVBoxLayout layout;
      TQTabWidget tabs;
      TDECModule *smbPage;
      TDECModule *lisaPage;
//      TDECModule *resLisaPage;
      TDECModule *tdeioLanPage;
      int smbPageTabNumber;
      int lisaPageTabNumber;
      int tdeioLanPageTabNumber;
};
#endif

