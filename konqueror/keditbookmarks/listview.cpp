// -*- indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "listview.h"

#include "toplevel.h"
#include "bookmarkinfo.h"
#include "commands.h"
#include "testlink.h"
#include "settings.h"

#include <stdlib.h>

#include <tqclipboard.h>
#include <tqpopupmenu.h>
#include <tqpainter.h>
#include <tqheader.h>


#include <tdelocale.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <tdeapplication.h>

#include <tdeaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <tdefiledialog.h>
#include <kkeydialog.h>
#include <tdemessagebox.h>
#include <klineedit.h>
#include <krun.h>
#include <tdelistviewsearchline.h>

#include <kbookmarkdrag.h>
#include <kbookmarkmanager.h>

// #define DEBUG_ADDRESSES

ListView* ListView::s_self = 0;

int ListView::s_myrenamecolumn = -1;
KEBListViewItem *ListView::s_myrenameitem = 0;

TQStringList ListView::s_selected_addresses;
TQString ListView::s_current_address;

ListView::ListView()
    : m_needToFixUp(false)
{
}

ListView::~ListView() {
    self()->m_listView->saveColumnSetting();
}

void ListView::createListViews(TQSplitter *splitter) {
    s_self = new ListView();
    self()->m_listView = new KEBListView(splitter, false);
    splitter->setSizes(TQValueList<int>() << 100 << 300);
}

void ListView::initListViews() {
    self()->m_listView->init();
}

void ListView::updateListViewSetup(bool readonly) {
    self()->m_listView->readonlyFlagInit(readonly);
}

void ListView::connectSignals() {
    m_listView->makeConnections();
}

bool lessAddress(TQString a, TQString b)
{
    if(a == b)
         return false;

    TQString error("ERROR");
    if(a == error)
        return false;
    if(b == error)
        return true;

    a += "/";
    b += "/";

    uint aLast = 0;
    uint bLast = 0;
    uint aEnd = a.length();
    uint bEnd = b.length();
    // Each iteration checks one "/"-delimeted part of the address
    // "" is treated correctly
    while(true)
    {
        // Invariant: a[0 ... aLast] == b[0 ... bLast]
        if(aLast + 1 == aEnd) //The last position was the last slash
            return true; // That means a is shorter than b
        if(bLast +1 == bEnd)
            return false;

        uint aNext = a.find("/", aLast + 1);
        uint bNext = b.find("/", bLast + 1);

        bool okay;
        uint aNum = a.mid(aLast + 1, aNext - aLast - 1).toUInt(&okay);
        if(!okay)
            return false;
        uint bNum = b.mid(bLast + 1, bNext - bLast - 1).toUInt(&okay);
        if(!okay)
            return true;

        if(aNum != bNum)
            return aNum < bNum;

        aLast = aNext;
        bLast = bNext;
    }
}

bool operator<(const KBookmark & first, const KBookmark & second) //FIXME Using internal represantation
{
    return lessAddress(first.address(), second.address());
}



TQValueList<KBookmark> ListView::itemsToBookmarks(const TQValueVector<KEBListViewItem *> & items) const
{
    TQValueList<KBookmark> bookmarks; //TODO optimize by using a QValueVector
    TQValueVector<KEBListViewItem *>::const_iterator it = items.constBegin();
    TQValueVector<KEBListViewItem *>::const_iterator end = items.constEnd();
    for( ; it!=end; ++it)
    {
        if(*it != m_listView->rootItem() )
            bookmarks.push_back( (*it)->bookmark() );
    }
    qHeapSort(bookmarks);
    return bookmarks;
}

void ListView::invalidate(const TQString & address)
{
    invalidate(getItemAtAddress(address));
}

void ListView::invalidate(TQListViewItem * item)
{
    if(item->isSelected())
    {
        m_listView->setSelected(item, false);
        m_needToFixUp = true;
    }

    if(m_listView->currentItem() == item)
    {
        // later overiden by fixUpCurrent
        m_listView->setCurrentItem(m_listView->rootItem()); 
        m_needToFixUp = true;
    }

    TQListViewItem * child = item->firstChild();
    while(child)
    {
        //invalidate(child);
        child = child->nextSibling();
    }
}

