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

#include <dcopref.h>
#include <tdeconfig.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstaticdeleter.h>
#include <tdeio/ioslave_defaults.h>

#include "ksaveioconfig.h"

class KSaveIOConfigPrivate
{
public:
  KSaveIOConfigPrivate ();
  ~KSaveIOConfigPrivate ();

  TDEConfig* config;
  TDEConfig* http_config;
};

static KSaveIOConfigPrivate *ksiocpref = 0;
static KStaticDeleter<KSaveIOConfigPrivate> ksiocp;

KSaveIOConfigPrivate::KSaveIOConfigPrivate (): config(0), http_config(0)
{
  ksiocp.setObject (ksiocpref, this);
}

KSaveIOConfigPrivate::~KSaveIOConfigPrivate ()
{
  delete config;
}

KSaveIOConfigPrivate* KSaveIOConfig::d = 0;

TDEConfig* KSaveIOConfig::config()
{
  if (!d)
     d = new KSaveIOConfigPrivate;

  if (!d->config)
     d->config = new TDEConfig("tdeioslaverc", false, false);

  return d->config;
}

TDEConfig* KSaveIOConfig::http_config()
{
  if (!d)
     d = new KSaveIOConfigPrivate;

  if (!d->http_config)
     d->http_config = new TDEConfig("tdeio_httprc", false, false);

  return d->http_config;
}

void KSaveIOConfig::reparseConfiguration ()
{
  delete d->config;
  d->config = 0;
}

void KSaveIOConfig::setReadTimeout( int _timeout )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry("ReadTimeout", TQMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KSaveIOConfig::setConnectTimeout( int _timeout )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry("ConnectTimeout", TQMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KSaveIOConfig::setProxyConnectTimeout( int _timeout )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry("ProxyConnectTimeout", TQMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KSaveIOConfig::setResponseTimeout( int _timeout )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry("ResponseTimeout", TQMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}


void KSaveIOConfig::setMarkPartial( bool _mode )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry( "MarkPartial", _mode );
  cfg->sync();
}

void KSaveIOConfig::setMinimumKeepSize( int _size )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry( "MinimumKeepSize", _size );
  cfg->sync();
}

void KSaveIOConfig::setAutoResume( bool _mode )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry( "AutoResume", _mode );
  cfg->sync();
}

void KSaveIOConfig::setUseCache( bool _mode )
{
  TDEConfig* cfg = http_config ();
  cfg->writeEntry( "UseCache", _mode );
  cfg->sync();
}

void KSaveIOConfig::setMaxCacheSize( int cache_size )
{
  TDEConfig* cfg = http_config ();
  cfg->writeEntry( "MaxCacheSize", cache_size );
  cfg->sync();
}

void KSaveIOConfig::setCacheControl(TDEIO::CacheControl policy)
{
  TDEConfig* cfg = http_config ();
  TQString tmp = TDEIO::getCacheControlString(policy);
  cfg->writeEntry("cache", tmp);
  cfg->sync();
}

void KSaveIOConfig::setMaxCacheAge( int cache_age )
{
  TDEConfig* cfg = http_config ();
  cfg->writeEntry( "MaxCacheAge", cache_age );
  cfg->sync();
}

void KSaveIOConfig::setUseReverseProxy( bool mode )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry("ReversedException", mode);
  cfg->sync();
}

void KSaveIOConfig::setProxyType(KProtocolManager::ProxyType type)
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "ProxyType", static_cast<int>(type) );
  cfg->sync();
}

void KSaveIOConfig::setProxyAuthMode(KProtocolManager::ProxyAuthMode mode)
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "AuthMode", static_cast<int>(mode) );
  cfg->sync();
}

void KSaveIOConfig::setNoProxyFor( const TQString& _noproxy )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "NoProxyFor", _noproxy );
  cfg->sync();
}

void KSaveIOConfig::setProxyFor( const TQString& protocol,
                                 const TQString& _proxy )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( protocol.lower() + "Proxy", _proxy );
  cfg->sync();
}

void KSaveIOConfig::setProxyConfigScript( const TQString& _url )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "Proxy Config Script", _url );
  cfg->sync();
}

void KSaveIOConfig::setPersistentProxyConnection( bool enable )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry( "PersistentProxyConnection", enable );
  cfg->sync();
}

void KSaveIOConfig::setPersistentConnections( bool enable )
{
  TDEConfig* cfg = config ();
  cfg->setGroup( TQString() );
  cfg->writeEntry( "PersistentConnections", enable );
  cfg->sync();
}

void KSaveIOConfig::updateRunningIOSlaves (TQWidget *parent)
{
  // Inform all running io-slaves about the changes...
  // if we cannot update, ioslaves inform the end user...
  if (!DCOPRef("*", "TDEIO::Scheduler").send("reparseSlaveConfiguration", TQString()))
  {
    TQString caption = i18n("Update Failed");
    TQString message = i18n("You have to restart the running applications "
                           "for these changes to take effect.");
    KMessageBox::information (parent, message, caption);
    return;
  }
}

void KSaveIOConfig::updateProxyScout( TQWidget * parent )
{
  // Inform the proxyscout kded module about changes
  // if we cannot update, ioslaves inform the end user...
  if (!DCOPRef("kded", "proxyscout").send("reset"))
  {
    TQString caption = i18n("Update Failed");
    TQString message = i18n("You have to restart TDE "
                           "for these changes to take effect.");
    KMessageBox::information (parent, message, caption);
    return;
  }
}

