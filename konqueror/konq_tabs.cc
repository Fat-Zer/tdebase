/*  This file is part of the KDE project

    Copyright (C) 2002-2003 Konqueror Developers
                  2002-2003 Douglas Hanley <douglash@caltech.edu>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA.
*/

#include "konq_tabs.h"

#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqptrlist.h>
#include <tqpopupmenu.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kurldrag.h>
#include <kstringhandler.h>

#include "konq_frame.h"
#include "konq_view.h"
#include "konq_viewmgr.h"
#include "konq_misc.h"
#include "konq_settingsxt.h"

#include <tdeaccelmanager.h>
#include <konq_pixmapprovider.h>
#include <tdestdaccel.h>
#include <tqtabbar.h>
#include <tqwhatsthis.h>
#include <tqstyle.h>

#define DUPLICATE_ID 3
#define RELOAD_ID 4
#define BREAKOFF_ID 5
#define CLOSETAB_ID 6
#define OTHERTABS_ID 7
#define MOVE_LEFT_ID 8
#define MOVE_RIGHT_ID 9

//###################################################################

KonqFrameTabs::KonqFrameTabs(TQWidget* parent, KonqFrameContainerBase* parentContainer,
                             KonqViewManager* viewManager, const char * name)
  : KTabWidget(parent, name), m_rightWidget(0), m_leftWidget(0), m_alwaysTabBar(false),
    m_closeOtherTabsId(0)
{
  TDEAcceleratorManager::setNoAccel(this);

  TQWhatsThis::add( tabBar(), i18n( "This bar contains the list of currently open tabs. Click on a tab to make it "
			  "active. The option to show a close button instead of the website icon in the left "
			  "corner of the tab is configurable. You can also use keyboard shortcuts to "
			  "navigate through tabs. The text on the tab is the title of the website "
			  "currently open in it, put your mouse over the tab too see the full title in "
			  "case it was truncated to fit the tab size." ) );
  //kdDebug(1202) << "KonqFrameTabs::KonqFrameTabs()" << endl;

  m_pParentContainer = parentContainer;
  m_pChildFrameList = new TQPtrList<KonqFrameBase>;
  m_pChildFrameList->setAutoDelete(false);
  m_pActiveChild = 0L;
  m_pViewManager = viewManager;

  connect( this, TQT_SIGNAL( currentChanged ( TQWidget * ) ),
           this, TQT_SLOT( slotCurrentChanged( TQWidget* ) ) );

  m_pPopupMenu = new TQPopupMenu( this );
  m_pPopupMenu->insertItem( SmallIcon( "tab_new" ),
                            i18n("&New Tab"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotAddTab() ),
                            m_pViewManager->mainWindow()->action("newtab")->shortcut() );
  m_pPopupMenu->insertItem( SmallIconSet( "reload" ),
                            i18n( "&Reload Tab" ),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotReloadPopup() ),
                            m_pViewManager->mainWindow()->action("reload")->shortcut(), RELOAD_ID );
  m_pPopupMenu->insertItem( SmallIconSet( "tab_duplicate" ),
                            i18n("&Duplicate Tab"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotDuplicateTabPopup() ),
                            m_pViewManager->mainWindow()->action("duplicatecurrenttab")->shortcut(), 
                            DUPLICATE_ID );
  m_pPopupMenu->insertItem( SmallIconSet( "tab_breakoff" ),
                            i18n("D&etach Tab"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotBreakOffTabPopup() ),
                            m_pViewManager->mainWindow()->action("breakoffcurrenttab")->shortcut(),
                            BREAKOFF_ID );
  m_pPopupMenu->insertSeparator();
  m_pPopupMenu->insertItem( SmallIconSet( "tab_move_left" ),
                            i18n("Move Tab &Left"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotMoveTabLeft() ),
                            m_pViewManager->mainWindow()->action("tab_move_left")->shortcut(),
                            MOVE_LEFT_ID );
  m_pPopupMenu->insertItem( SmallIconSet( "tab_move_right" ),
                            i18n("Move Tab &Right"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotMoveTabRight() ),
                            m_pViewManager->mainWindow()->action("tab_move_right")->shortcut(),
                            MOVE_RIGHT_ID );
  m_pPopupMenu->insertSeparator();
  m_pSubPopupMenuTab = new TQPopupMenu( this );
  m_pPopupMenu->insertItem( i18n("Other Tabs" ), m_pSubPopupMenuTab, OTHERTABS_ID );
  connect( m_pSubPopupMenuTab, TQT_SIGNAL( activated ( int ) ),
           this, TQT_SLOT( slotSubPopupMenuTabActivated( int ) ) );
  m_pPopupMenu->insertSeparator();
  m_pPopupMenu->insertItem( SmallIconSet( "tab_remove" ),
                            i18n("&Close Tab"),
                            m_pViewManager->mainWindow(),
                            TQT_SLOT( slotRemoveTabPopup() ),
                            m_pViewManager->mainWindow()->action("removecurrenttab")->shortcut(),
                            CLOSETAB_ID );
  connect( this, TQT_SIGNAL( contextMenu( TQWidget *, const TQPoint & ) ),
           TQT_SLOT(slotContextMenu( TQWidget *, const TQPoint & ) ) );
  connect( this, TQT_SIGNAL( contextMenu( const TQPoint & ) ),
           TQT_SLOT(slotContextMenu( const TQPoint & ) ) );

  m_MouseMiddleClickClosesTab = KonqSettings::mouseMiddleClickClosesTab();

  m_permanentCloseButtons = KonqSettings::permanentCloseButton();
  if (m_permanentCloseButtons) {
    setHoverCloseButton( true );
    setHoverCloseButtonDelayed( false );
  }
  else
    setHoverCloseButton( KonqSettings::hoverCloseButton() );
  setTabCloseActivatePrevious( KonqSettings::tabCloseActivatePrevious() );
  if (KonqSettings::tabPosition()=="Bottom")
    setTabPosition(TQTabWidget::Bottom);
  connect( this, TQT_SIGNAL( closeRequest( TQWidget * )), TQT_SLOT(slotCloseRequest( TQWidget * )));
  connect( this, TQT_SIGNAL( removeTabPopup() ),
           m_pViewManager->mainWindow(), TQT_SLOT( slotRemoveTabPopup() ) );

  if ( KonqSettings::addTabButton() ) {
    m_leftWidget = new TQToolButton( this );
    connect( m_leftWidget, TQT_SIGNAL( clicked() ),
             m_pViewManager->mainWindow(), TQT_SLOT( slotAddTab() ) );
    m_leftWidget->setIconSet( SmallIcon( "tab_new" ) );
    m_leftWidget->adjustSize();
    TQToolTip::add(m_leftWidget, i18n("Open a new tab"));
    setCornerWidget( m_leftWidget, TopLeft );
  }
  if ( KonqSettings::closeTabButton() ) {
    m_rightWidget = new TQToolButton( this );
    connect( m_rightWidget, TQT_SIGNAL( clicked() ),
             m_pViewManager->mainWindow(), TQT_SLOT( slotRemoveTab() ) );
    m_rightWidget->setIconSet( SmallIconSet( "tab_remove" ) );
    m_rightWidget->adjustSize();
    TQToolTip::add(m_rightWidget, i18n("Close the current tab"));
    setCornerWidget( m_rightWidget, TopRight );
  }

  setAutomaticResizeTabs( true );
  setTabReorderingEnabled( true );
  connect( this, TQT_SIGNAL( movedTab( int, int ) ),
           TQT_SLOT( slotMovedTab( int, int ) ) );
  connect( this, TQT_SIGNAL( mouseMiddleClick() ),
           TQT_SLOT( slotMouseMiddleClick() ) );
  connect( this, TQT_SIGNAL( mouseMiddleClick( TQWidget * ) ),
           TQT_SLOT( slotMouseMiddleClick( TQWidget * ) ) );
  connect( this, TQT_SIGNAL( mouseDoubleClick() ),
           m_pViewManager->mainWindow(), TQT_SLOT( slotAddTab() ) );

  connect( this, TQT_SIGNAL( testCanDecode(const TQDragMoveEvent *, bool & )),
           TQT_SLOT( slotTestCanDecode(const TQDragMoveEvent *, bool & ) ) );
  connect( this, TQT_SIGNAL( receivedDropEvent( TQDropEvent * )),
           TQT_SLOT( slotReceivedDropEvent( TQDropEvent * ) ) );
  connect( this, TQT_SIGNAL( receivedDropEvent( TQWidget *, TQDropEvent * )),
           TQT_SLOT( slotReceivedDropEvent( TQWidget *, TQDropEvent * ) ) );
  connect( this, TQT_SIGNAL( initiateDrag( TQWidget * )),
           TQT_SLOT( slotInitiateDrag( TQWidget * ) ) );
}

