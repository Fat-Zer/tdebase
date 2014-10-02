/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2002 Michael Brade <brade@kde.org>
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

#include "dirtree_module.h"
#include "dirtree_item.h"
#include <kdebug.h>
#include <kprotocolinfo.h>
#include <kdesktopfile.h>
#include <tdemessagebox.h>
#include <kiconloader.h>
#include <kdirlister.h>
#include "konqsidebariface_p.h"

KonqSidebarDirTreeModule::KonqSidebarDirTreeModule( KonqSidebarTree * parentTree , bool showHidden)
    : KonqSidebarTreeModule( parentTree, showHidden ), m_dirLister(0L), m_topLevelItem(0L)
{
    bool universalMode=false;
    /* Doesn't work reliable :-(

    KonqSidebarPlugin * plugin = parentTree->part();
    // KonqSidebarPlugin::universalMode() is protected :-|
    if ( plugin->parent() ) {
        KonqSidebarIface * ksi = static_cast<KonqSidebarIface*>( plugin->parent()->tqt_cast( "KonqSidebarIface" ) );
        universalMode = ksi ? ksi->universalMode() : false;
    } */

    TDEConfig * config = new TDEConfig( universalMode ? "konqsidebartng_kicker.rc" : "konqsidebartng.rc" );
    config->setGroup("");
    m_showArchivesAsFolders = config->readBoolEntry("ShowArchivesAsFolders",true);
    delete config;
}

KonqSidebarDirTreeModule::~KonqSidebarDirTreeModule()
{
    // KDirLister may still emit canceled while being deleted.
    if (m_dirLister)
    {
       disconnect( m_dirLister, TQT_SIGNAL( canceled( const KURL & ) ),
                   this, TQT_SLOT( slotListingStopped( const KURL & ) ) );
       delete m_dirLister;
    }
}

KURL::List KonqSidebarDirTreeModule::selectedUrls()
{
    KURL::List lst;
    KonqSidebarDirTreeItem *selection = static_cast<KonqSidebarDirTreeItem *>( m_pTree->selectedItem() );
    if( !selection )
    {
        kdError() << "KonqSidebarDirTreeModule::selectedUrls: no selection!" << endl;
        return lst;
    }
    lst.append(selection->fileItem()->url());
    return lst;
}

void KonqSidebarDirTreeModule::addTopLevelItem( KonqSidebarTreeTopLevelItem * item )
{
    if(m_topLevelItem) // We can handle only one at a time !
        kdError() << "KonqSidebarDirTreeModule::addTopLevelItem: Impossible, we can have only one toplevel item !" << endl;

    KDesktopFile cfg( item->path(), true );
    cfg.setDollarExpansion(true);

    KURL targetURL;
    targetURL.setPath(item->path());

    if ( cfg.hasLinkType() )
    {
        targetURL = cfg.readURL();
		// some services might want to make their URL configurable in kcontrol
		TQString configured = cfg.readEntry("X-TDE-ConfiguredURL");
		if (!configured.isEmpty()) {
			TQStringList list = TQStringList::split(':', configured);
			TDEConfig config(list[0]);
			if (list[1] != "noGroup") config.setGroup(list[1]);
			TQString conf_url = config.readEntry(list[2]);
			if (!conf_url.isEmpty()) {
				targetURL = conf_url;
			}
		}
    }
    else if ( cfg.hasDeviceType() )
    {
        // Determine the mountpoint
        TQString mp = cfg.readEntry("MountPoint");
        if ( mp.isEmpty() )
            return;

        targetURL.setPath(mp);
    }
    else
        return;

    bool bListable = KProtocolInfo::supportsListing( targetURL );
    //kdDebug(1201) << targetURL.prettyURL() << " listable : " << bListable << endl;

    if ( !bListable )
    {
        item->setExpandable( false );
        item->setListable( false );
    }

    item->setExternalURL( targetURL );
    addSubDir( item );

    m_topLevelItem = item;
}

