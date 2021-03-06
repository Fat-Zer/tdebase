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

#include "KonqViewIface.h"
#include "konq_view.h"

#include <tdeapplication.h>
#include <dcopclient.h>
#include <kdebug.h>

KonqViewIface::KonqViewIface( KonqView * view, const TQCString& name )
    : DCOPObject( name ), m_pView ( view )
{
}

KonqViewIface::~KonqViewIface()
{
}

void KonqViewIface::openURL( TQString url, const TQString & locationBarURL, const TQString & nameFilter )
{
  KURL u(url);
  m_pView->openURL( u, locationBarURL, nameFilter );
}

bool KonqViewIface::changeViewMode( const TQString &serviceType,
                                    const TQString &serviceName )
{
  return m_pView->changeViewMode( serviceType, serviceName );
}

void KonqViewIface::lockHistory()

{
  m_pView->lockHistory();
}

void KonqViewIface::stop()
{
  m_pView->stop();
}

TQString KonqViewIface::url()
{
  return m_pView->url().url();
}

TQString KonqViewIface::locationBarURL()
{
  return m_pView->locationBarURL();
}

TQString KonqViewIface::serviceType()
{
  return m_pView->serviceType();
}

TQStringList KonqViewIface::serviceTypes()
{
  return m_pView->serviceTypes();
}

DCOPRef KonqViewIface::part()
{
  DCOPRef res;

  KParts::ReadOnlyPart *part = m_pView->part();

  if ( !part )
    return res;

  TQVariant dcopProperty = part->property( "dcopObjectId" );

  if ( dcopProperty.type() != TQVariant::CString )
    return res;

  res.setRef( kapp->dcopClient()->appId(), dcopProperty.toCString() );
  return res;
}

void KonqViewIface::enablePopupMenu( bool b )
{
  m_pView->enablePopupMenu( b );
}

uint KonqViewIface::historyLength()const
{
    return m_pView->historyLength();
}

bool KonqViewIface::allowHTML() const
{
    return m_pView->allowHTML();
}

void KonqViewIface::goForward()
{
    m_pView->go(-1);
}

void KonqViewIface::goBack()
{
    m_pView->go(+1);
}

bool KonqViewIface::isPopupMenuEnabled() const
{
    return m_pView->isPopupMenuEnabled();
}

bool KonqViewIface::canGoBack()const
{
    return m_pView->canGoBack();
}

bool KonqViewIface::canGoForward()const
{
    return m_pView->canGoForward();
}

void KonqViewIface::reload()
{
    return m_pView->mainWindow()->slotReload( m_pView );
}