void ListView::fixUpCurrent(const TQString & address)
{
    if(!m_needToFixUp)
        return;
    m_needToFixUp = false;

    TQListViewItem * item;
    if(mSelectedItems.count() != 0)
    {
        TQString least = mSelectedItems.begin().key()->bookmark().address();
        TQMap<KEBListViewItem *, bool>::iterator it, end;
        end = mSelectedItems.end();
        for(it = mSelectedItems.begin(); it != end; ++it)
            if( lessAddress(it.key()->bookmark().address(), least))
                least = it.key()->bookmark().address();
        item = getItemAtAddress(least);
    }
    else
        item  = getItemAtAddress(address);
    m_listView->setSelected( item, true );
    m_listView->setCurrentItem( item );
}


void ListView::selected(KEBListViewItem * item, bool s)
{
    Q_ASSERT(item->bookmark().hasParent() || item == m_listView->rootItem());
    TQMap<KEBListViewItem *, bool>::iterator it;

    if(s)
        mSelectedItems[item] = item;
    else
        if((it = mSelectedItems.find(item)) != mSelectedItems.end())
            mSelectedItems.remove(it);

    KEBApp::self()->updateActions();

    const TQValueVector<KEBListViewItem *> & selected = selectedItemsMap();
    if (selected.count() != 1)
    {
        KEBApp::self()->bkInfo()->showBookmark(KBookmark());
        return;
    }
    //FIXME do it once somewhere
    if (!KEBApp::self()->bkInfo()->connected()) {
        connect(KEBApp::self()->bkInfo(), TQT_SIGNAL( updateListViewItem() ),
                                          TQT_SLOT( slotBkInfoUpdateListViewItem() ));
        KEBApp::self()->bkInfo()->setConnected(true);
    }

    KEBApp::self()->bkInfo()->showBookmark((*(selected.constBegin()))->bookmark());
    firstSelected()->modUpdate();
}

TQValueVector<KEBListViewItem *> ListView::selectedItemsMap() const
{
    TQValueVector<KEBListViewItem *> selected;
    TQMap<KEBListViewItem *, bool>::ConstIterator it, end;
    end = mSelectedItems.constEnd();
    for(it = mSelectedItems.constBegin(); it != end; ++it)
    {
        if( it.key()->isVisible())
            selected.push_back(it.key());
    }
    return selected;
}

KEBListViewItem * ListView::firstSelected() const
{
    if(mSelectedItems.isEmpty())
        return 0L;

    TQValueVector<KEBListViewItem *> selected = selectedItemsMap(); 
    if(selected.isEmpty())
        return 0L;
    else
        return *(selected.constBegin());
}

void ListView::deselectAllChildren(KEBListViewItem *item) 
{
    KEBListViewItem* child = static_cast<KEBListViewItem *>(item->firstChild());
    while(child)
    {
        if (child)
        {
            if(child->isSelected())
                child->listView()->setSelected(child, false); //calls deselectAllChildren
            else
                deselectAllChildren(child);
        }
        child->repaint();
        child = static_cast<KEBListViewItem *>(child->nextSibling());
    }
}

TQValueList<TQString> ListView::selectedAddresses()
{
    TQValueList<TQString> addresses;
    TQValueList<KBookmark> bookmarks = itemsToBookmarks( selectedItemsMap() );
    TQValueList<KBookmark>::const_iterator it, end;
    end = bookmarks.end();
    for( it = bookmarks.begin(); it != end; ++it)
        addresses.append( (*it).address() );
    return addresses;
}


TQValueList<KBookmark> ListView::selectedBookmarksExpanded() const {
    TQValueList<KBookmark> bookmarks;
    for (TQListViewItemIterator it(m_listView); it.current() != 0; ++it) {
        if (!it.current()->isSelected())
            continue;
        if(it.current() == m_listView->firstChild()) // root case
            continue;
        if(!it.current()->isVisible()) // skip over filtered bookmarks
            continue;
        if (it.current()->childCount() == 0) // non folder case
            bookmarks.append(static_cast<KEBListViewItem *>(it.current())->bookmark());
        else
            selectedBookmarksExpandedHelper(static_cast<KEBListViewItem *>(it.current()), bookmarks);
    }
    return bookmarks;
}


