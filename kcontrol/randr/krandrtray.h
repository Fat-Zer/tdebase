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

#include <qptrlist.h>

#include <ksystemtray.h>
#include <kglobalaccel.h>

#include <libkrandr/libkrandr.h>

class KHelpMenu;
class KPopupMenu;

class KRandRSystemTray :  public KSystemTray, public KRandrSimpleAPI
{
	Q_OBJECT

public:
	KRandRSystemTray(QWidget* parent = 0, const char *name = 0);
	KGlobalAccel *globalKeys;

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
	void slotColorConfig();
	void slotSKeys();
	void slotSettingsChanged(int category);
	void slotCycleDisplays();
	void slotOutputChanged(int parameter);
	void slotColorProfileChanged(int parameter);

protected:
	void mousePressEvent( QMouseEvent *e );

private:
	void populateMenu(KPopupMenu* menu);
	void addOutputMenu(KPopupMenu* menu);
	int GetDefaultResolutionParameter();
	int GetHackResolutionParameter();
	void findPrimaryDisplay();

	bool m_popupUp;
	KHelpMenu* m_help;
	QPtrList<KPopupMenu> m_screenPopups;

	Display *randr_display;
	ScreenInfo *randr_screen_info;
	QWidget* my_parent;

	int last_known_x;
	int last_known_y;

	KPopupMenu* m_menu;
	KSimpleConfig *t_config;
};

#endif
