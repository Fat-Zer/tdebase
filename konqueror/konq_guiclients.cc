/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Simon Hausmann <hausmann@kde.org>

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

#include <kdebug.h>
#include <tdelocale.h>
#include <tdemenubar.h>
#include "konq_view.h"
#include "konq_settingsxt.h"
#include "konq_frame.h"
#include "konq_guiclients.h"
#include "konq_viewmgr.h"
#include <kiconloader.h>

PopupMenuGUIClient::PopupMenuGUIClient( KonqMainWindow *mainWindow,
                                        const TDETrader::OfferList &embeddingServices,
                                        bool showEmbeddingServices, bool doTabHandling )
{
    //giving a name to each guiclient: just for debugging
    // (needs delete instance() in the dtor if enabled for good)
    //setInstance( new TDEInstance( "PopupMenuGUIClient" ) );

    m_mainWindow = mainWindow;

    m_doc = TQDomDocument( "kpartgui" );
    TQDomElement root = m_doc.createElement( "kpartgui" );
    root.setAttribute( "name", "konqueror" );
    m_doc.appendChild( root );

    TQDomElement menu = m_doc.createElement( "Menu" );
    root.appendChild( menu );
    menu.setAttribute( "name", "popupmenu" );

    if ( !mainWindow->menuBar()->isVisible() )
    {
        TQDomElement showMenuBarElement = m_doc.createElement( "action" );
        showMenuBarElement.setAttribute( "name", "options_show_menubar" );
        menu.appendChild( showMenuBarElement );

        menu.appendChild( m_doc.createElement( "separator" ) );
    }

    if ( mainWindow->fullScreenMode() )
    {
        TQDomElement stopFullScreenElement = m_doc.createElement( "action" );
        stopFullScreenElement.setAttribute( "name", "fullscreen" );
        menu.appendChild( stopFullScreenElement );

        menu.appendChild( m_doc.createElement( "separator" ) );
    }

    if ( showEmbeddingServices )
    {
        TDETrader::OfferList::ConstIterator it = embeddingServices.begin();
        TDETrader::OfferList::ConstIterator end = embeddingServices.end();

        if ( embeddingServices.count() == 1 )
        {
            KService::Ptr service = *embeddingServices.begin();
            addEmbeddingService( menu, 0, i18n( "Preview in %1" ).arg( service->name() ), service );
        }
        else if ( embeddingServices.count() > 1 )
        {
            int idx = 0;
            TQDomElement subMenu = m_doc.createElement( "menu" );
            menu.appendChild( subMenu );
            TQDomElement text = m_doc.createElement( "text" );
            subMenu.appendChild( text );
            text.appendChild( m_doc.createTextNode( i18n( "Preview In" ) ) );
            subMenu.setAttribute( "group", "preview" );
            subMenu.setAttribute( "name", "preview submenu" );

            bool inserted = false;

            for (; it != end; ++it, ++idx )
            {
                addEmbeddingService( subMenu, idx, (*it)->name(), *it );
                inserted = true;
            }

            if ( !inserted ) // oops, if empty then remove the menu :-]
                menu.removeChild( menu.namedItem( "menu" ) );
        }
    }

    if ( doTabHandling )
    {
        TQDomElement openInSameWindow = m_doc.createElement( "action" );
        openInSameWindow.setAttribute( "name", "sameview" );
        openInSameWindow.setAttribute( "group", "tabhandling" );
        menu.appendChild( openInSameWindow );

        TQDomElement openInWindow = m_doc.createElement( "action" );
        openInWindow.setAttribute( "name", "newview" );
        openInWindow.setAttribute( "group", "tabhandling" );
        menu.appendChild( openInWindow );

        TQDomElement openInTabElement = m_doc.createElement( "action" );
        openInTabElement.setAttribute( "name", "openintab" );
        openInTabElement.setAttribute( "group", "tabhandling" );
        menu.appendChild( openInTabElement );

        TQDomElement openInTabFrontElement = m_doc.createElement( "action" );
        openInTabFrontElement.setAttribute( "name", "openintabfront" );
        openInTabFrontElement.setAttribute( "group", "tabhandling" );
        menu.appendChild( openInTabFrontElement );

        TQDomElement separatorElement = m_doc.createElement( "separator" );
        separatorElement.setAttribute( "group", "tabhandling" );
        menu.appendChild( separatorElement );
    }

    //kdDebug() << k_funcinfo << m_doc.toString() << endl;

    setDOMDocument( m_doc );
}

