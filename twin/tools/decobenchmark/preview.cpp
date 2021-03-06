/*
 *
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "preview.h"

#include <kdebug.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tqlabel.h>
#include <tqstyle.h>
#include <kiconloader.h>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include <kdecorationfactory.h>
#include <kdecoration_plugins_p.h>

// FRAME the preview doesn't update to reflect the changes done in the kcm

KDecorationPreview::KDecorationPreview( KDecorationPlugins* plugin, TQWidget* parent, const char* name )
    :   TQWidget( parent, name ),
    m_plugin(plugin)
{
    options = new KDecorationPreviewOptions;

    bridge = new KDecorationPreviewBridge( this, true, "Deco Benchmark" );

    deco = 0;

    setFixedSize( 600, 500 );

    positionPreviews();
}

KDecorationPreview::~KDecorationPreview()
{
    delete deco;
    delete bridge;
    delete options;
}

void KDecorationPreview::performRepaintTest(int n)
{
    kdDebug() << "start " << n << " repaints..." << endl;
    bridge->setCaption("Deco Benchmark");
    deco->captionChange();
    positionPreviews(0);
    for (int i = 0; i < n; ++i) {
        deco->widget()->repaint();
        kapp->processEvents();
    }
}

void KDecorationPreview::performCaptionTest(int n)
{
    kdDebug() << "start " << n << " caption changes..." << endl;
    TQString caption = "Deco Benchmark %1";
    positionPreviews(0);
    for (int i = 0; i < n; ++i) {
        bridge->setCaption(caption.arg(i) );
        deco->captionChange();
        deco->widget()->repaint();
        kapp->processEvents();
    }
}

void KDecorationPreview::performResizeTest(int n)
{
    kdDebug() << "start " << n << " resizes..." << endl;
    bridge->setCaption("Deco Benchmark");
    deco->captionChange();
    for (int i = 0; i < n; ++i) {
        positionPreviews(i % 200);
        kapp->processEvents();
    }
}

void KDecorationPreview::performRecreationTest(int n)
{
    kdDebug() << "start " << n << " resizes..." << endl;
    bridge->setCaption("Deco Benchmark");
    deco->captionChange();
    positionPreviews(0);
    for (int i = 0; i < n; ++i) {
        recreateDecoration();
        kapp->processEvents();
    }
}

bool KDecorationPreview::recreateDecoration()
{
    delete deco;
    deco = m_plugin->createDecoration(bridge);
    deco->init();

    if (!deco)
        return false;

    positionPreviews();
    deco->widget()->show();

    return true;
}

void KDecorationPreview::positionPreviews(int shrink)
{
    if ( !deco )
        return;

    TQSize size = TQSize(width()-2*10-shrink, height()-2*10-shrink)/*.expandedTo(deco->minimumSize()*/;

    TQRect geometry(TQPoint(10, 10), size);
    deco->widget()->setGeometry(geometry);
}

void KDecorationPreview::setPreviewMask( const TQRegion& reg, int mode )
{
    TQWidget *widget = deco->widget();

    // FRAME duped from client.cpp
    if( mode == Unsorted )
        {
        XShapeCombineRegion( tqt_xdisplay(), widget->winId(), ShapeBounding, 0, 0,
            reg.handle(), ShapeSet );
        }
    else
        {
        TQMemArray< TQRect > rects = reg.rects();
        XRectangle* xrects = new XRectangle[ rects.count() ];
        for( unsigned int i = 0;
             i < rects.count();
             ++i )
            {
            xrects[ i ].x = rects[ i ].x();
            xrects[ i ].y = rects[ i ].y();
            xrects[ i ].width = rects[ i ].width();
            xrects[ i ].height = rects[ i ].height();
            }
        XShapeCombineRectangles( tqt_xdisplay(), widget->winId(), ShapeBounding, 0, 0,
	    xrects, rects.count(), ShapeSet, mode );
        delete[] xrects;
        }
}

TQRect KDecorationPreview::windowGeometry( bool active ) const
{
    TQWidget *widget = deco->widget();
    return widget->geometry();
}

TQRegion KDecorationPreview::unobscuredRegion( bool active, const TQRegion& r ) const
{
        return r;
}

KDecorationPreviewBridge::KDecorationPreviewBridge( KDecorationPreview* p, bool a, const TQString &c )
    :   preview( p ), active( a ), m_caption( c )
{
}

void KDecorationPreviewBridge::setCaption(const TQString &c)
{
    m_caption = c;
}

TQWidget* KDecorationPreviewBridge::initialParentWidget() const
    {
    return preview;
    }