void ListView::selectedBookmarksExpandedHelper(KEBListViewItem * item, TQValueList<KBookmark> & bookmarks) const
{
    KEBListViewItem* child = static_cast<KEBListViewItem *>(item->firstChild());
    while( child )
    {
        if(child->isVisible())
        {
            if (!child->isEmptyFolderPadder() && (child->childCount() == 0))
                bookmarks.append(child->bookmark());
            if(child->childCount())
                selectedBookmarksExpandedHelper(child, bookmarks);
        }
        child = static_cast<KEBListViewItem *>(child->nextSibling());
    }
}

TQValueList<KBookmark> ListView::allBookmarks() const {
    TQValueList<KBookmark> bookmarks;
    for (TQListViewItemIterator it(m_listView); it.current() != 0; ++it)
    {
        KEBListViewItem * item = static_cast<KEBListViewItem *>(it.current());
        if (!item->isEmptyFolderPadder() && (item->childCount() == 0))
            bookmarks.append(static_cast<KEBListViewItem *>(it.current())->bookmark());
    }
    return bookmarks;
}

// DESIGN - make + "/0" a kbookmark:: thing?

TQString ListView::userAddress() const 
{
    KBookmark current = firstSelected()->bookmark();
    return (current.isGroup()) 
        ? (current.address() + "/0") //FIXME internal represantation used
        : KBookmark::nextAddress(current.address());
}

void ListView::setCurrent(KEBListViewItem *item, bool select) {
    m_listView->setCurrentItem(item);
    if(select)
    {
        m_listView->clearSelection();
        m_listView->setSelected(item, true);
    }
}

KEBListViewItem* ListView::getItemAtAddress(const TQString &address) const {
    //FIXME uses internal represantation of bookmark address
    TQListViewItem *item = m_listView->rootItem();

    TQStringList addresses = TQStringList::split('/',address); // e.g /5/10/2

    for (TQStringList::Iterator it = addresses.begin(); it != addresses.end(); ++it) {
        if (item = item->firstChild(), !item)
            return 0;
        for (unsigned int i = 0; i < (*it).toUInt(); ++i)
            if (item = item->nextSibling(), !item)
                return 0;
    }
    return static_cast<KEBListViewItem *>(item);
}

void ListView::setOpen(bool open) {
    for (TQListViewItemIterator it(m_listView); it.current() != 0; ++it)
        if (it.current()->parent())
            it.current()->setOpen(open);
}

SelcAbilities ListView::getSelectionAbilities() const {
    SelcAbilities sa = { false, false, false, false, false, false, false, false, false };

    if (mSelectedItems.count() > 0) 
    {
        TQValueVector<KEBListViewItem *> selected = selectedItemsMap();
        if(!selected.isEmpty())
        {
            //Optimize
            KBookmark nbk = (*(selected.constBegin()))->bookmark();
            sa.itemSelected   = true;
            sa.group          = nbk.isGroup();
            sa.separator      = nbk.isSeparator();
            sa.urlIsEmpty     = nbk.url().isEmpty();
            sa.root           = (*(selected.constBegin()) == m_listView->rootItem());
            sa.multiSelect    = (selected.count() > 1);
            sa.singleSelect   = (!sa.multiSelect && sa.itemSelected);
            sa.tbShowState    = CmdGen::shownInToolbar(nbk);
        }
    }

    sa.notEmpty = (m_listView->rootItem()->childCount() > 0);

    return sa;
}

void ListView::handleDropped(KEBListView *, TQDropEvent *e, TQListViewItem *newParent, TQListViewItem *itemAfterQLVI) {
    bool inApp = (e->source() == m_listView->viewport());

    // drop before root item
    if (!newParent)
        return;

    KEBListViewItem *itemAfter = static_cast<KEBListViewItem *>(itemAfterQLVI);

    TQString newAddress 
        = (!itemAfter || itemAfter->isEmptyFolderPadder())
        ? (static_cast<KEBListViewItem *>(newParent)->bookmark().address() + "/0")
        : (KBookmark::nextAddress(itemAfter->bookmark().address()));

    KEBMacroCommand *mcmd = 0;

    if (!inApp) {
        mcmd = CmdGen::insertMimeSource(i18n("Drop Items"), e, newAddress);

    } else {
        const TQValueVector<KEBListViewItem *> & selected = selectedItemsMap();
        if (!(selected.count() > 0) || (*(selected.constBegin()) == itemAfterQLVI))
            return;
        bool copy = (e->action() == TQDropEvent::Copy);
        mcmd = CmdGen::itemsMoved(selected, newAddress, copy);
    }

    CmdHistory::self()->didCommand(mcmd);
}

