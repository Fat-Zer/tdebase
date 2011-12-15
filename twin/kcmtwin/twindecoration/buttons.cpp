/*
	This is the new twindecoration kcontrol module

	Copyright (c) 2004,  Sandro Giessl <sandro@giessl.com>
	Copyright (c) 2001
		Karol Szwed <gallium@kde.org>
		http://gallium.n3.net/

	Supports new twin configuration plugins, and titlebar button position
	modification via dnd interface.

	Based on original "twintheme" (Window Borders)
	Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include <tqheader.h>
#include <tqpainter.h>
#include <tqlabel.h>
#include <layout.h>
#include <tqstyle.h>

#include <kdebug.h>

#include <kdialog.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include <kdecorationfactory.h>

#include "buttons.h"
#include "pixmaps.h"


#define BUTTONDRAGMIMETYPE "application/x-kde_twindecoration_buttons"
ButtonDrag::ButtonDrag( Button btn, TQWidget* parent, const char* name)
	: TQStoredDrag( BUTTONDRAGMIMETYPE, parent, name)
{
	TQByteArray data;
	TQDataStream stream(data, IO_WriteOnly);
	stream << btn.name;
	stream << btn.icon;
	stream << btn.type.unicode();
	stream << (int) btn.duplicate;
	stream << (int) btn.supported;
	setEncodedData( data );
}


bool ButtonDrag::canDecode( TQDropEvent* e )
{
	return e->provides( BUTTONDRAGMIMETYPE );
}

bool ButtonDrag::decode( TQDropEvent* e, Button& btn )
{
	TQByteArray data = e->data( BUTTONDRAGMIMETYPE );
	if ( data.size() )
	{
		e->accept();
		TQDataStream stream(data, IO_ReadOnly);
		stream >> btn.name;
		stream >> btn.icon;
		ushort type;
		stream >> type;
		btn.type = TQChar(type);
		int duplicate;
		stream >> duplicate;
		btn.duplicate = duplicate;
		int supported;
		stream >> supported;
		btn.supported = supported;
		return TRUE;
	}
	return FALSE;
}


Button::Button()
{
}

Button::Button(const TQString& n, const TQBitmap& i, TQChar t, bool d, bool s)
	: name(n),
	  icon(i),
	  type(t),
	  duplicate(d),
	  supported(s)
{
}

Button::~Button()
{
}

// helper function to deal with the Button's bitmaps more easily...
TQPixmap bitmapPixmap(const TQBitmap& bm, const TQColor& color)
{
	TQPixmap pm(bm.size() );
	pm.setMask(bm);
	TQPainter p(&pm);
	p.setPen(color);
	p.drawPixmap(0,0,bm);
	p.end();
	return pm;
}


ButtonSource::ButtonSource(TQWidget *parent, const char* name)
	: KListView(parent, name)
{
	setSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Expanding);

	setResizeMode(TQListView::AllColumns);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropVisualizer(false);
	setSorting(-1);
	header()->setClickEnabled(false);
	header()->hide();

	addColumn(i18n("Buttons") );
}

ButtonSource::~ButtonSource()
{
}

TQSize ButtonSource::sizeHint() const
{
	// make the sizeHint height a bit smaller than the one of TQListView...

	if ( cachedSizeHint().isValid() )
		return cachedSizeHint();

	constPolish();

	TQSize s( header()->sizeHint() );

	if ( verticalScrollBar()->isVisible() )
		s.setWidth( s.width() + tqstyle().pixelMetric(TQStyle::PM_ScrollBarExtent) );
	s += TQSize(frameWidth()*2,frameWidth()*2);

	// size hint: 4 lines of text...
	s.setHeight( s.height() + fontMetrics().lineSpacing()*3 );

	setCachedSizeHint( s );

	return s;
}

void ButtonSource::hideAllButtons()
{
	TQListViewItemIterator it(this);
	while (it.current() ) {
		it.current()->setVisible(false);
		++it;
	}
}

void ButtonSource::showAllButtons()
{
	TQListViewItemIterator it(this);
	while (it.current() ) {
		it.current()->setVisible(true);
		++it;
	}
}

void ButtonSource::showButton( TQChar btn )
{
	TQListViewItemIterator it(this);
	while (it.current() ) {
		ButtonSourceItem *item = dynamic_cast<ButtonSourceItem*>(it.current() );
		if (item && item->button().type == btn) {
			it.current()->setVisible(true);
			return;
		}
		++it;
	}
}

void ButtonSource::hideButton( TQChar btn )
{
	TQListViewItemIterator it(this);
	while (it.current() ) {
		ButtonSourceItem *item = dynamic_cast<ButtonSourceItem*>(it.current() );
		if (item && item->button().type == btn && !item->button().duplicate) {
			it.current()->setVisible(false);
			return;
		}
		++it;
	}
}

bool ButtonSource::acceptDrag(TQDropEvent* e) const
{
	return acceptDrops() && ButtonDrag::canDecode(e);
}

TQDragObject *ButtonSource::dragObject()
{
	ButtonSourceItem *i = dynamic_cast<ButtonSourceItem*>(selectedItem() );

	if (i) {
		ButtonDrag *bd = new ButtonDrag(i->button(), viewport(), "button_drag");
		bd->setPixmap(bitmapPixmap(i->button().icon, colorGroup().foreground() ));
		return bd;
	}

	return 0;
}

ButtonDropSiteItem::ButtonDropSiteItem(const Button& btn)
	: m_button(btn)
{
}

ButtonDropSiteItem::~ButtonDropSiteItem()
{
}

Button ButtonDropSiteItem::button()
{
	return m_button;
}

int ButtonDropSiteItem::width()
{
// 	return m_button.icon.width();
	return 20;
}

int ButtonDropSiteItem::height()
{
// 	return m_button.icon.height();
	return 20;
}

void ButtonDropSiteItem::draw(TQPainter *p, const TQColorGroup& cg, TQRect r)
{
// 	p->fillRect(r, cg.base() );
	if (m_button.supported)
		p->setPen(cg.foreground() );
	else
		p->setPen(cg.mid() );
	TQBitmap &i = m_button.icon;
	p->drawPixmap(r.left()+(r.width()-i.width())/2, r.top()+(r.height()-i.height())/2, i);
}


ButtonDropSite::ButtonDropSite( TQWidget* parent, const char* name )
	: TQFrame( parent, name ),
	  m_selected(0)
{
	setAcceptDrops( TRUE );
	setFrameShape( WinPanel );
	setFrameShadow( Raised );
	setMinimumHeight( 26 );
	setMaximumHeight( 26 );
	setMinimumWidth( 250 );		// Ensure buttons will fit
}

ButtonDropSite::~ButtonDropSite()
{
	clearLeft();
	clearRight();
}

void ButtonDropSite::clearLeft()
{
	while (!buttonsLeft.isEmpty() ) {
		ButtonDropSiteItem *item = buttonsLeft.first();
		if (removeButton(item) ) {
			emit buttonRemoved(item->button().type);
			delete item;
		}
	}
}

void ButtonDropSite::clearRight()
{
	while (!buttonsRight.isEmpty() ) {
		ButtonDropSiteItem *item = buttonsRight.first();
		if (removeButton(item) ) {
			emit buttonRemoved(item->button().type);
			delete item;
		}
	}
}

void ButtonDropSite::dragMoveEvent( TQDragMoveEvent* e )
{
	TQPoint p = e->pos();
	if (leftDropArea().contains(p) || rightDropArea().contains(p) || buttonAt(p) ) {
		e->accept();

		// 2 pixel wide drop visualizer...
		TQRect r = contentsRect();
		int x = -1;
		if (leftDropArea().contains(p) ) {
			x = leftDropArea().left();
		} else if (rightDropArea().contains(p) ) {
			x = rightDropArea().right()+1;
		} else {
			ButtonDropSiteItem *item = buttonAt(p);
			if (item) {
				if (p.x() < item->rect.left()+item->rect.width()/2 ) {
					x = item->rect.left();
				} else {
					x = item->rect.right()+1;
				}
			}
		}
		if (x != -1) {
			TQRect tmpRect(x, r.y(), 2, r.height() );
			if (tmpRect != m_oldDropVisualizer) {
				cleanDropVisualizer();
				m_oldDropVisualizer = tmpRect;
				update(tmpRect);
			}
		}

	} else {
		e->ignore();

		cleanDropVisualizer();
	}
}

void ButtonDropSite::cleanDropVisualizer()
{
	if (m_oldDropVisualizer.isValid())
	{
		TQRect rect = m_oldDropVisualizer;
		m_oldDropVisualizer = TQRect(); // rect is invalid
		update(rect);
	}
}

void ButtonDropSite::dragEnterEvent( TQDragEnterEvent* e )
{
	if ( ButtonDrag::canDecode( e ) )
		e->accept();
}

void ButtonDropSite::dragLeaveEvent( TQDragLeaveEvent* /* e */ )
{
	cleanDropVisualizer();
}

