/* This file is part of the KDE project
   Copyright (C) 1999, 2000, 2001, 2002 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tdefileivi.h"
#include "kivdirectoryoverlay.h"
#include "kivfreespaceoverlay.h"
#include "konq_iconviewwidget.h"
#include "konq_operations.h"
#include "konq_settings.h"

#include <tqpainter.h>

#include <kurldrag.h>
#include <kiconeffect.h>
#include <tdefileitem.h>
#include <kdebug.h>
#include <krun.h>
#include <kservice.h>

#undef Bool

/**
 * Private data for KFileIVI
 */
struct KFileIVI::Private
{
    TQIconSet icons; // Icon states (cached to prevent re-applying icon effects
		    // every time)
    TQPixmap  thumb; // Raw unprocessed thumbnail
    TQString m_animatedIcon; // Name of animation
    bool m_animated;        // Animation currently running ?
	KIVDirectoryOverlay* m_directoryOverlay;
	KIVFreeSpaceOverlay* m_freeSpaceOverlay;
	TQPixmap m_overlay;
	TQString m_overlayName;
	int m_progress;
};

KFileIVI::KFileIVI( KonqIconViewWidget *iconview, KFileItem* fileitem, int size )
    : TDEIconViewItem( iconview, fileitem->text() ),
    m_size( size ), m_state( TDEIcon::DefaultState ),
    m_bDisabled( false ), m_bThumbnail( false ), m_fileitem( fileitem )
{
    d = new KFileIVI::Private;

    updatePixmapSize();
    setPixmap( m_fileitem->pixmap( m_size, m_state ) );
    setDropEnabled( S_ISDIR( m_fileitem->mode() ) );

    // Cache entry for the icon effects
    d->icons.reset( *pixmap(), TQIconSet::Large );
    d->m_animated = false;

    // iconName() requires the mimetype to be known
    if ( fileitem->isMimeTypeKnown() )
    {
        TQString icon = fileitem->iconName();
        if ( !icon.isEmpty() )
            setMouseOverAnimation( icon );
        else
            setMouseOverAnimation( "unknown" );
    }
    d->m_progress = -1;
    d->m_directoryOverlay = 0;
    d->m_freeSpaceOverlay = 0;
}

KFileIVI::~KFileIVI()
{
    delete d->m_directoryOverlay;
    delete d->m_freeSpaceOverlay;
    delete d;
}

void KFileIVI::invalidateThumb( int state, bool redraw )
{
    TQIconSet::Mode mode;
    switch( state )
    {
	case TDEIcon::DisabledState:
	    mode = TQIconSet::Disabled;
	    break;
	case TDEIcon::ActiveState:
	    mode = TQIconSet::Active;
	    break;
	case TDEIcon::DefaultState:
	default:
	    mode = TQIconSet::Normal;
	    break;
    }
    d->icons = TQIconSet();
    d->icons.setPixmap( TDEGlobal::iconLoader()->iconEffect()->
			apply( d->thumb, TDEIcon::Desktop, state ),
			TQIconSet::Large, mode );
    m_state = state;

    TQIconViewItem::setPixmap( d->icons.pixmap( TQIconSet::Large, mode ),
			      false, redraw );
}

void KFileIVI::setIcon( int size, int state, bool recalc, bool redraw )
{
    m_size = size;
    m_bThumbnail = false;
    if ( m_bDisabled )
      m_state = TDEIcon::DisabledState;
    else
      m_state = state;

    if ( d->m_overlayName.isNull() )
        d->m_overlay = TQPixmap();
    else {
        int halfSize;
        if (m_size == 0) {
            halfSize = IconSize(TDEIcon::Desktop) / 2;
        } else {
            halfSize = m_size / 2;
        }
        d->m_overlay = DesktopIcon(d->m_overlayName, halfSize);
    }

    setPixmapDirect(m_fileitem->pixmap( m_size, m_state ) , recalc, redraw );
}

