/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Faure David <faure@kde.org>

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

#include "konq_propsview.h"
#include "konq_settings.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kpixmap.h>
#include <tqpixmapcache.h>
#include <tqiconview.h>
#include <unistd.h>
#include <tqfile.h>
#include <iostream>
#include <ktrader.h>
#include <kinstance.h>
#include <assert.h>

#include <ksimpleconfig.h>

static TQPixmap wallpaperPixmap( const TQString & _wallpaper )
{
    TQString key = "wallpapers/";
    key += _wallpaper;
    KPixmap pix;

    if ( TQPixmapCache::find( key, pix ) )
      return pix;

    TQString path = locate("tiles", _wallpaper);
    if (path.isEmpty())
        path = locate("wallpaper", _wallpaper);
    if (!path.isEmpty())
    {
      // This looks really ugly, especially on an 8bit display.
      // I'm not sure what it's good for.
      // Anyway, if you change it here, keep konq_bgnddlg in sync (David)
      // pix.load( path, 0, KPixmap::LowColor );
      pix.load( path );
      if ( pix.isNull() )
        kdWarning(1203) << "Could not load wallpaper " << path << endl;
      else
        TQPixmapCache::insert( key, pix );
      return pix;
    } else kdWarning(1203) << "Couldn't locate wallpaper " << _wallpaper << endl;
    return TQPixmap();
}

struct KonqPropsView::Private
{
   TQStringList* previewsToShow;
   bool previewsEnabled:1;
   bool caseInsensitiveSort:1;
   bool dirsfirst:1;
   bool descending:1;
   TQString sortcriterion;
};

KonqPropsView::KonqPropsView( TDEInstance * instance, KonqPropsView * defaultProps )
    : m_bSaveViewPropertiesLocally( false ), // will be overridden by setSave... anyway
    // if this is the default properties instance, then keep config object for saving
    m_dotDirExists( true ), // HACK so that enterDir returns true initially
    m_currentConfig( defaultProps ? 0L : instance->config() ),
    m_defaultProps( defaultProps )
{

  TDEConfig *config = instance->config();
  TDEConfigGroupSaver cgs(config, "Settings");

  d = new Private;
  d->previewsToShow = 0;
  d->caseInsensitiveSort=config->readBoolEntry( "CaseInsensitiveSort", true );

  m_iIconSize = config->readNumEntry( "IconSize", 0 );
  m_iItemTextPos = config->readNumEntry( "ItemTextPos", TQIconView::Bottom );
  d->sortcriterion = config->readEntry( "SortingCriterion", "sort_nci" );
  d->dirsfirst = config->readBoolEntry( "SortDirsFirst", true );
  d->descending = config->readBoolEntry( "SortDescending", false );
  m_bShowDot = config->readBoolEntry( "ShowDotFiles", false );
  m_bShowDirectoryOverlays = config->readBoolEntry( "ShowDirectoryOverlays", false );
  m_bShowFreeSpaceOverlays = config->readBoolEntry( "ShowFreeSpaceOverlays", true );

  m_dontPreview = config->readListEntry( "DontPreview" );
  m_dontPreview.remove("audio/"); //Use the separate setting.
  //We default to this off anyway, so it's no harm to remove this

  //The setting for sound previews is stored separately, so we can force
  //the default-to-off bias to propagate up.
  if (!config->readBoolEntry("EnableSoundPreviews", false))
  {
    if (!m_dontPreview.contains("audio/"))
      m_dontPreview.append("audio/");
  }

  d->previewsEnabled = config->readBoolEntry( "PreviewsEnabled", true );

  TQColor tc = KonqFMSettings::settings()->normalTextColor();
  m_textColor = config->readColorEntry( "TextColor", &tc );
  m_bgColor = config->readColorEntry( "BgColor" ); // will be set to TQColor() if not found
  m_bgPixmapFile = config->readPathEntry( "BgImage" );
  //kdDebug(1203) << "KonqPropsView::KonqPropsView from \"config\" : BgImage=" << m_bgPixmapFile << endl;

  // colorsConfig is either the local file (.directory) or the application global file
  // (we want the same colors for all types of view)
  // The code above reads from the view's config file, for compatibility only.
  // So now we read the settings from the app global file, if this is the default props
  if (!defaultProps)
  {
      TDEConfigGroupSaver cgs2(TDEGlobal::config(), "Settings");
      m_textColor = TDEGlobal::config()->readColorEntry( "TextColor", &m_textColor );
      m_bgColor = TDEGlobal::config()->readColorEntry( "BgColor", &m_bgColor );
      m_bgPixmapFile = TDEGlobal::config()->readPathEntry( "BgImage", m_bgPixmapFile );
      //kdDebug(1203) << "KonqPropsView::KonqPropsView from TDEGlobal : BgImage=" << m_bgPixmapFile << endl;
  }

  TDEGlobal::dirs()->addResourceType("tiles",
                                   TDEGlobal::dirs()->kde_default("data") + "konqueror/tiles/");
}

