/*  This file is part of the KDE project
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 1999 David Faure <faure@kde.org>
    Copyright (C) 1999 Torben Weis <weis@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef __konq_factory_h__
#define __konq_factory_h__

#include <tqstring.h>
#include <tqstringlist.h>

#include <klibloader.h>
#include <ktrader.h>

#include <tdeparts/part.h>

class BrowserView;
class KonqMainWindow;
class TDEAboutData;

class KonqViewFactory
{
public:
  KonqViewFactory() : m_factory( 0L ), m_createBrowser( false ) {}

  KonqViewFactory( KLibFactory *factory, const TQStringList &args, bool createBrowser );

  KonqViewFactory( const KonqViewFactory &factory )
  { (*this) = factory; }

  KonqViewFactory &operator=( const KonqViewFactory &other )
  {
    m_factory = other.m_factory;
    m_args = other.m_args;
    m_createBrowser = other.m_createBrowser;
    return *this;
  }

  KParts::ReadOnlyPart *create( TQWidget *parentWidget, const char *widgetName,
                                TQObject *parent, const char *name );

  bool isNull() const { return m_factory ? false : true; }

private:
  KLibFactory *m_factory;
  TQStringList m_args;
  bool m_createBrowser;
};

class KonqFactory
{
public:
  static KonqViewFactory createView( const TQString &serviceType,
				     const TQString &serviceName = TQString::null,
				     KService::Ptr *serviceImpl = 0,
				     TDETrader::OfferList *partServiceOffers = 0,
				     TDETrader::OfferList *appServiceOffers = 0,
				     bool forceAutoEmbed = false );

  static void getOffers( const TQString & serviceType,
                         TDETrader::OfferList *partServiceOffers = 0,
                         TDETrader::OfferList *appServiceOffers = 0);

  static const TDEAboutData* aboutData();

private:
  static TDEAboutData *s_aboutData;
};

#endif
