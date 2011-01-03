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

#ifndef KONQ_HISTORYCOMM_H
#define KONQ_HISTORYCOMM_H

#include <tqdatetime.h>
#include <tqstringlist.h>

#include <dcopobject.h>
#include <kurl.h>

class KonqHistoryEntry
{
public:
    //Should URLs be marshaled as strings (for DCOP, V2 history format)?
    static bool marshalURLAsStrings;
    KonqHistoryEntry()
	: numberOfTimesVisited(1) {}

    KURL url;
    TQString typedURL;
    TQString title;
    TQ_UINT32 numberOfTimesVisited;
    TQDateTime firstVisited;
    TQDateTime lastVisited;
};


// TQDataStream operators (read and write a KonqHistoryEntry
// from/into a QDataStream
TQDataStream& operator<< (TQDataStream& s, const KonqHistoryEntry& e);
TQDataStream& operator>> (TQDataStream& s, KonqHistoryEntry& e);

///////////////////////////////////////////////////////////////////


/**
 * DCOP Methods for KonqHistoryManager. Has to be in a separate file, because
 * dcopidl2cpp barfs on every second construct ;(
 * Implementations of the pure virtual methods are in KonqHistoryManager
 */
class KonqHistoryComm : public DCOPObject
{
    K_DCOP

protected:
    KonqHistoryComm( TQCString objId ) : DCOPObject( objId ) {}

k_dcop:
    virtual ASYNC notifyHistoryEntry( KonqHistoryEntry e, TQCString saveId) = 0;
    virtual ASYNC notifyMaxCount( TQ_UINT32 count, TQCString saveId ) = 0;
    virtual ASYNC notifyMaxAge( TQ_UINT32 days, TQCString saveId ) = 0;
    virtual ASYNC notifyClear( TQCString saveId ) = 0;
    virtual ASYNC notifyRemove( KURL url, TQCString saveId ) = 0;
    virtual ASYNC notifyRemove( KURL::List url, TQCString saveId ) = 0;
    virtual TQStringList allURLs() const = 0;

};

#endif // KONQ_HISTORYCOMM_H