bool KonqPropsView::isCaseInsensitiveSort() const
{
   return d->caseInsensitiveSort;
}

bool KonqPropsView::isDirsFirst() const
{
   return d->dirsfirst;
}

bool KonqPropsView::isDescending() const
{
   return d->descending;
}

TDEConfigBase * KonqPropsView::currentConfig()
{
    if ( !m_currentConfig )
    {
        // 0L ? This has to be a non-default save-locally instance...
        assert ( m_bSaveViewPropertiesLocally );
        assert ( !isDefaultProperties() );

        if (!dotDirectory.isEmpty())
            m_currentConfig = new KSimpleConfig( dotDirectory );
        // the "else" is when we want to save locally but this is a remote URL -> no save
    }
    return m_currentConfig;
}

TDEConfigBase * KonqPropsView::currentColorConfig()
{
    // Saving locally ?
    if ( m_bSaveViewPropertiesLocally && !isDefaultProperties() )
        return currentConfig(); // Will create it if necessary
    else
        // Save color settings in app's file, not in view's file
        return TDEGlobal::config();
}

KonqPropsView::~KonqPropsView()
{
   delete d->previewsToShow;
   delete d;
   d=0;
}

bool KonqPropsView::enterDir( const KURL & dir )
{
  //kdDebug(1203) << "enterDir " << dir.prettyURL() << endl;
  // Can't do that with default properties
  assert( !isDefaultProperties() );

  // Check for .directory
  KURL u ( dir );
  u.addPath(".directory");
  bool dotDirExists = u.isLocalFile() && TQFile::exists( u.path() );
  dotDirectory = u.isLocalFile() ? u.path() : TQString::null;

  // Revert to default setting first - unless there is no .directory
  // in the previous dir nor in this one (then we can keep the current settings)
  if (dotDirExists || m_dotDirExists)
  {
    m_iIconSize = m_defaultProps->iconSize();
    m_iItemTextPos = m_defaultProps->itemTextPos();
    d->sortcriterion = m_defaultProps->sortCriterion();
    d->dirsfirst = m_defaultProps->isDirsFirst();
    d->descending = m_defaultProps->isDescending();
    m_bShowDot = m_defaultProps->isShowingDotFiles();
    d->caseInsensitiveSort=m_defaultProps->isCaseInsensitiveSort();
    m_dontPreview = m_defaultProps->m_dontPreview;
    m_textColor = m_defaultProps->m_textColor;
    m_bgColor = m_defaultProps->m_bgColor;
    m_bgPixmapFile = m_defaultProps->bgPixmapFile();
  }

  if (dotDirExists)
  {
    //kdDebug(1203) << "Found .directory file" << endl;
    KSimpleConfig * config = new KSimpleConfig( dotDirectory, true );
    config->setGroup("URL properties");

    m_iIconSize = config->readNumEntry( "IconSize", m_iIconSize );
    m_iItemTextPos = config->readNumEntry( "ItemTextPos", m_iItemTextPos );
    d->sortcriterion = config->readEntry( "SortingCriterion" , d->sortcriterion );
    d->dirsfirst = config->readBoolEntry( "SortDirsFirst", d->dirsfirst );
    d->descending = config->readBoolEntry( "SortDescending", d->descending );
    m_bShowDot = config->readBoolEntry( "ShowDotFiles", m_bShowDot );
    d->caseInsensitiveSort=config->readBoolEntry("CaseInsensitiveSort",d->caseInsensitiveSort);
    m_bShowDirectoryOverlays = config->readBoolEntry( "ShowDirectoryOverlays", m_bShowDirectoryOverlays );
    m_bShowFreeSpaceOverlays = config->readBoolEntry( "ShowFreeSpaceOverlays", m_bShowFreeSpaceOverlays );
    if (config->hasKey( "DontPreview" ))
    {
        m_dontPreview = config->readListEntry( "DontPreview" );

        //If the .directory file says something about sound previews,
        //obey it, otherwise propagate the setting up from the defaults
        //All this really should be split into a per-thumbnail setting,
        //but that's too invasive at this point
        if (config->hasKey("EnableSoundPreviews"))
        {

            if (!config->readBoolEntry("EnableSoundPreviews", false))
                if (!m_dontPreview.contains("audio/"))
                    m_dontPreview.append("audio/");
        }
        else
        {
            if (m_defaultProps->m_dontPreview.contains("audio/"))
                if (!m_dontPreview.contains("audio/"))
                    m_dontPreview.append("audio/");
        }
    }



    m_textColor = config->readColorEntry( "TextColor", &m_textColor );
    m_bgColor = config->readColorEntry( "BgColor", &m_bgColor );
    m_bgPixmapFile = config->readPathEntry( "BgImage", m_bgPixmapFile );
    //kdDebug(1203) << "KonqPropsView::enterDir m_bgPixmapFile=" << m_bgPixmapFile << endl;
    d->previewsEnabled = config->readBoolEntry( "PreviewsEnabled", d->previewsEnabled );
    delete config;
  }
  //if there is or was a .directory then the settings probably have changed
  bool configChanged=(m_dotDirExists|| dotDirExists);
  m_dotDirExists = dotDirExists;
  m_currentConfig = 0L; // new dir, not current config for saving yet
  //kdDebug(1203) << "KonqPropsView::enterDir returning " << configChanged << endl;
  return configChanged;
}