void ButtonDropSite::dropEvent( TQDropEvent* e )
{
	cleanDropVisualizer();

	TQPoint p = e->pos();

	// collect information where to insert the dropped button
	ButtonList *buttonList = 0;
	ButtonList::iterator buttonPosition;

	if (leftDropArea().contains(p) ) {
		buttonList = &buttonsLeft;
		buttonPosition = buttonsLeft.end();
	} else if (rightDropArea().contains(p) ) {
		buttonList = &buttonsRight;
		buttonPosition = buttonsRight.begin();
	} else {
		ButtonDropSiteItem *aboveItem = buttonAt(p);
		if (!aboveItem)
			return; // invalid drop. hasn't occured _over_ a button (or left/right dropArea), return...

		ButtonList::iterator it;
		if (!getItemIterator(aboveItem, buttonList, it) ) {
			// didn't find the aboveItem. unlikely to happen since buttonAt() already seems to have found
			// something valid. anyway...
			return;
		}

		// got the list and the aboveItem position. now determine if the item should be inserted
		// before aboveItem or after aboveItem.
		TQRect aboveItemRect = aboveItem->rect;
		if (!aboveItemRect.isValid() )
			return;

		if (p.x() < aboveItemRect.left()+aboveItemRect.width()/2 ) {
			// insert before the item
			buttonPosition = it;
		} else {
			if (it != buttonList->end() )
				buttonPosition = ++it;
			else
				buttonPosition = it; // already at the end(), can't increment the iterator!
		}
	}

	// know where to insert the button. now see if we can use an existing item (drag within the widget = move)
	// orneed to create a new one
	ButtonDropSiteItem *buttonItem = 0;
	if (e->source() == this && m_selected) {
		ButtonList *oldList = 0;
		ButtonList::iterator oldPos;
		if (getItemIterator(m_selected, oldList, oldPos) ) {
			if (oldPos == buttonPosition)
				return; // button didn't change its position during the drag...

			oldList->remove(oldPos);
			buttonItem = m_selected;
		} else {
			return; // m_selected not found, return...
		}
	} else {
		// create new button from the drop object...
		Button btn;
		if (ButtonDrag::decode(e, btn) ) {
			buttonItem = new ButtonDropSiteItem(btn);
		} else {
			return; // something has gone wrong while we were trying to decode the drop event
		}
	}

	// now the item can actually be inserted into the list! :)
	(*buttonList).insert(buttonPosition, buttonItem);
	emit buttonAdded(buttonItem->button().type);
	emit changed();
	recalcItemGeometry();
	update();
}

