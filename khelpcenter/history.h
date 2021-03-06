/*
 *  This file is part of the TDE Help Center
 *
 *  Copyright (C) 2002 Frerich Raabe <raabe@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef HISTORY_H
#define HISTORY_H

#include <kurl.h>

#include <tqobject.h>
#include <tqptrlist.h>

class TDEActionCollection;
class TDEMainWindow;
class TDEToolBarPopupAction;
class TQPopupMenu;

namespace KHC {

class View;

class History : public TQObject
{
    Q_OBJECT
  public:
    friend class foo; // to make gcc shut up
    struct Entry
    {
      Entry() : view( 0 ), search( false ) {}

      View *view;
      KURL url;
      TQString title;
      TQByteArray buffer;
      bool search;
    };

    static History &self();

    void setupActions( TDEActionCollection *coll );
    void updateActions();

    void installMenuBarHook( TDEMainWindow *mainWindow );

    void createEntry();
    void updateCurrentEntry( KHC::View *view );

  signals:
    void goInternalUrl( const KURL & );
    void goUrl( const KURL & );

  private slots:
    void backActivated( int id );
    void fillBackMenu();
    void forwardActivated( int id );
    void fillForwardMenu();
    void goMenuActivated( int id );
    void fillGoMenu();
    void back();
    void forward();
    void goHistoryActivated( int steps );
    void goHistory( int steps );
    void goHistoryDelayed();

  private:
    History();
    History( const History &rhs );
    History &operator=( const History &rhs );
    ~History();

    bool canGoBack() const;
    bool canGoForward() const;
    void fillHistoryPopup( TQPopupMenu *, bool, bool, bool, uint = 0 );

    static History *m_instance;

    TQPtrList<Entry> m_entries;


    int m_goBuffer;
    int m_goMenuIndex;
    int m_goMenuHistoryStartPos;
    int m_goMenuHistoryCurrentPos;
  public:
    TDEToolBarPopupAction *m_backAction;
    TDEToolBarPopupAction *m_forwardAction;
};

}

#endif // HISTORY_H
// vim:ts=2:sw=2:et
