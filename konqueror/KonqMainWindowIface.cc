/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "KonqMainWindowIface.h"
#include "KonqViewIface.h"
#include "konq_view.h"


#include <dcopclient.h>
#include <tdeapplication.h>
#include <kdcopactionproxy.h>
#include <kdcoppropertyproxy.h>
#include <kdebug.h>
#include <tdestartupinfo.h>
#include <twin.h>

KonqMainWindowIface::KonqMainWindowIface( KonqMainWindow * mainWindow )
    :
    // ARGL I hate this "virtual public DCOPObject" stuff!
    DCOPObject( mainWindow->name() ),
    TDEMainWindowInterface( mainWindow ), m_pMainWindow( mainWindow )
{
  m_dcopActionProxy = new KDCOPActionProxy( mainWindow->actionCollection(), this );
}

KonqMainWindowIface::~KonqMainWindowIface()
{
  delete m_dcopActionProxy;
}

void KonqMainWindowIface::openURL( TQString url )
{
  m_pMainWindow->openFilteredURL( url );
}

void KonqMainWindowIface::newTab( TQString url )
{
  m_pMainWindow->openFilteredURL( url, true );
}

void KonqMainWindowIface::openURL( TQString url, bool tempFile )
{
  m_pMainWindow->openFilteredURL( url, false, tempFile );
}

void KonqMainWindowIface::newTab( TQString url, bool tempFile )
{
  m_pMainWindow->openFilteredURL( url, true, tempFile );
}

void KonqMainWindowIface::newTabASN( TQString url, const TQCString& startup_id, bool tempFile )
{
  TDEStartupInfo::setNewStartupId( m_pMainWindow, startup_id );
  m_pMainWindow->openFilteredURL( url, true, tempFile );
}


void KonqMainWindowIface::splitViewHorizontally()
{
  m_pMainWindow->slotSplitViewHorizontal();
}

void KonqMainWindowIface::splitViewVertically()
{
  m_pMainWindow->slotSplitViewVertical();
}

void KonqMainWindowIface::reload()
{
  m_pMainWindow->slotReload();
}

DCOPRef KonqMainWindowIface::currentView()
{
  KonqView *view = m_pMainWindow->currentView();
  if ( !view )
    return DCOPRef();

  return DCOPRef( kapp->dcopClient()->appId(), view->dcopObject()->objId() );
}

DCOPRef KonqMainWindowIface::currentPart()
{
  DCOPRef res;

  KonqView *view = m_pMainWindow->currentView();
  if ( !view )
    return res;

  return view->dcopObject()->part();
}

DCOPRef KonqMainWindowIface::view(int viewNumber)
{
  KonqMainWindow::MapViews viewMap = m_pMainWindow->viewMap();
  KonqMainWindow::MapViews::const_iterator it = viewMap.begin();
  for ( int i = 0; it != viewMap.end() && i < viewNumber; ++i )
      ++it;
  if ( it == viewMap.end() )
      return DCOPRef();
  return DCOPRef( kapp->dcopClient()->appId(), (*it)->dcopObject()->objId() );
}

DCOPRef KonqMainWindowIface::part(int viewNumber)
{
  KonqMainWindow::MapViews viewMap = m_pMainWindow->viewMap();
  KonqMainWindow::MapViews::const_iterator it = viewMap.begin();
  for ( int i = 0; it != viewMap.end() && i < viewNumber; ++i )
      ++it;
  if ( it == viewMap.end() )
      return DCOPRef();
  return (*it)->dcopObject()->part();
}

DCOPRef KonqMainWindowIface::action( const TQCString &name )
{
  return DCOPRef( kapp->dcopClient()->appId(), m_dcopActionProxy->actionObjectId( name ) );
}

QCStringList KonqMainWindowIface::actions()
{
  QCStringList res;
  TQValueList<TDEAction *> lst = m_dcopActionProxy->actions();
  TQValueList<TDEAction *>::ConstIterator it = lst.begin();
  TQValueList<TDEAction *>::ConstIterator end = lst.end();
  for (; it != end; ++it )
    res.append( (*it)->name() );

  return res;
}

TQMap<TQCString,DCOPRef> KonqMainWindowIface::actionMap()
{
  return m_dcopActionProxy->actionMap();
}

QCStringList KonqMainWindowIface::functionsDynamic()
{
    return DCOPObject::functionsDynamic() + KDCOPPropertyProxy::functions( TQT_TQOBJECT(m_pMainWindow) );
}

bool KonqMainWindowIface::processDynamic( const TQCString &fun, const TQByteArray &data, TQCString &replyType, TQByteArray &replyData )
{
    if ( KDCOPPropertyProxy::isPropertyRequest( fun, TQT_TQOBJECT(m_pMainWindow) ) )
        return KDCOPPropertyProxy::processPropertyRequest( fun, data, replyType, replyData, TQT_TQOBJECT(m_pMainWindow) );

    return DCOPObject::processDynamic( fun, data, replyType, replyData );
}

bool KonqMainWindowIface::windowCanBeUsedForTab()
{
    KWin::WindowInfo winfo = KWin::windowInfo( m_pMainWindow->winId(), NET::WMDesktop );
    if( !winfo.isOnCurrentDesktop() )
        return false; // this window shows on different desktop
    if( KonqMainWindow::isPreloaded() )
        return false; // we want a tab in an already shown window
    return true;
}