Qt::WFlags KDecorationPreviewBridge::initialWFlags() const
    {
    return 0;
    }

bool KDecorationPreviewBridge::isActive() const
    {
    return active;
    }

bool KDecorationPreviewBridge::isCloseable() const
    {
    return true;
    }

bool KDecorationPreviewBridge::isMaximizable() const
    {
    return true;
    }

KDecoration::MaximizeMode KDecorationPreviewBridge::maximizeMode() const
    {
    return KDecoration::MaximizeRestore;
    }

bool KDecorationPreviewBridge::isMinimizable() const
    {
    return true;
    }

bool KDecorationPreviewBridge::providesContextHelp() const
    {
    return true;
    }

int KDecorationPreviewBridge::desktop() const
    {
    return 1;
    }

bool KDecorationPreviewBridge::isModal() const
    {
    return false;
    }

bool KDecorationPreviewBridge::isShadeable() const
    {
    return true;
    }

bool KDecorationPreviewBridge::isShade() const
    {
    return false;
    }

bool KDecorationPreviewBridge::isSetShade() const
    {
    return false;
    }

bool KDecorationPreviewBridge::keepAbove() const
    {
    return false;
    }

bool KDecorationPreviewBridge::keepBelow() const
    {
    return false;
    }

bool KDecorationPreviewBridge::isMovable() const
    {
    return true;
    }

bool KDecorationPreviewBridge::isResizable() const
    {
    return true;
    }

NET::WindowType KDecorationPreviewBridge::windowType( unsigned long ) const
    {
    return NET::Normal;
    }

TQIconSet KDecorationPreviewBridge::icon() const
    {
    return SmallIconSet( "xapp" );
    }

TQString KDecorationPreviewBridge::caption() const
{
    return m_caption;
}

void KDecorationPreviewBridge::processMousePressEvent( TQMouseEvent* )
    {
    }

void KDecorationPreviewBridge::showWindowMenu( const TQRect &)
    {
    }

void KDecorationPreviewBridge::showWindowMenu( TQPoint )
    {
    }

void KDecorationPreviewBridge::performWindowOperation( WindowOperation )
    {
    }

void KDecorationPreviewBridge::setMask( const TQRegion& reg, int mode )
    {
    preview->setPreviewMask( reg, mode );
    }

bool KDecorationPreviewBridge::isPreview() const
    {
    return false;
    }

TQRect KDecorationPreviewBridge::geometry() const
    {
    return preview->windowGeometry( active );
    }

TQRect KDecorationPreviewBridge::iconGeometry() const
    {
    return TQRect();
    }

TQRegion KDecorationPreviewBridge::unobscuredRegion( const TQRegion& r ) const
    {
    return preview->unobscuredRegion( active, r );
    }

TQWidget* KDecorationPreviewBridge::workspaceWidget() const
    {
    return preview;
    }

WId KDecorationPreviewBridge::windowId() const
    {
    return 0; // no decorated window
    }

void KDecorationPreviewBridge::closeWindow()
    {
    }

void KDecorationPreviewBridge::maximize( MaximizeMode )
    {
    }

void KDecorationPreviewBridge::minimize()
    {
    }

void KDecorationPreviewBridge::showContextHelp()
    {
    }

void KDecorationPreviewBridge::setDesktop( int )
    {
    }

void KDecorationPreviewBridge::titlebarDblClickOperation()
    {
    }

void KDecorationPreviewBridge::setShade( bool )
    {
    }

void KDecorationPreviewBridge::setKeepAbove( bool )
    {
    }

void KDecorationPreviewBridge::setKeepBelow( bool )
    {
    }

int KDecorationPreviewBridge::currentDesktop() const
    {
    return 1;
    }

void KDecorationPreviewBridge::helperShowHide( bool )
    {
    }

void KDecorationPreviewBridge::grabXServer( bool )
    {
    }

KDecorationPreviewOptions::KDecorationPreviewOptions()
    {
    d = new KDecorationOptionsPrivate;
    d->defaultKWinSettings();
    updateSettings();
    }

KDecorationPreviewOptions::~KDecorationPreviewOptions()
    {
    delete d;
    }

unsigned long KDecorationPreviewOptions::updateSettings()
{
    TDEConfig cfg( "twinrc", true );
    unsigned long changed = 0;
    changed |= d->updateKWinSettings( &cfg );

    return changed;
}

bool KDecorationPreviewPlugins::provides( Requirement )
    {
    return false;
    }

// #include "preview.moc"
