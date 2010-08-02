/***************************************************************************
                          ksysinfo.h  -  description
                              -------------------
    begin                : Don Jul 11 2002
    copyright            : (C) 2002 by Carsten Wolff, Christoph Held
    email                :             wolff@kde.org, c-held@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSYSINFO_H
#define KSYSINFO_H

class TQString;
class TQFont;
class TQFontDatabase;

class KSysInfo {
public:
	KSysInfo();
	~KSysInfo();
	/* XServer - info */
	bool isXfromXFreeInc();
	bool isXfromXOrg();
	int getXRelease();
	bool getRenderSupport();
	/* font - info */
	TQFont getNormalFont();
	TQFont getSmallFont();
	TQFont getBoldFont();
	TQFont getFixedWidthFont();
	/* Hardware - info */
	int getCpuSpeed();
private:
	void initXInfo();
	void initFontFamilies();
	void initHWInfo();
private:
	/* XServer - info */
	TQString m_xvendor;
	bool m_xfree_inc;
	bool m_xorg;
	int m_xrelease;
	bool m_xrender;
	/* font - info */
	TQFontDatabase* m_fdb;
	TQString m_normal_font;
	TQString m_fixed_font;
	/* Hardware - info */
	int m_cpu_speed;
};

#endif
