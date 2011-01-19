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

#include "moduleIface.h"
#include "moduleIface.moc"

#include <kdebug.h>
#include <kconfig.h>

ModuleIface::ModuleIface(TQObject *parent, const char *name) 
	: TQObject(parent, name), DCOPObject(name) {

	_parent = TQT_TQWIDGET(parent);

}

ModuleIface::~ModuleIface() {
}

TQFont ModuleIface::getFont() {
	return _parent->font(); 
}

TQPalette ModuleIface::getPalette(){
	kdDebug(1208) << "Returned Palette" << endl;
	return _parent->palette();
}

TQString ModuleIface::getStyle() {
	KConfig config(  "kdeglobals" );
	config.setGroup( "General" );
	return config.readEntry("widgetStyle");
}

void ModuleIface::invokeHandbook() {
	emit handbookClicked();
}

void ModuleIface::invokeHelp() {
	emit helpClicked();
}