bool ButtonDropSite::getItemIterator(ButtonDropSiteItem *item, ButtonList* &list, ButtonList::iterator &iterator)
{
	if (!item)
		return false;

	ButtonList::iterator it = buttonsLeft.find(item); // try the left list first...
	if (it == buttonsLeft.end() ) {
		it = buttonsRight.find(item); // try the right list...
		if (it == buttonsRight.end() ) {
			return false; // item hasn't been found in one of the list, return...
		} else {
			list = &buttonsRight;
			iterator = it;
		}
	} else {
		list = &buttonsLeft;
		iterator = it;
	}

	return true;
}

TQRect ButtonDropSite::leftDropArea()
{
	// return a 10 pixel drop area...
	TQRect r = contentsRect();

	int leftButtonsWidth = calcButtonListWidth(buttonsLeft);
	return TQRect(r.left()+leftButtonsWidth, r.top(), 10, r.height() );
}

TQRect ButtonDropSite::rightDropArea()
{
	// return a 10 pixel drop area...
	TQRect r = contentsRect();

	int rightButtonsWidth = calcButtonListWidth(buttonsRight);
	return TQRect(r.right()-rightButtonsWidth-10, r.top(), 10, r.height() );
}

void ButtonDropSite::mousePressEvent( TQMouseEvent* e )
{
	// TODO: only start the real drag after some drag distance
	m_selected = buttonAt(e->pos() );
	if (m_selected) {
		ButtonDrag *bd = new ButtonDrag(m_selected->button(), this);
		bd->setPixmap(bitmapPixmap(m_selected->button().icon, colorGroup().foreground() ) );
		bd->dragMove();
	}
}

void ButtonDropSite::resizeEvent(TQResizeEvent*)
{
	recalcItemGeometry();
}

