/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (c) 2003, Karol Szwed <kszwed@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include "geometrytip.h"

namespace KWinInternal
{

GeometryTip::GeometryTip( const XSizeHints* xSizeHints, bool save_under ):
    TQLabel(NULL, "kwingeometry" )
    {
    setMargin(1);
    setIndent(0);
    setLineWidth(1);
    setFrameStyle( TQFrame::Raised | TQFrame::StyledPanel );
    tqsetAlignment( AlignCenter | AlignTop );
    tqsizeHints = xSizeHints;
    if( save_under )
        {
        XSetWindowAttributes attr;
        attr.save_under = True; // use saveunder if possible to avoid weird effects in transparent mode
        XChangeWindowAttributes( qt_xdisplay(), winId(), CWSaveUnder, &attr );
        }
    }

GeometryTip::~GeometryTip()
    {
    }

void GeometryTip::setGeometry( const TQRect& geom )
    {
    int w = geom.width();
    int h = geom.height();

    if (tqsizeHints) 
        {
        if (tqsizeHints->flags & PResizeInc) 
            {
            w = ( w - tqsizeHints->base_width ) / tqsizeHints->width_inc;
            h = ( h - tqsizeHints->base_height ) / tqsizeHints->height_inc; 
            }
        }

    h = QMAX( h, 0 ); // in case of isShade() and PBaseSize
    TQString pos;
    pos.sprintf( "%+d,%+d<br>(<b>%d&nbsp;x&nbsp;%d</b>)",
                     geom.x(), geom.y(), w, h );
    setText( pos );
    adjustSize();
    move( geom.x() + ((geom.width()  - width())  / 2),
          geom.y() + ((geom.height() - height()) / 2) );
    }

} // namespace

#include "geometrytip.moc"
