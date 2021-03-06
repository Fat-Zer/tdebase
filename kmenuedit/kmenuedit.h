/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
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

#ifndef __kmenuedit_h__
#define __kmenuedit_h__

#include <tdemainwindow.h>
#include <treeview.h>

class BasicTab;
class TQSplitter;
class TDEAction;
class TDEToggleAction;

class KMenuEdit : public TDEMainWindow
{
    Q_OBJECT

public:
    KMenuEdit( bool controlCenter, TQWidget *parent=0, const char *name=0 );
    ~KMenuEdit();

    void selectMenu(const TQString &menu) { m_tree->selectMenu(menu); }
    void selectMenuEntry(const TQString &menuEntry) { m_tree->selectMenuEntry(menuEntry); }

protected:
    void setupView();
    void setupActions();
    bool queryClose();

protected slots:
    void slotSave();
    void slotSave_and_close();
    void slotChangeView();
    void slotConfigureToolbars();
protected:
    TreeView           *m_tree;
    BasicTab           *m_basicTab;
    TQSplitter          *m_splitter;

    TDEAction *m_actionDelete;
    TDEToggleAction *m_actionShowHidden;
    bool m_showHidden;
    bool m_controlCenter;
};

#endif