PopupMenuGUIClient::~PopupMenuGUIClient()
{
}

TDEAction *PopupMenuGUIClient::action( const TQDomElement &element ) const
{
  TDEAction *res = KXMLGUIClient::action( element );

  if ( !res )
    res = m_mainWindow->action( element );

  return res;
}

void PopupMenuGUIClient::addEmbeddingService( TQDomElement &menu, int idx, const TQString &name, const KService::Ptr &service )
{
  TQDomElement action = m_doc.createElement( "action" );
  menu.appendChild( action );

  TQCString actName;
  actName.setNum( idx );

  action.setAttribute( "name", TQString::number( idx ) );

  action.setAttribute( "group", "preview" );

  (void)new TDEAction( name, service->pixmap( TDEIcon::Small ), 0,
                     TQT_TQOBJECT(m_mainWindow), TQT_SLOT( slotOpenEmbedded() ), actionCollection(), actName );
}

ToggleViewGUIClient::ToggleViewGUIClient( KonqMainWindow *mainWindow )
: TQObject( mainWindow )
{
  m_mainWindow = mainWindow;
  m_actions.setAutoDelete( true );

  TDETrader::OfferList offers = TDETrader::self()->query( "Browser/View" );
  TDETrader::OfferList::Iterator it = offers.begin();
  while ( it != offers.end() )
  {
    TQVariant prop = (*it)->property( "X-TDE-BrowserView-Toggable" );
    TQVariant orientation = (*it)->property( "X-TDE-BrowserView-ToggableView-Orientation" );

    if ( !prop.isValid() || !prop.toBool() ||
         !orientation.isValid() || orientation.toString().isEmpty() )
    {
      offers.remove( it );
      it = offers.begin();
    }
    else
      ++it;
  }

  m_empty = ( offers.count() == 0 );

  if ( m_empty )
    return;

  TDETrader::OfferList::ConstIterator cIt = offers.begin();
  TDETrader::OfferList::ConstIterator cEnd = offers.end();
  for (; cIt != cEnd; ++cIt )
  {
    TQString description = i18n( "Show %1" ).arg( (*cIt)->name() );
    TQString name = (*cIt)->desktopEntryName();
    //kdDebug(1202) << "ToggleViewGUIClient: name=" << name << endl;
    TDEToggleAction *action = new TDEToggleAction( description, 0, mainWindow->actionCollection(), name.latin1() );
    action->setCheckedState( i18n( "Hide %1" ).arg( (*cIt)->name() ) );

    // HACK
    if ( (*cIt)->icon() != "unknown" )
      action->setIcon( (*cIt)->icon() );

    connect( action, TQT_SIGNAL( toggled( bool ) ),
             this, TQT_SLOT( slotToggleView( bool ) ) );

    m_actions.insert( name, action );

    TQVariant orientation = (*cIt)->property( "X-TDE-BrowserView-ToggableView-Orientation" );
    bool horizontal = orientation.toString().lower() == "horizontal";
    m_mapOrientation.insert( name, horizontal );
  }

  connect( m_mainWindow, TQT_SIGNAL( viewAdded( KonqView * ) ),
           this, TQT_SLOT( slotViewAdded( KonqView * ) ) );
  connect( m_mainWindow, TQT_SIGNAL( viewRemoved( KonqView * ) ),
           this, TQT_SLOT( slotViewRemoved( KonqView * ) ) );
}

ToggleViewGUIClient::~ToggleViewGUIClient()
{
}

TQPtrList<TDEAction> ToggleViewGUIClient::actions() const
{
  TQPtrList<TDEAction> res;

  TQDictIterator<TDEAction> it( m_actions );
  for (; it.current(); ++it )
    res.append( it.current() );

  return res;
}