void ButtonDropSite::recalcItemGeometry()
{
	TQRect r = contentsRect();

	// update the geometry of the items in the left button list
	int offset = r.left();
	for (ButtonList::const_iterator it = buttonsLeft.begin(); it != buttonsLeft.end(); ++it) {
		int w = (*it)->width();
		(*it)->rect = TQRect(offset, r.top(), w, (*it)->height() );
		offset += w;
	}

	// the right button list...
	offset = r.right() - calcButtonListWidth(buttonsRight);
	for (ButtonList::const_iterator it = buttonsRight.begin(); it != buttonsRight.end(); ++it) {
		int w = (*it)->width();
		(*it)->rect = TQRect(offset, r.top(), w, (*it)->height() );
		offset += w;
	}
}

ButtonDropSiteItem *ButtonDropSite::buttonAt(TQPoint p) {
	// try to find the item in the left button list
	for (ButtonList::const_iterator it = buttonsLeft.begin(); it != buttonsLeft.end(); ++it) {
		if ( (*it)->rect.contains(p) ) {
			return *it;
		}
	}

	// try to find the item in the right button list
	for (ButtonList::const_iterator it = buttonsRight.begin(); it != buttonsRight.end(); ++it) {
		if ( (*it)->rect.contains(p) ) {
			return *it;
		}
	}

	return 0;
}

bool ButtonDropSite::removeButton(ButtonDropSiteItem *item) {
	if (!item)
		return false;

	// try to remove the item from the left button list
	if (buttonsLeft.remove(item) >= 1) {
		return true;
	}

	// try to remove the item from the right button list
	if (buttonsRight.remove(item) >= 1) {
		return true;
	}

	return false;
}

int ButtonDropSite::calcButtonListWidth(const ButtonList& btns)
{
	int w = 0;
	for (ButtonList::const_iterator it = btns.begin(); it != btns.end(); ++it) {
		w += (*it)->width();
	}

	return w;
}

bool ButtonDropSite::removeSelectedButton()
{
	bool succ = removeButton(m_selected);
	if (succ) {
		emit buttonRemoved(m_selected->button().type);
		emit changed();
		delete m_selected;
		m_selected = 0;
		recalcItemGeometry();
		update(); // repaint...
	}

	return succ;
}

void ButtonDropSite::drawButtonList(TQPainter *p, const ButtonList& btns, int offset)
{
	for (ButtonList::const_iterator it = btns.begin(); it != btns.end(); ++it) {
		TQRect itemRect = (*it)->rect;
		if (itemRect.isValid() ) {
			(*it)->draw(p, colorGroup(), itemRect);
		}
		offset += (*it)->width();
	}
}

void ButtonDropSite::drawContents( TQPainter* p )
{
	int leftoffset = calcButtonListWidth( buttonsLeft );
	int rightoffset = calcButtonListWidth( buttonsRight );
	int offset = 3;

	TQRect r = contentsRect();

	// Shrink by 1
	r.moveBy(1 + leftoffset, 1);
	r.setWidth( r.width() - 2 - leftoffset - rightoffset );
	r.setHeight( r.height() - 2 );

	drawButtonList( p, buttonsLeft, offset );

	TQColor c1( 0x0A, 0x5F, 0x89 );		// KDE 2 titlebar default colour
	p->fillRect( r, c1 );
	p->setPen( Qt::white );
	p->setFont( TQFont( KGlobalSettings::generalFont().family(), 12, TQFont::Bold) );
	p->drawText( r, AlignLeft | AlignVCenter, i18n("KDE") );

	offset = geometry().width() - 3 - rightoffset;
	drawButtonList( p, buttonsRight, offset );

	if (m_oldDropVisualizer.isValid() )
	{
		p->fillRect(m_oldDropVisualizer, Dense4Pattern);
	}
}

ButtonSourceItem::ButtonSourceItem(TQListView * parent, const Button& btn)
	: TQListViewItem(parent),
	  m_button(btn),
	  m_dirty(true)
{
	setButton(btn);
}

ButtonSourceItem::~ButtonSourceItem()
{
}

void ButtonSourceItem::paintCell(TQPainter *p, const TQColorGroup &cg, int column, int width, int align)
{
	// we need the color group cg, so to the work here, not in setButton...
	if (m_dirty) {
		if (m_button.supported) {
			setPixmap(0, bitmapPixmap(m_button.icon, cg.foreground() ) );
		} else {
			setPixmap(0, bitmapPixmap(m_button.icon, cg.mid() ) );
		}
		m_dirty = false;
	}

	if (m_button.supported) {
		TQListViewItem::paintCell(p,cg,column,width,align);
	} else {
		// grey out unsupported buttons
		TQColorGroup cg2 = cg;
		cg2.setColor(TQColorGroup::Text, cg.mid() );
		TQListViewItem::paintCell(p,cg2,column,width,align);
	}
}

