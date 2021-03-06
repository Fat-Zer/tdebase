/* This file is part of the KDE Project
   Copyright (c) 2005 K�vin Ottens <ervin ipsquad net>
   Copyright (c) 2006 Valentine Sinitsyn <e_val@inbox.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _MANAGERMODULE_H_
#define _MANAGERMODULE_H_

#include <tqmap.h>
#include <tdecmodule.h>

class ManagerModuleView;

class ManagerModule : public TDECModule
{
	Q_OBJECT

public:
	ManagerModule( TQWidget* parent = 0, const char* name = 0);

	void load();
	void save();
	void defaults();
	
private:
	void rememberSettings();

	ManagerModuleView *view;
	TQMap<TQObject *, int> settings;
	
private slots:
	void emitChanged();	
};

#endif
