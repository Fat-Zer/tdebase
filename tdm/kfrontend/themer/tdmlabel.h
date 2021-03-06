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

#ifndef KDELABEL_H
#define KDELABEL_H

#include "tdmitem.h"

#include <tqcolor.h>
#include <tqfont.h>

class TQTimer;

/*
 * KdmLabel. A label element
 */

class KdmLabel : public KdmItem {
	Q_OBJECT

public:
	KdmLabel( KdmItem *parent, const TQDomNode &node, const char *name = 0 );
	void setText( const TQString &txt );

	/* Method to lookup the caption associated with an item */
	TQString lookupStock( const TQString &stock );

protected:
	// reimplemented; returns the minimum size of rendered text
	virtual TQSize sizeHint();

	// draw the label
	virtual void drawContents( TQPainter *p, const TQRect &r );

	// handle switching between normal / active / prelight configurations
	virtual void statusChanged();

public:
	struct LabelStruct {
		TQString text;
		bool isTimer;
		bool hasId;
		TQString id;
		struct LabelClass {
			TQColor color;
			TQFont font;
			bool present;
		} normal, active, prelight;
		int maximumWidth;
	} label;

	TQTimer *timer;

public slots:
	void update();
	void slotAccel();

private:
	/* Lookup variables in the text */
	TQString lookupText( const TQString &t );

	TQString cText;
	int cAccel;
	TQAccel *myAccel;

	void setTextInt(const TQString &);
};

#endif
