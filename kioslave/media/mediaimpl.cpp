/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "mediaimpl.h"

#include <klocale.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kio/netaccess.h>

#include <kmimetype.h>

#include <kapplication.h>
#include <tqeventloop.h>

#include <sys/stat.h>

#include "medium.h"

#include <config.h>

MediaImpl::MediaImpl() : TQObject(), DCOPObject("mediaimpl"), mp_mounting(0L)
{

}

bool MediaImpl::parseURL(const KURL &url, TQString &name, TQString &path) const
{
	TQString url_path = url.path();

	int i = url_path.find('/', 1);
        if (i > 0)
        {
                name = url_path.mid(1, i-1);
                path = url_path.mid(i+1);
        }
        else
        {
                name = url_path.mid(1);
                path = TQString::null;
        }

	return name != TQString::null;
}

bool MediaImpl::realURL(const TQString &name, const TQString &path, KURL &url)
{
	bool ok;
	Medium m = findMediumByName(name, ok);
	if ( !ok ) return false;

	ok = ensureMediumMounted(m);
	if ( !ok ) return false;

	url = m.prettyBaseURL();
	url.addPath(path);
	return true;
}


bool MediaImpl::statMedium(const TQString &name, KIO::UDSEntry &entry)
{
	kdDebug(1219) << "MediaImpl::statMedium: " << name << endl;

	DCOPRef mediamanager("kded", "mediamanager");
	DCOPReply reply = mediamanager.call( "properties", name );

	if ( !reply.isValid() )
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		return false;
	}

	Medium m = Medium::create(reply);

	if (m.id().isEmpty())
	{
		entry.clear();
		return false;
	}

	createMediumEntry(entry, m);

	return true;
}

bool MediaImpl::statMediumByLabel(const TQString &label, KIO::UDSEntry &entry)
{
	kdDebug(1219) << "MediaImpl::statMediumByLabel: " << label << endl;

	DCOPRef mediamanager("kded", "mediamanager");
	DCOPReply reply = mediamanager.call( "nameForLabel", label );

	if ( !reply.isValid() )
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		return false;
	}

	TQString name = reply;

	if (name.isEmpty())
	{
		entry.clear();
		return false;
	}

	return statMedium(name, entry);
}


bool MediaImpl::listMedia(TQValueList<KIO::UDSEntry> &list)
{
	kdDebug(1219) << "MediaImpl::listMedia" << endl;

	DCOPRef mediamanager("kded", "mediamanager");
	DCOPReply reply = mediamanager.call( "fullList" );

	if ( !reply.isValid() )
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		return false;
	}

	const Medium::MList media = Medium::createList(reply);

	KIO::UDSEntry entry;

	Medium::MList::const_iterator it = media.begin();
	Medium::MList::const_iterator end = media.end();

	for(; it!=end; ++it)
	{
		if (!(*it).hidden()) {
			entry.clear();

			createMediumEntry(entry, *it);

			list.append(entry);
		}
	}

	return true;
}

bool MediaImpl::setUserLabel(const TQString &name, const TQString &label)
{
	kdDebug(1219) << "MediaImpl::setUserLabel: " << name << ", " << label << endl;


	DCOPRef mediamanager("kded", "mediamanager");
	DCOPReply reply = mediamanager.call( "nameForLabel", label );

	if ( !reply.isValid() )
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		return false;
	}
	else
	{
		TQString returned_name = reply;
		if (!returned_name.isEmpty()
		 && returned_name!=name)
		{
			m_lastErrorCode = KIO::ERR_DIR_ALREADY_EXIST;
			m_lastErrorMessage = i18n("This media name already exists.");
			return false;
		}
	}

	reply = mediamanager.call( "setUserLabel", name, label );

	if ( !reply.isValid() )
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		return false;
	}
	else
	{
		return true;
	}
}

const Medium MediaImpl::findMediumByName(const TQString &name, bool &ok)
{
	DCOPRef mediamanager("kded", "mediamanager");
	DCOPReply reply = mediamanager.call( "properties", name );

	if ( reply.isValid() )
	{
		ok = true;
	}
	else
	{
		m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		m_lastErrorMessage = i18n("The TDE mediamanager is not running.");
		ok = false;
	}

	return Medium::create(reply);
}

bool MediaImpl::ensureMediumMounted(Medium &medium)
{
	if (medium.id().isEmpty())
	{
		m_lastErrorCode = KIO::ERR_COULD_NOT_MOUNT;
		m_lastErrorMessage = i18n("No such medium.");
		return false;
	}

#ifdef COMPILE_HALBACKEND
	if ( medium.isEncrypted() && medium.clearDeviceUdi().isEmpty() )
	{
		m_lastErrorCode = KIO::ERR_COULD_NOT_MOUNT;
		m_lastErrorMessage = i18n("The drive is encrypted.");
		return false;
	}
#endif // COMPILE_HALBACKEND

	if ( medium.needMounting() )
	{
		m_lastErrorCode = 0;

		mp_mounting = &medium;
		
		
		/*
		KIO::Job* job = KIO::mount(false, 0,
		                           medium.deviceNode(),
		                           medium.mountPoint());
		job->setAutoWarningHandlingEnabled(false);
		connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
		         this, TQT_SLOT( slotMountResult( KIO::Job * ) ) );
		connect( job, TQT_SIGNAL( warning( KIO::Job *, const TQString & ) ),
		         this, TQT_SLOT( slotWarning( KIO::Job *, const TQString & ) ) );
		*/
		kapp->dcopClient()
		->connectDCOPSignal("kded", "mediamanager",
		                    "mediumChanged(TQString, bool)",
		                    "mediaimpl",
		                    "slotMediumChanged(TQString)",
		                    false);

		DCOPRef mediamanager("kded", "mediamanager");
		DCOPReply reply = mediamanager.call( "mount", medium.id());
		if (reply.isValid())
		  reply.get(m_lastErrorMessage);
		else
		  m_lastErrorMessage = i18n("Internal Error");
		if (!m_lastErrorMessage.isEmpty())
		  m_lastErrorCode = KIO::ERR_SLAVE_DEFINED;
		else {
		  tqApp->eventLoop()->enterLoop();
		}

		mp_mounting = 0L;
		
		kapp->dcopClient()
		->disconnectDCOPSignal("kded", "mediamanager",
		                       "mediumChanged(TQString, bool)",
		                       "mediaimpl",
		                       "slotMediumChanged(TQString)");
		
		return m_lastErrorCode==0;
	}

	if (medium.id().isEmpty())
	{
		m_lastErrorCode = KIO::ERR_COULD_NOT_MOUNT;
		m_lastErrorMessage = i18n("No such medium.");
		return false;
	}

	return true;
}

