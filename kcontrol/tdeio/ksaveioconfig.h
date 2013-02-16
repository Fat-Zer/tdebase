/*
   Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSAVEIO_CONFIG_H_
#define KSAVEIO_CONFIG_H_

#include <tdeprotocolmanager.h>

class TQWidget;
class KSaveIOConfigPrivate;

class KSaveIOConfig
{
public:

  /* Reload config file (tdeioslaverc) */
  static void reparseConfiguration();


  /** Timeout Settings */
  static void setReadTimeout( int );

  static void setConnectTimeout( int );

  static void setProxyConnectTimeout( int );

  static void setResponseTimeout( int );


  /** Cache Settings */
  static void setMaxCacheAge( int );

  static void setUseCache( bool );

  static void setMaxCacheSize( int );

  static void setCacheControl( TDEIO::CacheControl );


  /** Proxy Settings */
  static void setUseReverseProxy( bool );

  static void setProxyType( KProtocolManager::ProxyType );

  static void setProxyAuthMode( KProtocolManager::ProxyAuthMode );

  static void setProxyConfigScript( const TQString&  );

  static void setProxyFor( const TQString&, const TQString&  );

  static void setNoProxyFor( const TQString& );


  /** Miscelaneous Settings */
  static void setMarkPartial( bool );

  static void setMinimumKeepSize( int );

  static void setAutoResume( bool );

  static void setPersistentConnections( bool );

  static void setPersistentProxyConnection( bool );


  /** Update all running io-slaves */
  static void updateRunningIOSlaves (TQWidget * parent = 0L);

  /** Update proxy scout */
  static void updateProxyScout( TQWidget * parent = 0L );

protected:
  static TDEConfig* config ();
  static TDEConfig* http_config ();
  KSaveIOConfig ();

private:
  static KSaveIOConfigPrivate* d;
};
#endif
