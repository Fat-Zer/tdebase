/*
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _BGDIALOG_H_
#define _BGDIALOG_H_

#include <tqptrvector.h>
#include <tqmap.h>
#include <tqvaluevector.h>

#include "bgdialog_ui.h"
#include "bgrender.h"
#include "bgsettings.h"
#include "bgdefaults.h"

class BGMonitorArrangement;
class TDEStandardDirs;

class BGDialog : public BGDialog_UI
{
   Q_OBJECT
public:
   BGDialog(TQWidget* parent, TDEConfig* _config, bool _multidesktop = true);
   ~BGDialog();

   void load( bool useDefaults );
   void save();
   void defaults();

   void makeReadOnly();

   TQString quickHelp() const;

signals:
   void changed(bool);

protected:
   void initUI();
   void updateUI();
   KBackgroundRenderer * eRenderer();

   void setWallpaper(const TQString &);

   void loadWallpaperFilesList();

protected slots:
   void slotIdentifyScreens();
   void slotSelectScreen(int screen);
   void slotSelectDesk(int desk);
   void slotWallpaperTypeChanged(int i);
   void slotWallpaper(int i);
   void slotWallpaperPos(int);
   void slotWallpaperSelection();
   void slotSetupMulti();
   void slotPrimaryColor(const TQColor &color);
   void slotSecondaryColor(const TQColor &color);
   void slotPattern(int pattern);
   void slotImageDropped(const TQString &uri);
   void slotPreviewDone(int desk, int screen);
   void slotAdvanced();
   void slotGetNewStuff();
   void slotBlendMode(int mode);
   void slotBlendBalance(int value);
   void slotBlendReverse(bool b);
   void desktopResized();
   void setBlendingEnabled(bool);
   void slotCrossFadeBg(bool);

protected:
   void getEScreen();
   TDEGlobalBackgroundSettings *m_pGlobals;
   TDEStandardDirs *m_pDirs;
   bool m_multidesktop;
   bool m_useViewports;
   int m_curDesk;
   unsigned m_numDesks;
   unsigned m_numViewports;
   unsigned m_numScreens;
   int m_desk;
   int m_screen;
   int m_eDesk;
   int m_eScreen;
   TQValueVector< TQPtrVector<KBackgroundRenderer> > m_renderer; // m_renderer[desk][screen]
   TQMap<TQString,int> m_wallpaper;
   TQStringList m_patterns;
   int m_slideShowRandom; // Remembers last Slide Show setting
   int m_wallpaperPos; // Remembers last wallpaper pos

   BGMonitorArrangement * m_pMonitorArrangement;

   bool m_previewUpdates;
   bool m_copyAllDesktops;
   bool m_copyAllScreens;
};

#endif
