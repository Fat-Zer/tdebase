/*
 *  navigatoritem.h - part of the KDE Help Center
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
#ifndef KHC_NAVIGATORITEM_H
#define KHC_NAVIGATORITEM_H

#include <tqlistview.h>

namespace KHC {

class TOC;
class DocEntry;

class NavigatorItem : public QListViewItem
{
  public:
    NavigatorItem( DocEntry *entry, TQListView *parent );
    NavigatorItem( DocEntry *entry, TQListViewItem *parent );

    NavigatorItem( DocEntry *entry, TQListView *parent,
                   TQListViewItem *after );
    NavigatorItem( DocEntry *entry, TQListViewItem *parent,
                   TQListViewItem *after );

    ~NavigatorItem();

    DocEntry *entry() const;

    void setAutoDeleteDocEntry( bool );

    void updateItem();

    TOC *toc() const { return mToc; }

    TOC *createTOC();
  
    void setOpen( bool open );

  private:
    void init( DocEntry * );
    
    TOC *mToc;

    DocEntry *mEntry;
    bool mAutoDeleteDocEntry;
};

}

#endif

// vim:ts=2:sw=2:et
