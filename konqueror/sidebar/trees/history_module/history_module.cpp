/* This file is part of the KDE project
   Copyright (C) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                 2000 David Faure <faure@kde.org>

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

#include <tqapplication.h>
#include <tqpopupmenu.h>

#include <tdeapplication.h>
#include <tdeaction.h>
#include <kcursor.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <krun.h>
#include <kstaticdeleter.h>

#include <konq_faviconmgr.h>

#include "history_module.h"
#include "history_settings.h"

static KStaticDeleter<KonqSidebarHistorySettings> sd;
KonqSidebarHistorySettings * KonqSidebarHistoryModule::s_settings = 0L;

KonqSidebarHistoryModule::KonqSidebarHistoryModule( KonqSidebarTree * parentTree, const char *name )
    : TQObject( 0L, name ), KonqSidebarTreeModule( parentTree ),
      m_dict( 349 ),
      m_topLevelItem( 0L ),
      m_dlg( 0L ),
      m_initialized( false )
{
    if ( !s_settings ) {
	sd.setObject( s_settings,
                      new KonqSidebarHistorySettings( 0, "history settings" ));
	s_settings->readSettings( true );
    }

    connect( s_settings, TQT_SIGNAL( settingsChanged() ), TQT_SLOT( slotSettingsChanged() ));

    m_dict.setAutoDelete( true );
    m_currentTime = TQDateTime::currentDateTime();

    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver cs( kc, "HistorySettings" );
    m_sortsByName = kc->readEntry( "SortHistory", "byDate" ) == "byName";


    KonqHistoryManager *manager = KonqHistoryManager::kself();

    connect( manager, TQT_SIGNAL( loadingFinished() ), TQT_SLOT( slotCreateItems() ));
    connect( manager, TQT_SIGNAL( cleared() ), TQT_SLOT( clear() ));

    connect( manager, TQT_SIGNAL( entryAdded( const KonqHistoryEntry * ) ),
	     TQT_SLOT( slotEntryAdded( const KonqHistoryEntry * ) ));
    connect( manager, TQT_SIGNAL( entryRemoved( const KonqHistoryEntry *) ),
	     TQT_SLOT( slotEntryRemoved( const KonqHistoryEntry *) ));

    connect( parentTree, TQT_SIGNAL( expanded( TQListViewItem * )),
	     TQT_SLOT( slotItemExpanded( TQListViewItem * )));

    m_collection = new TDEActionCollection( this, "history actions" );
    (void) new TDEAction( i18n("New &Window"), "window-new", 0, this,
 			TQT_SLOT( slotNewWindow() ), m_collection, "open_new");
    (void) new TDEAction( i18n("&Remove Entry"), "edit-delete", 0, this,
			TQT_SLOT( slotRemoveEntry() ), m_collection, "remove");
    (void) new TDEAction( i18n("C&lear History"), "history_clear", 0, this,
			TQT_SLOT( slotClearHistory() ), m_collection, "clear");
    (void) new TDEAction( i18n("&Preferences..."), "configure", 0, this,
			TQT_SLOT( slotPreferences()), m_collection, "preferences");

    TDERadioAction *sort;
    sort = new TDERadioAction( i18n("By &Name"), 0, this,
			     TQT_SLOT( slotSortByName() ), m_collection, "byName");
    sort->setExclusiveGroup("SortGroup");
    sort->setChecked( m_sortsByName );

    sort = new TDERadioAction( i18n("By &Date"), 0, this,
			     TQT_SLOT( slotSortByDate() ), m_collection, "byDate");
    sort->setExclusiveGroup("SortGroup");
    sort->setChecked( !m_sortsByName );

    m_folderClosed = SmallIcon( "folder" );
    m_folderOpen = SmallIcon( "folder_open" );

    slotSettingsChanged(); // read the settings
}

KonqSidebarHistoryModule::~KonqSidebarHistoryModule()
{
    HistoryItemIterator it( m_dict );
    TQStringList openGroups;
    while ( it.current() ) {
	if ( it.current()->isOpen() )
	    openGroups.append( it.currentKey() );
	++it;
    }

    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver cs( kc, "HistorySettings" );
    kc->writeEntry("OpenGroups", openGroups);
    kc->sync();
}

void KonqSidebarHistoryModule::slotSettingsChanged()
{
    KonqSidebarHistoryItem::setSettings( s_settings );
    tree()->triggerUpdate();
}

void KonqSidebarHistoryModule::slotCreateItems()
{
    TQApplication::setOverrideCursor( KCursor::waitCursor() );
    clear();

    KonqSidebarHistoryItem *item;
    KonqHistoryEntry *entry;
    KonqHistoryList entries( KonqHistoryManager::kself()->entries() );
    KonqHistoryIterator it( entries );
    m_currentTime = TQDateTime::currentDateTime();

    // the group item and the item of the serverroot '/' get a fav-icon
    // if available. All others get the protocol icon.
    while ( (entry = it.current()) ) {
	KonqSidebarHistoryGroupItem *group = getGroupItem( entry->url );
	item = new KonqSidebarHistoryItem( entry, group, m_topLevelItem );

	++it;
    }

    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver cs( kc, "HistorySettings" );
    TQStringList openGroups = kc->readListEntry("OpenGroups");
    TQStringList::Iterator it2 = openGroups.begin();
    KonqSidebarHistoryGroupItem *group;
    while ( it2 != openGroups.end() ) {
	group = m_dict.find( *it2 );
	if ( group )
	    group->setOpen( true );

	++it2;
    }

    TQApplication::restoreOverrideCursor();
    m_initialized = true;
}

// deletes the listview items but does not affect the history backend
void KonqSidebarHistoryModule::clear()
{
    m_dict.clear();
}

void KonqSidebarHistoryModule::slotEntryAdded( const KonqHistoryEntry *entry )
{
    if ( !m_initialized )
	return;

    m_currentTime = TQDateTime::currentDateTime();
    KonqSidebarHistoryGroupItem *group = getGroupItem( entry->url );
    KonqSidebarHistoryItem *item = group->findChild( entry );
    if ( !item )
	item = new KonqSidebarHistoryItem( entry, group, m_topLevelItem );
    else
	item->update( entry );

    // TQListView scrolls when calling sort(), so we have to hack around that
    // (we don't want no scrolling every time an entry is added)
    KonqSidebarTree *t = tree();
    t->lockScrolling( true );
    group->sort();
    m_topLevelItem->sort();
    tqApp->processOneEvent();
    t->lockScrolling( false );
}

void KonqSidebarHistoryModule::slotEntryRemoved( const KonqHistoryEntry *entry )
{
    if ( !m_initialized )
	return;

    TQString groupKey = groupForURL( entry->url );
    KonqSidebarHistoryGroupItem *group = m_dict.find( groupKey );
    if ( !group )
	return;

    delete group->findChild( entry );

    if ( group->childCount() == 0 )
	m_dict.remove( groupKey );
}

void KonqSidebarHistoryModule::addTopLevelItem( KonqSidebarTreeTopLevelItem * item )
{
    m_topLevelItem = item;
}

bool KonqSidebarHistoryModule::handleTopLevelContextMenu( KonqSidebarTreeTopLevelItem *,
                                                          const TQPoint& pos )
{
    showPopupMenu( ModuleContextMenu, pos );
    return true;
}

void KonqSidebarHistoryModule::showPopupMenu()
{
    showPopupMenu( EntryContextMenu | ModuleContextMenu, TQCursor::pos() );
}

void KonqSidebarHistoryModule::showPopupMenu( int which, const TQPoint& pos )
{
    TQPopupMenu *sortMenu = new TQPopupMenu;
    m_collection->action("byName")->plug( sortMenu );
    m_collection->action("byDate")->plug( sortMenu );

    TQPopupMenu *menu = new TQPopupMenu;

    if ( which & EntryContextMenu )
    {
        m_collection->action("open_new")->plug( menu );
        menu->insertSeparator();
        m_collection->action("remove")->plug( menu );
    }

    m_collection->action("clear")->plug( menu );
    menu->insertSeparator();
    menu->insertItem( i18n("Sort"), sortMenu );
    menu->insertSeparator();
    m_collection->action("preferences")->plug( menu );

    menu->exec( pos );
    delete menu;
    delete sortMenu;
}

void KonqSidebarHistoryModule::slotNewWindow()
{
    kdDebug(1201)<<"void KonqSidebarHistoryModule::slotNewWindow()"<<endl;

    TQListViewItem *item = tree()->selectedItem();
    KonqSidebarHistoryItem *hi = dynamic_cast<KonqSidebarHistoryItem*>( item );
    if ( hi )
       {
          kdDebug(1201)<<"void KonqSidebarHistoryModule::slotNewWindow(): emitting createNewWindow"<<endl;
   	  emit tree()->createNewWindow( hi->url() );
       }
}

void KonqSidebarHistoryModule::slotRemoveEntry()
{
    TQListViewItem *item = tree()->selectedItem();
    KonqSidebarHistoryItem *hi = dynamic_cast<KonqSidebarHistoryItem*>( item );
    if ( hi ) // remove a single entry
	KonqHistoryManager::kself()->emitRemoveFromHistory( hi->externalURL());

    else { // remove a group of entries
	KonqSidebarHistoryGroupItem *gi = dynamic_cast<KonqSidebarHistoryGroupItem*>( item );
	if ( gi )
	    gi->remove();
    }
}

void KonqSidebarHistoryModule::slotPreferences()
{
    // Run the history sidebar settings.
    KRun::run( "tdecmshell kcmhistory", KURL::List() );
}

void KonqSidebarHistoryModule::slotSortByName()
{
    m_sortsByName = true;
    sortingChanged();
}

void KonqSidebarHistoryModule::slotSortByDate()
{
    m_sortsByName = false;
    sortingChanged();
}

void KonqSidebarHistoryModule::sortingChanged()
{
    m_topLevelItem->sort();

    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver cs( kc, "HistorySettings" );
    kc->writeEntry( "SortHistory", m_sortsByName ? "byName" : "byDate" );
    kc->sync();
}

void KonqSidebarHistoryModule::slotItemExpanded( TQListViewItem *item )
{
    if ( item == m_topLevelItem && !m_initialized )
	slotCreateItems();
}

void KonqSidebarHistoryModule::groupOpened( KonqSidebarHistoryGroupItem *item, bool open )
{
    if ( item->hasFavIcon() )
	return;

    if ( open )
	item->setPixmap( 0, m_folderOpen );
    else
	item->setPixmap( 0, m_folderClosed );
}


KonqSidebarHistoryGroupItem * KonqSidebarHistoryModule::getGroupItem( const KURL& url )
{
    const TQString& groupKey = groupForURL( url );
    KonqSidebarHistoryGroupItem *group = m_dict.find( groupKey );
    if ( !group ) {
	group = new KonqSidebarHistoryGroupItem( url, m_topLevelItem );

	TQString icon = KonqFavIconMgr::iconForURL( url.url() );
	if ( icon.isEmpty() )
	    group->setPixmap( 0, m_folderClosed );
	else
	    group->setFavIcon( SmallIcon( icon ) );

	group->setText( 0, groupKey );

	m_dict.insert( groupKey, group );
    }

    return group;
}

void KonqSidebarHistoryModule::slotClearHistory()
{
    KGuiItem guiitem = KStdGuiItem::clear();
    guiitem.setIconSet( SmallIconSet("history_clear"));

    if ( KMessageBox::warningContinueCancel( tree(),
				     i18n("Do you really want to clear "
					  "the entire history?"),
				     i18n("Clear History?"), guiitem )
	 == KMessageBox::Continue )
	KonqHistoryManager::kself()->emitClear();
}


extern "C"
{
	KDE_EXPORT KonqSidebarTreeModule* create_konq_sidebartree_history(KonqSidebarTree* par, const bool)
	{
		return new KonqSidebarHistoryModule(par);
	}
}



#include "history_module.moc"
