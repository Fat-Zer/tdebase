/***************************************************************************
 *   Copyright (C) 2006 by Stephan Binner <binner@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "mykickoffsearchinterface.h"
#include "../ui/k_new_mnu.h"

MyKickoffSearchInterface::MyKickoffSearchInterface( KMenu* menu, TQObject* parent, const char* name  )
    : KickoffSearchInterface( parent, name ), _menu( menu )
{
}

bool MyKickoffSearchInterface::anotherHitMenuItemAllowed(int cat)
{
   return _menu->anotherHitMenuItemAllowed(cat);
}

void MyKickoffSearchInterface::addHitMenuItem(HitMenuItem* item)
{
   _menu->addHitMenuItem(item);
}


void MyKickoffSearchInterface::searchOver()
{
   _menu->searchOver();
}

void MyKickoffSearchInterface::initCategoryTitlesUpdate()
{
   _menu->initCategoryTitlesUpdate();
}

void MyKickoffSearchInterface::updateCategoryTitles()
{
   _menu->updateCategoryTitles();
}

#include "mykickoffsearchinterface.moc"