void ToggleViewGUIClient::slotToggleView( bool toggle )
{
  TQString serviceName = TQString::fromLatin1( TQT_TQOBJECT_CONST(sender())->name() );

  bool horizontal = m_mapOrientation[ serviceName ];

  KonqViewManager *viewManager = m_mainWindow->viewManager();

  if ( toggle )
  {

    KonqView *childView = viewManager->splitWindow( horizontal ? Qt::Vertical : Qt::Horizontal,
                                                    TQString::fromLatin1( "Browser/View" ),
                                                    serviceName,
                                                    !horizontal /* vertical = make it first */);

    TQValueList<int> newSplitterSizes;

    if ( horizontal )
      newSplitterSizes << 100 << 30;
    else
      newSplitterSizes << 30 << 100;

    if (!childView || !childView->frame())
        return;

    // Toggleviews don't need their statusbar
    childView->frame()->statusbar()->hide();

    KonqFrameContainerBase *newContainer = childView->frame()->parentContainer();

    if (newContainer->frameType()=="Container")
      static_cast<KonqFrameContainer*>(newContainer)->setSizes( newSplitterSizes );

#if 0 // already done by splitWindow
    if ( m_mainWindow->currentView() )
    {
        TQString locBarURL = m_mainWindow->currentView()->url().prettyURL(); // default one in case it doesn't set it
        childView->openURL( m_mainWindow->currentView()->url(), locBarURL );
    }
#endif

    // If not passive, set as active :)
    if (!childView->isPassiveMode())
      viewManager->setActivePart( childView->part() );

    kdDebug() << "ToggleViewGUIClient::slotToggleView setToggleView(true) on " << childView << endl;
    childView->setToggleView( true );

    m_mainWindow->viewCountChanged();

  }
  else
  {
    TQPtrList<KonqView> viewList;

    m_mainWindow->listViews( &viewList );

    TQPtrListIterator<KonqView> it( viewList );
    for (; it.current(); ++it )
      if ( it.current()->service()->desktopEntryName() == serviceName )
        // takes care of choosing the new active view, and also calls slotViewRemoved
        viewManager->removeView( it.current() );
  }

}


void ToggleViewGUIClient::saveConfig( bool add, const TQString &serviceName )
{
  // This is used on konqueror's startup....... so it's never used, since
  // the TDE menu only contains calls to kfmclient......
  // Well, it could be useful again in the future.
  // Currently, the profiles save this info.
  TQStringList toggableViewsShown = KonqSettings::toggableViewsShown();
  if (add)
  {
      if ( !toggableViewsShown.contains( serviceName ) )
          toggableViewsShown.append(serviceName);
  }
  else
      toggableViewsShown.remove(serviceName);
  KonqSettings::setToggableViewsShown( toggableViewsShown );
}

void ToggleViewGUIClient::slotViewAdded( KonqView *view )
{
  TQString name = view->service()->desktopEntryName();

  TDEAction *action = m_actions[ name ];

  if ( action )
  {
    static_cast<TDEToggleAction *>( action )->setChecked( true );
    saveConfig( true, name );

    // KonqView::isToggleView() is not set yet.. so just check for the orientation

#if 0
    TQVariant vert = view->service()->property( "X-TDE-BrowserView-ToggableView-Orientation");
    bool vertical = vert.toString().lower() == "vertical";
    TQVariant nohead = view->service()->property( "X-TDE-BrowserView-ToggableView-NoHeader");
    bool noheader = nohead.isValid() ? nohead.toBool() : false;
    // if it is a vertical toggle part, turn on the header.
    // this works even when konq loads the view from a profile.
    if ( vertical && (!noheader))
    {
        view->frame()->header()->setText(view->service()->name());
        view->frame()->header()->setAction(action);
    }
#endif
  }
}

void ToggleViewGUIClient::slotViewRemoved( KonqView *view )
{
  TQString name = view->service()->desktopEntryName();

  TDEAction *action = m_actions[ name ];

  if ( action )
  {
    static_cast<TDEToggleAction *>( action )->setChecked( false );
    saveConfig( false, name );
  }
}

#include "konq_guiclients.moc"
