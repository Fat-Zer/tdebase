/*
 * Keramik KWin client configuration module
 *
 * Copyright (C) 2002 Fredrik H�glund <fredrik@kde.org>
 *
 * Based on the Quartz configuration module,
 *     Copyright (c) 2001 Karol Szwed <gallium@kde.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KWIN_KERAMIK_CONFIG_H
#define __KWIN_KERAMIK_CONFIG_H

#include <tdeconfig.h>

#include "keramitdeconfig.h"

class KeramikConfig: public TQObject
{
	Q_OBJECT
	

	public:
		KeramikConfig( TDEConfig* conf, TQWidget* parent );
		~KeramikConfig();

	// These public signals/slots work similar to KCM modules
	signals:
		void changed();

	public slots:
		void load( TDEConfig* conf );	
		void save( TDEConfig* conf );
		void defaults();

	private:
		KeramikConfigUI *ui;
		TDEConfig *c;
};


#endif

// vim: set noet ts=4 sw=4:
