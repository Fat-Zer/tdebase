/***************************************************************************
 *   Copyright (C) 2006 by Stephan Binner <binner@kde.org>                 *
 *   Copyright (c) 2006 Debajyoti Bera <dbera.web@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "kickoff-beagle-plugin.h"

#include <tqregexp.h>
#include <tqtimer.h>

#include <kapplication.h>
#include <kdesktopfile.h>
#include <kgenericfactory.h>
#include <kservice.h>

TQString dc_identifier = "dc:identifier";
TQString dc_title = "dc:title";
TQString parent_dc_title = "parent:dc:title";
TQString exactfilename = "beagle:ExactFilename";
TQString fixme_name = "fixme:Name";
TQString beagle_filename = "beagle:Filename";
TQString fixme_attachment_title = "fixme:attachment_title";
TQString fixme_hasattachments = "fixme:hasAttachments";
TQString parent_prefix = "parent:";
TQString fixme_folder = "fixme:folder";
TQString fixme_categories = "fixme:Categories";
TQString fixme_comment = "fixme:Comment";
TQString fixme_width = "fixme:width";
TQString fixme_height = "fixme:height";
TQString fixme_from_address = "fixme:from_address";
TQString fixme_artist = "fixme:artist";
TQString fixme_album = "fixme:album";
TQString dc_source = "dc:source";
TQString dc_publisher = "dc:publisher";
TQString digikam_tag = "digikam:Tag";
TQString fixme_speakingto = "fixme:speakingto";
TQString fixme_starttime = "fixme:starttime";
TQString comma_string = ",";
TQString vCard_FN = "vCard:FN";
TQString vCard_PREFEMAIL = "vCard:PREFEMAIL";
TQString fixme_uid = "fixme:uid";

static CATEGORY getHitCategory (Hit *hit)
{
    TQString hittype = hit->getType();
    TQString hitsource = hit->getSource();

    // if hit source is None, dont handle it. Might be anthrax-envelope :)
    if (hitsource.isNull())
        return OTHER;

    if (hitsource == "documentation")
        return DOCS;

    if (hittype == "IMLog")
        return CHATS;

    // sure shots
    if (hittype == "FeedItem")
        return FEEDS;
    if (hittype == "WebHistory")
        return WEBHIST;
    if (hittype == "MailMessage")
        return MAILS;
    if (hittype == "Note")
        return NOTES;

    // check for applications
    if (hittype == "File" && (*hit) ["beagle:FilenameExtension"] == ".desktop")
        return APPS;

    // check for music
    TQString hitmimetype = hit->getMimeType();
    if (hitsource == "Amarok"
        || hitmimetype.startsWith ("audio")
        || hitmimetype == "application/ogg")
        return MUSIC; // not an exhaustive search

    // check for images from files
    if (hitsource == "Files" && hitmimetype.startsWith ("image"))
        return PICS;

    if (hitsource == "Files" && hitmimetype.startsWith ("video"))
        return VIDEOS;

    if (hitsource == "Files")
        return FILES;

    if (hitsource == "KAddressBook")
        return ACTIONS;

    return OTHER;
}

K_EXPORT_COMPONENT_FACTORY( kickoffsearch_beagle,
                            KGenericFactory<KickoffBeaglePlugin>( "kickoffsearch_beagle" ) )

KickoffBeaglePlugin::KickoffBeaglePlugin(TQObject *parent, const char* name, const TQStringList&)
            : KickoffSearch::Plugin(parent, name ), genericTitle( true )
{
    g_type_init ();
    current_beagle_client = NULL;
}

bool KickoffBeaglePlugin::daemonRunning()
{
    return beagle_util_daemon_is_running();
}

void KickoffBeaglePlugin::query(TQString term, bool _genericTitle)
{
    genericTitle = _genericTitle;
    current_query_str = term;

    // Beagle search
    if (current_beagle_client != NULL) {
	kdDebug () << "Previous client w/id " << current_beagle_client->id << " still running ... ignoring it." << endl;
	current_beagle_client->stopClient ();
    }
    current_beagle_client_id = TDEApplication::random ();
    kdDebug () << "Creating client with id:" << current_beagle_client_id << endl;

    BeagleClient *beagle_client = beagle_client_new (NULL);
    if (beagle_client == NULL) {
        kdDebug() << "beagle service not running ..." << endl;
        return;
    }

    TQStringList sources, types;
    BeagleQuery *beagle_query = BeagleUtil::createQueryFromString (term, sources, types, 99); // maximum 99 results, if this doesnt work, blame the stars

    current_beagle_client = new BeagleSearchClient (
                                current_beagle_client_id,
                                this,
                                beagle_client,
                                beagle_query,
                                false);
    current_beagle_client->start();
//    kdDebug () << "Query dispatched at " << time (NULL) << endl;
}

void KickoffBeaglePlugin::cleanClientList ()
{
    toclean_list_mutex.lock ();
    BeagleSearchClient *old_client = toclean_client_list.take (0);
    if (old_client != NULL) { // failsafe
	kdDebug () << "Cleanup old client " << old_client->id << endl;
	delete old_client;
    }
    toclean_list_mutex.unlock ();
}

void KickoffBeaglePlugin::customEvent (TQCustomEvent *e)
{
    if (e->type () == RESULTFOUND) {
//        kdDebug () << "Quick query thread at " << time (NULL) << " with current_id=" << current_beagle_client_id <<  " finished ..." << endl;
        BeagleSearchResult *result = (BeagleSearchResult *) e->data ();
        if (current_beagle_client_id != result->client_id) {
            kdDebug () << "Stale result from " << result->client_id << endl;
	    delete result;
	    // FIXME: Should I also free e ?
        } else {
            kdDebug () << "Good results ...total=" << result->total << endl;
            showResults (result);
        }
        //KPassivePopup::message( "This is the message", this );
    } else if (e->type () == SEARCHOVER) {
        BeagleSearchClient *client = (BeagleSearchClient *) e->data ();
	if (client == NULL) {
//	    kdDebug () << "Query finished event at " << time (NULL) << " but client is already deleted" << endl;
	    return;
	}
//        kdDebug () << "Query finished event at " << time (NULL) << " for id=" << client->id << endl;
	if (current_beagle_client_id == client->id) {
	    kickoffSearchInterface()->searchOver();
 	    current_beagle_client = NULL; // important !
	}
    } else if (e->type () == KILLME) {
        BeagleSearchClient *client = (BeagleSearchClient *) e->data ();
	if (client->finished ())
	    delete client;
	else {
	    // add client to cleanup list
	    toclean_list_mutex.lock ();
	    toclean_client_list.append (client);
	    kdDebug () << "Scheduling client to be deleted in 500ms" << endl;
	    toclean_list_mutex.unlock ();
	    TQTimer::singleShot (500, this, TQT_SLOT (cleanClientList ()));
	}
    }
}

// this method decides what to display in the result list
HitMenuItem *KickoffBeaglePlugin::hitToHitMenuItem (int category, Hit *hit)
{
    TQString title, info, mimetype, icon;
    int score = 0;
    KURL uri;

#if 0
    kdDebug() << "*** " << hit->getUri() << endl;
    TQDict<TQStringList> all = hit->getAllProperties();
    TQDictIterator<TQStringList> it( all );
    for( ; it.current(); ++it )
        kdDebug() << it.currentKey() << ": " << *(it.current()) << endl;
#endif

    switch (category) {
	case FILES:
	    {
		uri = hit->getUri ();
		TQString uristr = uri.path ();
	    	title = (*hit) [exactfilename];
	    	int last_slash = uristr.findRev ('/', -1);
                info = i18n("Folder: %1").arg(last_slash == 0 ? "/" 
                        : uristr.section ('/', -2, -2));
	    }
	    break;
        case ACTIONS:
            {
                if (hit->getSource()=="KAddressBook"){
		    title = i18n("Send Email to %1").arg((*hit)[vCard_FN]);
		    info = (*hit)[vCard_PREFEMAIL];
		    uri = "mailto:"+(*hit)[vCard_PREFEMAIL];
		    mimetype = hit->getMimeType ();
		    icon = "mail_new";

		    HitMenuItem * first_item=new HitMenuItem (title, info, uri, mimetype, 0, category, icon, score);
		    kickoffSearchInterface()->addHitMenuItem(first_item);

		    title =i18n("Open Addressbook at %1").arg((*hit)[vCard_FN]);
		    uri = "kaddressbook:/"+(*hit)[fixme_uid];
		    icon = "kaddressbook";
                }
                break;
            }
	case MAILS:
	    {
		TQString prefix = TQString::null;
		bool is_attachment = ((*hit) [parent_prefix + fixme_hasattachments] == "true");
		bool has_parent = (! hit->getParentUri ().isEmpty ());
		bool parent_mbox_file = false;
		if (has_parent)
		    parent_mbox_file = ((*hit) [parent_prefix + fixme_folder] == TQString::null);

		// Logic:
		// If has_parent == false, everything is normal
		// If has_parent == true, parent_mbox_file == false, everything is normal, use uri
		// FIXME: If has_parent == true, parent_mbox_file == true, ???
		// If has_parent == true, is_attachment == true, hit is attach and access with prefix "parent:", use parenturi
		// Else, not attachment (multipart), access with prefix "parent:", use parenturi

		if (has_parent && !parent_mbox_file) {
		    uri = hit->getParentUri ();
		    prefix = parent_prefix;
		    if (is_attachment)
			title = (*hit) [fixme_attachment_title];
		    if (title.isEmpty ())
			title = (*hit) [prefix + dc_title];
		    if (title.isEmpty ())
			title = i18n("No subject");
		    if (is_attachment)
			title = title.prepend (i18n("(Attachment) "));
		    info = (i18n("From %1").arg((*hit) [prefix + fixme_from_address]));
		} else {
		    uri = hit->getUri ();
		    title = (*hit) [dc_title];
		    info = (i18n("From %1").arg((*hit) [fixme_from_address]));
		}
	    }
	    mimetype = "message/rfc822"; // to handle attachment results
	    break;
 	case MUSIC:
	    uri = hit->getUri ();
	    title = (*hit) [exactfilename];
	    {
		TQString artist = (*hit) [fixme_artist];
		TQString album = (*hit) [fixme_album];
		if (! artist.isEmpty ())
		    info = (i18n("By %1").arg(artist));
		else if (! album.isEmpty ())
		    info = (i18n("From Album %1").arg(album));
		else {
		    TQString uristr = uri.path ();
		    int last_slash = uristr.findRev ('/', -1);
                    info = i18n("Folder: %1")
                        .arg(last_slash == 0 ? "/" : uristr.section ('/', -2, -2));
		}
	    }
	    break;
 	case VIDEOS:
	    uri = hit->getUri ();
	    title = (*hit) [exactfilename];
	    {
		TQString uristr = uri.path ();
		int last_slash = uristr.findRev ('/', -1);
                info = i18n("Folder: %1").arg(last_slash == 0 ? "/" : uristr.section ('/', -2, -2));
	    }
	    break;
	case WEBHIST:
	    uri = hit->getUri ();
	    title = (*hit) [dc_title];
	    title = title.replace(TQRegExp("\n")," ");
	    mimetype = "text/html";
	    if (title.isEmpty () || title.stripWhiteSpace ().isEmpty ()) {
		title = uri.prettyURL ();
	    } else {
		info = uri.host () + uri.path ();
	    }
	    break;
	case FEEDS:
	    {
		uri = KURL ((*hit) [dc_identifier]);
	    	title = (*hit) [dc_title];
	    	mimetype = "text/html";
	    	TQString publisher = (*hit) [dc_publisher];
	    	TQString source = (*hit) [dc_source];
	    	if (! publisher.isEmpty ())
	    	    info = publisher;
	    	else if (! source.isEmpty ())
	    	    info = source;
	    }
	    break;
	case PICS:
	    {
		uri = hit->getUri ();
		title = (*hit) [exactfilename];
		TQString width = (*hit) [fixme_width];
		TQString height = (*hit) [fixme_height];
		if (width.isEmpty () || height.isEmpty ()) {
		    TQString uristr = uri.path ();
		    int last_slash = uristr.findRev ('/', -1);
                    info = i18n("Folder: %1")
                        .arg(last_slash == 0 ? "/" : uristr.section ('/', -2, -2));
		    break;
		}
		info = (TQString (" (%1x%2)").arg (width).arg (height));
		const TQStringList *tags = hit->getProperties (digikam_tag);
		if (tags == NULL)
		    break;
		TQString tags_string = tags->join (comma_string);
		info += (" " + tags_string);
	    }
	    break;
	case APPS:
	    {
		uri = hit->getUri ();
	    	title = (*hit) [dc_title];
		KDesktopFile desktopfile(uri.path(),true);
		if (genericTitle && !desktopfile.readGenericName().isEmpty()) {
		  title = desktopfile.readGenericName();
		  info = desktopfile.readName();
		}
		else {
		  title = desktopfile.readName();
		  info = desktopfile.readGenericName();
		}
		icon = desktopfile.readIcon();
                TQString input = current_query_str.lower();
		TQString command = desktopfile.readEntry("Exec");
		if (command==input)
                  score = 100;
                else if (command.find(input)==0)
                  score = 50;
                else if (command.find(input)!=-1)
                  score = 10;
		else if (title==input)
                  score = 100;
                else if (title.find(input)==0)
                  score = 50;
                else if (title.find(input)!=-1)
                  score = 10;
	    	break;
	    }
	    break;
	case NOTES:
            {
	        uri = hit->getUri ();
	        title = (*hit) [dc_title];
	        title = i18n("Title: %1").arg(title.isEmpty() ? i18n("Untitled") : title);

	        if (hit->getSource()=="KNotes")
                   icon="knotes";
                else
                   icon="contents2";
            }
	    break;
	case CHATS:
            {
	        uri = hit->getUri ();
	        title = (*hit) [fixme_speakingto];
	        title = i18n("Conversation With %1").arg(title.isEmpty() ? i18n("Unknown Person") : title);
	        TQDateTime datetime;
	        datetime = datetimeFromString((*hit) [fixme_starttime]);
                info=i18n("Date: %1").arg(TDEGlobal::locale()->formatDateTime(datetime,false));
	        if (hit->getMimeType()=="beagle/x-kopete-log")
                   icon="kopete";
                else
                   icon="gaim";
            }
	    break;
	case DOCS:
	    {
		uri = hit->getUri ();
		title = (*hit) [dc_title];
		if (title.isEmpty () || title.stripWhiteSpace ().isEmpty ())
		    title = uri.prettyURL ();
		else {
			TQString uristr = uri.path ();
			int last_slash = uristr.findRev ('/', -1);
                        info = i18n("Folder: %1").arg(last_slash == 0 ? "/" : uristr.section ('/',
                                    -2, -2));
		}
	    }
	    break;
	default:
	    return NULL;
    }
    if (mimetype.isEmpty ())
	mimetype = hit->getMimeType ();
    return new HitMenuItem (title, info, uri, mimetype, 0, category, icon, score);
}

void KickoffBeaglePlugin::showResults(BeagleSearchResult *result)
{
    if (result->total == 0 ) {
	// Dont report error from here ...
        kdDebug() << "No matches found" << endl;
	delete result;
	return;
    }

    const TQPtrList<Hit> *hits = result->getHits();
    if (hits == NULL) {
        kdDebug () << "Hmm... null" << endl;
	delete result;
        return;
    }
    kickoffSearchInterface()->initCategoryTitlesUpdate();

    TQPtrListIterator<Hit> it (*hits);
    Hit *hit;
    for (; (hit = it.current ()) != NULL; ++it) {
	CATEGORY category = getHitCategory (hit);

	// if category is not handled, continue
	if (category == OTHER)
          continue;

        if ( category == APPS ) {
            // we need to check if this is useful
            KService cs( hit->getUri().path() );
            if ( cs.noDisplay() )
                continue;
        }

        if (!kickoffSearchInterface()->anotherHitMenuItemAllowed(category))
          continue;

        HitMenuItem *hit_item = hitToHitMenuItem (category, hit);

        if (!hit_item)
	   continue;

        kickoffSearchInterface()->addHitMenuItem(hit_item);
    }

    kickoffSearchInterface()->updateCategoryTitles();

    delete result;
}

TQDateTime KickoffBeaglePlugin::datetimeFromString( const TQString& s)
{
      int year( s.mid( 0, 4 ).toInt() );
      int month( s.mid( 4, 2 ).toInt() );
      int day( s.mid( 6, 2 ).toInt() );
      int hour( s.mid( 8, 2 ).toInt() );
      int min( s.mid( 10, 2 ).toInt() );
      int sec( s.mid( 12, 2 ).toInt() );
      return TQDateTime(TQDate(year,month,day),TQTime(hour,min,sec));
}

#include "kickoff-beagle-plugin.moc"