void KFileIVI::setOverlay( const TQString& iconName )
{
    d->m_overlayName = iconName;

    refreshIcon(true);
}

void KFileIVI::setOverlayProgressBar( const int progress )
{
    d->m_progress = progress;

    refreshIcon(true);
}

KIVDirectoryOverlay* KFileIVI::setShowDirectoryOverlay( bool show )
{
    if ( !m_fileitem->isDir() || m_fileitem->iconName() != "folder" )
        return 0;

    if (show) {
        if (!d->m_directoryOverlay)
            d->m_directoryOverlay = new KIVDirectoryOverlay(this);
        return d->m_directoryOverlay;
    } else {
        delete d->m_directoryOverlay;
        d->m_directoryOverlay = 0;
        setOverlay(TQString());
        return 0;
    }
}

bool KFileIVI::showDirectoryOverlay(  )
{
    return (bool)d->m_directoryOverlay;
}

KIVFreeSpaceOverlay* KFileIVI::setShowFreeSpaceOverlay( bool show )
{
    if ( !m_fileitem->mimetype().startsWith("media/") ) {
        return 0;
    }

    if (show) {
        if (!d->m_freeSpaceOverlay)
            d->m_freeSpaceOverlay = new KIVFreeSpaceOverlay(this);
        return d->m_freeSpaceOverlay;
    } else {
        delete d->m_freeSpaceOverlay;
        d->m_freeSpaceOverlay = 0;
        setOverlayProgressBar(-1);
        return 0;
    }
}

bool KFileIVI::showFreeSpaceOverlay(  )
{
    return (bool)d->m_freeSpaceOverlay;
}

void KFileIVI::setPixmapDirect( const TQPixmap& pixmap, bool recalc, bool redraw )
{
    TQIconSet::Mode mode;
    switch( m_state )
    {
	case TDEIcon::DisabledState:
	    mode = TQIconSet::Disabled;
	    break;
	case TDEIcon::ActiveState:
	    mode = TQIconSet::Active;
	    break;
	case TDEIcon::DefaultState:
	default:
	    mode = TQIconSet::Normal;
	    break;
    }

    // We cannot just reset() the iconset here, because setIcon can be
    // called with any state and not just normal state. So we just
    // create a dummy empty iconset as base object.
    d->icons = TQIconSet();
    d->icons.setPixmap( pixmap, TQIconSet::Large, mode );

    updatePixmapSize();
    TQIconViewItem::setPixmap( d->icons.pixmap( TQIconSet::Large, mode ),
			      recalc, redraw );
}

void KFileIVI::setDisabled( bool disabled )
{
    if ( m_bDisabled != disabled )
    {
        m_bDisabled = disabled;
        bool active = ( m_state == TDEIcon::ActiveState );
        setEffect( m_bDisabled ? TDEIcon::DisabledState : 
                   ( active ? TDEIcon::ActiveState : TDEIcon::DefaultState ) );
    }
}

void KFileIVI::setThumbnailPixmap( const TQPixmap & pixmap )
{
    m_bThumbnail = true;
    d->thumb = pixmap;
    // TQIconSet::reset() doesn't seem to clear the other generated pixmaps,
    // so we just create a blank TQIconSet here
    d->icons = TQIconSet();
    d->icons.setPixmap( TDEGlobal::iconLoader()->iconEffect()->
		    apply( pixmap, TDEIcon::Desktop, TDEIcon::DefaultState ),
		    TQIconSet::Large, TQIconSet::Normal );

    m_state = TDEIcon::DefaultState;

    // Recalc when setting this pixmap!
    updatePixmapSize();
    TQIconViewItem::setPixmap( d->icons.pixmap( TQIconSet::Large,
			      TQIconSet::Normal ), true );
}

void KFileIVI::setActive( bool active )
{
    if ( active )
        setEffect( TDEIcon::ActiveState );
    else
        setEffect( m_bDisabled ? TDEIcon::DisabledState : TDEIcon::DefaultState );
}

