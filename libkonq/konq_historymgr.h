/* This file is part of the KDE project
   Copyright (C) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KONQ_HISTORY_H
#define KONQ_HISTORY_H

#include <tqdatastream.h>
#include <tqfile.h>
#include <tqptrlist.h>
#include <tqobject.h>
#include <tqmap.h>
#include <tqtimer.h>

#include <dcopobject.h>

#include <kcompletion.h>
#include <kurl.h>
#include <tdeparts/historyprovider.h>

#include "konq_historycomm.h"

#include <libkonq_export.h>

class TDECompletion;


typedef TQPtrList<KonqHistoryEntry> KonqBaseHistoryList;
typedef TQPtrListIterator<KonqHistoryEntry> KonqHistoryIterator;

class LIBKONQ_EXPORT KonqHistoryList : public KonqBaseHistoryList
{
public:
    /**
     * Finds an entry by URL. The found item will also be current().
     * If no matching entry is found, 0L is returned and current() will be
     * the first item in the list.
     */
    KonqHistoryEntry * findEntry( const KURL& url );

protected:
    /**
     * Ensures that the items are sorted by the lastVisited date
     */
    virtual int compareItems( TQPtrCollection::Item, TQPtrCollection::Item );
};


///////////////////////////////////////////////////////////////////


/**
 * This class maintains and manages a history of all URLs visited by one
 * Konqueror instance. Additionally it synchronizes the history with other
 * Konqueror instances via DCOP to keep one global and persistant history.
 *
 * It keeps the history in sync with one TDECompletion object
 */
