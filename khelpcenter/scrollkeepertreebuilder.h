/*
 *  This file is part of the TDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
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
#ifndef KHC_SCROLLKEEPERTREEBUILDER_H
#define KHC_SCROLLKEEPERTREEBUILDER_H

#include <tqobject.h>
#include <tqptrlist.h>

#include "navigatoritem.h"

class KProcIO;

class TQDomNode;

namespace KHC {

class ScrollKeeperTreeBuilder : public TQObject
{
  Q_OBJECT
  public:
    ScrollKeeperTreeBuilder( TQObject *parent, const char *name = 0 );

    NavigatorItem *build( NavigatorItem *parent, NavigatorItem *after );

  private slots:
    void getContentsList( KProcIO *proc );

  private:
    void loadConfig();
    int insertSection( NavigatorItem *parent, NavigatorItem *after,
                       const TQDomNode &sectNode, NavigatorItem *&created );
    void insertDoc( NavigatorItem *parent, const TQDomNode &docNode );

    bool mShowEmptyDirs;
    TQString mContentsList;
    TQPtrList<NavigatorItem> mItems;
};

}

#endif // KHC_SCROLLKEEPERTREEBUILDER_H
// vim:ts=2:sw=2:et
