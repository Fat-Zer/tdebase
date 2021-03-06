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

#include "actionsimpl.h"

#include "toplevel.h"
#include "commands.h"
#include "importers.h"
#include "favicons.h"
#include "testlink.h"
#include "listview.h"
#include "exporters.h"
#include "bookmarkinfo.h"

#include <stdlib.h>

#include <tqclipboard.h>
#include <tqpopupmenu.h>
#include <tqpainter.h>

#include <tdelocale.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kdebug.h>
#include <tdeapplication.h>

#include <tdeaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <tdefiledialog.h>
#include <kkeydialog.h>
#include <tdemessagebox.h>
#include <kinputdialog.h>
#include <krun.h>

#include <kdatastream.h>
#include <tdetempfile.h>
#include <kstandarddirs.h>

#include <tdeparts/part.h>
#include <tdeparts/componentfactory.h>

#include <kicondialog.h>
#include <kiconloader.h>

#include <kbookmarkdrag.h>
#include <kbookmarkmanager.h>
#include <kbookmarkimporter.h>

#include <kbookmarkimporter_ie.h>
#include <kbookmarkimporter_opera.h>
#include <kbookmarkexporter.h>

ActionsImpl* ActionsImpl::s_self = 0;

// decoupled from resetActions in toplevel.cpp
// as resetActions simply uses the action groups
// specified in the ui.rc file
void KEBApp::createActions() {

    ActionsImpl *actn = ActionsImpl::self();

    // save and quit should probably not be in the toplevel???
    (void) KStdAction::quit(
        TQT_TQOBJECT(this), TQT_SLOT( close() ), actionCollection());
    KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()), actionCollection());
    (void) KStdAction::configureToolbars(
        TQT_TQOBJECT(this), TQT_SLOT( slotConfigureToolbars() ), actionCollection());

    if (m_browser) {
        (void) KStdAction::open(
            TQT_TQOBJECT(actn), TQT_SLOT( slotLoad() ), actionCollection());
        (void) KStdAction::saveAs(
            TQT_TQOBJECT(actn), TQT_SLOT( slotSaveAs() ), actionCollection());
    }

    (void) KStdAction::cut(TQT_TQOBJECT(actn), TQT_SLOT( slotCut() ), actionCollection());
    (void) KStdAction::copy(TQT_TQOBJECT(actn), TQT_SLOT( slotCopy() ), actionCollection());
    (void) KStdAction::paste(TQT_TQOBJECT(actn), TQT_SLOT( slotPaste() ), actionCollection());
    (void) KStdAction::print(TQT_TQOBJECT(actn), TQT_SLOT( slotPrint() ), actionCollection());

    // settings menu
    (void) new TDEToggleAction(
        i18n("&Show Netscape Bookmarks in Konqueror"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotShowNS() ), actionCollection(),
        "settings_showNS");

    // actions
    (void) new TDEAction(
        i18n("&Delete"), "edit-delete", Key_Delete,
        TQT_TQOBJECT(actn), TQT_SLOT( slotDelete() ), actionCollection(), "delete");
    (void) new TDEAction(
        i18n("Rename"), "text", Key_F2,
        TQT_TQOBJECT(actn), TQT_SLOT( slotRename() ), actionCollection(), "rename");
    (void) new TDEAction(
        i18n("C&hange URL"), "text", Key_F3,
        TQT_TQOBJECT(actn), TQT_SLOT( slotChangeURL() ), actionCollection(), "changeurl");
    (void) new TDEAction(
        i18n("C&hange Comment"), "text", Key_F4,
        TQT_TQOBJECT(actn), TQT_SLOT( slotChangeComment() ), actionCollection(), "changecomment");
    (void) new TDEAction(
        i18n("Chan&ge Icon..."), "icons", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotChangeIcon() ), actionCollection(), "changeicon");
    (void) new TDEAction(
        i18n("Update Favicon"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotUpdateFavIcon() ), actionCollection(), "updatefavicon");
    (void) new TDEAction(
        i18n("Recursive Sort"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotRecursiveSort() ), actionCollection(), "recursivesort");
    (void) new TDEAction(
        i18n("&New Folder..."), "folder-new", CTRL+Key_N,
        TQT_TQOBJECT(actn), TQT_SLOT( slotNewFolder() ), actionCollection(), "newfolder");
    (void) new TDEAction(
        i18n("&New Bookmark"), "www", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotNewBookmark() ), actionCollection(), "newbookmark");
    (void) new TDEAction(
        i18n("&Insert Separator"), CTRL+Key_I,
        TQT_TQOBJECT(actn), TQT_SLOT( slotInsertSeparator() ), actionCollection(),
        "insertseparator");
    (void) new TDEAction(
        i18n("&Sort Alphabetically"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotSort() ), actionCollection(), "sort");
    (void) new TDEAction(
        i18n("Set as T&oolbar Folder"), "bookmark_toolbar", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotSetAsToolbar() ), actionCollection(), "setastoolbar");
    (void) new TDEAction(
        i18n("Show in T&oolbar"), "bookmark_toolbar", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotShowInToolbar() ), actionCollection(), "showintoolbar");
    (void) new TDEAction(
        i18n("Hide in T&oolbar"), "bookmark_toolbar", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotHideInToolbar() ), actionCollection(), "hideintoolbar");
    (void) new TDEAction(
        i18n("&Expand All Folders"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExpandAll() ), actionCollection(), "expandall");
    (void) new TDEAction(
        i18n("Collapse &All Folders"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotCollapseAll() ), actionCollection(), "collapseall" );
    (void) new TDEAction(
        i18n("&Open in Konqueror"), "document-open", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotOpenLink() ), actionCollection(), "openlink" );
    (void) new TDEAction(
        i18n("Check &Status"), "bookmark", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotTestSelection() ), actionCollection(), "testlink" );

    (void) new TDEAction(
        i18n("Check Status: &All"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotTestAll() ), actionCollection(), "testall" );
    (void) new TDEAction(
        i18n("Update All &Favicons"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotUpdateAllFavIcons() ), actionCollection(),
        "updateallfavicons" );
    (void) new TDEAction(
        i18n("Cancel &Checks"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotCancelAllTests() ), actionCollection(), "canceltests" );
    (void) new TDEAction(
        i18n("Cancel &Favicon Updates"), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotCancelFavIconUpdates() ), actionCollection(),
        "cancelfaviconupdates" );
    (void) new TDEAction(
        i18n("Import &Netscape Bookmarks..."), "netscape", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importNS");
    (void) new TDEAction(
        i18n("Import &Opera Bookmarks..."), "opera", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importOpera");
    (void) new TDEAction(
        i18n("Import All &Crash Sessions as Bookmarks..."), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importCrashes");
    (void) new TDEAction(
        i18n("Import &Galeon Bookmarks..."), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importGaleon");
    (void) new TDEAction(
        i18n("Import &KDE2/KDE3 Bookmarks..."), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importKDE2");
    (void) new TDEAction(
        i18n("Import &IE Bookmarks..."), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importIE");
    (void) new TDEAction(
        i18n("Import &Mozilla Bookmarks..."), "mozilla", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotImport() ), actionCollection(), "importMoz");
    (void) new TDEAction(
        i18n("Export to &Netscape Bookmarks"), "netscape", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExportNS() ), actionCollection(), "exportNS");
    (void) new TDEAction(
        i18n("Export to &Opera Bookmarks..."), "opera", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExportOpera() ), actionCollection(), "exportOpera");
    (void) new TDEAction(
        i18n("Export to &HTML Bookmarks..."), "text-html", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExportHTML() ), actionCollection(), "exportHTML");
    (void) new TDEAction(
        i18n("Export to &IE Bookmarks..."), 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExportIE() ), actionCollection(), "exportIE");
    (void) new TDEAction(
        i18n("Export to &Mozilla Bookmarks..."), "mozilla", 0,
        TQT_TQOBJECT(actn), TQT_SLOT( slotExportMoz() ), actionCollection(), "exportMoz");
}

