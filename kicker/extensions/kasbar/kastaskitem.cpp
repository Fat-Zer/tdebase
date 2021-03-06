/* kastaskitem.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
#include <errno.h>

#include <tqbitmap.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqmetaobject.h>
#include <tqpainter.h>
#include <tqregexp.h>
#include <tqtabwidget.h>
#include <tqtextview.h>
#include <tqtimer.h>
#include <tqvbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <kpassivepopup.h>
#include <tdepopupmenu.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <tdeapplication.h>

#include <taskmanager.h>
#include <taskrmbmenu.h>

#include "kastasker.h"
#include "kastaskpopup.h"
#include "kastaskitem.h"
#include "kasbarextension.h"

#define Icon(x) TDEGlobal::iconLoader()->loadIcon( x, TDEIcon::NoGroup, TDEIcon::SizeMedium )

static const int CHECK_ATTENTION_DELAY = 2000;

KasTaskItem::KasTaskItem( KasTasker *parent, Task::Ptr task )
    : KasItem( parent ),
      task_(task),
      thumbTimer(0),
      attentionTimer(0)
{
    setIcon( icon() );
    setAttention( task->demandsAttention() );
    updateTask(false);

    connect( task, TQT_SIGNAL( changed(bool) ), this, TQT_SLOT( updateTask(bool) ) );
    connect( task, TQT_SIGNAL( activated() ), this, TQT_SLOT( startAutoThumbnail() ) );
    connect( task, TQT_SIGNAL( deactivated() ), this, TQT_SLOT( stopAutoThumbnail() ) );
    connect( task, TQT_SIGNAL( iconChanged() ), this, TQT_SLOT( iconChanged() ) );
    connect( task, TQT_SIGNAL( thumbnailChanged() ), this, TQT_SLOT( iconChanged() ) );

    connect( this, TQT_SIGNAL(leftButtonClicked(TQMouseEvent *)), TQT_SLOT(toggleActivateAction()) );
    connect( this, TQT_SIGNAL(rightButtonClicked(TQMouseEvent *)), TQT_SLOT(showWindowMenuAt(TQMouseEvent *) ) );

    attentionTimer = new TQTimer( this, "attentionTimer" );
    connect( attentionTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( checkAttention() ) );
    attentionTimer->start( CHECK_ATTENTION_DELAY );
}

KasTaskItem::~KasTaskItem()
{
}

KasTasker *KasTaskItem::kasbar() const
{
    return static_cast<KasTasker *> (KasItem::kasbar());
}

TQPixmap KasTaskItem::icon()
{
    int sizes[] = { TDEIcon::SizeEnormous,
		    TDEIcon::SizeHuge,
		    TDEIcon::SizeLarge,
		    TDEIcon::SizeMedium,
		    TDEIcon::SizeSmall };

    if ( kasbar()->embedThumbnails() && task_->hasThumbnail() ) {
	usedIconLoader = true;

	TQPixmap thumb = task_->thumbnail();
	TQSize sz = thumb.size();
	sz.scale( sizes[kasbar()->itemSize()], sizes[kasbar()->itemSize()], TQSize::ScaleMin );

	TQImage img = thumb.convertToImage();
	img = img.smoothScale( sz );

	bool ok = thumb.convertFromImage( img );
	if ( ok )
	    return thumb;
    }

    usedIconLoader = false;
    TQPixmap p = task_->bestIcon( sizes[kasbar()->itemSize()], usedIconLoader );
    if ( !p.isNull() )
	return p;

    return task_->icon( sizes[kasbar()->itemSize()], sizes[kasbar()->itemSize()], true );
}

void KasTaskItem::iconChanged()
{
    iconHasChanged = true;
    setIcon( icon() );
    update();
}

void KasTaskItem::checkAttention()
{
    setAttention( task_->demandsAttention() );
}

void KasTaskItem::updateTask(bool geometryChangeOnly)
{
    if (geometryChangeOnly)
    {
        return;
    }

    bool updates = kasbar()->isUpdatesEnabled();
    kasbar()->setUpdatesEnabled( false );

    setProgress( kasbar()->showProgress() ? 0 : -1 );
    setText( task_->visibleIconicName() );
    setModified( task_->isModified() );
    setActive( task_->isActive() );

    kasbar()->setUpdatesEnabled( updates );
    update();
}

void KasTaskItem::paint( TQPainter *p )
{
    KasItem::paint( p );

    KasResources *res = resources();
    TQColor c = task_->isActive() ? res->activePenColor() : res->inactivePenColor();
    p->setPen( c );

    //
    // Overlay the small icon if the icon has changed, we have space,
    // and we are using a TDEIconLoader icon rather than one from the NET props.
    // This only exists because we are almost always using the icon loader for
    // large icons.
    //
    KasTasker *kas = kasbar();
    bool haveSpace = ( kas->itemSize() != KasBar::Small )
	          && ( kas->itemSize() != KasBar::Medium );

    if ( usedIconLoader && iconHasChanged && haveSpace ) {
	TQPixmap pix = icon();
	int x = (extent() - 4 - pix.width()) / 2;
	TQPixmap overlay = task_->pixmap();
	p->drawPixmap( x-4+pix.width()-overlay.width(), 18, overlay );
    }

    //
    // Draw window state.
    //
    if( task_->isIconified() )
	paintStateIcon( p, StateIcon );
    else if ( task_->isShaded() )
	paintStateIcon( p, StateShaded );
    else
	paintStateIcon( p, StateNormal );

    //
    // Draw desktop number.
    //

    // Check if we only have one desktop
    bool oneDesktop = (TaskManager::the()->numberOfDesktops() == 1) ? true : false;

    TQString deskStr;
    if ( task_->isOnAllDesktops() )
	deskStr = i18n( "All" );
    else
	deskStr.setNum( task_->desktop() );


    if ( kas->itemSize() != KasBar::Small ) {
	// Medium and Large modes
	if ( !oneDesktop )
	    p->drawText( extent()-fontMetrics().width(deskStr)-3, 15+fontMetrics().ascent(), deskStr );

	// Draw document state.
	if ( kas->showModified() )
	    paintModified( p );
    }
    else {
	// Small mode
	if ( !oneDesktop )
	    p->drawText( extent()-fontMetrics().width(deskStr)-2, 13+fontMetrics().ascent(), deskStr );
    }
}

void KasTaskItem::toggleActivateAction()
{
    hidePopup();

    if ( task_->isActive() && task_->isShaded() ) {
	task_->setShaded( false );
    }
    else {
	task_->activateRaiseOrIconify();
    }
}

void KasTaskItem::showWindowMenuAt( TQMouseEvent *ev )
{
    hidePopup();
    showWindowMenuAt( ev->globalPos() );
}

KasPopup *KasTaskItem::createPopup()
{
    KasPopup *pop = new KasTaskPopup( this );
    pop->adjustSize();
    return pop;
}

void KasTaskItem::dragOverAction()
{
    if ( !task_->isOnCurrentDesktop() )
	task_->toCurrentDesktop();
    if ( task_->isShaded() )
	task_->setShaded( false );
    if ( task_->isIconified() )
	task_->restore();
    if ( !task_->isActive() )
	task_->activate();
}

void KasTaskItem::startAutoThumbnail()
{
    if ( thumbTimer )
	return;
    if ( !kasbar()->thumbnailsEnabled() )
	return;

    if ( kasbar()->thumbnailUpdateDelay() > 0 ) {
	thumbTimer = new TQTimer( this, "thumbTimer" );
	connect( thumbTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( refreshThumbnail() ) );

	thumbTimer->start( kasbar()->thumbnailUpdateDelay() * 1000 );
    }

    TQTimer::singleShot( 200, this, TQT_SLOT( refreshThumbnail() ) );
}

void KasTaskItem::stopAutoThumbnail()
{
    if ( !thumbTimer )
	return;

    delete thumbTimer;
    thumbTimer = 0;
}

void KasTaskItem::refreshThumbnail()
{
    if ( !kasbar()->thumbnailsEnabled() )
	return;
    if ( !task_->isActive() )
	return;

    // TODO: Check if the popup obscures the window
    KasItem *i = kasbar()->itemUnderMouse();
    if ( i && i->isShowingPopup() ) {
	TQTimer::singleShot( 200, this, TQT_SLOT( refreshThumbnail() ) );
	return;
    }

    task_->setThumbnailSize( kasbar()->thumbnailSize() );
    task_->updateThumbnail();
}

void KasTaskItem::showWindowMenuAt( TQPoint p )
{
    TaskRMBMenu *tm = new TaskRMBMenu(task_, true, kasbar());
    tm->insertItem( i18n("To &Tray" ), this, TQT_SLOT( sendToTray() ) );
    tm->insertSeparator();
    tm->insertItem( i18n("&Kasbar"), kasbar()->contextMenu() );
    tm->insertSeparator();
    tm->insertItem( i18n("&Properties" ), this, TQT_SLOT( showPropertiesDialog() ) );

    mouseLeave();
    kasbar()->updateMouseOver();

    tm->exec( p );
}

void KasTaskItem::sendToTray()
{
    TQString s;
    s.setNum( task_->window() );

    TDEProcess proc;
    proc << "ksystraycmd";
    proc << "--wid" << s << "--hidden";

    bool ok = proc.start( TDEProcess::DontCare );
    if ( !ok ) {
	kdWarning(1345) << "Unable to launch ksystraycmd" << endl;
	KPassivePopup::message( i18n("Could Not Send to Tray"),
				i18n("%1").arg(strerror(errno)),
				Icon("error"),
				kasbar() );
	return;
    }

    proc.detach();
}

void KasTaskItem::showPropertiesDialog()
{
    //
    // Create Dialog
    //
    TQDialog *dlg = new TQDialog( /*kasbar()*/0L, "task_props", false );

    //
    // Title
    //
    TDEPopupTitle *title = new TDEPopupTitle( dlg, "title" );
    dlg->setCaption( i18n("Task Properties") );
    title->setText( i18n("Task Properties") );
    title->setIcon( icon() );

    //
    // Tabbed View
    //
    TQTabWidget *tabs = new TQTabWidget( dlg );
    tabs->addTab( createX11Props( tabs ), i18n("General") );
    tabs->addTab( createTaskProps( task_, tabs ), i18n("Task") );

    tabs->addTab( createTaskProps( this, tabs ), i18n("Item") );
    tabs->addTab( createTaskProps( TQT_TQOBJECT(kasbar()), tabs, false ), i18n("Bar") );

