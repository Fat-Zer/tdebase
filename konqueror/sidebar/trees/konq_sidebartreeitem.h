/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

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

#ifndef konq_treeitem_h
#define konq_treeitem_h

#include <tqlistview.h>
#include <tqstringlist.h>
#include <kurl.h>

class TQPainter;
class TQDragObject;
class TQStrList;
class KonqSidebarTree;
class KonqSidebarTreeItem;
class KonqSidebarTreeModule;
class KonqSidebarTreeTopLevelItem;

/**
 * The base class for any item in the tree.
 * Items belonging to a given module are created and managed by the module,
 * but they should all be KonqSidebarTreeItems, for the event handling in KonqSidebarTree.
 */
class KonqSidebarTreeItem : public TQListViewItem
{
public:
    // Create an item under another one
    KonqSidebarTreeItem( KonqSidebarTreeItem *parentItem, KonqSidebarTreeTopLevelItem *topLevelItem );

    void initItem( KonqSidebarTreeTopLevelItem *topLevelItem );

    virtual ~KonqSidebarTreeItem();

    // Whether the item accepts a drop consisting in those @p formats
    virtual bool acceptsDrops( const TQStrList & ) { return false; }

    // Handle a drop on this item. If you didn't want it, you shouln't
    // have return true in acceptsDrops :)
    virtual void drop( TQDropEvent * ) {}

    // Create a drag object from this item.
    virtual TQDragObject * dragObject( TQWidget * parent, bool move = false ) = 0;

    virtual void middleButtonClicked();
    virtual void rightButtonPressed() = 0;

    virtual void paste() {}
    virtual void trash() {}
    virtual void del() {}
    virtual void shred() {}
    virtual void rename() {}
    virtual void rename( const TQString& ) {}

    // The URL to open when this link is clicked
    virtual KURL externalURL() const = 0;

    // The mimetype to use when this link is clicked
    // If unknown, return TQString::null, konq will determine the mimetype itself
    virtual TQString externalMimeType() const { return TQString::null; }

    // overwrite this if you want a tooltip shown on your item
    virtual TQString toolTipText() const { return TQString::null; }

    // Called when this item is selected
    // Reimplement, and call tree()->part()->extension()->enableActions(...)
    virtual void itemSelected() = 0;

    // Basically, true for directories and toplevel items
    void setListable( bool b ) { m_bListable = b; }
    bool isListable() const { return m_bListable; }

    // Whether clicking on the item should open the "external URL" of the item
    void setClickable( bool b ) { m_bClickable = b; }
    bool isClickable() const { return m_bClickable; }

    // Whether the item is a toplevel item
    virtual bool isTopLevelItem() const { return false; }

    KonqSidebarTreeTopLevelItem * topLevelItem() const { return m_topLevelItem; }

    // returns the module associated to our toplevel item
    KonqSidebarTreeModule * module() const;

    // returns the tree inside which this item is
    KonqSidebarTree *tree() const;

    virtual TQString key( int column, bool ) const { return text( column ).lower(); }

    // List of alternative names (URLs) this entry is known under
    TQStringList alias;
protected:
    // Create an item at the toplevel - only for toplevel items -> protected
    KonqSidebarTreeItem( KonqSidebarTree *parent, KonqSidebarTreeTopLevelItem *topLevelItem );

    KonqSidebarTreeTopLevelItem *m_topLevelItem;
    bool m_bListable:1;
    bool m_bClickable:1;
};

#endif