KonqFrameTabs::~KonqFrameTabs()
{
  //kdDebug(1202) << "KonqFrameTabs::~KonqFrameTabs() " << this << " - " << className() << endl;
  m_pChildFrameList->setAutoDelete(true);
  delete m_pChildFrameList;
}

void KonqFrameTabs::listViews( ChildViewList *viewList ) {
  for( TQPtrListIterator<KonqFrameBase> it( *m_pChildFrameList ); *it; ++it )
    it.current()->listViews(viewList);
}

void KonqFrameTabs::saveConfig( TDEConfig* config, const TQString &prefix, bool saveURLs,
                                KonqFrameBase* docContainer, int id, int depth )
{
  //write children
  TQStringList strlst;
  int i = 0;
  TQString newPrefix;
  for (KonqFrameBase* it = m_pChildFrameList->first(); it; it = m_pChildFrameList->next())
    {
      newPrefix = TQString::fromLatin1( it->frameType() ) + "T" + TQString::number(i);
      strlst.append( newPrefix );
      newPrefix.append( '_' );
      it->saveConfig( config, newPrefix, saveURLs, docContainer, id, depth + i );
      i++;
    }

  config->writeEntry( TQString::fromLatin1( "Children" ).prepend( prefix ), strlst );

  config->writeEntry( TQString::fromLatin1( "activeChildIndex" ).prepend( prefix ),
                      currentPageIndex() );
}