void ActionsImpl::slotLoad() {
    TQString bookmarksFile
        = KFileDialog::getOpenFileName(TQString::null, "*.xml", KEBApp::self());
    if (bookmarksFile.isNull())
        return;
    KEBApp::self()->m_caption = TQString::null;
    KEBApp::self()->m_bookmarksFilename = bookmarksFile;
    KEBApp::self()->construct();
}

void ActionsImpl::slotSaveAs() {
    KEBApp::self()->bkInfo()->commitChanges();
    TQString saveFilename
        = KFileDialog::getSaveFileName(TQString::null, "*.xml", KEBApp::self());
    if (!saveFilename.isEmpty())
        CurrentMgr::self()->saveAs(saveFilename);
}

void CurrentMgr::doExport(ExportType type, const TQString & _path) {
    if(KEBApp::self())
        KEBApp::self()->bkInfo()->commitChanges();
    TQString path(_path);
    // TODO - add a factory and make all this use the base class
    if (type == OperaExport) {
        if (path.isNull())
            path = KOperaBookmarkImporterImpl().findDefaultLocation(true);
        KOperaBookmarkExporterImpl exporter(mgr(), path);
        exporter.write(mgr()->root());
        return;

    } else if (type == HTMLExport) {
        if (path.isNull())
            path = KFileDialog::getSaveFileName(
                        TQDir::homeDirPath(),
                        i18n("*.html|HTML Bookmark Listing") );
        HTMLExporter exporter;
        exporter.write(mgr()->root(), path);
        return;

    } else if (type == IEExport) {
        if (path.isNull())
            path = KIEBookmarkImporterImpl().findDefaultLocation(true);
        KIEBookmarkExporterImpl exporter(mgr(), path);
        exporter.write(mgr()->root());
        return;
    }

    bool moz = (type == MozillaExport);

    if (path.isNull())
        path = (moz) ? KNSBookmarkImporter::mozillaBookmarksFile(true)
            : KNSBookmarkImporter::netscapeBookmarksFile(true);

    if (!path.isEmpty()) {
        KNSBookmarkExporter exporter(mgr(), path);
        exporter.write(moz);
    }
}

