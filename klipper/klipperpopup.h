// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>
   Copytight (C) by Andrew Stanley-Jones
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
#ifndef _KLIPPERPOPUP_H_
#define _KLIPPERPOPUP_H_

#include <tdepopupmenu.h>
#include <tqptrlist.h>
#include <tqstring.h>

class History;
class KlipperWidget;
class KHelpMenu;
class TDEAction;
class PopupProxy;
class KLineEdit;

/**
 * Default view of clipboard history.
 *
 */
class KlipperPopup : public TDEPopupMenu
{
    Q_OBJECT

public:
    KlipperPopup( History* history, TQWidget* parent=0, const char* name=0 );
    ~KlipperPopup();
    void plugAction( TDEAction* action );

    /**
     * Normally, the popupmenu is only rebuilt just before showing.
     * If you need the pixel-size or similar of the this menu, call
     * this beforehand.
     */
    void ensureClean();

    History* history() { return m_history; }
    const History* history() const { return m_history; }

public slots:
    void slotHistoryChanged() { m_dirty = true; }
    void slotAboutToShow();

private:
    void rebuild( const TQString& filter = TQString::null );
    void buildFromScratch();

    void insertSearchFilter();
    void removeSearchFilter();

protected:
     virtual void keyPressEvent( TQKeyEvent* e );

private:
    bool m_dirty : 1; // true if menu contents needs to be rebuild.

    /**
     * Contains the string shown if the menu is empty.
     */
    TQString QSempty;

    /**
     * Contains the string shown if the search string has no
     * matches and the menu is not empty.
     */
    TQString QSnomatch;

    /**
     * The "document" (clipboard history)
     */
    History* m_history;

    /**
     * The help menu
     */
    KHelpMenu* helpmenu;

    /**
     * (unowned) actions to plug into the primary popup menu
     */
    TQPtrList<TDEAction> m_actions;

    /**
     * Proxy helper object used to track history items
     */
    PopupProxy* m_popupProxy;

    /**
     * search filter widget
     */
    KLineEdit* m_filterWidget;

    /**
     * id of search widget, for convenience
     */
    int m_filterWidgetId;

    /**
     * The current number of history items in the clipboard
     */
    int n_history_items;

signals:
    void clearHistory();
    void configure();
    void quit();

};

#endif
