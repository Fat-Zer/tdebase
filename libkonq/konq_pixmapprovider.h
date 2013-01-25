/* This file is part of the KDE project
   Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KONQ_PIXMAPPROVIDER_H
#define KONQ_PIXMAPPROVIDER_H

#include <tqmap.h>

#include <kpixmapprovider.h>
#include "konq_faviconmgr.h"

#include <libkonq_export.h>

class TDEConfig;

class LIBKONQ_EXPORT KonqPixmapProvider : public KonqFavIconMgr, virtual public KPixmapProvider
{
public:
    static KonqPixmapProvider * self();

    virtual ~KonqPixmapProvider();

    /**
     * Looks up a pixmap for @p url. Uses a cache for the iconname of url.
     */
    virtual TQPixmap pixmapFor( const TQString& url, int size = 0 );

    /**
     * Loads the cache to @p kc from the current TDEConfig-group from key @p key.
     */
    void load( TDEConfig * kc, const TQString& key );
    /**
     * Saves the cache to @p kc into the current TDEConfig-group as key @p key.
     * Only those @p items are saved, otherwise the cache would grow forever.
     */
    void save( TDEConfig *, const TQString& key, const TQStringList& items );

    /**
     * Clears the pixmap cache
     */
    void clear();

    /**
     * Looks up an iconname for @p url. Uses a cache for the iconname of url.
     * @since 3.4.1
     */
    TQString iconNameFor( const TQString& url );

protected:
    KonqPixmapProvider( TQObject *parent=0, const char *name=0 );

    /**
     * Overridden from KonqFavIconMgr to update the cache
     */
    virtual void notifyChange( bool isHost, TQString hostOrURL, TQString iconName );

    TQPixmap loadIcon( const TQString& url, const TQString& icon, int size );

private:
    TQMap<TQString,TQString> iconMap;
    static KonqPixmapProvider * s_self;
};


#endif // KONQ_PIXMAPPROVIDER_H