void KonqFrameTabs::copyHistory( KonqFrameBase *other )
{
  if( other->frameType() != "Tabs" ) {
    kdDebug(1202) << "Frame types are not the same" << endl;
    return;
  }

  for (uint i = 0; i < m_pChildFrameList->count(); i++ )
    {
      m_pChildFrameList->at(i)->copyHistory( static_cast<KonqFrameTabs *>( other )->m_pChildFrameList->at(i) );
    }
}

void KonqFrameTabs::printFrameInfo( const TQString& spaces )
{
  kdDebug(1202) << spaces << "KonqFrameTabs " << this << " visible="
                << TQString("%1").arg(isVisible()) << " activeChild="
                << m_pActiveChild << endl;

  if (!m_pActiveChild)
      kdDebug(1202) << "WARNING: " << this << " has a null active child!" << endl;

  KonqFrameBase* child;
  int childFrameCount = m_pChildFrameList->count();
  for (int i = 0 ; i < childFrameCount ; i++) {
    child = m_pChildFrameList->at(i);
    if (child != 0L)
      child->printFrameInfo(spaces + "  ");
    else
      kdDebug(1202) << spaces << "  Null child" << endl;
  }
}

void KonqFrameTabs::reparentFrame( TQWidget* parent, const TQPoint & p, bool showIt )
{
  TQWidget::reparent( parent, p, showIt );
}

void KonqFrameTabs::setTitle( const TQString &title , TQWidget* sender)
{
  // kdDebug(1202) << "KonqFrameTabs::setTitle( " << title << " , " << sender << " )" << endl;
  setTabLabel( sender,title );
}

void KonqFrameTabs::setTabIcon( const KURL &url, TQWidget* sender )
{
  //kdDebug(1202) << "KonqFrameTabs::setTabIcon( " << url << " , " << sender << " )" << endl;
  TQIconSet iconSet;
  if (m_permanentCloseButtons)
    iconSet =  SmallIcon( "fileclose" );
  else
    iconSet =  SmallIconSet( KonqPixmapProvider::self()->iconNameFor( url.url() ) );
  if (tabIconSet( sender ).pixmap().serialNumber() != iconSet.pixmap().serialNumber())
    setTabIconSet( sender, iconSet );
}

void KonqFrameTabs::activateChild()
{
  if (m_pActiveChild)
    {
      showPage( m_pActiveChild->widget() );
      m_pActiveChild->activateChild();
    }
}