void KEBApp::setActionsEnabled(SelcAbilities sa) {
    TDEActionCollection * coll = actionCollection();

    TQStringList toEnable;

    if (sa.multiSelect || (sa.singleSelect && !sa.root))
        toEnable << "edit_copy";

    if (sa.multiSelect || (sa.singleSelect && !sa.root && !sa.urlIsEmpty && !sa.group && !sa.separator))
            toEnable << "openlink";

    if (!m_readOnly) {
        if (sa.notEmpty)
            toEnable << "testall" << "updateallfavicons";

        if ( sa.multiSelect || (sa.singleSelect && !sa.root) )
                toEnable << "delete" << "edit_cut";

        if( sa.singleSelect)
            if (m_canPaste)
                toEnable << "edit_paste";

        if( sa.multiSelect || (sa.singleSelect && !sa.root && (sa.group || !sa.urlIsEmpty) && !sa.separator))
            toEnable << "testlink" << "updatefavicon";

        if(sa.multiSelect)
            toEnable << "showintoolbar" << "hideintoolbar";
        else if(sa.itemSelected)
            toEnable << (sa.tbShowState ? "hideintoolbar" : "showintoolbar");

        if (sa.singleSelect && !sa.root && !sa.separator) {
            toEnable << "rename" << "changeicon" << "changecomment";
            if (!sa.group)
                toEnable << "changeurl";
        }

        if (sa.singleSelect) {
            toEnable << "newfolder" << "newbookmark" << "insertseparator";
            if (sa.group)
                toEnable << "sort" << "recursivesort" << "setastoolbar";
        }
    }

    for ( TQStringList::Iterator it = toEnable.begin();
            it != toEnable.end(); ++it )
    {
        coll->action((*it).ascii())->setEnabled(true);
        // kdDebug() << (*it) << endl;
    }
}

void KEBApp::setCancelFavIconUpdatesEnabled(bool enabled) {
    actionCollection()->action("cancelfaviconupdates")->setEnabled(enabled);
}

void KEBApp::setCancelTestsEnabled(bool enabled) {
    actionCollection()->action("canceltests")->setEnabled(enabled);
}

void ActionsImpl::slotCut() {
    KEBApp::self()->bkInfo()->commitChanges();
    slotCopy();
    DeleteManyCommand *mcmd = new DeleteManyCommand( i18n("Cut Items"), ListView::self()->selectedAddresses() );
    CmdHistory::self()->addCommand(mcmd);

}