void ButtonSourceItem::setButton(const Button& btn)
{
	m_button = btn;
	m_dirty = true; // update the pixmap when in paintCell()...
	if (btn.supported) {
		setText(0, btn.name);
	} else {
		setText(0, i18n("%1 (unavailable)").arg(btn.name) );
	}
}

Button ButtonSourceItem::button() const
{
	return m_button;
}


ButtonPositionWidget::ButtonPositionWidget(TQWidget *parent, const char* name)
    : TQWidget(parent,name),
      m_factory(0)
{
	TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );
	setSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Maximum);

	TQLabel* label = new TQLabel( this );
	m_dropSite = new ButtonDropSite( this );
	label->setAlignment( int( TQLabel::WordBreak ) );
	label->setText( i18n( "To add or remove titlebar buttons, simply <i>drag</i> items "
		"between the available item list and the titlebar preview. Similarly, "
		"drag items within the titlebar preview to re-position them.") );
	m_buttonSource = new ButtonSource(this, "button_source");

	layout->addWidget(label);
	layout->addWidget(m_dropSite);
	layout->addWidget(m_buttonSource);

	connect( m_dropSite, TQT_SIGNAL(buttonAdded(TQChar)), m_buttonSource, TQT_SLOT(hideButton(TQChar)) );
	connect( m_dropSite, TQT_SIGNAL(buttonRemoved(TQChar)), m_buttonSource, TQT_SLOT(showButton(TQChar)) );
	connect( m_buttonSource, TQT_SIGNAL(dropped(TQDropEvent*, TQListViewItem*)), m_dropSite, TQT_SLOT(removeSelectedButton()) );

	connect( m_dropSite, TQT_SIGNAL(changed()), TQT_SIGNAL(changed()) );

	// insert all possible buttons into the source (backwards to keep the preferred order...)
	bool dummy;
	new ButtonSourceItem(m_buttonSource, getButton('R', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('L', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('B', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('F', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('X', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('A', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('I', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('H', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('S', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('M', dummy) );
	new ButtonSourceItem(m_buttonSource, getButton('_', dummy) );
}

ButtonPositionWidget::~ButtonPositionWidget()
{
}

void ButtonPositionWidget::setDecorationFactory(KDecorationFactory *factory)
{
	if (!factory)
		return;

	m_factory = factory;

	// get the list of supported buttons
	if (m_factory->supports(KDecorationDefines::AbilityAnnounceButtons) ) {
		TQString supportedButtons;

		if (m_factory->supports(KDecorationDefines::AbilityButtonMenu) )
			supportedButtons.append('M');
		if (m_factory->supports(KDecorationDefines::AbilityButtonOnAllDesktops) )
			supportedButtons.append('S');
		if (m_factory->supports(KDecorationDefines::AbilityButtonSpacer) )
			supportedButtons.append('_');
		if (m_factory->supports(KDecorationDefines::AbilityButtonHelp) )
			supportedButtons.append('H');
		if (m_factory->supports(KDecorationDefines::AbilityButtonMinimize) )
			supportedButtons.append('I');
		if (m_factory->supports(KDecorationDefines::AbilityButtonMaximize) )
			supportedButtons.append('A');
		if (m_factory->supports(KDecorationDefines::AbilityButtonClose) )
			supportedButtons.append('X');
		if (m_factory->supports(KDecorationDefines::AbilityButtonAboveOthers) )
			supportedButtons.append('F');
		if (m_factory->supports(KDecorationDefines::AbilityButtonBelowOthers) )
			supportedButtons.append('B');
		if (m_factory->supports(KDecorationDefines::AbilityButtonShade) )
			supportedButtons.append('L');
		if (m_factory->supports(KDecorationDefines::AbilityButtonResize) )
			supportedButtons.append('R');

		m_supportedButtons = supportedButtons;
	} else {
		// enable only buttons available before AbilityButton* introduction
		m_supportedButtons = "MSHIAX_";
	}

	// update the button lists...
	// 1. set status on the source items...
	TQListViewItemIterator it(m_buttonSource);
	while (it.current() ) {
		ButtonSourceItem *i = dynamic_cast<ButtonSourceItem*>(it.current() );
		if (i) {
			Button b = i->button();
			b.supported = m_supportedButtons.contains(b.type);
			i->setButton(b);
		}
		++it;
	}
	// 2. rebuild the drop site items...
	setButtonsLeft(buttonsLeft() );
	setButtonsRight(buttonsRight() );
}

Button ButtonPositionWidget::getButton(TQChar type, bool& success) {
	success = true;

	if (type == 'R') {
		TQBitmap bmp(resize_width, resize_height, resize_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Resize"), bmp, 'R', false, m_supportedButtons.contains('R') );
	} else if (type == 'L') {
		TQBitmap bmp(shade_width, shade_height, shade_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Shade"), bmp, 'L', false, m_supportedButtons.contains('L') );
	} else if (type == 'B') {
		TQBitmap bmp(keepbelowothers_width, keepbelowothers_height, keepbelowothers_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Keep Below Others"), bmp, 'B', false, m_supportedButtons.contains('B') );
	} else if (type == 'F') {
		TQBitmap bmp(keepaboveothers_width, keepaboveothers_height, keepaboveothers_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Keep Above Others"), bmp, 'F', false, m_supportedButtons.contains('F') );
	} else if (type == 'X') {
		TQBitmap bmp(close_width, close_height, close_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Close"), bmp, 'X', false,  m_supportedButtons.contains('X') );
	} else if (type == 'A') {
		TQBitmap bmp(maximize_width, maximize_height, maximize_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Maximize"), bmp, 'A', false, m_supportedButtons.contains('A') );
	} else if (type == 'I') {
		TQBitmap bmp(minimize_width, minimize_height, minimize_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Minimize"), bmp, 'I', false, m_supportedButtons.contains('I') );
	} else if (type == 'H') {
		TQBitmap bmp(help_width, help_height, help_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Help"), bmp, 'H', false, m_supportedButtons.contains('H') );
	} else if (type == 'S') {
		TQBitmap bmp(onalldesktops_width, onalldesktops_height, onalldesktops_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("On All Desktops"), bmp, 'S', false, m_supportedButtons.contains('S') );
	} else if (type == 'M') {
		TQBitmap bmp(menu_width, menu_height, menu_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("Menu"), bmp, 'M', false, m_supportedButtons.contains('M') );
	} else if (type == '_') {
		TQBitmap bmp(spacer_width, spacer_height, spacer_bits, true);
		bmp.setMask(bmp);
		return Button(i18n("--- spacer ---"), bmp, '_', true, m_supportedButtons.contains('_') );
	} else {
		success = false;
		return Button();
	}
}

TQString ButtonPositionWidget::buttonsLeft() const
{
	ButtonList btns = m_dropSite->buttonsLeft;
	TQString btnString = "";
	for (ButtonList::const_iterator it = btns.begin(); it != btns.end(); ++it) {
		btnString.append( (*it)->button().type );
	}
	return btnString;
}

TQString ButtonPositionWidget::buttonsRight() const
{
	ButtonList btns = m_dropSite->buttonsRight;
	TQString btnString = "";
	for (ButtonList::const_iterator it = btns.begin(); it != btns.end(); ++it) {
		btnString.append( (*it)->button().type );
	}
	return btnString;
}

void ButtonPositionWidget::setButtonsLeft(const TQString &buttons)
{
	// to keep the button lists consistent, first remove all left buttons, then add buttons again...
	m_dropSite->clearLeft();

	for (uint i = 0; i < buttons.length(); ++i) {
		bool succ = false;
		Button btn = getButton(buttons[i], succ);
		if (succ) {
			m_dropSite->buttonsLeft.append(new ButtonDropSiteItem(btn) );
			m_buttonSource->hideButton(btn.type);
		}
	}
	m_dropSite->recalcItemGeometry();
	m_dropSite->update();
}
	
void ButtonPositionWidget::setButtonsRight(const TQString &buttons)
{
	// to keep the button lists consistent, first remove all left buttons, then add buttons again...
	m_dropSite->clearRight();

	for (uint i = 0; i < buttons.length(); ++i) {
		bool succ = false;
		Button btn = getButton(buttons[i], succ);
		if (succ) {
			m_dropSite->buttonsRight.append(new ButtonDropSiteItem(btn) );
			m_buttonSource->hideButton(btn.type);
		}
	}
	m_dropSite->recalcItemGeometry();
	m_dropSite->update();
}

#include "buttons.moc"
// vim: ts=4
// kate: space-indent off; tab-width 4;