void ListView::updateStatus(TQString url) {
    m_listView->updateByURL(url);
}

void ListView::updateListView() 
{
    // this is upper border of the visible are
    int lastCurrentY = m_listView->contentsY();

    //Save selected items (restored in fillWithGroup)
    s_selected_addresses.clear();
    TQMap<KEBListViewItem *, bool>::const_iterator it, end;
    it = mSelectedItems.begin();
    end = mSelectedItems.end();
    for ( ; it != end; ++it)
        s_selected_addresses << it.key()->bookmark().address();
    if(m_listView->currentItem())
    {
        KEBListViewItem * item = static_cast<KEBListViewItem*>(m_listView->currentItem());
        if(item->isEmptyFolderPadder())
            s_current_address = static_cast<KEBListViewItem*>(item->parent())->bookmark().address();
        else
            s_current_address = item->bookmark().address();
    }
    else
        s_current_address = TQString::null;

    updateTree();
    m_searchline->updateSearch();

    // ensureVisible wants to have the midpoint of the new visible area
    m_listView->ensureVisible(0, lastCurrentY + m_listView->visibleHeight() / 2, 0, m_listView->visibleHeight() / 2 );
}

void ListView::updateTree() {
    KBookmarkGroup root = CurrentMgr::self()->mgr()->root();
    fillWithGroup(m_listView, root);
}

void ListView::fillWithGroup(KEBListView *lv, KBookmarkGroup group, KEBListViewItem *parentItem) {
    KEBListViewItem *lastItem = 0;
    if (!parentItem) 
    {
        lv->clear();
        KEBListViewItem *tree = new KEBListViewItem(lv, group);
        fillWithGroup(lv, group, tree);
        tree->TQListViewItem::setOpen(true);
        if (s_selected_addresses.contains(tree->bookmark().address()))
            lv->setSelected(tree, true);
        if(!s_current_address.isNull() && s_current_address == tree->bookmark().address())
            lv->setCurrentItem(tree);
        return;
    }
    for (KBookmark bk = group.first(); !bk.isNull(); bk = group.next(bk)) {
        KEBListViewItem *item = 0;
        if (bk.isGroup()) {
            KBookmarkGroup grp = bk.toGroup();
            item = (parentItem)
                ? new KEBListViewItem(parentItem, lastItem, grp)
                : new KEBListViewItem(lv, lastItem, grp);
            fillWithGroup(lv, grp, item);
            if (grp.isOpen())
                item->TQListViewItem::setOpen(true);
            if (grp.first().isNull())
                new KEBListViewItem(item, item); // empty folder
            lastItem = item;

        }
        else
        {
            item = (parentItem)   
                ? ( (lastItem)
                        ? new KEBListViewItem(parentItem, lastItem, bk)
                        : new KEBListViewItem(parentItem, bk))
                : ( (lastItem)
                        ? new KEBListViewItem(lv, lastItem, bk)
                        : new KEBListViewItem(lv, bk));
            lastItem = item;
        }
        if (s_selected_addresses.contains(bk.address()))
            lv->setSelected(item, true);
        if(!s_current_address.isNull() && s_current_address == bk.address())
            lv->setCurrentItem(item);
    }
}

void ListView::handleMoved(KEBListView *) {
    // kdDebug() << "ListView::handleMoved()" << endl;  
    /* TODO - neil's wishlist item - unfortunately handleMoved is not called sometimes...
     * KMacroCommand *mcmd = CmdGen::self()->deleteItems( i18n("Moved Items"), 
     * ListView::self()->selectedItems());
     * CmdHistory::self()->didCommand(mcmd);
     */
}

void ListView::slotBkInfoUpdateListViewItem() {
    // its not possible that the selection changed inbetween as 
    // handleSelectionChanged is the one that sets up bkInfo() 
    // to emit this signal, otoh. maybe this can cause various
    // differing responses.
    // kdDebug() << "slotBkInfoUpdateListViewItem()" << endl;
    KEBListViewItem *i = firstSelected();
    Q_ASSERT(i);
    KBookmark bk = i->bookmark();
    i->setText(KEBListView::NameColumn, bk.fullText());
    i->setText(KEBListView::UrlColumn, bk.url().pathOrURL());
    TQString commentStr = NodeEditCommand::getNodeText(bk, TQStringList() << "desc");
    i->setText(KEBListView::CommentColumn, commentStr);
}