void KFileIVI::setEffect( int state )
{
    TQIconSet::Mode mode;
    switch( state )
    {
	case TDEIcon::DisabledState:
	    mode = TQIconSet::Disabled;
	    break;
	case TDEIcon::ActiveState:
	    mode = TQIconSet::Active;
	    break;
	case TDEIcon::DefaultState:
	default:
	    mode = TQIconSet::Normal;
	    break;
    }
    // Do not update if the fingerprint is identical (prevents flicker)!

    TDEIconEffect *effect = TDEGlobal::iconLoader()->iconEffect();

    bool haveEffect = effect->hasEffect( TDEIcon::Desktop, m_state ) !=
                      effect->hasEffect( TDEIcon::Desktop, state );

                //kdDebug(1203) << "desktop;defaultstate=" <<
                //      effect->fingerprint(TDEIcon::Desktop, TDEIcon::DefaultState) <<
                //      endl;
                //kdDebug(1203) << "desktop;activestate=" <<
                //      effect->fingerprint(TDEIcon::Desktop, TDEIcon::ActiveState) <<
                //      endl;

    if( haveEffect &&
        effect->fingerprint( TDEIcon::Desktop, m_state ) !=
	effect->fingerprint( TDEIcon::Desktop, state ) )
    {
	// Effects on are not applied until they are first accessed to
	// save memory. Do this now when needed
	if( m_bThumbnail )
	{
	    if( d->icons.isGenerated( TQIconSet::Large, mode ) )
		d->icons.setPixmap( effect->apply( d->thumb, TDEIcon::Desktop, state ),
				    TQIconSet::Large, mode );
	}
	else
	{
	    if( d->icons.isGenerated( TQIconSet::Large, mode ) )
		d->icons.setPixmap( m_fileitem->pixmap( m_size, state ),
				    TQIconSet::Large, mode );
	}
	TQIconViewItem::setPixmap( d->icons.pixmap( TQIconSet::Large, mode ) );
    }
    m_state = state;
}

void KFileIVI::refreshIcon( bool redraw )
{
    if (!isThumbnail()) {
        setIcon( m_size, m_state, true, redraw );
    }
}

void KFileIVI::invalidateThumbnail()
{
    d->thumb = TQPixmap();
}

bool KFileIVI::isThumbnailInvalid() const
{
    return d->thumb.isNull();
}

bool KFileIVI::acceptDrop( const TQMimeSource *mime ) const
{
    if ( mime->provides( "text/uri-list" ) ) // We're dragging URLs
    {
        if ( m_fileitem->acceptsDrops() ) // Directory, executables, ...
            return true;

        // Use cache
        KURL::List uris = ( static_cast<KonqIconViewWidget*>(iconView()) )->dragURLs();

        // Check if we want to drop something on itself
        // (Nothing will happen, but it's a convenient way to move icons)
        KURL::List::Iterator it = uris.begin();
        for ( ; it != uris.end() ; it++ )
        {
            if ( m_fileitem->url().equals( *it, true /*ignore trailing slashes*/ ) )
                return true;
        }
    }
    return TQIconViewItem::acceptDrop( mime );
}

void KFileIVI::setKey( const TQString &key )
{
    TQString theKey = key;

    TQVariant sortDirProp = iconView()->property( "sortDirectoriesFirst" );

    bool isdir = ( S_ISDIR( m_fileitem->mode() ) && ( !sortDirProp.isValid() || ( sortDirProp.type() == TQVariant::Bool && sortDirProp.toBool() ) ) );

    // The order is: .dir (0), dir (1), .file (2), file (3)
    int sortChar = isdir ? 1 : 3;
    if ( m_fileitem->text()[0] == '.' )
        --sortChar;

    if ( !iconView()->sortDirection() ) // reverse sorting
        sortChar = 3 - sortChar;

    theKey.prepend( TQChar( sortChar + '0' ) );

    TQIconViewItem::setKey( theKey );
}

void KFileIVI::dropped( TQDropEvent *e, const TQValueList<TQIconDragItem> & )
{
    KonqOperations::doDrop( item(), item()->url(), e, iconView() );
}

