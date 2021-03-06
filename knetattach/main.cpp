/*
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdelocale.h>

#include "knetattach.h"

int main(int argc, char **argv) {
	TDEAboutData about("knetattach", I18N_NOOP("TDE Network Wizard"), "1.0",
		I18N_NOOP("TDE Network Wizard"),
		TDEAboutData::License_GPL,
		I18N_NOOP("(c) 2004 George Staikos"), 0,
		"http://www.kde.org/");

	about.addAuthor("George Staikos", I18N_NOOP("Primary author and maintainer"), "staikos@kde.org");

	TDECmdLineArgs::init(argc, argv, &about);
	TDEApplication a;

	KNetAttach na;
	a.setMainWidget(&na);
	na.show();

	return a.exec();
}