void ListView::handleContextMenu(KEBListView *, TDEListView *, TQListViewItem *qitem, const TQPoint &p) {
    KEBListViewItem *item = static_cast<KEBListViewItem *>(qitem);
    const char *type = ( !item
            || (item == m_listView->rootItem()) 
            || (item->bookmark().isGroup()) 
            || (item->isEmptyFolderPadder()))
        ? "popup_folder" : "popup_bookmark";
    TQWidget* popup = KEBApp::self()->popupMenuFactory(type);
    if (popup)
        static_cast<TQPopupMenu*>(popup)->popup(p);
}

void ListView::handleDoubleClicked(KEBListView *lv, TQListViewItem *item, const TQPoint &, int column) {
    lv->rename(item, column);
}

void ListView::handleItemRenamed(KEBListView *lv, TQListViewItem *item, const TQString &newText, int column) {
    Q_ASSERT(item);
    KBookmark bk = static_cast<KEBListViewItem *>(item)->bookmark();
    KCommand *cmd = 0;
    if (column == KEBListView::NameColumn) {
        if (newText.isEmpty()) {
            // can't have an empty name, therefore undo the user action
            item->setText(KEBListView::NameColumn, bk.fullText());
        } else if (bk.fullText() != newText) {
            cmd = new NodeEditCommand(bk.address(), newText, "title");
        }

    } else if (column == KEBListView::UrlColumn && !lv->isFolderList()) {
        if (bk.url().pathOrURL() != newText)
        {
            KURL u = KURL::fromPathOrURL(newText);
            cmd = new EditCommand(bk.address(), EditCommand::Edition("href", u.url(0, 106)), i18n("URL"));
        }

    } else if (column == KEBListView::CommentColumn && !lv->isFolderList()) {
        if (NodeEditCommand::getNodeText(bk, "desc") != newText)
            cmd = new NodeEditCommand(bk.address(), newText, "desc");
    }
    CmdHistory::self()->addCommand(cmd);
}

// used by f2 and f3 shortcut slots - see actionsimpl
void ListView::rename(int column) {
    m_listView->rename(firstSelected(), column);
}

void ListView::clearSelection() {
    m_listView->clearSelection();
}

void ListView::startRename(int column, KEBListViewItem *item) {
    s_myrenamecolumn = column;
    s_myrenameitem = item;
}

void ListView::renameNextCell(bool fwd) {
    KEBListView *lv = m_listView;
    while (1) {
        if (fwd && s_myrenamecolumn < KEBListView::CommentColumn) {
            s_myrenamecolumn++;
        } else if (!fwd && s_myrenamecolumn > KEBListView::NameColumn) {
            s_myrenamecolumn--;
        } else {
            s_myrenameitem    = 
                static_cast<KEBListViewItem *>(
                        fwd ? ( s_myrenameitem->itemBelow() 
                            ? s_myrenameitem->itemBelow() : lv->firstChild() ) 
                        : ( s_myrenameitem->itemAbove()
                            ? s_myrenameitem->itemAbove() : lv->lastItem() ) );
            s_myrenamecolumn  
                = fwd ? KEBListView::NameColumn 
                : KEBListView::CommentColumn;
        }
        if (s_myrenameitem 
                && s_myrenameitem != m_listView->rootItem()
                && !s_myrenameitem->isEmptyFolderPadder()
                && !s_myrenameitem->bookmark().isSeparator()
                && !(s_myrenamecolumn == KEBListView::UrlColumn && s_myrenameitem->bookmark().isGroup())
           ) {
            break;
        }
    }
    lv->rename(s_myrenameitem, s_myrenamecolumn);
}

/* -------------------------------------- */

class KeyPressEater : public TQObject {
    public:
        KeyPressEater( TQWidget *parent = 0, const char *name = 0 )
            : TQObject(parent, name) {
            m_allowedToTab = true;
        }
    protected:
        bool eventFilter(TQObject *, TQEvent *);
        bool m_allowedToTab;
};