void KFileIVI::returnPressed()
{
    if ( static_cast<KonqIconViewWidget*>(iconView())->isDesktop() ) {
        KURL url = m_fileitem->url();
        if (url.protocol() == "media") {
            // The user reasonably expects to be placed within the media:/ tree
            // when opening a media device from the desktop
            KService::Ptr service = KService::serviceByDesktopName("konqueror");
            if (service) {
                // HACK
                // There doesn't seem to be a way to prevent KRun from resolving the URL to its
                // local path, so simpy launch Konqueror with the correct arguments instead...
                KRun::runCommand("konqueror " + url.url(), "konqueror", service->icon());
            }
            else {
                (void) new KRun( url, m_fileitem->mode(), m_fileitem->isLocalFile() );
            }
        }
        else {
            // When clicking on a link to e.g. $HOME from the desktop, we want to open $HOME
            // Symlink resolution must only happen on the desktop though (#63014)
            if ( m_fileitem->isLink() && url.isLocalFile() ) {
                url = KURL( url, m_fileitem->linkDest() );
            }

            (void) new KRun( url, m_fileitem->mode(), m_fileitem->isLocalFile() );
        }
    }
    else {
        m_fileitem->run();
    }
}


void KFileIVI::paintItem( TQPainter *p, const TQColorGroup &c )
{
    TQColorGroup cg = updateColors(c);
    paintFontUpdate( p );

    //*** TEMPORARY CODE - MUST BE MADE CONFIGURABLE FIRST - Martijn
    // SET UNDERLINE ON HOVER ONLY
    /*if ( ( ( KonqIconViewWidget* ) iconView() )->m_pActiveItem == this )
    {
        TQFont f( p->font() );
        f.setUnderline( TRUE );
        p->setFont( f );
    }*/

    TDEIconViewItem::paintItem( p, cg );
    paintOverlay(p);
    paintOverlayProgressBar(p);

}

void KFileIVI::paintOverlay( TQPainter *p ) const
{
    if ( !d->m_overlay.isNull() ) {
        TQRect rect = pixmapRect(true);
        p->drawPixmap(x() + rect.x() , y() + pixmapRect().height() - d->m_overlay.height(), d->m_overlay);
    }
}

void KFileIVI::paintOverlayProgressBar( TQPainter *p ) const
{
    if (d->m_progress != -1) {
//         // Pie chart
//         TQRect rect = pixmapRect(true);
//         rect.setX(x() + rect.x());
//         rect.setY(y() + rect.y() + ((pixmapRect().height()*3)/4));
//         rect.setWidth(pixmapRect().width()/4);
//         rect.setHeight(pixmapRect().height()/4);
//
//         p->save();
//
//         p->setPen(TQPen::NoPen);
//         p->setBrush(TQt::red);
//         p->drawEllipse(rect);
//         p->setBrush(TQt::green);
//         p->drawPie(rect, 1440, (((100-d->m_progress)*5760)/100));

//         // Horizontal progress bar
//         TQRect rect = pixmapRect(true);
//         int verticalOffset = 0;
//         int usedBarWidth = ((d->m_progress*pixmapRect().width())/100);
//         int endPosition = x() + rect.x() + usedBarWidth;
// 
//         p->save();
// 
//         p->setPen(TQPen::NoPen);
//         p->setBrush(TQt::red);
//         p->drawRect(TQRect(x() + rect.x(), y() + rect.y() + (pixmapRect().height() - verticalOffset), usedBarWidth, 1));
//         p->setBrush(TQt::green);
//         p->drawRect(TQRect(endPosition, y() + rect.y() + (pixmapRect().height() - verticalOffset), pixmapRect().width() - usedBarWidth, 1));

        // Vertical progress bar
        TQRect rect = pixmapRect(true);
        int horizontalOffset = 0;
        int usedBarHeight = (((100-d->m_progress)*pixmapRect().height())/100);
        int endPosition = y() + rect.y() + usedBarHeight;

        p->save();

        p->setPen(TQPen::NoPen);
        p->setBrush(TQt::green);
        p->drawRect(TQRect(x() + rect.x() + (pixmapRect().width() - horizontalOffset), y() + rect.y(), 1, usedBarHeight));
        p->setBrush(TQt::red);
        p->drawRect(TQRect(x() + rect.x() + (pixmapRect().width() - horizontalOffset), endPosition, 1, pixmapRect().height() - usedBarHeight));

        p->restore();
    }
}

