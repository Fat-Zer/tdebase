/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqtimer.h>
#include <tqtooltip.h>
#include <tqdragobject.h>

#include <tdeconfig.h>
#include <tdelocale.h>
#include <konq_operations.h>
#include <tdefileitem.h>

#include "kicker.h"
#include "browser_mnu.h"
#include "browser_dlg.h"
#include "global.h"

#include "browserbutton.h"
#include "browserbutton.moc"

BrowserButton::BrowserButton( const TQString& icon, const TQString& startDir, TQWidget* parent )
    : PanelPopupButton( parent, "BrowserButton" )
    , topMenu( 0 )
{
    initialize( icon, startDir );
}

BrowserButton::BrowserButton( const TDEConfigGroup& config, TQWidget* parent )
    : PanelPopupButton( parent, "BrowserButton" )
    , topMenu( 0 )
{
    initialize( config.readEntry("Icon", "kdisknav"), config.readPathEntry("Path") );
}

BrowserButton::~BrowserButton()
{
    delete topMenu;
}

void BrowserButton::initialize( const TQString& icon, const TQString& path )
{
    _icon = icon;

    // Don't parent to this, so that the tear of menu is not always-on-top.
    topMenu = new PanelBrowserMenu( path );
    setPopup(topMenu);

    _menuTimer = new TQTimer( this, "_menuTimer" );
    connect( _menuTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotDelayedPopup()) );

    TQToolTip::add(this, i18n("Browse: %1").arg(path));
    setTitle( path );
    setIcon ( _icon );
}

void BrowserButton::saveConfig( TDEConfigGroup& config ) const
{
    config.writeEntry("Icon", _icon);
    config.writePathEntry("Path", topMenu->path());
}

void BrowserButton::dragEnterEvent( TQDragEnterEvent *ev )
{
    if ((ev->source() != this) && KURLDrag::canDecode(ev))
    {
        _menuTimer->start(500, true);
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
    PanelButton::dragEnterEvent(ev);
}

void BrowserButton::dragLeaveEvent( TQDragLeaveEvent *ev )
{
   _menuTimer->stop();
   PanelButton::dragLeaveEvent(ev);
}

void BrowserButton::dropEvent( TQDropEvent *ev )
{
    KURL path ( topMenu->path() );
    _menuTimer->stop();
    KFileItem item( path, TQString::fromLatin1( "inode/directory" ), KFileItem::Unknown );
    KonqOperations::doDrop( &item, path, ev, this );
    PanelButton::dropEvent(ev);
}

void BrowserButton::initPopup()
{
    topMenu->initialize();
}

void BrowserButton::slotDelayedPopup()
{
    topMenu->initialize();
    topMenu->popup(KickerLib::popupPosition(popupDirection(), topMenu, this));
    setDown(false);
}

void BrowserButton::properties()
{
    PanelBrowserDialog dlg( topMenu->path(), _icon, this );

    if( dlg.exec() == TQDialog::Accepted ){
	_icon = dlg.icon();
	TQString path = dlg.path();

	if ( path != topMenu->path() ) {
	    delete topMenu;
	    topMenu = new PanelBrowserMenu( path, this );
	    setPopup( topMenu );
	    setTitle( path );
	}
	setIcon( _icon );
	emit requestSave();
    }
}

void BrowserButton::startDrag()
{
    KURL url(topMenu->path());
    emit dragme(KURL::List(url), labelIcon());
}