void KonqSidebarDirTreeModule::openTopLevelItem( KonqSidebarTreeTopLevelItem * item )
{
    if ( !item->childCount() && item->isListable() )
        openSubFolder( item );
}

void KonqSidebarDirTreeModule::addSubDir( KonqSidebarTreeItem *item )
{
    TQString id = item->externalURL().url(-1);
    kdDebug(1201) << this << " KonqSidebarDirTreeModule::addSubDir " << id << endl;
    m_dictSubDirs.insert(id, item );

    KonqSidebarDirTreeItem *ditem = dynamic_cast<KonqSidebarDirTreeItem*>(item);
    if (ditem) {
       m_ptrdictSubDirs.insert(ditem->fileItem(), item);
    }
}

// Remove <key, item> from dict, taking into account that there maybe
// other items with the same key.
static void remove(TQDict<KonqSidebarTreeItem> &dict, const TQString &key, KonqSidebarTreeItem *item)
{
   TQPtrList<KonqSidebarTreeItem> *otherItems = 0;
   while (true) {
      KonqSidebarTreeItem *takeItem = dict.take(key);
      if (!takeItem || (takeItem == item)) {
         if (!otherItems) {
            return;
         }

         // Insert the otherItems back in
         for (KonqSidebarTreeItem *otherItem; (otherItem = otherItems->take(0));)
         {
            dict.insert(key, otherItem);
         }
         delete otherItems;
         return;
      }
      // Not the item we are looking for
      if (!otherItems) {
         otherItems = new TQPtrList<KonqSidebarTreeItem>();
      }

      otherItems->prepend(takeItem);
   }
}

// Looks up key in dict and returns it in item, if there are multiple items
// with the same key, additional items are returned in itemList which should
// be deleted by the caller.
static void lookupItems(TQDict<KonqSidebarTreeItem> &dict, const TQString &key, KonqSidebarTreeItem *&item, TQPtrList<KonqSidebarTreeItem> *&itemList)
{
   itemList = 0;
   item = dict.take(key);
   if (!item) {
       return;
   }

   while (true) {
       KonqSidebarTreeItem *takeItem = dict.take(key);
       if (!takeItem) {
           // Insert itemList back in
           if (itemList) {
               for (KonqSidebarTreeItem *otherItem = itemList->first(); otherItem; otherItem = itemList->next()) {
                   dict.insert(key, otherItem);
               }
           }
           dict.insert(key, item);
           return;
       }
       if (!itemList) {
          itemList = new TQPtrList<KonqSidebarTreeItem>();
       }
       itemList->prepend(takeItem);
   }
}

// Remove <key, item> from dict, taking into account that there maybe
// other items with the same key.
static void remove(TQPtrDict<KonqSidebarTreeItem> &dict, void *key, KonqSidebarTreeItem *item)
{
   TQPtrList<KonqSidebarTreeItem> *otherItems = 0;
   while (true) {
      KonqSidebarTreeItem *takeItem = dict.take(key);
      if (!takeItem || (takeItem == item)) {
         if (!otherItems) {
            return;
         }

         // Insert the otherItems back in
         for (KonqSidebarTreeItem *otherItem; (otherItem = otherItems->take(0));) {
            dict.insert(key, otherItem);
         }
         delete otherItems;
         return;
      }
      // Not the item we are looking for
      if (!otherItems)
         otherItems = new TQPtrList<KonqSidebarTreeItem>();

      otherItems->prepend(takeItem);
   }
}

// Looks up key in dict and returns it in item, if there are multiple items
// with the same key, additional items are returned in itemList which should
// be deleted by the caller.
static void lookupItems(TQPtrDict<KonqSidebarTreeItem> &dict, void *key, KonqSidebarTreeItem *&item, TQPtrList<KonqSidebarTreeItem> *&itemList)
{
   itemList = 0;
   item = dict.take(key);
   if (!item) {
       return;
   }

   while(true) {
       KonqSidebarTreeItem *takeItem = dict.take(key);
       if (!takeItem) {
           // Insert itemList back in
           if (itemList) {
               for(KonqSidebarTreeItem *otherItem = itemList->first(); otherItem; otherItem = itemList->next()) {
                   dict.insert(key, otherItem);
               }
           }
           dict.insert(key, item);
           return;
       }
       if (!itemList) {
          itemList = new TQPtrList<KonqSidebarTreeItem>();
       }
       itemList->prepend(takeItem);
   }
}


