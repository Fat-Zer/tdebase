/*
 * krootwm.h Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *           (C) 1997 Torben Weis, weis@kde.org
 *           (C) 1998 S.u.S.E, weis@suse.de
 *
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

#ifndef __krootwm_h__
#define __krootwm_h__

#include <tqpixmap.h>
#include <tqobject.h>

// we need Window but do not want to include X.h since it
// #defines way too many constants
typedef unsigned long XID;
typedef XID Window;

class KMenuBar;
class KDesktop;
class SaverEngine;
class TQPopupMenu;
class KCMultiDialog;
class KNewMenu;
class KWinModule;
class KBookmarkMenu;
class KHelpMenu;
class TDEActionCollection;
class TDEActionMenu;
class KWindowListMenu;

enum {
  ITEM_HELP=100,
  ITEM_PASTE,
  ITEM_EXECUTE,
  ITEM_CONFIGURE_BACKGROUND,
  ITEM_CONFIGURE_ICONS,
  ITEM_UNCLUTTER_WINDOWS,
  ITEM_CASCADE_WINDOWS,
  ITEM_ARRANGE_ICONS,
  ITEM_LOCK_SCREEN,
  ITEM_LOGOUT
};

class TQEventLoopThread;
class KRootWmThreadHelperObject;

/**
 * This class is the handler for the menus (root popup menu and desktop menubar)
 */
class KRootWm: public TQObject {
  Q_OBJECT

public:
  KRootWm(SaverEngine*, KDesktop*);
  ~KRootWm();

  bool startup;
  void mousePressed( const TQPoint& _global, int _button );
  bool hasLeftButtonMenu() { return leftButtonChoice != NOTHING; }

  /**
   * Return the unique KRootWm instance
   */
  static KRootWm * self() { return s_rootWm; }

  /**
   * share this with desktop.cc
   */
  KNewMenu * newMenu() const { return menuNew; }

  /**
   * The position of the (usually RMB) click that opened the 'desktop' menu
   */
  TQPoint desktopMenuPosition() const { return m_desktopMenuPosition; }

  /**
   * Read and apply configuration
   */
  void initConfig();

  /**
   * List of config modules used by Configure Desktop
   */
  static TQStringList configModules();

public slots:
  void slotArrangeByNameCS();
  void slotArrangeByNameCI();
  void slotArrangeBySize();
  void slotArrangeByType();
  void slotArrangeByDate();
  void slotLineupIconsHoriz();
  void slotLineupIconsVert();
  void slotLineupIcons();
  void slotRefreshDesktop();
  void slotConfigureDesktop();
  void slotToggleDirFirst( bool );
  void slotToggleAutoAlign( bool );
  void slotToggleLockIcons( bool );
  void slotToggleDesktopMenu();
  void slotUnclutterWindows();
  void slotCascadeWindows();
  void slotWindowList();
  void slotLock();
  void slotLogout();
  void slotSave();
  void slotSwitchUser();
  void slotPopulateSessions();
  void slotSessionActivated( int );
  void slotNewSession();
  void slotOpenTerminal();
  void slotLockNNewSession();

private:
  SaverEngine* m_pSaver;
  KDesktop* m_pDesktop;

  // The five root menus :
  KWindowListMenu* windowListMenu;
  TQPopupMenu* desktopMenu;
  // the appMenu is (will be) provided by kicker
  TQPopupMenu* customMenu1;
  TQPopupMenu* customMenu2;
  KCMultiDialog* m_configDialog;

  // Configuration for the root menus :
  typedef enum { NOTHING = 0, WINDOWLISTMENU, DESKTOPMENU, APPMENU, CUSTOMMENU1, CUSTOMMENU2, BOOKMARKSMENU, SESSIONSMENU } menuChoice;
  menuChoice leftButtonChoice;
  menuChoice middleButtonChoice;
  menuChoice rightButtonChoice;

  KNewMenu* menuNew;
  TDEActionMenu* bookmarks;
  KBookmarkMenu* bookmarkMenu;
  TDEActionCollection * m_actionCollection;
  TQPoint m_desktopMenuPosition;

  void activateMenu( menuChoice choice, const TQPoint& global );
  void buildMenus();

  bool m_bShowMenuBar;
  bool m_bGlobalMenuBar;
  bool m_bInit;
  bool m_bDesktopEnabled;
  KMenuBar *menuBar;

  TQPopupMenu *file;
  TQPopupMenu *desk;
  KHelpMenu *help;

  TQPixmap defaultPixmap;

  void doNewSession( bool lock );
  TQPopupMenu *sessionsMenu;

  static KRootWm * s_rootWm;

private slots:

  void slotMenuItemActivated(int);
  void slotFileNewAboutToShow();
  void slotWindowListAboutToShow();
  void slotConfigClosed();
};

#endif