void KonqPropsView::setSaveViewPropertiesLocally( bool value )
{
    assert( !isDefaultProperties() );
    //kdDebug(1203) << "KonqPropsView::setSaveViewPropertiesLocally " << value << endl;

    if ( m_bSaveViewPropertiesLocally )
        delete m_currentConfig; // points to a KSimpleConfig

    m_bSaveViewPropertiesLocally = value;
    m_currentConfig = 0L; // mark as dirty
}

void KonqPropsView::setIconSize( int size )
{
    m_iIconSize = size;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setIconSize( size );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "IconSize", m_iIconSize );
        currentConfig()->sync();
    }
}

void KonqPropsView::setItemTextPos( int pos )
{
    m_iItemTextPos = pos;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setItemTextPos( pos );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "ItemTextPos", m_iItemTextPos );
        currentConfig()->sync();
    }
}

void KonqPropsView::setSortCriterion( const TQString &criterion )
{
    d->sortcriterion = criterion;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setSortCriterion( criterion );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "SortingCriterion", d->sortcriterion );
        currentConfig()->sync();
    }
}

void KonqPropsView::setDirsFirst( bool first)
{
    d->dirsfirst = first;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setDirsFirst( first );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "SortDirsFirst", d->dirsfirst );
        currentConfig()->sync();
    }
}