void KonqSidebarDirTreeModule::removeSubDir( KonqSidebarTreeItem *item, bool childrenOnly )
{
    kdDebug(1201) << this << " KonqSidebarDirTreeModule::removeSubDir item=" << item << endl;
    if ( item->firstChild() )
    {
        KonqSidebarTreeItem * it = static_cast<KonqSidebarTreeItem *>(item->firstChild());
        KonqSidebarTreeItem * next = 0L;
        while ( it ) {
            next = static_cast<KonqSidebarTreeItem *>(it->nextSibling());
            removeSubDir( it );
            delete it;
            it = next;
        }
    }

    if ( !childrenOnly )
    {
        TQString id = item->externalURL().url(-1);
        remove(m_dictSubDirs, id, item);
	while (!(item->alias.isEmpty()))
	{
	        remove(m_dictSubDirs, item->alias.front(), item);
		item->alias.pop_front();
	}

	KonqSidebarDirTreeItem *ditem = dynamic_cast<KonqSidebarDirTreeItem*>(item);
        if (ditem)
           remove(m_ptrdictSubDirs, ditem->fileItem(), item);
    }
}


void KonqSidebarDirTreeModule::openSubFolder( KonqSidebarTreeItem *item )
{
    kdDebug(1201) << this << " openSubFolder( " << item->externalURL().prettyURL() << " )" << endl;

    if ( !m_dirLister ) // created on demand
    {
        m_dirLister = new KDirLister( true );
        //m_dirLister->setDirOnlyMode( true );
//	TQStringList mimetypes;
//	mimetypes<<TQString("inode/directory");
//	m_dirLister->setMimeFilter(mimetypes);

        connect( m_dirLister, TQT_SIGNAL( newItems( const KFileItemList & ) ),
                 this, TQT_SLOT( slotNewItems( const KFileItemList & ) ) );
        connect( m_dirLister, TQT_SIGNAL( refreshItems( const KFileItemList & ) ),
                 this, TQT_SLOT( slotRefreshItems( const KFileItemList & ) ) );
        connect( m_dirLister, TQT_SIGNAL( deleteItem( KFileItem * ) ),
                 this, TQT_SLOT( slotDeleteItem( KFileItem * ) ) );
        connect( m_dirLister, TQT_SIGNAL( completed( const KURL & ) ),
                 this, TQT_SLOT( slotListingStopped( const KURL & ) ) );
        connect( m_dirLister, TQT_SIGNAL( canceled( const KURL & ) ),
                 this, TQT_SLOT( slotListingStopped( const KURL & ) ) );
        connect( m_dirLister, TQT_SIGNAL( redirection( const KURL &, const KURL & ) ),
                 this, TQT_SLOT( slotRedirection( const KURL &, const KURL & ) ) );
    }


    if ( !item->isTopLevelItem() &&
         static_cast<KonqSidebarDirTreeItem *>(item)->hasStandardIcon() )
    {
        int size = TDEGlobal::iconLoader()->currentSize( TDEIcon::Small );
        TQPixmap pix = DesktopIcon( "folder_open", size );
        m_pTree->startAnimation( item, "trinity", 6, &pix );
    }
    else
        m_pTree->startAnimation( item );

    listDirectory( item );
}

