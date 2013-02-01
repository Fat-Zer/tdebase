// -*- indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

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

#include "toplevel.h"

#include "bookmarkinfo.h"
#include "listview.h"
#include "actionsimpl.h"
#include "dcop.h"
#include "exporters.h"
#include "settings.h"
#include "commands.h"
#include "kebsearchline.h"

#include <stdlib.h>

#include <tqclipboard.h>
#include <tqsplitter.h>
#include <tqlayout.h>
#include <tqlabel.h>

#include <klocale.h>
#include <kdebug.h>

#include <kapplication.h>
#include <kstdaction.h>
#include <kaction.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <kedittoolbar.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <tdefiledialog.h>

#include <kdebug.h>

#include <kbookmarkdrag.h>
#include <kbookmarkmanager.h>

CmdHistory* CmdHistory::s_self = 0;

CmdHistory::CmdHistory(TDEActionCollection *collection)
    : m_commandHistory(collection) {
    connect(&m_commandHistory, TQT_SIGNAL( commandExecuted(KCommand *) ),
            TQT_SLOT( slotCommandExecuted(KCommand *) ));
    assert(!s_self);
    s_self = this; // this is hacky
}

CmdHistory* CmdHistory::self() {
    assert(s_self);
    return s_self;
}

void CmdHistory::slotCommandExecuted(KCommand *k) {
    KEBApp::self()->notifyCommandExecuted();

    IKEBCommand * cmd = dynamic_cast<IKEBCommand *>(k);
    Q_ASSERT(cmd);

    KBookmark bk = CurrentMgr::bookmarkAt(cmd->affectedBookmarks());
    Q_ASSERT(bk.isGroup());
    CurrentMgr::self()->notifyManagers(bk.toGroup());

    // sets currentItem to something sensible
    // if the currentItem was invalidated by executing
    // CreateCommand or DeleteManyCommand
    // otherwise does nothing
    // sensible is either a already selected item or cmd->currentAddress()
    ListView::self()->fixUpCurrent( cmd->currentAddress() );
}

void CmdHistory::notifyDocSaved() {
    m_commandHistory.documentSaved();
}

void CmdHistory::didCommand(KCommand *cmd) {
    if (!cmd)
        return;
    m_commandHistory.addCommand(cmd, false);
    CmdHistory::slotCommandExecuted(cmd);
}

void CmdHistory::addCommand(KCommand *cmd) {
    if (!cmd)
        return;
    m_commandHistory.addCommand(cmd);
}

void CmdHistory::addInFlightCommand(KCommand *cmd)
{
    if(!cmd)
        return;
    m_commandHistory.addCommand(cmd, false);
}

void CmdHistory::clearHistory() {
    m_commandHistory.clear();
}

/* -------------------------- */

CurrentMgr *CurrentMgr::s_mgr = 0;

KBookmark CurrentMgr::bookmarkAt(const TQString &a) {
    return self()->mgr()->findByAddress(a);
}

bool CurrentMgr::managerSave() { return mgr()->save(); }
void CurrentMgr::saveAs(const TQString &fileName) { mgr()->saveAs(fileName); }
void CurrentMgr::setUpdate(bool update) { mgr()->setUpdate(update); }
TQString CurrentMgr::path() const { return mgr()->path(); }
bool CurrentMgr::showNSBookmarks() const { return mgr()->showNSBookmarks(); }

void CurrentMgr::createManager(const TQString &filename) {
    if (m_mgr) {
        kdDebug()<<"ERROR calling createManager twice"<<endl;
        disconnect(m_mgr, 0, 0, 0);
        // still todo - delete old m_mgr
    }

    m_mgr = KBookmarkManager::managerForFile(filename, false);

    connect(m_mgr, TQT_SIGNAL( changed(const TQString &, const TQString &) ),
            TQT_SLOT( slotBookmarksChanged(const TQString &, const TQString &) ));
}

void CurrentMgr::slotBookmarksChanged(const TQString &, const TQString &) {
    if(ignorenext > 0) //We ignore the first changed signal after every change we did
    {
        --ignorenext;
        return;
    }

    CmdHistory::self()->clearHistory();
    ListView::self()->updateListView();
    KEBApp::self()->updateActions();
}

void CurrentMgr::notifyManagers(KBookmarkGroup grp)
{
    ++ignorenext;
    mgr()->emitChanged(grp);
}

void CurrentMgr::notifyManagers() {
    notifyManagers( mgr()->root() );
}

void CurrentMgr::reloadConfig() {
    mgr()->emitConfigChanged();
}

TQString CurrentMgr::makeTimeStr(const TQString & in)
{
    int secs;
    bool ok;
    secs = in.toInt(&ok);
    if (!ok)
        return TQString::null;
    return makeTimeStr(secs);
}

TQString CurrentMgr::makeTimeStr(int b)
{
    TQDateTime dt;
    dt.setTime_t(b);
    return (dt.daysTo(TQDateTime::currentDateTime()) > 31)
        ? TDEGlobal::locale()->formatDate(TQT_TQDATE_OBJECT(dt.date()), false)
        : TDEGlobal::locale()->formatDateTime(dt, false);
}

/* -------------------------- */

KEBApp *KEBApp::s_topLevel = 0;

