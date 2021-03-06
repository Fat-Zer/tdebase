/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include "konq_settings.h"
#include "konq_defaults.h"
#include "tdeglobalsettings.h"
#include <tdeglobal.h>
#include <kservicetype.h>
#include <kdesktopfile.h>
#include <kdebug.h>
#include <assert.h>
#include <tqfontmetrics.h>

struct KonqFMSettingsPrivate
{
    KonqFMSettingsPrivate() {
        showPreviewsInFileTips = true;
        m_renameIconDirectly = false;
    }

    bool showPreviewsInFileTips;
    bool m_renameIconDirectly;
    bool localeAwareCompareIsCaseSensitive;
    int m_iconTextWidth;
};

//static
KonqFMSettings * KonqFMSettings::s_pSettings = 0L;

//static
KonqFMSettings * KonqFMSettings::settings()
{
  if (!s_pSettings)
  {
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cgs(config, "FMSettings");
    s_pSettings = new KonqFMSettings(config);
  }
  return s_pSettings;
}

//static
void KonqFMSettings::reparseConfiguration()
{
  if (s_pSettings)
  {
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cgs(config, "FMSettings");
    s_pSettings->init( config );
  }
}

KonqFMSettings::KonqFMSettings( TDEConfig * config )
{
  d = new KonqFMSettingsPrivate;
  init( config );
}

KonqFMSettings::~KonqFMSettings()
{
  delete d;
}

void KonqFMSettings::init( TDEConfig * config )
{
  // Fonts and colors
  m_standardFont = config->readFontEntry( "StandardFont" );

  m_normalTextColor = TDEGlobalSettings::textColor();
  m_normalTextColor = config->readColorEntry( "NormalTextColor", &m_normalTextColor );
  m_highlightedTextColor = TDEGlobalSettings::highlightedTextColor();
  m_highlightedTextColor = config->readColorEntry( "HighlightedTextColor", &m_highlightedTextColor );
  m_itemTextBackground = config->readColorEntry( "ItemTextBackground" );

  d->m_iconTextWidth = config->readNumEntry( "TextWidth", DEFAULT_TEXTWIDTH );
  if ( d->m_iconTextWidth == DEFAULT_TEXTWIDTH )
    d->m_iconTextWidth = TQFontMetrics(m_standardFont).width("0000000000");

  m_iconTextHeight = config->readNumEntry( "TextHeight", 0 );
  if ( m_iconTextHeight == 0 ) {
    if ( config->readBoolEntry( "WordWrapText", true ) )
      m_iconTextHeight = DEFAULT_TEXTHEIGHT;
    else
      m_iconTextHeight = 1;
  }
  m_bWordWrapText = ( m_iconTextHeight > 1 );

  m_underlineLink = config->readBoolEntry( "UnderlineLinks", DEFAULT_UNDERLINELINKS );
  d->m_renameIconDirectly = config->readBoolEntry( "RenameIconDirectly", DEFAULT_RENAMEICONDIRECTLY );
  m_fileSizeInBytes = config->readBoolEntry( "DisplayFileSizeInBytes", DEFAULT_FILESIZEINBYTES );
  m_iconTransparency = config->readNumEntry( "TextpreviewIconOpacity", DEFAULT_TEXTPREVIEW_ICONTRANSPARENCY );
  if ( m_iconTransparency < 0 || m_iconTransparency > 255 )
      m_iconTransparency = DEFAULT_TEXTPREVIEW_ICONTRANSPARENCY;

  // Behaviour
  m_alwaysNewWin = config->readBoolEntry( "AlwaysNewWin", FALSE );

  m_homeURL = config->readPathEntry("HomeURL", "~");

  m_showFileTips = config->readBoolEntry("ShowFileTips", true);
  d->showPreviewsInFileTips = config->readBoolEntry("ShowPreviewsInFileTips", true);
  m_numFileTips = config->readNumEntry("FileTipsItems", 6);

  m_embedMap = config->entryMap( "EmbedSettings" );

  /// true if TQString::localeAwareCompare is case sensitive (it usually isn't, when LC_COLLATE is set)
  d->localeAwareCompareIsCaseSensitive = TQString( "a" ).localeAwareCompare( "B" ) > 0; // see #40131
}

bool KonqFMSettings::shouldEmbed( const TQString & serviceType ) const
{
    // First check in user's settings whether to embed or not
    // 1 - in the mimetype file itself
    KServiceType::Ptr serviceTypePtr = KServiceType::serviceType( serviceType );
    bool hasLocalProtocolRedirect = false;
    if ( serviceTypePtr )
    {
        hasLocalProtocolRedirect = !serviceTypePtr->property( "X-TDE-LocalProtocol" ).toString().isEmpty();
        TQVariant autoEmbedProp = serviceTypePtr->property( "X-TDE-AutoEmbed" );
        if ( autoEmbedProp.isValid() )
        {
            bool autoEmbed = autoEmbedProp.toBool();
            kdDebug(1203) << "X-TDE-AutoEmbed set to " << (autoEmbed ? "true" : "false") << endl;
            return autoEmbed;
        } else
            kdDebug(1203) << "No X-TDE-AutoEmbed, looking for group" << endl;
    }
    // 2 - in the configuration for the group if nothing was found in the mimetype
    TQString serviceTypeGroup = serviceType.left(serviceType.find("/"));
    kdDebug(1203) << "KonqFMSettings::shouldEmbed : serviceTypeGroup=" << serviceTypeGroup << endl;
    if ( serviceTypeGroup == "inode" || serviceTypeGroup == "Browser" || serviceTypeGroup == "Konqueror" )
        return true; //always embed mimetype inode/*, Browser/* and Konqueror/*
    TQMap<TQString, TQString>::ConstIterator it = m_embedMap.find( TQString::fromLatin1("embed-")+serviceTypeGroup );
    if ( it != m_embedMap.end() ) {
        kdDebug(1203) << "KonqFMSettings::shouldEmbed: " << it.data() << endl;
        return it.data() == TQString::fromLatin1("true");
    }
    // 3 - if no config found, use default.
    // Note: if you change those defaults, also change kcontrol/filetypes/typeslistitem.cpp !
    // Embedding is false by default except for image/* and for zip, tar etc.
    if ( serviceTypeGroup == "image" || hasLocalProtocolRedirect )
        return true;
    return false;
}

bool KonqFMSettings::showPreviewsInFileTips() const
{
    return d->showPreviewsInFileTips;
}

bool KonqFMSettings::renameIconDirectly() const
{
    return d->m_renameIconDirectly;
}

int KonqFMSettings::caseSensitiveCompare( const TQString& a, const TQString& b ) const
{
    if ( d->localeAwareCompareIsCaseSensitive ) {
        return a.localeAwareCompare( b );
    }
    else // can't use localeAwareCompare, have to fallback to normal TQString compare
        return a.compare( b );
}

int KonqFMSettings::iconTextWidth() const
{
    return d->m_iconTextWidth;
}