void KonqSidebarDirTreeModule::listDirectory( KonqSidebarTreeItem *item )
{
    // This causes a reparsing, but gets rid of the trailing slash
    TQString strUrl = item->externalURL().url(-1);
    KURL url( strUrl );

    TQPtrList<KonqSidebarTreeItem> *itemList;
    KonqSidebarTreeItem * openItem;
    lookupItems(m_dictSubDirs, strUrl, openItem, itemList);

    while (openItem) {
        if (openItem->childCount()) {
            break;
        }
        openItem = itemList ? itemList->take(0) : 0;
    }
    delete itemList;

    if (openItem) {
       // We have this directory listed already, just copy the entries as we
       // can't use the dirlister, it would invalidate the old entries
       int size = TDEGlobal::iconLoader()->currentSize( TDEIcon::Small );
       KonqSidebarTreeItem * parentItem = item;
       KonqSidebarDirTreeItem *oldItem = static_cast<KonqSidebarDirTreeItem *> (openItem->firstChild());
       while (oldItem) {
          KFileItem * fileItem = oldItem->fileItem();
          if (! fileItem->isDir() )
          {
	      KMimeType::Ptr ptr;

	      if ( fileItem->url().isLocalFile() && (((ptr=fileItem->mimeTypePtrFast())!=0) && (ptr->is("inode/directory") || m_showArchivesAsFolders) && ((!ptr->property("X-TDE-LocalProtocol").toString().isEmpty()) ))) {
		kdDebug()<<"Something not really a directory"<<endl;
	      } else {
//	              kdError() << "Item " << fileItem->url().prettyURL() << " is not a directory!" << endl;
        	      continue;
	      }
          }

          KonqSidebarDirTreeItem *dirTreeItem = new KonqSidebarDirTreeItem( parentItem, m_topLevelItem, fileItem );
          dirTreeItem->setPixmap( 0, fileItem->pixmap( size ) );
          dirTreeItem->setText( 0, TDEIO::decodeFileName( fileItem->name() ) );

          oldItem = static_cast<KonqSidebarDirTreeItem *> (oldItem->nextSibling());
       }
       m_pTree->stopAnimation( item );

       return;
    }

    m_dirLister->setShowingDotFiles( showHidden());

    if (tree()->isOpeningFirstChild()) {
	m_dirLister->setAutoErrorHandlingEnabled(false,0);
    }
    else {
	m_dirLister->setAutoErrorHandlingEnabled(true,tree());
    }

    m_dirLister->openURL( url, true /*keep*/ );
}

void KonqSidebarDirTreeModule::slotNewItems( const KFileItemList& entries )
{
    kdDebug(1201) << this << " KonqSidebarDirTreeModule::slotNewItems " << entries.count() << endl;

    Q_ASSERT(entries.count());
    KFileItem * firstItem = const_cast<KFileItemList&>(entries).last(); // TQList sucks for constness

    // Find parent item - it's the same for all the items
    KURL dir( firstItem->url().url(-1) );
    dir.setFileName( "" );
    kdDebug(1201) << this << " KonqSidebarDirTreeModule::slotNewItems dir=" << dir.url(-1) << endl;

    TQPtrList<KonqSidebarTreeItem> *parentItemList;
    KonqSidebarTreeItem * parentItem;
    lookupItems(m_dictSubDirs, dir.url(-1), parentItem, parentItemList);

    if ( !parentItem ) {   // hack for dnssd://domain/type/service listed in dnssd:/type/ dir
        dir.setHost( TQString::null );
        lookupItems( m_dictSubDirs, dir.url(-1), parentItem, parentItemList );
    }

    if ( !parentItem ) {
        // Use the top level item as the parent
        parentItem = m_topLevelItem;
    }

    kdDebug()<<"number of additional parent items:"<< (parentItemList?parentItemList->count():0)<<endl;
    int size = TDEGlobal::iconLoader()->currentSize( TDEIcon::Small );
    do {
        kdDebug()<<"Parent Item URL:"<<parentItem->externalURL()<<endl;
        TQPtrListIterator<KFileItem> kit ( entries );
        for( ; kit.current(); ++kit ) {
            KFileItem * fileItem = *kit;

            if (! fileItem->isDir() ) {
                KMimeType::Ptr ptr;

                if ( fileItem->url().isLocalFile() && (( (ptr=fileItem->mimeTypePtrFast())!=0) && (ptr->is("inode/directory") || m_showArchivesAsFolders) &&  ((!ptr->property("X-TDE-LocalProtocol").toString().isEmpty()) ))) {
                    kdDebug()<<"Something really a directory"<<endl;
                }
                else {
                    //kdError() << "Item " << fileItem->url().prettyURL() << " is not a directory!" << endl;
                    continue;
                }
            }

            KonqSidebarDirTreeItem *dirTreeItem = new KonqSidebarDirTreeItem( parentItem, m_topLevelItem, fileItem );
            dirTreeItem->setPixmap( 0, fileItem->pixmap( size ) );
            dirTreeItem->setText( 0, TDEIO::decodeFileName( fileItem->name() ) );
        }
    } while ((parentItem = parentItemList ? parentItemList->take(0) : 0));
    delete parentItemList;
}