void KonqPropsView::setDescending( bool descend)
{
    d->descending = descend;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setDescending( descend );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "SortDescending", d->descending );
        currentConfig()->sync();
    }
}

void KonqPropsView::setShowingDotFiles( bool show )
{
    kdDebug(1203) << "KonqPropsView::setShowingDotFiles " << show << endl;
    m_bShowDot = show;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        kdDebug(1203) << "Saving in default properties" << endl;
        m_defaultProps->setShowingDotFiles( show );
    }
    else if (currentConfig())
    {
        kdDebug(1203) << "Saving in current config" << endl;
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "ShowDotFiles", m_bShowDot );
        currentConfig()->sync();
    }
}

void KonqPropsView::setCaseInsensitiveSort( bool on )
{
    kdDebug(1203) << "KonqPropsView::setCaseInsensitiveSort " << on << endl;
    d->caseInsensitiveSort = on;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        kdDebug(1203) << "Saving in default properties" << endl;
        m_defaultProps->setCaseInsensitiveSort( on );
    }
    else if (currentConfig())
    {
        kdDebug(1203) << "Saving in current config" << endl;
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "CaseInsensitiveSort", d->caseInsensitiveSort );
        currentConfig()->sync();
    }
}

void KonqPropsView::setShowingDirectoryOverlays( bool show )
{
    kdDebug(1203) << "KonqPropsView::setShowingDirectoryOverlays " << show << endl;
    m_bShowDirectoryOverlays = show;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        kdDebug(1203) << "Saving in default properties" << endl;
        m_defaultProps->setShowingDirectoryOverlays( show );
    }
    else if (currentConfig())
    {
        kdDebug(1203) << "Saving in current config" << endl;
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "ShowDirectoryOverlays", m_bShowDirectoryOverlays );
        currentConfig()->sync();
    }
}

void KonqPropsView::setShowingFreeSpaceOverlays( bool show )
{
    kdDebug(1203) << "KonqPropsView::setShowingFreeSpaceOverlays " << show << endl;
    m_bShowFreeSpaceOverlays = show;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        kdDebug(1203) << "Saving in default properties" << endl;
        m_defaultProps->setShowingFreeSpaceOverlays( show );
    }
    else if (currentConfig())
    {
        kdDebug(1203) << "Saving in current config" << endl;
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "ShowFreeSpaceOverlays", m_bShowFreeSpaceOverlays );
        currentConfig()->sync();
    }
}

void KonqPropsView::setShowingPreview( const TQString &preview, bool show )
{
    if ( m_dontPreview.contains( preview ) != show )
        return;
    else if ( show )
        m_dontPreview.remove( preview );
    else
        m_dontPreview.append( preview );
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
        m_defaultProps->setShowingPreview( preview, show );
    else if (currentConfig())
    {
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());

        //Audio is special-cased, as we use a binary setting
        //for it to get it to follow the defaults right.
        bool audioEnabled = !m_dontPreview.contains("audio/");

        //Don't write it out into the DontPreview line
        if (!audioEnabled)
            m_dontPreview.remove("audio/");
        currentConfig()->writeEntry( "DontPreview", m_dontPreview );
        currentConfig()->writeEntry( "EnableSoundPreviews", audioEnabled );
        currentConfig()->sync();
        if (!audioEnabled)
            m_dontPreview.append("audio/");

    }

    delete d->previewsToShow;
    d->previewsToShow = 0;
}

void KonqPropsView::setShowingPreview( bool show )
{
    d->previewsEnabled = show;

    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        kdDebug(1203) << "Saving in default properties" << endl;
        m_defaultProps-> setShowingPreview( show );
    }
    else if (currentConfig())
    {
        kdDebug(1203) << "Saving in current config" << endl;
        TDEConfigGroupSaver cgs(currentConfig(), currentGroup());
        currentConfig()->writeEntry( "PreviewsEnabled", d->previewsEnabled );
        currentConfig()->sync();
    }

    delete d->previewsToShow;
    d->previewsToShow = 0;
}