KEBApp::KEBApp(
    const TQString &bookmarksFile, bool readonly,
    const TQString &address, bool browser, const TQString &caption
) : TDEMainWindow(), m_dcopIface(0), m_bookmarksFilename(bookmarksFile),
    m_caption(caption), m_readOnly(readonly), m_browser(browser) {

    m_cmdHistory = new CmdHistory(actionCollection());

    s_topLevel = this;

    int h = 20;

    TQSplitter *vsplitter = new TQSplitter(this);

    TDEToolBar *quicksearch = new TDEToolBar(vsplitter, "search toolbar");

    TDEAction *resetQuickSearch = new TDEAction( i18n( "Reset Quick Search" ),
        TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase",
        0, actionCollection(), "reset_quicksearch" );
    resetQuickSearch->setWhatsThis( i18n( "<b>Reset Quick Search</b><br>"
        "Resets the quick search so that all bookmarks are shown again." ) );
    resetQuickSearch->plug( quicksearch );

    TQLabel *lbl = new TQLabel(i18n("Se&arch:"), quicksearch, "kde toolbar widget");

    TDEListViewSearchLine *searchLineEdit = new KEBSearchLine(quicksearch, 0, "TDEListViewSearchLine");
    quicksearch->setStretchableWidget(searchLineEdit);
    lbl->setBuddy(searchLineEdit);
    connect(resetQuickSearch, TQT_SIGNAL(activated()), searchLineEdit, TQT_SLOT(clear()));
    connect(searchLineEdit, TQT_SIGNAL(searchUpdated()), TQT_SLOT(updateActions()));

    ListView::createListViews(vsplitter);
    ListView::self()->initListViews();
    searchLineEdit->setListView(static_cast<TDEListView*>(ListView::self()->widget()));
    ListView::self()->setSearchLine(searchLineEdit);

    m_bkinfo = new BookmarkInfoWidget(vsplitter);

    vsplitter->setOrientation(Qt::Vertical);
    vsplitter->setSizes(TQValueList<int>() << h << 380
                                          << m_bkinfo->sizeHint().height() );

    setCentralWidget(vsplitter);
    resize(ListView::self()->widget()->sizeHint().width(),
           vsplitter->sizeHint().height());

    createActions();
    if (m_browser)
        createGUI();
    else
        createGUI("keditbookmarks-genui.rc");

    m_dcopIface = new KBookmarkEditorIface();

    connect(kapp->clipboard(), TQT_SIGNAL( dataChanged() ),
                               TQT_SLOT( slotClipboardDataChanged() ));

    ListView::self()->connectSignals();

    TDEGlobal::locale()->insertCatalogue("libkonq");

    m_canPaste = false;

    construct();

    ListView::self()->setCurrent(ListView::self()->getItemAtAddress(address), true);

    setCancelFavIconUpdatesEnabled(false);
    setCancelTestsEnabled(false);
    updateActions();
}

void KEBApp::construct() {
    CurrentMgr::self()->createManager(m_bookmarksFilename);

    ListView::self()->updateListViewSetup(m_readOnly);
    ListView::self()->updateListView();
    ListView::self()->widget()->setFocus();

    slotClipboardDataChanged();
    setAutoSaveSettings();
}

void KEBApp::updateStatus(TQString url)
{
    if(m_bkinfo->bookmark().url() == url)
        m_bkinfo->updateStatus();
}

KEBApp::~KEBApp() {
    s_topLevel = 0;
    delete m_cmdHistory;
    delete m_dcopIface;
    delete ActionsImpl::self();
    delete ListView::self();
}

TDEToggleAction* KEBApp::getToggleAction(const char *action) const {
    return static_cast<TDEToggleAction*>(actionCollection()->action(action));
}

void KEBApp::resetActions() {
    stateChanged("disablestuff");
    stateChanged("normal");

    if (!m_readOnly)
        stateChanged("notreadonly");

    getToggleAction("settings_showNS")
        ->setChecked(CurrentMgr::self()->showNSBookmarks());
}

bool KEBApp::nsShown() const {
    return getToggleAction("settings_showNS")->isChecked();
}

// this should be pushed from listview, not pulled
void KEBApp::updateActions() {
    resetActions();
    setActionsEnabled(ListView::self()->getSelectionAbilities());
}

void KEBApp::slotClipboardDataChanged() {
    // kdDebug() << "KEBApp::slotClipboardDataChanged" << endl;
    if (!m_readOnly) {
        m_canPaste = KBookmarkDrag::canDecode(
                        kapp->clipboard()->data(TQClipboard::Clipboard));
        updateActions();
    }
}

/* -------------------------- */

void KEBApp::notifyCommandExecuted() {
    // kdDebug() << "KEBApp::notifyCommandExecuted()" << endl;
    if (!m_readOnly) {        
        ListView::self()->updateListView();
        updateActions();
    }
}

/* -------------------------- */

void KEBApp::slotConfigureToolbars() {
    saveMainWindowSettings(TDEGlobal::config(), "MainWindow");
    KEditToolbar dlg(actionCollection());
    connect(&dlg, TQT_SIGNAL( newToolbarConfig() ),
                  TQT_SLOT( slotNewToolbarConfig() ));
    dlg.exec();
}

void KEBApp::slotNewToolbarConfig() {
    // called when OK or Apply is clicked
    createGUI();
    applyMainWindowSettings(TDEGlobal::config(), "MainWindow");
}

/* -------------------------- */

#include "toplevel.moc"