void ActionsImpl::slotCopy() {
    KEBApp::self()->bkInfo()->commitChanges();
    // this is not a command, because it can't be undone
    Q_ASSERT(ListView::self()->selectedItemsMap().count() != 0);
    TQValueList<KBookmark> bookmarks
        = ListView::self()->itemsToBookmarks(ListView::self()->selectedItemsMap());
    KBookmarkDrag* data = KBookmarkDrag::newDrag(bookmarks, 0 /* not this ! */);
    kapp->clipboard()->setData(data, TQClipboard::Clipboard);
}

void ActionsImpl::slotPaste() {
    KEBApp::self()->bkInfo()->commitChanges();
    KEBMacroCommand *mcmd =
        CmdGen::insertMimeSource(
                            i18n("Paste"),
                            kapp->clipboard()->data(TQClipboard::Clipboard),
                            ListView::self()->userAddress());
    CmdHistory::self()->didCommand(mcmd);
}

/* -------------------------------------- */

void ActionsImpl::slotNewFolder() {
    KEBApp::self()->bkInfo()->commitChanges();
    bool ok;
    TQString str = KInputDialog::getText( i18n( "Create New Bookmark Folder" ),
            i18n( "New folder:" ), TQString::null, &ok );
    if (!ok)
        return;

    CreateCommand *cmd = new CreateCommand(
                                ListView::self()->userAddress(),
                                str, "bookmark_folder", /*open*/ true);
    CmdHistory::self()->addCommand(cmd);
}

void ActionsImpl::slotNewBookmark() {
    KEBApp::self()->bkInfo()->commitChanges();
    // TODO - make a setCurrentItem(Command *) which uses finaladdress interface
    CreateCommand * cmd = new CreateCommand(
                                ListView::self()->userAddress(),
                                TQString::null, "www", KURL("http://"));
    CmdHistory::self()->addCommand(cmd);
}

void ActionsImpl::slotInsertSeparator() {
    KEBApp::self()->bkInfo()->commitChanges();
    CreateCommand * cmd = new CreateCommand(ListView::self()->userAddress());
    CmdHistory::self()->addCommand(cmd);
}

void ActionsImpl::slotImport() {
    KEBApp::self()->bkInfo()->commitChanges();
    // kdDebug() << "ActionsImpl::slotImport() where sender()->name() == "
    //           << sender()->name() << endl;
    ImportCommand* import
        = ImportCommand::performImport(TQT_TQOBJECT_CONST(sender())->name()+6, KEBApp::self());
    if (!import)
        return;
    CmdHistory::self()->addCommand(import);
    ListView::self()->setCurrent( ListView::self()->getItemAtAddress(import->groupAddress()), true);
}

// TODO - this is getting ugly and repetitive. cleanup!

void ActionsImpl::slotExportOpera() {
    KEBApp::self()->bkInfo()->commitChanges();
    CurrentMgr::self()->doExport(CurrentMgr::OperaExport); }
void ActionsImpl::slotExportHTML() {
    KEBApp::self()->bkInfo()->commitChanges();
    CurrentMgr::self()->doExport(CurrentMgr::HTMLExport); }
void ActionsImpl::slotExportIE() {
    KEBApp::self()->bkInfo()->commitChanges();
    CurrentMgr::self()->doExport(CurrentMgr::IEExport); }
void ActionsImpl::slotExportNS() {
    KEBApp::self()->bkInfo()->commitChanges();
    CurrentMgr::self()->doExport(CurrentMgr::NetscapeExport); }
void ActionsImpl::slotExportMoz() {
    KEBApp::self()->bkInfo()->commitChanges();
    CurrentMgr::self()->doExport(CurrentMgr::MozillaExport); }

/* -------------------------------------- */

static TQCString s_appId, s_objId;
static KParts::ReadOnlyPart *s_part;

