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
    TQLabel(NULL, "twingeometry" )
    {
    setMargin(1);
    setIndent(0);
    setLineWidth(1);
    setFrameStyle( TQFrame::Raised | TQFrame::StyledPanel );
    setAlignment( AlignCenter | AlignTop );
    sizeHints = xSizeHints;
    if( save_under )
        {
        XSetWindowAttributes attr;
        attr.save_under = True; // use saveunder if possible to avoid weird effects in transparent mode
        XChangeWindowAttributes( tqt_xdisplay(), winId(), CWSaveUnder, &attr );
        }
    }

GeometryTip::~GeometryTip()
    {
    }

void GeometryTip::setGeometry( const TQRect& geom )
    {
    int w = geom.width();
    int h = geom.height();

    if (sizeHints) 
        {
        if (sizeHints->flags & PResizeInc) 
            {
            w = ( w - sizeHints->base_width ) / sizeHints->width_inc;
            h = ( h - sizeHints->base_height ) / sizeHints->height_inc; 
            }
        }

    h = TQMAX( h, 0 ); // in case of isShade() and PBaseSize
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