void MediaImpl::slotWarning( KIO::Job * /*job*/, const TQString &msg )
{
	emit warning( msg );
}

void MediaImpl::slotMountResult(KIO::Job *job)
{
	kdDebug(1219) << "MediaImpl::slotMountResult" << endl;
	
	if ( job->error() != 0)
	{
		m_lastErrorCode = job->error();
		m_lastErrorMessage = job->errorText();
		tqApp->eventLoop()->exitLoop();
	}
}

void MediaImpl::slotMediumChanged(const TQString &name)
{
	kdDebug(1219) << "MediaImpl::slotMediumChanged:" << name << endl;

	if (mp_mounting->name()==name)
	{
		kdDebug(1219) << "MediaImpl::slotMediumChanged: updating mp_mounting" << endl;
		bool ok;
		*mp_mounting = findMediumByName(name, ok);
		tqApp->eventLoop()->exitLoop();
	}
}

static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l,
                    const TQString &s = TQString::null)
{
	KIO::UDSAtom atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}


void MediaImpl::createTopLevelEntry(KIO::UDSEntry& entry) const
{
	entry.clear();
	addAtom(entry, KIO::UDS_URL, 0, "media:/");
	addAtom(entry, KIO::UDS_NAME, 0, ".");
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0555);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/directory");
	addAtom(entry, KIO::UDS_ICON_NAME, 0, "blockdevice");
}

void MediaImpl::slotStatResult(KIO::Job *job)
{
	if ( job->error() == 0)
	{
		KIO::StatJob *stat_job = static_cast<KIO::StatJob *>(job);
		m_entryBuffer = stat_job->statResult();
	}

	tqApp->eventLoop()->exitLoop();
}

KIO::UDSEntry MediaImpl::extractUrlInfos(const KURL &url)
{
	m_entryBuffer.clear();

	KIO::StatJob *job = KIO::stat(url, false);
	job->setAutoWarningHandlingEnabled( false );
	connect( job, TQT_SIGNAL( result(KIO::Job *) ),
	         this, TQT_SLOT( slotStatResult(KIO::Job *) ) );
	connect( job, TQT_SIGNAL( warning( KIO::Job *, const TQString & ) ),
	         this, TQT_SLOT( slotWarning( KIO::Job *, const TQString & ) ) );
	tqApp->eventLoop()->enterLoop();

	KIO::UDSEntry::iterator it = m_entryBuffer.begin();
	KIO::UDSEntry::iterator end = m_entryBuffer.end();

	KIO::UDSEntry infos;

	for(; it!=end; ++it)
	{
		switch( (*it).m_uds )
		{
		case KIO::UDS_ACCESS:
		case KIO::UDS_USER:
		case KIO::UDS_GROUP:
		case KIO::UDS_CREATION_TIME:
		case KIO::UDS_MODIFICATION_TIME:
		case KIO::UDS_ACCESS_TIME:
			infos.append(*it);
			break;
		default:
			break;
		}
	}

	if (url.isLocalFile())
	{
		addAtom(infos, KIO::UDS_LOCAL_PATH, 0, url.path());
	}
	
	return infos;
}


void MediaImpl::createMediumEntry(KIO::UDSEntry& entry,
                                  const Medium &medium)
{
	kdDebug(1219) << "MediaProtocol::createMedium" << endl;

	TQString url = "media:/"+medium.name();

	kdDebug(1219) << "url = " << url << ", mime = " << medium.mimeType() << endl;

	entry.clear();

	addAtom(entry, KIO::UDS_URL, 0, url);

	TQString label = KIO::encodeFileName( medium.prettyLabel() );
	addAtom(entry, KIO::UDS_NAME, 0, label);

	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);

	addAtom(entry, KIO::UDS_MIME_TYPE, 0, medium.mimeType());
	addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "inode/directory");

	if (!medium.iconName().isEmpty())
	{
		addAtom(entry, KIO::UDS_ICON_NAME, 0, medium.iconName());
	}
	else
	{
		TQString mime = medium.mimeType();
		TQString icon = KMimeType::mimeType(mime)->icon(mime, false);
		addAtom(entry, KIO::UDS_ICON_NAME, 0, icon);
	}

	if (medium.needMounting())
	{
		addAtom(entry, KIO::UDS_ACCESS, 0400);
	}
	else
	{
		KURL url = medium.prettyBaseURL();
		entry+= extractUrlInfos(url);
	}
}

#include "mediaimpl.moc"
