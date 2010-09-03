/*****************************************************************

   Copyright (c) 2006 Debajyoti Bera <dbera.web@gmail.com>

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

******************************************************************/

#ifndef BEAGLESEARCH_H
#define BEAGLESEARCH_H

#include <tqdict.h>
#include <tqptrlist.h>
#include <tqthread.h>
#include <tqevent.h>
#include <tqmutex.h>

#include <kdebug.h>
#include <kurl.h>

extern "C" {
#include <glib.h>
#include <beagle/beagle.h>
}

// BeagleSearchClient sends 3 types of events
// when results are to be sent as they arrive,
//  - RESULTFOUND : when result is found
//  - SEARCHOVER :  when search is over
//  - KILLME : just before thread finishes - used to cleanup the thread object
// when results are to be sent after receiving all of them
//  - RESULTFOUND : when all results are obtained
//  - KILLME : just before thread finishes - used to cleanup the thread object
#define RESULTFOUND (TQEvent::Type)1001 /* TQEvent::User + 1 */
#define SEARCHOVER  (TQEvent::Type)1002 /* TQEvent::User + 2 */
#define KILLME      (TQEvent::Type)1003 /* TQEvent::User + 3 */

class TQStringList;

// IMPORTANT: Call this before any beagle calls
void beagle_init ();

class Hit {
public:
    Hit (BeagleHit *_hit);
    ~Hit ();
    
    // convenience wrappers
    // remember that the hit values are utf8 strings
    const KURL getUri () const { return KURL (TQString::fromUtf8 (beagle_hit_get_uri (hit)));}
    const TQString getType () const { return TQString::fromUtf8 (beagle_hit_get_type (hit));}
    const TQString getMimeType () const { return TQString::fromUtf8 (beagle_hit_get_mime_type (hit));}
    const TQString getSource () const { return TQString::fromUtf8 (beagle_hit_get_source (hit));}
    const KURL getParentUri () const { return KURL (TQString::fromUtf8 (beagle_hit_get_parent_uri (hit)));}
    const TQDict<TQStringList>& getAllProperties ()
    {
	if (! processed)
	    processProperties ();
	return property_map;
    }
    const TQStringList* getProperties (TQString prop_name)
    {
	if (! processed)
	    processProperties ();
	return property_map [prop_name];
    }
    const TQString operator[] (TQString prop_name);

private:
    BeagleHit *hit;
    TQDict<TQStringList> property_map;
    // not every hit may be used. so, do a lazy processing of property_map
    bool processed;
    void processProperties ();
};

class BeagleSearchResult{
public:
    BeagleSearchResult(int client_id);
    ~BeagleSearchResult();
    void addHit (BeagleHit *hit);
    TQString getHitCategory (Hit *hit);

    // id of the bsclient
    int client_id;
    // time taken to finish query
    int query_msec;
    // total number of results in this query
    int total;

    const TQPtrList<Hit> *getHits () const;

private:
    // lists of hits
    TQPtrList<Hit> *hitlist;
};

// caller should delete bsclient->result and bsclient
class BeagleSearchClient : public TQThread {
public:
    // passing NULL for client makes bsclient create client itself and
    // delete it later
    BeagleSearchClient (int id,
                        TQObject *y,
                        BeagleClient *client,
                        BeagleQuery *query,
                        bool collate_results)
    : id (id), kill_me (false), object (y), client (client),
      query (query), destroy_client (false), collate_results (collate_results)
    {
        if (client == NULL) {
	    client = beagle_client_new (NULL);
            destroy_client = true;
        }
	
//        if (client == NULL)
//            throw -1;

        main_loop = g_main_loop_new (NULL, FALSE);
        if (collate_results)
            result = new BeagleSearchResult (id);

	client_mutex = new TQMutex ();
    }

    // It is never safe to delete BeagleSearchClient directly, the thread might still be running
    ~BeagleSearchClient ()
    {
	if (! finished ()) {
	    kdDebug () << "Thread " << id << " still running. Waiting.........." << endl;
	    wait ();
	}

        if (destroy_client)
            g_object_unref (client);
        g_main_loop_unref (main_loop);
        g_object_unref (query);
        kdDebug() << "Deleting client ..." << id << endl;
	delete client_mutex;
    }

private:
    static void hitsAddedSlot (BeagleQuery *query,
                               BeagleHitsAddedResponse *response,
                               BeagleSearchClient *bsclient);

    static void finishedSlot (BeagleQuery  *query,
                              BeagleFinishedResponse *response,
                              BeagleSearchClient *bsclient);

public:
    // run() starts the query and sends the result as follows:
    // - either wait till get back all results and send it as RESULTFOUND
    // - or, send results as it gets them as RESULTFOUND and 
    //       send SEARCHOVER when finished
    // collate_results controls the behaviour
    virtual void run ( );
    
    // after stopClient() is called, application can safely go and remove previous menu entries
    // - i.e. after stopClient is called, app doesnt except the eventhandler to receive any results
    // - use client_id to determine which is the current client, set it right after stopclient
    // - Eventhandler checks client id, if it is current, it adds stuff to the menu
    //   else, it discards everything
    // Once eventhandler is being processed, doQuery() wont be called and vice versa
    //   so no need to serialize eventhandler and doquery
    //
    // stopClient needs to make sure that once it is called, the thread is finished asap. Use a mutex
    // to serialize actions. callbacks need to use mutex too.
    // stopclient has to remove signal handlers to prevent further signal calls, set kill_me flag
    //   and quite main loop
    // stopClient can be called at the following times:
    // - Waiting for the first result:
    //   nothing extra
    // - in hitsAddedSlot, processing results
    //   in callback, before processing, if killme is set, just return.
    // - in hitsAddedSlot, after sending results
    //   before sending, if killme is set, dont send results
    //   (doing it twice in hitsAdded because forming BeagleSearchResult can take time)
    // - Waiting for more results
    //   nothing extra
    // - in finishedSlot, before sending finishedMsg
    //   if killme is set, just return
    // - in finishedSlot, after sending finishedMsg
    //   if killme is set, just return
    //  in Run(), when return from mainloop, if killme is set, dont do anything more but call delete this
    void stopClient ();

    // id of the client
    // this is required in case applications fires many clients in rapid succession
    int id;
    
    GMainLoop * main_loop;
    BeagleSearchResult *result;
    
    // this is set if the client is obsolete now i.e.
    // the application doesnt need the results from the client anymore
    bool kill_me;
private:
    // the application; need this to send events to the application
    TQObject *object;
    // mutex to control setting the kill_me shared variable
    TQMutex *client_mutex;
    BeagleClient *client;
    BeagleQuery *query;
    // should the client be destroyed by the client
    // if the client created it, then most probably it should
    bool destroy_client;
    bool collate_results;
};

class BeagleUtil {
public:

    static BeagleQuery *createQueryFromString (TQString query_str,
					       TQStringList &sources,
                                               TQStringList &types,
					       int max_hits_per_source = 100);
    static BeagleTimestamp *timestringToBeagleTimestamp (TQString timestring);
};

#endif