class LIBKONQ_EXPORT KonqHistoryManager : public KParts::HistoryProvider,
			   public KonqHistoryComm
{
    Q_OBJECT

public:
    static KonqHistoryManager *kself() {
	return static_cast<KonqHistoryManager*>( KParts::HistoryProvider::self() );
    }

    KonqHistoryManager( TQObject *parent, const char *name );
    ~KonqHistoryManager();

    /**
     * Sets a new maximum size of history and truncates the current history
     * if necessary. Notifies all other Konqueror instances via DCOP
     * to do the same.
     *
     * The history is saved after receiving the DCOP call.
     */
    void emitSetMaxCount( TQ_UINT32 count );

    /**
     * Sets a new maximum age of history entries and removes all entries that
     * are older than @p days. Notifies all other Konqueror instances via DCOP
     * to do the same.
     *
     * An age of 0 means no expiry based on the age.
     *
     * The history is saved after receiving the DCOP call.
     */
    void emitSetMaxAge( TQ_UINT32 days );

    /**
     * Removes the history entry for @p url, if existant. Tells all other
     * Konqueror instances via DCOP to do the same.
     *
     * The history is saved after receiving the DCOP call.
     */
    void emitRemoveFromHistory( const KURL& url );

    /**
     * Removes the history entries for the given list of @p urls. Tells all
     * other Konqueror instances via DCOP to do the same.
     *
     * The history is saved after receiving the DCOP call.
     */
    void emitRemoveFromHistory( const KURL::List& urls );

    /**
     * @returns the current maximum number of history entries.
     */
    TQ_UINT32 maxCount() const { return m_maxCount; }

    /**
     * @returns the current maximum age (in days) of history entries.
     */
    TQ_UINT32 maxAge() const { return m_maxAgeDays; }

    /**
     * Adds a pending entry to the history. Pending means, that the entry is
     * not verified yet, i.e. it is not sure @p url does exist at all. You
     * probably don't know the title of the url in that case either.
     * Call @ref confirmPending() as soon you know the entry is good and should
     * be updated.
     *
     * If an entry with @p url already exists,
     * it will be updated (lastVisited date will become the current time
     * and the number of visits will be incremented).
     *
     * @param url The url of the history entry
     * @param typedURL the string that the user typed, which resulted in url
     *                 Doesn't have to be a valid url, e.g. "slashdot.org".
     * @param title The title of the URL. If you don't know it (yet), you may
                    specify it in @ref confirmPending().
     */
    void addPending( const KURL& url, const TQString& typedURL = TQString::null,
		     const TQString& title = TQString::null );

    /**
     * Confirms and updates the entry for @p url.
     */
    void confirmPending( const KURL& url,
			 const TQString& typedURL = TQString::null,
			 const TQString& title = TQString::null );

    /**
     * Removes a pending url from the history, e.g. when the url does not
     * exist, or the user aborted loading.
     */
    void removePending( const KURL& url );

    /**
     * @returns the TDECompletion object.
     */
    TDECompletion * completionObject() const { return m_pCompletion; }

    /**
     * @returns the list of all history entries, sorted by date
     * (oldest entries first)
     */
    const KonqHistoryList& entries() const { return m_history; }

    // HistoryProvider interfae, let konq handle this
    /**
     * Reimplemented in such a way that all URLs that would be filtered
     * out normally (see @ref filterOut()) will still be added to the history.
     * By default, file:/ urls will be filtered out, but if they come thru
     * the HistoryProvider interface, they are added to the history.
     */
    virtual void insert( const TQString& );
    virtual void remove( const TQString& ) {}
    virtual void clear() {}


public slots:
    /**
     * Loads the history and fills the completion object.
     */
    bool loadHistory();

    /**
     * Saves the entire history.
     */
    bool saveHistory();

    /**
     * Clears the history and tells all other Konqueror instances via DCOP
     * to do the same.
     * The history is saved afterwards, if necessary.
     */
    void emitClear();


signals:
    /**
     * Emitted after the entire history was loaded from disk.
     */
    void loadingFinished();

    /**
     * Emitted after a new entry was added
     */
    void entryAdded( const KonqHistoryEntry *entry );

    /**
     * Emitted after an entry was removed from the history
     * Note, that this entry will be deleted immediately after you got
     * that signal.
     */
    void entryRemoved( const KonqHistoryEntry *entry );

protected:
    /**
     * Resizes the history list to contain less or equal than m_maxCount
     * entries. The first (oldest) entries are removed.
     */
    void adjustSize();

    /**
     * @returns true if @p entry is older than the given maximum age,
     * otherwise false.
     */
    inline bool isExpired( KonqHistoryEntry *entry ) {
	return (entry && m_maxAgeDays > 0 && entry->lastVisited <
		TQDateTime(TQDate::currentDate().addDays( -m_maxAgeDays )));
    }

    /**
     * Notifes all running instances about a new HistoryEntry via DCOP
     */
    void emitAddToHistory( const KonqHistoryEntry& entry );

    /**
     * Every konqueror instance broadcasts new history entries to the other
     * konqueror instances. Those add the entry to their list, but don't
     * save the list, because the sender saves the list.
     *
     * @param e the new history entry
     * @param saveId is the DCOPObject::objId() of the sender so that
     * only the sender saves the new history.
     */
    virtual void notifyHistoryEntry( KonqHistoryEntry e, TQCString saveId );

    /**
     * Called when the configuration of the maximum count changed.
     * Called via DCOP by some config-module
     */
    virtual void notifyMaxCount( TQ_UINT32 count, TQCString saveId );

    /**
     * Called when the configuration of the maximum age of history-entries
     * changed. Called via DCOP by some config-module
     */
    virtual void notifyMaxAge( TQ_UINT32 days, TQCString saveId );

    /**
     * Clears the history completely. Called via DCOP by some config-module
     */
    virtual void notifyClear( TQCString saveId );

    /**
     * Notifes about a url that has to be removed from the history.
     * The instance where saveId == objId() has to save the history.
     */
    virtual void notifyRemove( KURL url, TQCString saveId );

    /**
     * Notifes about a list of urls that has to be removed from the history.
     * The instance where saveId == objId() has to save the history.
     */
    virtual void notifyRemove( KURL::List urls, TQCString saveId );

    /**
     * @returns a list of all urls in the history.
     */
    virtual TQStringList allURLs() const;

    /**
     * Does the work for @ref addPending() and @ref confirmPending().
     *
     * Adds an entry to the history. If an entry with @p url already exists,
     * it will be updated (lastVisited date will become the current time
     * and the number of visits will be incremented).
     * @p pending means, the entry has not been "verified", it's been added
     * right after typing the url.
     * If @p pending is false, @p url will be removed from the pending urls
     * (if available) and NOT be added again in that case.
     */
    void addToHistory( bool pending, const KURL& url,
		       const TQString& typedURL = TQString::null,
		       const TQString& title = TQString::null );


    /**
     * @returns true if the given @p url should be filtered out and not be
     * added to the history. By default, all local urls (url.isLocalFile())
     * will return true, as well as urls with an empty host.
     */
    virtual bool filterOut( const KURL& url );

    void addToUpdateList( const TQString& url ) {
        m_updateURLs.append( url );
        m_updateTimer->start( 500, true );
    }

    /**
     * The list of urls that is going to be emitted in slotEmitUpdated. Add
     * urls to it whenever you modify the list of history entries and start
     * m_updateTimer.
     */
    TQStringList m_updateURLs;

private slots:
    /**
     * Called by the updateTimer to emit the KParts::HistoryProvider::updated()
     * signal so that tdehtml can repaint the updated links.
     */
    void slotEmitUpdated();

private:
    /**
     * Returns whether the DCOP call we are handling was a call from us self
     */
    bool isSenderOfBroadcast();

    void clearPending();
    /**
     * a little optimization for KonqHistoryList::findEntry(),
     * checking the dict of KParts::HistoryProvider before traversing the list.
     * Can't be used everywhere, because it always returns 0L for "pending"
     * entries, as those are not added to the dict, currently.
     */
    KonqHistoryEntry * findEntry( const KURL& url );

    /**
     * Stuff to create a proper history out of KDE 2.0's konq_history for
     * completion.
     */
    bool loadFallback();
    KonqHistoryEntry * createFallbackEntry( const TQString& ) const;

    void addToCompletion( const TQString& url, const TQString& typedURL, int numberOfTimesVisited = 1 );
    void removeFromCompletion( const TQString& url, const TQString& typedURL );

    TQString m_filename;
    KonqHistoryList m_history;

    /**
     * List of pending entries, which were added to the history, but not yet
     * confirmed (i.e. not yet added with pending = false).
     * Note: when removing an entry, you have to delete the KonqHistoryEntry
     * of the item you remove.
     */
    TQMap<TQString,KonqHistoryEntry*> m_pending;

    TQ_UINT32 m_maxCount;   // maximum of history entries
    TQ_UINT32 m_maxAgeDays; // maximum age of a history entry

    TDECompletion *m_pCompletion; // the completion object we sync with

    /**
     * A timer that will emit the KParts::HistoryProvider::updated() signal
     * thru the slotEmitUpdated slot.
     */
    TQTimer *m_updateTimer;

    static const TQ_UINT32 s_historyVersion;
};


#endif // KONQ_HISTORY_H