bool KeyPressEater::eventFilter(TQObject *, TQEvent *pe) {
    if (pe->type() == TQEvent::KeyPress) {
        TQKeyEvent *k = (TQKeyEvent *) pe;
        if ((k->key() == Qt::Key_Backtab || k->key() == Qt::Key_Tab)
                && !(k->state() & ControlButton || k->state() & AltButton)
           ) {
            if (m_allowedToTab) {
                bool fwd = (k->key() == Key_Tab && !(k->state() & ShiftButton));
                ListView::self()->renameNextCell(fwd);
            }
            return true;
        } else {
            m_allowedToTab = (k->key() == Qt::Key_Escape || k->key() == Qt::Key_Enter);
        }
    }
    return false;
}

/* -------------------------------------- */

void KEBListView::loadColumnSetting() 
{
    header()->resizeSection(KEBListView::NameColumn, KEBSettings::name());
    header()->resizeSection(KEBListView::UrlColumn, KEBSettings::uRL());
    header()->resizeSection(KEBListView::CommentColumn, KEBSettings::comment());
    header()->resizeSection(KEBListView::StatusColumn, KEBSettings::status());
#ifdef DEBUG_ADDRESSES
    header()->resizeSection(KEBListView::AddressColumn, KEBSettings::address());
#endif
    m_widthsDirty = false;
}

void KEBListView::saveColumnSetting () 
{
    if (m_widthsDirty) {
        KEBSettings::setName( header()->sectionSize(KEBListView::NameColumn));
        KEBSettings::setURL( header()->sectionSize(KEBListView::UrlColumn));
        KEBSettings::setComment( header()->sectionSize(KEBListView::CommentColumn));
        KEBSettings::setStatus( header()->sectionSize(KEBListView::StatusColumn));
#ifdef DEBUG_ADDRESSES
        KEBSettings::setAddress( header()->sectionSize(KEBListView::AddressColumn));
#endif
        KEBSettings::writeConfig();
    }
}

void KEBListView::slotColumnSizeChanged(int, int, int)
{
    m_widthsDirty = true;
}

void KEBListView::init() {
    setRootIsDecorated(false);
    if (!m_folderList) {
        addColumn(i18n("Bookmark"), 0); // KEBListView::NameColumn
        addColumn(i18n("URL"), 0);
        addColumn(i18n("Comment"), 0);
        addColumn(i18n("Status"), 0);
#ifdef DEBUG_ADDRESSES
        addColumn(i18n("Address"), 0);
#endif
    } else {
        addColumn(i18n("Folder"), 0);
    }
    loadColumnSetting();
    setRenameable(KEBListView::NameColumn);
    setRenameable(KEBListView::UrlColumn);
    setRenameable(KEBListView::CommentColumn);
    setTabOrderedRenaming(false);
    setSorting(-1, false);
    setDragEnabled(true);
    setSelectionModeExt((!m_folderList) ? TDEListView::Extended: TDEListView::Single);
    setAllColumnsShowFocus(true);
    connect(header(), TQT_SIGNAL(sizeChange(int, int, int)),
            this, TQT_SLOT(slotColumnSizeChanged(int, int, int)));
}

void KEBListView::makeConnections() {
    connect(this, TQT_SIGNAL( moved() ),
            TQT_SLOT( slotMoved() ));
    connect(this, TQT_SIGNAL( contextMenu(TDEListView *, TQListViewItem*, const TQPoint &) ),
            TQT_SLOT( slotContextMenu(TDEListView *, TQListViewItem *, const TQPoint &) ));
    connect(this, TQT_SIGNAL( itemRenamed(TQListViewItem *, const TQString &, int) ),
            TQT_SLOT( slotItemRenamed(TQListViewItem *, const TQString &, int) ));
    connect(this, TQT_SIGNAL( doubleClicked(TQListViewItem *, const TQPoint &, int) ),
            TQT_SLOT( slotDoubleClicked(TQListViewItem *, const TQPoint &, int) ));
    connect(this, TQT_SIGNAL( dropped(TQDropEvent*, TQListViewItem*, TQListViewItem*) ),
            TQT_SLOT( slotDropped(TQDropEvent*, TQListViewItem*, TQListViewItem*) ));
}

void KEBListView::readonlyFlagInit(bool readonly) {
    setItemsMovable(readonly); // we move items ourselves (for undo)
    setItemsRenameable(!readonly);
    setAcceptDrops(!readonly);
    setDropVisualizer(!readonly);
}

