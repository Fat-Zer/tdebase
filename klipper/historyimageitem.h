// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

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

#ifndef _HISTORYIMAGEITEM_H_
#define _HISTORYIMAGEITEM_H_

#include "historyitem.h"
#include <tqpixmap.h>
#include <tqdragobject.h>

/**
 * A image entry in the clipboard history.
 */
class HistoryImageItem : public HistoryItem
{
public:
    HistoryImageItem( const TQPixmap& data );
    virtual ~HistoryImageItem() {}
    virtual TQString text() const;
    virtual bool operator==( const HistoryItem& rhs) const {
        if ( const HistoryImageItem* casted_rhs = dynamic_cast<const HistoryImageItem*>( &rhs ) ) {
            return &casted_rhs->m_data == &m_data; // Not perfect, but better than nothing.
        }
        return false;
    }
    virtual const TQPixmap& image() const { return m_data; }
    virtual TQMimeSource* mimeSource() const { return new TQImageDrag( m_data.convertToImage()) ; }

    virtual void write( TQDataStream& stream ) const;

private:
    /**
     *
     */
    const TQPixmap m_data;
    /**
     * Cache for m_data's string representation
     */
    mutable TQString m_text;
};



#endif