void KonqFrameTabs::insertChildFrame( KonqFrameBase* frame, int index )
{
  //kdDebug(1202) << "KonqFrameTabs " << this << ": insertChildFrame " << frame << endl;

  if (frame)
    {
      //kdDebug(1202) << "Adding frame" << endl;
      bool showTabBar = (count() == 1);
      insertTab(frame->widget(),"", index);
      frame->setParentContainer(this);
      if (index == -1) m_pChildFrameList->append(frame);
      else m_pChildFrameList->insert(index, frame);
      if (m_rightWidget)
        m_rightWidget->setEnabled( m_pChildFrameList->count()>1 );
      KonqView* activeChildView = frame->activeChildView();
      if (activeChildView != 0L) {
        activeChildView->setCaption( activeChildView->caption() );
        activeChildView->setTabIcon( activeChildView->url() );
      }
      if (showTabBar)
          setTabBarHidden(false);
      else if ( count() == 1 )
          this->hideTabBar();//the first frame inserted (initialization)
    }
  else
    kdWarning(1202) << "KonqFrameTabs " << this << ": insertChildFrame(0L) !" << endl;
}

void KonqFrameTabs::removeChildFrame( KonqFrameBase * frame )
{
  //kdDebug(1202) << "KonqFrameTabs::RemoveChildFrame " << this << ". Child " << frame << " removed" << endl;
  if (frame) {
    removePage(frame->widget());
    m_pChildFrameList->remove(frame);
    if (m_rightWidget)
      m_rightWidget->setEnabled( m_pChildFrameList->count()>1 );
    if (count() == 1)
      hideTabBar();
  }
  else
    kdWarning(1202) << "KonqFrameTabs " << this << ": removeChildFrame(0L) !" << endl;

  //kdDebug(1202) << "KonqFrameTabs::RemoveChildFrame finished" << endl;
}

void KonqFrameTabs::slotCurrentChanged( TQWidget* newPage )
{
  setTabColor( newPage, TDEGlobalSettings::textColor() );
  KonqFrameBase* currentFrame = tqt_dynamic_cast<KonqFrameBase*>(newPage);

  if (currentFrame && !m_pViewManager->isLoadingProfile()) {
    m_pActiveChild = currentFrame;
    currentFrame->activateChild();
  }
}

void KonqFrameTabs::moveTabBackward( int index )
{
  if ( index == 0 )
    return;
  moveTab( index, index-1 );
}

void KonqFrameTabs::moveTabForward( int index )
{
  if ( index == count()-1 )
    return;
  moveTab( index, index+1 );
}

void KonqFrameTabs::slotMovedTab( int from, int to )
{
  KonqFrameBase* fromFrame = m_pChildFrameList->at( from );
  m_pChildFrameList->remove( fromFrame );
  m_pChildFrameList->insert( to, fromFrame );

  KonqFrameBase* currentFrame = tqt_dynamic_cast<KonqFrameBase*>( currentPage() );
  if ( currentFrame && !m_pViewManager->isLoadingProfile() ) {
    m_pActiveChild = currentFrame;
    currentFrame->activateChild();
  }
}

void KonqFrameTabs::slotContextMenu( const TQPoint &p )
{
  refreshSubPopupMenuTab();
  
  m_pPopupMenu->setItemEnabled( RELOAD_ID, false );
  m_pPopupMenu->setItemEnabled( DUPLICATE_ID, false );
  m_pPopupMenu->setItemEnabled( BREAKOFF_ID, false );
  m_pPopupMenu->setItemEnabled( MOVE_LEFT_ID, false );
  // The following line fails to build. Adapted from konq_mainwindow.cc: 4243. Help!
  // m_pPopupMenu->setItemEnabled( MOVE_LEFT_ID, m_pViewManager->mainWindow()->currentView() ? m_pViewManager->mainWindow()->currentView()->frame()!=(TQApplication::reverseLayout() ? childFrameList->last() : childFrameList->first()) : false );
  m_pPopupMenu->setItemEnabled( MOVE_RIGHT_ID, false );
  // The following line fails to build. Adapted from konq_mainwindow.cc: 4245. Help!
  // m_pPopupMenu->setItemEnabled( MOVE_RIGHT_ID, m_pViewManager->mainWindow()->currentView() ? m_pViewManager->mainWindow()->currentView()->frame()!=(TQApplication::reverseLayout() ? childFrameList->first() : childFrameList->last()) : false );
  m_pPopupMenu->setItemEnabled( CLOSETAB_ID, false );
  m_pPopupMenu->setItemEnabled( OTHERTABS_ID, true );
  m_pSubPopupMenuTab->setItemEnabled( m_closeOtherTabsId, false );

  m_pPopupMenu->exec( p );
}