bool KonqPropsView::isShowingPreview()
{
    return d->previewsEnabled;
}

void KonqPropsView::setBgColor( const TQColor & color )
{
    m_bgColor = color;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        m_defaultProps->setBgColor( color );
    }
    else
    {
        TDEConfigBase * colorConfig = currentColorConfig();
        if (colorConfig) // 0L when saving locally but remote URL
        {
            TDEConfigGroupSaver cgs(colorConfig, currentGroup());
            colorConfig->writeEntry( "BgColor", m_bgColor );
            colorConfig->sync();
        }
    }
}

const TQColor & KonqPropsView::bgColor( TQWidget * widget ) const
{
    if ( !m_bgColor.isValid() )
        return widget->colorGroup().base();
    else
        return m_bgColor;
}

void KonqPropsView::setTextColor( const TQColor & color )
{
    m_textColor = color;
    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        m_defaultProps->setTextColor( color );
    }
    else
    {
        TDEConfigBase * colorConfig = currentColorConfig();
        if (colorConfig) // 0L when saving locally but remote URL
        {
            TDEConfigGroupSaver cgs(colorConfig, currentGroup());
            colorConfig->writeEntry( "TextColor", m_textColor );
            colorConfig->sync();
        }
    }
}

const TQColor & KonqPropsView::textColor( TQWidget * widget ) const
{
    if ( !m_textColor.isValid() )
        return widget->colorGroup().text();
    else
        return m_textColor;
}

void KonqPropsView::setBgPixmapFile( const TQString & file )
{
    m_bgPixmapFile = file;

    if ( m_defaultProps && !m_bSaveViewPropertiesLocally )
    {
        m_defaultProps->setBgPixmapFile( file );
    }
    else
    {
        TDEConfigBase * colorConfig = currentColorConfig();
        if (colorConfig) // 0L when saving locally but remote URL
        {
            TDEConfigGroupSaver cgs(colorConfig, currentGroup());
            colorConfig->writePathEntry( "BgImage", file );
            colorConfig->sync();
        }
    }
}

TQPixmap KonqPropsView::loadPixmap() const
{
    //kdDebug(1203) << "KonqPropsView::loadPixmap " << m_bgPixmapFile << endl;
    TQPixmap bgPixmap;
    if ( !m_bgPixmapFile.isEmpty() )
        bgPixmap = wallpaperPixmap( m_bgPixmapFile );
    return bgPixmap;
}

void KonqPropsView::applyColors(TQWidget * widget) const
{
    if ( m_bgPixmapFile.isEmpty() )
        widget->setPaletteBackgroundColor( bgColor( widget ) );
    else
    {
        TQPixmap pix = loadPixmap();
        // don't set an null pixmap, as this leads to
        // undefined results with regards to the background of widgets
        // that have the iconview as a parent and on the iconview itself
        // e.g. the rename textedit widget when renaming a QIconViewItem
        // Qt-issue: N64698
        if ( ! pix.isNull() )
            widget->setBackgroundPixmap( pix );
        // setPaletteBackgroundPixmap leads to flicker on window activation(!)
    }

    if ( m_textColor.isValid() )
        widget->setPaletteForegroundColor( m_textColor );
}

const TQStringList& KonqPropsView::previewSettings()
{
    if ( ! d->previewsToShow )
    {
        d->previewsToShow = new TQStringList;

        if (d->previewsEnabled) {
            TDETrader::OfferList plugins = TDETrader::self()->query( "ThumbCreator" );
            for ( TDETrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it )
            {
            TQString name = (*it)->desktopEntryName();
            if ( ! m_dontPreview.contains(name) )
                    d->previewsToShow->append( name );
            }
            if ( ! m_dontPreview.contains( "audio/" ) )
            d->previewsToShow->append( "audio/" );
        }
    }

    return *(d->previewsToShow);
}

const TQString& KonqPropsView::sortCriterion() const {
    return d->sortcriterion;
}