void KonqSidebarDirTreeModule::slotRefreshItems( const KFileItemList &entries )
{
    int size = TDEGlobal::iconLoader()->currentSize( TDEIcon::Small );

    TQPtrListIterator<KFileItem> kit ( entries );
    kdDebug(1201) << "KonqSidebarDirTreeModule::slotRefreshItems " << entries.count() << " entries. First: " << kit.current()->url().url() << endl;
    for( ; kit.current(); ++kit )
    {
        KFileItem *fileItem = kit.current();

        TQPtrList<KonqSidebarTreeItem> *itemList;
        KonqSidebarTreeItem * item;
        lookupItems(m_ptrdictSubDirs, fileItem, item, itemList);

        if (!item)
        {
            if ( fileItem->isDir() ) // don't warn for files
                kdWarning(1201) << "KonqSidebarDirTreeModule::slotRefreshItems can't find old entry for " << kit.current()->url().url(-1) << endl;
            continue;
        }

        do 
        {
            if ( item->isTopLevelItem() ) // we only have dirs and one toplevel item in the dict
            {
                kdWarning(1201) << "KonqSidebarDirTreeModule::slotRefreshItems entry for " << kit.current()->url().url(-1) << " matches against toplevel." << endl;
                break;
            }

            KonqSidebarDirTreeItem * dirTreeItem = static_cast<KonqSidebarDirTreeItem *>(item);
            // Item renamed ?
            if ( dirTreeItem->id != fileItem->url().url( -1 ) )
            {
                // We need to update the URL in m_dictSubDirs, and to get rid of the child items, so remove and add.
                // Then remove + delete
                removeSubDir( dirTreeItem, true /*children only*/ );
                remove(m_dictSubDirs, dirTreeItem->id, dirTreeItem);

                dirTreeItem->reset(); // Reset id
                dirTreeItem->setPixmap( 0, fileItem->pixmap( size ) );
                dirTreeItem->setText( 0, TDEIO::decodeFileName( fileItem->name() ) );

                // Make sure the item doesn't get inserted twice!
                // dirTreeItem->id points to the new name
                remove(m_dictSubDirs, dirTreeItem->id, dirTreeItem);
                m_dictSubDirs.insert(dirTreeItem->id, dirTreeItem);
            }
            else
            {
                dirTreeItem->setPixmap( 0, fileItem->pixmap( size ) );
                dirTreeItem->setText( 0, TDEIO::decodeFileName( fileItem->name() ) );
            }

        } while ((item = itemList ? itemList->take(0) : 0));
        delete itemList;
    }
}

void KonqSidebarDirTreeModule::slotDeleteItem( KFileItem *fileItem )
{
    kdDebug(1201) << "KonqSidebarDirTreeModule::slotDeleteItem( " << fileItem->url().url(-1) << " )" << endl;

    // All items are in m_ptrdictSubDirs, so look it up fast
    TQPtrList<KonqSidebarTreeItem> *itemList;
    KonqSidebarTreeItem * item;
    lookupItems(m_ptrdictSubDirs, fileItem, item, itemList);
    while (item) {
        removeSubDir( item );
        delete item;
        item = itemList ? itemList->take(0) : 0;
    }
    delete itemList;
}

