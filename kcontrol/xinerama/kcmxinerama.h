/**
 * kcmxinerama.h
 *
 * Copyright (c) 2002-2004 George Staikos <staikos@kde.org>
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

#ifndef _KCM_XINERAMA_H
#define _KCM_XINERAMA_H

#include <tdecmodule.h>
#include <tqptrlist.h>
#include <tqtimer.h>

#include "xineramawidget.h"

class TDEConfig;
class TQWidget;

class KCMXinerama : public TDECModule {
	Q_OBJECT
public:
	KCMXinerama(TQWidget *parent = 0L, const char *name = 0L);
	virtual ~KCMXinerama();

	void load();
	void load(bool useDefaults );
	void save();
	void defaults();

public slots:
	void windowIndicator(int dpy);
	void clearIndicator();

	void indicateWindows();

private:
	TQWidget* indicator(int dpy);
	TDEConfig *config, *ksplashrc;
	XineramaWidget *xw;
	TQTimer _timer;
	TQPtrList<TQWidget> _indicators;

	int _displays;

};

#endif