void KonqFrameTabs::slotContextMenu( TQWidget *w, const TQPoint &p )
{
  refreshSubPopupMenuTab();
  
  uint tabCount = m_pChildFrameList->count();
  m_pPopupMenu->setItemEnabled( RELOAD_ID, true );
  m_pPopupMenu->setItemEnabled( DUPLICATE_ID, true );
  m_pPopupMenu->setItemEnabled( BREAKOFF_ID, tabCount>1 );
  m_pPopupMenu->setItemEnabled( MOVE_LEFT_ID, tabCount>1 );
  m_pPopupMenu->setItemEnabled( MOVE_RIGHT_ID, tabCount>1 );
  m_pPopupMenu->setItemEnabled( CLOSETAB_ID, tabCount>1 );
  m_pPopupMenu->setItemEnabled( OTHERTABS_ID, tabCount>1 );
  m_pSubPopupMenuTab->setItemEnabled( m_closeOtherTabsId, true );

  // Yes, I know this is an unchecked tqt_dynamic_cast - I'm casting sideways in a
  // class hierarchy and it could crash one day, but I haven't checked
  // setWorkingTab so I don't know if it can handle nulls.

  m_pViewManager->mainWindow()->setWorkingTab( tqt_dynamic_cast<KonqFrameBase*>(w) );
  m_pPopupMenu->exec( p );
}

void KonqFrameTabs::refreshSubPopupMenuTab()
{
    m_pSubPopupMenuTab->clear();
    int i=0;
    m_pSubPopupMenuTab->insertItem( SmallIcon( "reload_all_tabs" ),
                                    i18n( "&Reload All Tabs" ),
                                    m_pViewManager->mainWindow(),
                                    TQT_SLOT( slotReloadAllTabs() ),
                                    m_pViewManager->mainWindow()->action("reload_all_tabs")->shortcut() );
    m_pSubPopupMenuTab->insertSeparator();
    for (KonqFrameBase* it = m_pChildFrameList->first(); it; it = m_pChildFrameList->next())
    {
        KonqFrame* frame = dynamic_cast<KonqFrame *>(it);
        if ( frame && frame->activeChildView() )
        {
            TQString title = frame->title().stripWhiteSpace();
            if ( title.isEmpty() )
                title = frame->activeChildView()->url().url();
            title = KStringHandler::csqueeze( title, 50 );
            m_pSubPopupMenuTab->insertItem( TQIconSet( KonqPixmapProvider::self()->pixmapFor( frame->activeChildView()->url().url() ) ), title, i );

        }
        i++;
    }
    m_pSubPopupMenuTab->insertSeparator();
    m_closeOtherTabsId =
      m_pSubPopupMenuTab->insertItem( SmallIconSet( "tab_remove_other" ),
				      i18n( "Close &Other Tabs" ),
				      m_pViewManager->mainWindow(),
				      TQT_SLOT( slotRemoveOtherTabsPopup() ),
				      m_pViewManager->mainWindow()->action("removeothertabs")->shortcut() );
}

void KonqFrameTabs::slotCloseRequest( TQWidget *w )
{
  if ( m_pChildFrameList->count() > 1 ) {
    // Yes, I know this is an unchecked tqt_dynamic_cast - I'm casting sideways in a class hierarchy and it could crash one day, but I haven't checked setWorkingTab so I don't know if it can handle nulls.
    m_pViewManager->mainWindow()->setWorkingTab( tqt_dynamic_cast<KonqFrameBase*>(w) );
    emit ( removeTabPopup() );
  }
}

void KonqFrameTabs::slotSubPopupMenuTabActivated( int _id)
{
    setCurrentPage( _id );
}