#if 0
    tabs->addTab( createNETProps( tabs ), i18n("NET") );
#endif

    //
    // Layout Dialog
    //
    TQVBoxLayout *vbl = new TQVBoxLayout( dlg, KDialog::marginHint(), KDialog::spacingHint() );
    vbl->addWidget( title );
    vbl->addWidget( tabs );

    dlg->resize( 470, 500 );
    dlg->show();

}

TQWidget *KasTaskItem::createTaskProps( TQObject *target, TQWidget *parent, bool recursive )
{
    TQVBox *vb = new TQVBox( parent );
    vb->setSpacing( KDialog::spacingHint() );
    vb->setMargin( KDialog::marginHint() );

    // Create List View
    TDEListView *taskprops = new TDEListView( vb, "props_view" );
    taskprops->setResizeMode( TQListView::LastColumn );
    taskprops->addColumn( i18n("Property"), 0 );
    taskprops->addColumn( i18n("Type"), 0 );
    taskprops->addColumn( i18n("Value") );

    // Create List Items
    TQMetaObject *mo = target->metaObject();
    for ( int i = 0; i < mo->numProperties( recursive ); i++ ) {
	const TQMetaProperty *p = mo->property(i, recursive);

	(void) new TDEListViewItem( taskprops,
				  p->name(), p->type(),
				  target->property( p->name() ).toString() );
    }

    return vb;
}

