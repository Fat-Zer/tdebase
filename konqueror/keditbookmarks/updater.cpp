// -*- indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqtimer.h>

#include "updater.h"

#include "bookmarkiterator.h"
#include "listview.h"
#include "toplevel.h"

#include <kdebug.h>
#include <tdelocale.h>
#include <tdeapplication.h>

#include <tdeio/job.h>

#include <tdeparts/part.h>
#include <tdeparts/componentfactory.h>
#include <tdeparts/browserextension.h>

FavIconUpdater::FavIconUpdater(TQObject *parent, const char *name)
    : KonqFavIconMgr(parent, name) {
    m_part = 0;
    m_webGrabber = 0;
    m_browserIface = 0;
    m_timer = 0;
}

void FavIconUpdater::slotCompleted() {
    // kdDebug() << "FavIconUpdater::slotCompleted" << endl;
    // kdDebug() << "emit done(true)" << endl;
     m_timer->stop();
    emit done(true);
}

void FavIconUpdater::timerDone() {
    // Timeout: set completed
  kdDebug() << "FavIconUpdater: Timeout" << endl;
  slotCompleted();
}

void FavIconUpdater::downloadIcon(const KBookmark &bk) {
    TQString favicon = KonqFavIconMgr::iconForURL(bk.url().url());
    if (!favicon.isNull()) {
        // kdDebug() << "downloadIcon() - favicon" << favicon << endl;
        bk.internalElement().setAttribute("icon", favicon);
        KEBApp::self()->notifyCommandExecuted();
        // kdDebug() << "emit done(true)" << endl;
        emit done(true);

    } else {
        KonqFavIconMgr::downloadHostIcon(bk.url());
        favicon = KonqFavIconMgr::iconForURL(bk.url().url());
        // kdDebug() << "favicon == " << favicon << endl;
        if (favicon.isNull()) {
            downloadIconActual(bk);
        }
    }
}

FavIconUpdater::~FavIconUpdater() {
    // kdDebug() << "~FavIconUpdater" << endl;
    delete m_browserIface;
    delete m_webGrabber;
    delete m_part;
    delete m_timer;
}

void FavIconUpdater::downloadIconActual(const KBookmark &bk) {
    m_bk = bk;

    if (!m_part) {
        KParts::ReadOnlyPart *part 
            = KParts::ComponentFactory
            ::createPartInstanceFromQuery<KParts::ReadOnlyPart>("text/html", TQString::null);

        part->setProperty("pluginsEnabled", TQVariant(false, 1));
        part->setProperty("javaScriptEnabled", TQVariant(false, 1));
        part->setProperty("javaEnabled", TQVariant(false, 1));
        part->setProperty("autoloadImages", TQVariant(false, 1));

        connect(part, TQT_SIGNAL( canceled(const TQString &) ),
                this, TQT_SLOT( slotCompleted() ));
        connect(part, TQT_SIGNAL( completed() ),
                this, TQT_SLOT( slotCompleted() ));

        KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject(part);
        assert(ext);

        m_browserIface = new FavIconBrowserInterface(this, "browseriface");
        ext->setBrowserInterface(m_browserIface);

        connect(ext, TQT_SIGNAL( setIconURL(const KURL &) ),
                this, TQT_SLOT( setIconURL(const KURL &) ));

        m_part = part;
    }
    
    if (!m_timer) {
        // Timeout to stop the updating hanging
      m_timer = new TQTimer(this);
      connect( m_timer, TQT_SIGNAL(timeout()), this, TQT_SLOT(timerDone()) );
    }
    m_timer->start(15000,false);
    m_webGrabber = new FavIconWebGrabber(m_part, bk.url());
}

// tdehtml callback
void FavIconUpdater::setIconURL(const KURL &iconURL) {
    setIconForURL(m_bk.url(), iconURL);
}

void FavIconUpdater::notifyChange(bool isHost, TQString hostOrURL, TQString iconName) {
    // kdDebug() << "FavIconUpdater::notifyChange()" << endl;

    Q_UNUSED(isHost);
    // kdDebug() << isHost << endl;
    Q_UNUSED(hostOrURL);
    // kdDebug() << "FavIconUpdater::notifyChange" <<  hostOrURL << "==" << m_bk.url().url() << "-> " << iconName << endl;

    m_bk.internalElement().setAttribute("icon", iconName);
    KEBApp::self()->notifyCommandExecuted();
}

/* -------------------------- */

FavIconWebGrabber::FavIconWebGrabber(KParts::ReadOnlyPart *part, const KURL &url)
    : m_part(part), m_url(url) {

    // kdDebug() << "FavIconWebGrabber::FavIconWebGrabber starting TDEIO::get() " << url << endl;

//     the use of TDEIO rather than directly using TDEHTML is to allow silently abort on error

    TDEIO::Job *job = TDEIO::get(m_url, false, false);
    job->addMetaData( TQString("cookies"), TQString("none") );
    connect(job, TQT_SIGNAL( result( TDEIO::Job *)),
            this, TQT_SLOT( slotFinished(TDEIO::Job *) ));
    connect(job, TQT_SIGNAL( mimetype( TDEIO::Job *, const TQString &) ),
            this, TQT_SLOT( slotMimetype(TDEIO::Job *, const TQString &) ));
}

void FavIconWebGrabber::slotMimetype(TDEIO::Job *job, const TQString & /*type*/) {
    TDEIO::SimpleJob *sjob = static_cast<TDEIO::SimpleJob *>(job);
    m_url = sjob->url(); // allow for redirection
    sjob->putOnHold();

    // kdDebug() << "FavIconWebGrabber::slotMimetype " << m_url << "\n";
   
    // TQString typeLocal = typeUncopied; // local copy
    // kdDebug() << "slotMimetype : " << typeLocal << endl;
    // TODO - what to do if typeLocal is not text/html ??

    m_part->openURL(m_url);
}

void FavIconWebGrabber::slotFinished(TDEIO::Job *job) {
    if (job->error()) {
        // kdDebug() << "FavIconWebGrabber::slotFinished() " << job->errorString() << endl;
    }
}

#include "updater.moc"
