/*
  Copyright (c) 2001 Daniel Molkentin <molkentin@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
*/

#ifndef __moduleIface_h__
#define __moduleIface_h__

#include <dcopobject.h> 

#include <tqfont.h>
#include <tqpalette.h>
#include <tqwidget.h>

class ModuleIface : public TQObject, public DCOPObject {

Q_OBJECT
K_DCOP

public:
	ModuleIface(TQObject *parent, const char *name);
	~ModuleIface();

k_dcop:
	TQFont getFont();
	TQPalette getPalette();
	TQString getStyle();
	void invokeHandbook();
	void invokeHelp();

signals:
	void handbookClicked();
	void helpClicked();

private:
	TQWidget *_parent;

};

#endif