void KEBListView::slotMoved() 
{ ListView::self()->handleMoved(this); }
void KEBListView::slotContextMenu(TDEListView *a, TQListViewItem *b, const TQPoint &c) 
{ ListView::self()->handleContextMenu(this, a,b,c); }
void KEBListView::slotItemRenamed(TQListViewItem *a, const TQString &b, int c) 
{ ListView::self()->handleItemRenamed(this, a,b,c); }
void KEBListView::slotDoubleClicked(TQListViewItem *a, const TQPoint &b, int c) 
{ ListView::self()->handleDoubleClicked(this, a,b,c); }
void KEBListView::slotDropped(TQDropEvent *a, TQListViewItem *b, TQListViewItem *c) 
{ ListView::self()->handleDropped(this, a,b,c); }

void KEBListView::rename(TQListViewItem *qitem, int column) {
    KEBListViewItem *item = static_cast<KEBListViewItem *>(qitem);
    if ( !(column == NameColumn || column == UrlColumn || column == CommentColumn)
            || KEBApp::self()->readonly()
            || !item 
            || item == firstChild() 
            || item->isEmptyFolderPadder()
            || item->bookmark().isSeparator()
            || (column == UrlColumn && item->bookmark().isGroup())
       ) {
        return;
    }
    ListView::startRename(column, item);
    KeyPressEater *keyPressEater = new KeyPressEater(this);
    renameLineEdit()->installEventFilter(keyPressEater);
    TDEListView::rename(item, column);
}

KEBListViewItem* KEBListView::rootItem() const {
    return static_cast<KEBListViewItem *>(firstChild());
}

// Update display of bookmarks containing URL
void KEBListView::updateByURL(TQString url) {
    for (TQListViewItemIterator it(this); it.current(); it++) {
        KEBListViewItem *p = static_cast<KEBListViewItem *>(it.current());
        if (p->text(1) == url) {
            p->modUpdate();
        }
    }
}

bool KEBListView::acceptDrag(TQDropEvent * e) const {
    return (e->source() == viewport() || KBookmarkDrag::canDecode(e));
}

TQDragObject *KEBListView::dragObject() {
    TQValueList<KBookmark> bookmarks = 
        ListView::self()->itemsToBookmarks(ListView::self()->selectedItemsMap());
    KBookmarkDrag *drag = KBookmarkDrag::newDrag(bookmarks, viewport());
    const TQString iconname = 
        (bookmarks.size() == 1) ? bookmarks.first().icon() : TQString("bookmark");
    drag->setPixmap(SmallIcon(iconname)) ;
    return drag;
}

/* -------------------------------------- */

bool KEBListViewItem::parentSelected(TQListViewItem * item)
{
    TQListViewItem *root = item->listView()->firstChild();
    for( TQListViewItem *parent = item->parent(); parent ; parent = parent->parent())
        if (parent->isSelected() && parent != root)
            return true;
    return false;
}

void KEBListViewItem::setSelected(bool s)
{
    if( isEmptyFolderPadder())
    {
        parent()->setSelected(true);
        return;
    }

    if(listView()->firstChild() == this)
    {
        ListView::self()->selected(this, s);
        TQListViewItem::setSelected( s );
        return;
    }

    if(s == false)
    {
        ListView::self()->selected(this, false);
        TQListViewItem::setSelected( false );
        ListView::deselectAllChildren( this ); //repaints
    }
    else if(parentSelected(this))
        return;
    else
    {
        ListView::self()->selected(this, true);
        TQListViewItem::setSelected( true );
        ListView::deselectAllChildren(this);
    }
}

void KEBListViewItem::normalConstruct(const KBookmark &bk) {
#ifdef DEBUG_ADDRESSES
    setText(KEBListView::AddressColumn, bk.address());
#endif
    setText(KEBListView::CommentColumn, NodeEditCommand::getNodeText(bk, "desc"));
    bool shown = CmdGen::shownInToolbar(bk);
    setPixmap(0, SmallIcon(shown ? TQString("bookmark_toolbar") : bk.icon()));
    // DESIGN - modUpdate badly needs a redesign
    modUpdate();
}

// DESIGN - following constructors should be names classes or else just explicit

