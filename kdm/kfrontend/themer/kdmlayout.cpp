/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdmlayout.h"
#include "kdmconfig.h"
#include "kdmitem.h"

#include <kdebug.h>

#include <tqdom.h>
#include <tqrect.h>

KdmLayoutFixed::KdmLayoutFixed( const TQDomNode &/*node*/ )
{
	//Parsing FIXED parameters on 'node' [NONE!]
}

void
KdmLayoutFixed::update( const TQRect &parentGeometry, bool force )
{
	kdDebug() << timestamp() << " KdmLayoutFixed::update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (parentGeometry.width() < 0 || parentGeometry.height() < 0) {
		kdDebug() << timestamp() << " invalid\n";
		return;
	}
	// For each child in list I ask their hinted size and set it!
	for (TQValueList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it)
		(*it)->setGeometry( (*it)->placementHint( parentGeometry ), force );
}

KdmLayoutBox::KdmLayoutBox( const TQDomNode &node )
{
	//Parsing BOX parameters
	TQDomNode n = node;
	TQDomElement el = n.toElement();
	box.isVertical = el.attribute( "orientation", "vertical" ) != "horizontal";
	box.xpadding = el.attribute( "xpadding", "0" ).toInt();
	box.ypadding = el.attribute( "ypadding", "0" ).toInt();
	box.spacing = el.attribute( "spacing", "0" ).toInt();
	box.minwidth = el.attribute( "min-width", "0" ).toInt();
	box.minheight = el.attribute( "min-height", "0" ).toInt();
	box.homogeneous = el.attribute( "homogeneous", "false" ) == "true";
}

void
KdmLayoutBox::update( const TQRect &parentGeometry, bool force )
{
	kdDebug() << this << " update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (!parentGeometry.isValid() || parentGeometry.isEmpty())
		return;

	// Check if box size was computed. If not compute it
	// TODO check if this prevents updating changing items
//	if (!hintedSize.isValid())
//		tqsizeHint();

//	kdDebug() << this << " hintedSize " << hintedSize << endl;

	//XXX why was this asymmetric? it broke things big time.
	TQRect childrenRect = /*box.isVertical ? TQRect( parentGeometry.topLeft(), hintedSize ) :*/ parentGeometry;
	// Begin cutting the parent rectangle to attach children on the right place
	childrenRect.addCoords( box.xpadding, box.ypadding, -box.xpadding, -box.ypadding );

	kdDebug() << this << " childrenRect " << childrenRect << endl;

	// For each child in list ...
	if (box.homogeneous) {
		int ccnt = 0;
		for (TQValueList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it)
			if (!(*it)->isExplicitlyHidden())
				ccnt++;
		int height = (childrenRect.height() - (ccnt - 1) * box.spacing) / ccnt;
		int width = (childrenRect.width() - (ccnt - 1) * box.spacing) / ccnt;

		for (TQValueList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
			if ((*it)->isExplicitlyHidden())
				continue;
			if (box.isVertical) {
				TQRect temp( childrenRect.left(), childrenRect.top(), childrenRect.width(), height );
				(*it)->setGeometry( temp, force );
				childrenRect.setTop( childrenRect.top() + height + box.spacing );
			} else {
				TQRect temp( childrenRect.left(), childrenRect.top(), width, childrenRect.height() );
				kdDebug() << timestamp() << " placement " << *it << " " << temp << " " << (*it)->placementHint( temp ) << endl;
				temp = (*it)->placementHint( temp );
				(*it)->setGeometry( temp, force );
				childrenRect.setLeft( childrenRect.left() + width + box.spacing );
			}
		}
	} else {
		for (TQValueList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
			if ((*it)->isExplicitlyHidden())
				continue;

			TQRect temp = childrenRect, tqitemRect;
			if (box.isVertical) {
				temp.setHeight( 0 );
				tqitemRect = (*it)->placementHint( temp );
				temp.setHeight( tqitemRect.height() );
				childrenRect.setTop( childrenRect.top() + tqitemRect.size().height() + box.spacing );
			} else {
				temp.setWidth( 0 );
				tqitemRect = (*it)->placementHint( temp );
				kdDebug() << this << " placementHint " << *it << " " << temp << " " << tqitemRect << endl;
				temp.setWidth( tqitemRect.width() );
				childrenRect.setLeft( childrenRect.left() + tqitemRect.size().width() + box.spacing );
				kdDebug() << timestamp() << " childrenRect after " << *it << " " << childrenRect << endl;
			}
			tqitemRect = (*it)->placementHint( temp );
			kdDebug() << this << " placementHint2 " << *it << " " << temp << " " << tqitemRect << endl;
			(*it)->setGeometry( tqitemRect, force );
		}
	}
}

//FIXME truly experimental (is so close to greeter_geometry.c)
TQSize
KdmLayoutBox::tqsizeHint()
{
	// Sum up area taken by children
	int w = 0, h = 0;
	for (TQValueList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
		TQSize s = (*it)->placementHint( TQRect() ).size();
		if (box.isVertical) {
			if (s.width() > w)
				w = s.width();
			h += s.height();
		} else {
			if (s.height() > h)
				h = s.height();
			w += s.width();
		}
	}

	// Add padding and items spacing
	w += 2 * box.xpadding;
	h += 2 * box.ypadding;
	if (box.isVertical)
		h += box.spacing * (m_children.count() - 1);
	else
		w += box.spacing * (m_children.count() - 1);

	// Make hint at least equal to minimum size (if set)
	return TQSize( w < box.minwidth ? box.minwidth : w,
		      h < box.minheight ? box.minheight : h );
}