void KonqFrameTabs::slotMouseMiddleClick()
{
  TQApplication::clipboard()->setSelectionMode( TQClipboard::Selection );
  KURL filteredURL ( KonqMisc::konqFilteredURL( this, TQApplication::clipboard()->text() ) );
  if ( !filteredURL.isEmpty() ) {
    KonqView* newView = m_pViewManager->addTab(TQString::null, TQString::null, false, false);
    if (newView == 0L) return;
    m_pViewManager->mainWindow()->openURL( newView, filteredURL, TQString::null );
    m_pViewManager->showTab( newView );
    m_pViewManager->mainWindow()->focusLocationBar();
  }
}

void KonqFrameTabs::slotMouseMiddleClick( TQWidget *w )
{
  if ( m_MouseMiddleClickClosesTab ) {
    if ( m_pChildFrameList->count() > 1 ) {
      // Yes, I know this is an unchecked tqt_dynamic_cast - I'm casting sideways in a class hierarchy and it could crash one day, but I haven't checked setWorkingTab so I don't know if it can handle nulls.
      m_pViewManager->mainWindow()->setWorkingTab( tqt_dynamic_cast<KonqFrameBase*>(w) );
      emit ( removeTabPopup() );
    }
  }
  else {
  TQApplication::clipboard()->setSelectionMode( TQClipboard::Selection );
  KURL filteredURL ( KonqMisc::konqFilteredURL( this, TQApplication::clipboard()->text() ) );
  if ( !filteredURL.isEmpty() ) {
    KonqFrameBase* frame = tqt_dynamic_cast<KonqFrameBase*>(w);
    if (frame) {
      m_pViewManager->mainWindow()->openURL( frame->activeChildView(), filteredURL );
    }
  }
  }
}

void KonqFrameTabs::slotTestCanDecode(const TQDragMoveEvent *e, bool &accept /* result */)
{
  accept = KURLDrag::canDecode( e );
}

void KonqFrameTabs::slotReceivedDropEvent( TQDropEvent *e )
{
  KURL::List lstDragURLs;
  bool ok = KURLDrag::decode( e, lstDragURLs );
  if ( ok && lstDragURLs.first().isValid() ) {
    KonqView* newView = m_pViewManager->addTab(TQString::null, TQString::null, false, false);
    if (newView == 0L) return;
    m_pViewManager->mainWindow()->openURL( newView, lstDragURLs.first(), TQString::null );
    m_pViewManager->showTab( newView );
    m_pViewManager->mainWindow()->focusLocationBar();
  }
}

void KonqFrameTabs::slotReceivedDropEvent( TQWidget *w, TQDropEvent *e )
{
  KURL::List lstDragURLs;
  bool ok = KURLDrag::decode( e, lstDragURLs );
  KonqFrameBase* frame = tqt_dynamic_cast<KonqFrameBase*>(w);
  if ( ok && lstDragURLs.first().isValid() && frame ) {
    KURL lstDragURL = lstDragURLs.first();
    if ( lstDragURL != frame->activeChildView()->url() )
      m_pViewManager->mainWindow()->openURL( frame->activeChildView(), lstDragURL );
  }
}

void KonqFrameTabs::slotInitiateDrag( TQWidget *w )
{
  KonqFrameBase* frame = tqt_dynamic_cast<KonqFrameBase*>( w );
  if (frame) {
    KURL::List lst;
    lst.append( frame->activeChildView()->url() );
    KURLDrag *d = new KURLDrag( lst, this );
    d->setPixmap( KMimeType::pixmapForURL( lst.first(), 0, TDEIcon::Small ) );
    d->dragCopy();
  }
}

void KonqFrameTabs::hideTabBar()
{
  if ( !m_alwaysTabBar ) {
    setTabBarHidden(true);
  }
  m_pPopupMenu->setItemEnabled( BREAKOFF_ID, false );
  m_pPopupMenu->setItemEnabled( MOVE_LEFT_ID, false );
  m_pPopupMenu->setItemEnabled( MOVE_RIGHT_ID, false );
  m_pPopupMenu->setItemEnabled( CLOSETAB_ID, false );
}

void KonqFrameTabs::setAlwaysTabbedMode( bool enable )
{
  bool update = ( enable != m_alwaysTabBar );

  m_alwaysTabBar = enable;
  if ( update ) {
    if ( m_alwaysTabBar )
      setTabBarHidden(false);
    else
      hideTabBar();
  }
}

#include "konq_tabs.moc"