TQString KasTaskItem::expandMacros( const TQString &format, TQObject *data )
{
    TQString s = format;
    TQRegExp re("\\$(\\w+)");

    int pos = 0;
    while ( pos >= 0 ) {
        pos = re.search( s, pos );
        if ( pos >= 0 ) {
	    TQVariant val = data->property( TQString(re.cap(1)).latin1() );
	    TQString v = val.asString();
	    s.replace( pos, re.matchedLength(), v );
            pos = pos + v.length();
        }
    }

    return s;
}

TQWidget *KasTaskItem::createX11Props( TQWidget *parent )
{
    TQVBox *vb2 = new TQVBox( parent );
    vb2->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Preferred );
    vb2->setSpacing( KDialog::spacingHint() );
    vb2->setMargin( KDialog::marginHint() );

    // Create View
    new TQLabel( i18n("General"), vb2, "view" );
    TQTextView *tv = new TQTextView( vb2 );

    TQString fmt = i18n(
	"<html>"
	"<body>"
	"<b>Name</b>: $name<br>"
	"<b>Visible name</b>: $visibleName<br>"
	"<br>"
	"<b>Iconified</b>: $iconified<br>"
	"<b>Minimized</b>: $minimized<br>"
	"<b>Maximized</b>: $maximized<br>"
	"<b>Shaded</b>: $shaded<br>"
	"<b>Always on top</b>: $alwaysOnTop<br>"
	"<br>"
	"<b>Desktop</b>: $desktop<br>"
	"<b>All desktops</b>: $onAllDesktops<br>"
	"<br>"
	"<b>Iconic name</b>: $iconicName<br>"
	"<b>Iconic visible name</b>: $visibleIconicName<br>"
	"<br>"
	"<b>Modified</b>: $modified<br>"
	"<b>Demands attention</b>: $demandsAttention<br>"
	"</body>"
	"</html>"
	);

    tv->setText( expandMacros( fmt, task_ ) );
    tv->setWordWrap( TQTextEdit::WidgetWidth );

    return vb2;
}

TQWidget *KasTaskItem::createNETProps( TQWidget *parent )
{
    TQVBox *vb3 = new TQVBox( parent );
    vb3->setSpacing( KDialog::spacingHint() );
    vb3->setMargin( KDialog::marginHint() );

    // Create View
    new TQLabel( i18n("NET WM Specification Info"), vb3, "view" );
    new TQTextView( vb3 );

    return vb3;
}

#include "kastaskitem.moc"
