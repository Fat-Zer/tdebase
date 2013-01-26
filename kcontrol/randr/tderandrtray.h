/*
 * Copyright (c) 2002 Hamish Rodda <rodda@kde.org>
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

#ifndef KRANDRTRAY_H
#define KRANDRTRAY_H

#include <tqptrlist.h>

#include <ksystemtray.h>
#include <kglobalaccel.h>

#include <libtderandr/libtderandr.h>
#include <tdehardwaredevices.h>

class KHelpMenu;
class KPopupMenu;

class KRandRSystemTray :  public KSystemTray, public KRandrSimpleAPI
{
	Q_OBJECT

public:
	KRandRSystemTray(TQWidget* parent = 0, const char *name = 0);
	TDEGlobalAccel *globalKeys;

	virtual void contextMenuAboutToShow(KPopupMenu* menu);

	void configChanged();

signals:
	void screenSizeChanged(int x, int y);

protected slots:
	void slotScreenActivated();
	void slotResolutionChanged(int parameter);
	void slotOrientationChanged(int parameter);
	void slotRefreshRateChanged(int parameter);
	void slotPrefs();
	void slotDisplayConfig();
	void slotSKeys();
	void slotSettingsChanged(int category);
	void slotCycleDisplays();
	void slotOutputChanged(int parameter);
	void slotColorProfileChanged(int parameter);
	void slotDisplayProfileChanged(int parameter);

protected:
	void mousePressEvent( TQMouseEvent *e );
	void resizeEvent ( TQResizeEvent * );

private:
	void populateMenu(KPopupMenu* menu);
	void addOutputMenu(KPopupMenu* menu);
	int GetDefaultResolutionParameter();
	int GetHackResolutionParameter();
	void findPrimaryDisplay();
	void reloadDisplayConfiguration();

	bool m_popupUp;
	KHelpMenu* m_help;
	TQPtrList<KPopupMenu> m_screenPopups;

	Display *randr_display;
	ScreenInfo *randr_screen_info;
	TQWidget* my_parent;

	int last_known_x;
	int last_known_y;

	KPopupMenu* m_menu;
	KSimpleConfig *r_config;
	KSimpleConfig *t_config;

private slots:
	void _quit();
	void deviceChanged (TDEGenericDevice*);
};

#endif