void ActionsImpl::slotPrint() {
    KEBApp::self()->bkInfo()->commitChanges();
    s_part = KParts::ComponentFactory
                        ::createPartInstanceFromQuery<KParts::ReadOnlyPart>(
                                "text/html", TQString::null);
    s_part->setProperty("pluginsEnabled", TQVariant(false, 1));
    s_part->setProperty("javaScriptEnabled", TQVariant(false, 1));
    s_part->setProperty("javaEnabled", TQVariant(false, 1));

    // doc->openStream( "text/html", KURL() );
    // doc->writeStream( TQCString( "<HTML><BODY>FOO</BODY></HTML>" ) );
    // doc->closeStream();

    HTMLExporter exporter;
    KTempFile tmpf(locateLocal("tmp", "print_bookmarks"), ".html");
    TQTextStream *tstream = tmpf.textStream();
    tstream->setEncoding(TQTextStream::Unicode);
    (*tstream) << exporter.toString(CurrentMgr::self()->mgr()->root(), true);
    tmpf.close();

    s_appId = kapp->dcopClient()->appId();
    s_objId = s_part->property("dcopObjectId").toString().latin1();
    connect(s_part, TQT_SIGNAL(completed()), this, TQT_SLOT(slotDelayedPrint()));

    s_part->openURL(KURL( tmpf.name() ));
}

void ActionsImpl::slotDelayedPrint() {
    Q_ASSERT(s_part);
    DCOPRef(s_appId, s_objId).send("print", false);
    delete s_part;
    s_part = 0;
}

/* -------------------------------------- */

void ActionsImpl::slotShowNS() {
    KEBApp::self()->bkInfo()->commitChanges();
    bool shown = KEBApp::self()->nsShown();
    CurrentMgr::self()->mgr()->setShowNSBookmarks(shown);
    // TODO - need to force a save here
    CurrentMgr::self()->reloadConfig();
}

void ActionsImpl::slotCancelFavIconUpdates() {
    FavIconsItrHolder::self()->cancelAllItrs();
}

void ActionsImpl::slotCancelAllTests() {
    TestLinkItrHolder::self()->cancelAllItrs();
}

void ActionsImpl::slotTestAll() {
    TestLinkItrHolder::self()->insertItr(
            new TestLinkItr(ListView::self()->allBookmarks()));
}

void ActionsImpl::slotUpdateAllFavIcons() {
    FavIconsItrHolder::self()->insertItr(
            new FavIconsItr(ListView::self()->allBookmarks()));
}

ActionsImpl::~ActionsImpl() {
    delete FavIconsItrHolder::self();
    delete TestLinkItrHolder::self();
}

/* -------------------------------------- */

void ActionsImpl::slotTestSelection() {
    KEBApp::self()->bkInfo()->commitChanges();
    TestLinkItrHolder::self()->insertItr(new TestLinkItr(ListView::self()->selectedBookmarksExpanded()));
}

void ActionsImpl::slotUpdateFavIcon() {
    KEBApp::self()->bkInfo()->commitChanges();
    FavIconsItrHolder::self()->insertItr(new FavIconsItr(ListView::self()->selectedBookmarksExpanded()));
}

/* -------------------------------------- */

class KBookmarkGroupList : private KBookmarkGroupTraverser {
public:
    KBookmarkGroupList(KBookmarkManager *);
    TQValueList<KBookmark> getList(const KBookmarkGroup &);
private:
    virtual void visit(const KBookmark &) { ; }
    virtual void visitEnter(const KBookmarkGroup &);
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    KBookmarkManager *m_manager;
    TQValueList<KBookmark> m_list;
};

KBookmarkGroupList::KBookmarkGroupList( KBookmarkManager *manager ) {
    m_manager = manager;
}

TQValueList<KBookmark> KBookmarkGroupList::getList( const KBookmarkGroup &grp ) {
    traverse(grp);
    return m_list;
}

void KBookmarkGroupList::visitEnter(const KBookmarkGroup &grp) {
    m_list << grp;
}

