/*
    kwrited is a write(1) receiver for KDE.
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef KWRITED_H
#define KWRITED_H

#include <tqtextedit.h>
#include <kdedmodule.h>
#include <tqpopupmenu.h>
#include <tqtextedit.h>

class KPty;

class KWrited : public TQTextEdit
{ Q_OBJECT
public:
  KWrited();
 ~KWrited();
protected:
  virtual TQPopupMenu *createPopupMenu(const TQPoint &);
private slots:
  void block_in(int fd);
  void clearText(void);
private:
  KPty* pty;
};

class KWritedModule : public KDEDModule
{
  Q_OBJECT
  K_DCOP
public:
  KWritedModule( const TQCString& obj );
 ~KWritedModule();
private:
  KWrited* pro;
};

#endif