void KFileIVI::paintFontUpdate( TQPainter *p ) const
{
    if ( m_fileitem->isLink() )
    {
        TQFont f( p->font() );
        f.setItalic( TRUE );
        p->setFont( f );
    }
}

TQColorGroup KFileIVI::updateColors( const TQColorGroup &c ) const
{
    TQColorGroup cg( c );
    cg.setColor( TQColorGroup::Text, static_cast<KonqIconViewWidget*>(iconView())->itemColor() );
    return cg;
}

bool KFileIVI::move( int x, int y )
{
    if ( static_cast<KonqIconViewWidget*>(iconView())->isDesktop() ) {
	if ( x < 5 )
	    x = 5;
	if ( x > iconView()->viewport()->width() - ( width() + 5 ) )
	    x = iconView()->viewport()->width() - ( width() + 5 );
	if ( y < 5 )
	    y = 5;
	if ( y > iconView()->viewport()->height() - ( height() + 5 ) )
	    y = iconView()->viewport()->height() - ( height() + 5 );
    }
    return TQIconViewItem::move( x, y );
}

bool KFileIVI::hasAnimation() const
{
    return !d->m_animatedIcon.isEmpty() && !m_bThumbnail;
}

void KFileIVI::setMouseOverAnimation( const TQString& movieFileName )
{
    if ( !movieFileName.isEmpty() )
    {
        //kdDebug(1203) << "TDEIconViewItem::setMouseOverAnimation " << movieFileName << endl;
        d->m_animatedIcon = movieFileName;
    }
}

TQString KFileIVI::mouseOverAnimation() const
{
    return d->m_animatedIcon;
}

bool KFileIVI::isAnimated() const
{
    return d->m_animated;
}

void KFileIVI::setAnimated( bool a )
{
    d->m_animated = a;
}

int KFileIVI::compare( TQIconViewItem *i ) const
{
    KonqIconViewWidget* view = static_cast<KonqIconViewWidget*>(iconView());
    if ( view->caseInsensitiveSort() )
        return key().localeAwareCompare( i->key() );
    else
        return view->m_pSettings->caseSensitiveCompare( key(), i->key() );
}

void KFileIVI::updatePixmapSize()
{
    int size = m_size ? m_size :
        TDEGlobal::iconLoader()->currentSize( TDEIcon::Desktop );

    KonqIconViewWidget* view = static_cast<KonqIconViewWidget*>( iconView() );

    bool mimeDetermined = false;
    if ( m_fileitem->isMimeTypeKnown() ) {
        mimeDetermined = true;
    }

    if (mimeDetermined) {
        bool changed = false;
        if ( view && view->canPreview( item() ) ) {
            int previewSize = view->previewIconSize( size );
            if (previewSize != size) {
                setPixmapSize( TQSize( previewSize, previewSize ) );
                changed = true;
            }
        }
        else {
            TQSize pixSize = TQSize( size, size );
            if ( pixSize != pixmapSize() ) {
                setPixmapSize( pixSize );
                changed = true;
            }
        }
        if (changed) {
            view->adjustItems();
        }
    }
    else {
        TQSize pixSize = TQSize( size, size );
        if ( pixSize != pixmapSize() ) {
            setPixmapSize( pixSize );
        }
    }
}

void KFileIVI::mimeTypeAndIconDetermined()
{
    updatePixmapSize();
}

/* vim: set noet sw=4 ts=8 softtabstop=4: */