void ActionsImpl::slotRecursiveSort() {
    KEBApp::self()->bkInfo()->commitChanges();
    KBookmark bk = ListView::self()->firstSelected()->bookmark();
    Q_ASSERT(bk.isGroup());
    KEBMacroCommand *mcmd = new KEBMacroCommand(i18n("Recursive Sort"));
    KBookmarkGroupList lister(CurrentMgr::self()->mgr());
    TQValueList<KBookmark> bookmarks = lister.getList(bk.toGroup());
    bookmarks << bk.toGroup();
    for (TQValueListConstIterator<KBookmark> it = bookmarks.begin(); it != bookmarks.end(); ++it) {
        SortCommand *cmd = new SortCommand("", (*it).address());
        cmd->execute();
        mcmd->addCommand(cmd);
    }
    CmdHistory::self()->didCommand(mcmd);
}

void ActionsImpl::slotSort() {
    KEBApp::self()->bkInfo()->commitChanges();
    KBookmark bk = ListView::self()->firstSelected()->bookmark();
    Q_ASSERT(bk.isGroup());
    SortCommand *cmd = new SortCommand(i18n("Sort Alphabetically"), bk.address());
    CmdHistory::self()->addCommand(cmd);
}

/* -------------------------------------- */

void ActionsImpl::slotDelete() {
    KEBApp::self()->bkInfo()->commitChanges();
    DeleteManyCommand *mcmd = new DeleteManyCommand(i18n("Delete Items"), ListView::self()->selectedAddresses());
    CmdHistory::self()->addCommand(mcmd);
}

void ActionsImpl::slotOpenLink() {
    KEBApp::self()->bkInfo()->commitChanges();
    TQValueList<KBookmark> bks = ListView::self()->itemsToBookmarks(ListView::self()->selectedItemsMap());
    TQValueListIterator<KBookmark> it;
    for (it = bks.begin(); it != bks.end(); ++it) {
        if ((*it).isGroup() || (*it).isSeparator())
            continue;
        (void)new KRun((*it).url());
    }
}

/* -------------------------------------- */

void ActionsImpl::slotRename() {
    KEBApp::self()->bkInfo()->commitChanges();
    ListView::self()->rename(KEBListView::NameColumn);
}

void ActionsImpl::slotChangeURL() {
    KEBApp::self()->bkInfo()->commitChanges();
    ListView::self()->rename(KEBListView::UrlColumn);
}

void ActionsImpl::slotChangeComment() {
    KEBApp::self()->bkInfo()->commitChanges();
    ListView::self()->rename(KEBListView::CommentColumn);
}

void ActionsImpl::slotSetAsToolbar() {
    KEBApp::self()->bkInfo()->commitChanges();
    KBookmark bk = ListView::self()->firstSelected()->bookmark();
    Q_ASSERT(bk.isGroup());
    KEBMacroCommand *mcmd = CmdGen::setAsToolbar(bk);
    CmdHistory::self()->addCommand(mcmd);
}

void ActionsImpl::slotShowInToolbar() {
    KEBApp::self()->bkInfo()->commitChanges();
    TQValueList<KBookmark> bks = ListView::self()->itemsToBookmarks(ListView::self()->selectedItemsMap());
    KEBMacroCommand *mcmd = CmdGen::setShownInToolbar(bks, true);
    CmdHistory::self()->addCommand(mcmd);
}

void ActionsImpl::slotHideInToolbar() {
    KEBApp::self()->bkInfo()->commitChanges();
    TQValueList<KBookmark> bks = ListView::self()->itemsToBookmarks(ListView::self()->selectedItemsMap());
    KEBMacroCommand *mcmd = CmdGen::setShownInToolbar(bks, false);
    CmdHistory::self()->addCommand(mcmd);
}

void ActionsImpl::slotChangeIcon() {
    KEBApp::self()->bkInfo()->commitChanges();
    KBookmark bk = ListView::self()->firstSelected()->bookmark();
    TDEIconDialog dlg(KEBApp::self());
    TQString newIcon = dlg.selectIcon(TDEIcon::Small, TDEIcon::Place);
    if (newIcon.isEmpty())
        return;
    EditCommand *cmd = new EditCommand(
                                bk.address(),
                                EditCommand::Edition("icon", newIcon),
                                i18n("Icon"));
    CmdHistory::self()->addCommand(cmd);
}

void ActionsImpl::slotExpandAll() {
    ListView::self()->setOpen(true);
}

void ActionsImpl::slotCollapseAll() {
    ListView::self()->setOpen(false);
}

#include "actionsimpl.moc"
