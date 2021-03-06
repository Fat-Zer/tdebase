/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <tqsplitter.h>

#include <tdeaction.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kservice.h>
#include <kstdaction.h>
#include <tdestdaccel.h>

#include "treeview.h"
#include "basictab.h"
#include "kmenuedit.h"
#include "kmenuedit.moc"

KMenuEdit::KMenuEdit (bool controlCenter, TQWidget *, const char *name)
  : TDEMainWindow (0, name), m_tree(0), m_basicTab(0), m_splitter(0), m_controlCenter(controlCenter)
{
#if 0
    m_showHidden = config->readBoolEntry("ShowHidden");
#else
    m_showHidden = false;
#endif

    // setup GUI
    setupActions();
    slotChangeView();
}

KMenuEdit::~KMenuEdit()
{
    TDEConfig *config = TDEGlobal::config();
    config->setGroup("General");
    config->writeEntry("SplitterSizes", m_splitter->sizes());

    config->sync();
}

void KMenuEdit::setupActions()
{
    (void)new TDEAction(i18n("&New Submenu..."), "menu_new", 0, actionCollection(), "newsubmenu");
    (void)new TDEAction(i18n("New &Item..."), "document-new", TDEStdAccel::openNew(), actionCollection(), "newitem");
    if (!m_controlCenter)
       (void)new TDEAction(i18n("New S&eparator"), "menu_new_sep", 0, actionCollection(), "newsep");

    (void)new TDEAction(i18n("Save && Quit"), "filesave_and_close", 0, TQT_TQOBJECT(this), TQT_SLOT( slotSave_and_close()), actionCollection(), "file_save_and_quit");

    m_actionDelete = 0;

    KStdAction::save(TQT_TQOBJECT(this), TQT_SLOT( slotSave() ), actionCollection());
    KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT( close() ), actionCollection());
    KStdAction::cut(0, 0, actionCollection());
    KStdAction::copy(0, 0, actionCollection());
    KStdAction::paste(0, 0, actionCollection());
}

void KMenuEdit::setupView()
{
    m_splitter = new TQSplitter(Qt::Horizontal, this);
    m_tree = new TreeView(m_controlCenter, actionCollection(), m_splitter);
    m_basicTab = new BasicTab(m_splitter);

    connect(m_tree, TQT_SIGNAL(entrySelected(MenuFolderInfo *)),
            m_basicTab, TQT_SLOT(setFolderInfo(MenuFolderInfo *)));
    connect(m_tree, TQT_SIGNAL(entrySelected(MenuEntryInfo *)),
            m_basicTab, TQT_SLOT(setEntryInfo(MenuEntryInfo *)));
    connect(m_tree, TQT_SIGNAL(disableAction()),
            m_basicTab, TQT_SLOT(slotDisableAction() ) );

    connect(m_basicTab, TQT_SIGNAL(changed(MenuFolderInfo *)),
            m_tree, TQT_SLOT(currentChanged(MenuFolderInfo *)));

    connect(m_basicTab, TQT_SIGNAL(changed(MenuEntryInfo *)),
            m_tree, TQT_SLOT(currentChanged(MenuEntryInfo *)));

    connect(m_basicTab, TQT_SIGNAL(findServiceShortcut(const TDEShortcut&, KService::Ptr &)),
            m_tree, TQT_SLOT(findServiceShortcut(const TDEShortcut&, KService::Ptr &)));

    // restore splitter sizes
    TDEConfig* config = TDEGlobal::config();
    TQValueList<int> sizes = config->readIntListEntry("SplitterSizes");

    if (sizes.isEmpty())
        sizes << 1 << 3;
    m_splitter->setSizes(sizes);
    m_tree->setFocus();

    setCentralWidget(m_splitter);
}

void KMenuEdit::slotChangeView()
{
#if 0
    m_showHidden = m_actionShowHidden->isChecked();
#else
    m_showHidden = false;
#endif

    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );
    guiFactory()->removeClient( this );

    delete m_actionDelete;

    m_actionDelete = new TDEAction(i18n("&Delete"), "edit-delete", Key_Delete, actionCollection(), "delete");

    if (!m_splitter)
       setupView();
    if (m_controlCenter)
       setupGUI(TDEMainWindow::ToolBar|Keys|Save|Create, "kcontroleditui.rc");
    else
       setupGUI(TDEMainWindow::ToolBar|Keys|Save|Create, "kmenueditui.rc");

    m_tree->setViewMode(m_showHidden);
}

void KMenuEdit::slotSave()
{
    m_tree->save();
}

void KMenuEdit::slotSave_and_close()
{
    if (m_tree->save())
        close();
}

bool KMenuEdit::queryClose()
{
    if (!m_tree->dirty()) return true;


    int result;
    if (m_controlCenter)
    {
       result = KMessageBox::warningYesNoCancel(this,
                    i18n("You have made changes to the Control Center.\n"
                         "Do you want to save the changes or discard them?"),
                    i18n("Save Control Center Changes?"),
                    KStdGuiItem::save(), KStdGuiItem::discard() );
    }
    else
    {
       result = KMessageBox::warningYesNoCancel(this,
                    i18n("You have made changes to the menu.\n"
                         "Do you want to save the changes or discard them?"),
                    i18n("Save Menu Changes?"),
                    KStdGuiItem::save(), KStdGuiItem::discard() );
    }

    switch(result)
    {
      case KMessageBox::Yes:
         return m_tree->save();

      case KMessageBox::No:
         return true;

      default:
         break;
    }
    return false;
}

void KMenuEdit::slotConfigureToolbars()
{
    KEditToolbar dlg( factory() );
      
    dlg.exec();
}