void KonqSidebarDirTreeModule::slotRedirection( const KURL & oldUrl, const KURL & newUrl )
{
    kdDebug(1201) << "******************************KonqSidebarDirTreeModule::slotRedirection(" << newUrl.prettyURL() << ")" << endl;

    TQString oldUrlStr = oldUrl.url(-1);
    TQString newUrlStr = newUrl.url(-1);

    TQPtrList<KonqSidebarTreeItem> *itemList;
    KonqSidebarTreeItem * item;
    lookupItems(m_dictSubDirs, oldUrlStr, item, itemList);

    if (!item) {
        kdWarning(1201) << "NOT FOUND   oldUrl=" << oldUrlStr << endl;
        return;
    }

    do {
        if (item->alias.contains(newUrlStr)) continue;
        kdDebug()<<"Redirectiong element"<<endl;
        // We need to update the URL in m_dictSubDirs
        m_dictSubDirs.insert( newUrlStr, item );
        item->alias << newUrlStr;

        kdDebug(1201) << "Updating url of " << item << " to " << newUrlStr << endl;

    } while ((item = itemList ? itemList->take(0) : 0));
    delete itemList;
}

void KonqSidebarDirTreeModule::slotListingStopped( const KURL & url )
{
    kdDebug(1201) << "KonqSidebarDirTree::slotListingStopped " << url.url(-1) << endl;

    // Use internal reference URL if present
    // Otherwise, the dirlister animation may never stop on redirected URLs such as system:/documents
    TQString urlToStop = url.internalReferenceURL();
    if (urlToStop == "") {
        urlToStop = url.url(-1);
    }

    TQPtrList<KonqSidebarTreeItem> *itemList;
    KonqSidebarTreeItem * item = NULL;
    lookupItems(m_dictSubDirs, urlToStop, item, itemList);

    while (item) {
        if ( item->childCount() == 0 ) {
            item->setExpandable( false );
            item->repaint();
        }
        m_pTree->stopAnimation( item );
        item = itemList ? itemList->take(0) : 0;
    }
    delete itemList;

    kdDebug(1201) << "m_selectAfterOpening " << m_selectAfterOpening.prettyURL() << endl;
    if ( !m_selectAfterOpening.isEmpty() && url.isParentOf( m_selectAfterOpening ) ) {
        KURL theURL( m_selectAfterOpening );
        m_selectAfterOpening = KURL();
        followURL( theURL );
    }
}

void KonqSidebarDirTreeModule::followURL( const KURL & url )
{
    // Check if we already know this URL
    KonqSidebarTreeItem * item = m_dictSubDirs[ url.url(-1) ];
    if (item) { // found it  -> ensure visible, select, return.
        m_pTree->ensureItemVisible( item );
        m_pTree->setSelected( item, true );
        return;
    }

    KURL uParent( url );
    KonqSidebarTreeItem * parentItem = 0L;
    // Go up to the first known parent
    do {
        uParent = uParent.upURL();
        parentItem = m_dictSubDirs[ uParent.url(-1) ];
    } while ( !parentItem && !uParent.path().isEmpty() && uParent.path() != "/" );

    // Not found !?!
    if (!parentItem) {
        kdDebug() << "No parent found for url " << url.prettyURL() << endl;
        return;
    }
    kdDebug(1202) << "Found parent " << uParent.prettyURL() << endl;

    // That's the parent directory we found. Open if not open...
    if ( !parentItem->isOpen() ) {
        parentItem->setOpen( true );
        if ( parentItem->childCount() && m_dictSubDirs[ url.url(-1) ] ) {
            // Immediate opening, if the dir was already listed
            followURL( url ); // equivalent to a goto-beginning-of-method
        }
        else {
            m_selectAfterOpening = url;
            kdDebug(1202) << "KonqSidebarDirTreeModule::followURL: m_selectAfterOpening=" << m_selectAfterOpening.url() << endl;
        }
    }
}


extern "C"
{
        KDE_EXPORT KonqSidebarTreeModule *create_konq_sidebartree_dirtree(KonqSidebarTree* par,const bool showHidden)
	{
		return new KonqSidebarDirTreeModule(par,showHidden);
	}
}



#include "dirtree_module.moc"