// toplevel item (there should be only one!)
KEBListViewItem::KEBListViewItem(TQListView *parent, const KBookmarkGroup &gp)
    : TQListViewItem(parent, KEBApp::self()->caption().isNull() 
                                ? i18n("Bookmarks")
                                : i18n("%1 Bookmarks").arg(KEBApp::self()->caption())), 
    m_bookmark(gp), m_emptyFolderPadder(false) {

    setPixmap(0, SmallIcon("bookmark"));
    setExpandable(true);
}

// empty folder item
KEBListViewItem::KEBListViewItem(KEBListViewItem *parent, TQListViewItem *after)
    : TQListViewItem(parent, after, i18n("Empty Folder") ), m_emptyFolderPadder(true) {
    setPixmap(0, SmallIcon("bookmark"));
}

// group
KEBListViewItem::KEBListViewItem(KEBListViewItem *parent, TQListViewItem *after, const KBookmarkGroup &gp)
    : TQListViewItem(parent, after, gp.fullText()), m_bookmark(gp), m_emptyFolderPadder(false) {
    setExpandable(true);
    normalConstruct(gp);
}

// bookmark (first of its group)
KEBListViewItem::KEBListViewItem(KEBListViewItem *parent, const KBookmark & bk)
    : TQListViewItem(parent, bk.fullText(), bk.url().pathOrURL()), m_bookmark(bk), m_emptyFolderPadder(false) {
    normalConstruct(bk);
}

// bookmark (after another)
KEBListViewItem::KEBListViewItem(KEBListViewItem *parent, TQListViewItem *after, const KBookmark &bk)
    : TQListViewItem(parent, after, bk.fullText(), bk.url().pathOrURL()), m_bookmark(bk), m_emptyFolderPadder(false) {
    normalConstruct(bk);
}

// root bookmark (first of its group)
KEBListViewItem::KEBListViewItem(TQListView *parent, const KBookmark & bk)
    : TQListViewItem(parent, bk.fullText(), bk.url().pathOrURL()), m_bookmark(bk), m_emptyFolderPadder(false) {
    normalConstruct(bk);
}

// root bookmark (after another)
KEBListViewItem::KEBListViewItem(TQListView *parent, TQListViewItem *after, const KBookmark &bk)
    : TQListViewItem(parent, after, bk.fullText(), bk.url().pathOrURL()), m_bookmark(bk), m_emptyFolderPadder(false) {
    normalConstruct(bk);
}

// DESIGN - move this into kbookmark or into a helper
void KEBListViewItem::setOpen(bool open) {
    if (!parent())
        return;
    m_bookmark.internalElement().setAttribute("folded", open ? "no" : "yes");
    TQListViewItem::setOpen(open);
}

void KEBListViewItem::greyStyle(TQColorGroup &cg) {
  int h, s, v;
  cg.background().hsv(&h, &s, &v);
  TQColor color = (v > 180 && v < 220) ? (Qt::darkGray) : (Qt::gray);
  cg.setColor(TQColorGroup::Text, color);
}

void KEBListViewItem::boldStyle(TQPainter *p) {
  TQFont font = p->font();
  font.setBold(true);
  p->setFont(font);
}

void KEBListViewItem::paintCell(TQPainter *p, const TQColorGroup &ocg, int col, int w, int a) {
    TQColorGroup cg(ocg);
    if (parentSelected(this)) {
        int base_h, base_s, base_v;
        cg.background().hsv(&base_h, &base_s, &base_v);

        int hilite_h, hilite_s, hilite_v;
        cg.highlight().hsv(&hilite_h, &hilite_s, &hilite_v);

        TQColor col(hilite_h,
                   (hilite_s + base_s + base_s ) / 3,  
                   (hilite_v + base_v + base_v ) / 3,  
                   TQColor::Hsv);
        cg.setColor(TQColorGroup::Base, col);
    }

    if (col == KEBListView::StatusColumn) {
        switch (m_paintStyle) {
           case KEBListViewItem::GreyStyle:
                {
                    greyStyle(cg);
                    break;
                }
            case KEBListViewItem::BoldStyle:
                {
                    boldStyle(p);
                    break;
                }
            case KEBListViewItem::GreyBoldStyle:
                {
                    greyStyle(cg);
                    boldStyle(p);
                    break;
                }
            case KEBListViewItem::DefaultStyle:
                break;
        }
    }

    TQListViewItem::paintCell(p, cg, col, w,a);
}

#include "listview.moc"
