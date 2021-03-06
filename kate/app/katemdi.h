/* This file is part of the KDE libraries
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef __KATE_MDI_H__
#define __KATE_MDI_H__

#include <tdeparts/mainwindow.h>

#include <tdemultitabbar.h>
#include <kxmlguiclient.h>
#include <tdeaction.h>

#include <tqdict.h>
#include <tqintdict.h>
#include <tqmap.h>
#include <tqsplitter.h>
#include <tqpixmap.h>
#include <tqptrlist.h>

namespace KateMDI {


/** This class is needed because TQSplitter cant return an index for a widget. */
class Splitter : public TQSplitter
{
  Q_OBJECT
  

  public:
    Splitter(Orientation o, TQWidget* parent=0, const char* name=0);
    ~Splitter();

    /** Since there is supposed to be only 2 childs of a katesplitter,
     * any child other than the last is the first.
     * This method uses TQSplitter::idAfter(widget) which
     * returns 0 if there is no widget after this one.
     * This results in an error if widget is not a child
     * in this splitter */
    bool isLastChild(TQWidget* w) const;

    int idAfter ( TQWidget * w ) const;
};

class ToggleToolViewAction : public TDEToggleAction
{
  Q_OBJECT
  

  public:
    ToggleToolViewAction ( const TQString& text, const TDEShortcut& cut,
                           class ToolView *tv, TQObject* parent = 0, const char* name = 0 );

    virtual ~ToggleToolViewAction();

  protected slots:
    void slotToggled(bool);
    void visibleChanged(bool);

  private:
    ToolView *m_tv;
};

class GUIClient : public TQObject, public KXMLGUIClient
{
  Q_OBJECT
  

  public:
    GUIClient ( class MainWindow *mw );
    virtual ~GUIClient();

    void registerToolView (ToolView *tv);
    void unregisterToolView (ToolView *tv);
    void updateSidebarsVisibleAction();

  private slots:
    void clientAdded( KXMLGUIClient *client );
    void updateActions();

  private:
    MainWindow *m_mw;
    TDEToggleAction *m_showSidebarsAction;
    TQPtrList<TDEAction> m_toolViewActions;
    TQMap<ToolView*, TDEAction*> m_toolToAction;
    TDEActionMenu *m_toolMenu;
};

class ToolView : public TQVBox
{
  Q_OBJECT
  

  friend class Sidebar;
  friend class MainWindow;
  friend class GUIClient;
  friend class ToggleToolViewAction;

  protected:
    /**
     * ToolView
     * Objects of this clas represent a toolview in the mainwindow
     * you should only add one widget as child to this toolview, it will
     * be automatically set to be the focus proxy of the toolview
     * @param mainwin main window for this toolview
     * @param sidebar sidebar of this toolview
     * @param parent parent widget, e.g. the splitter of one of the sidebars
     */
    ToolView (class MainWindow *mainwin, class Sidebar *sidebar, TQWidget *parent);

  public:
    /**
     * destuct me, this is allowed for all, will care itself that the toolview is removed
     * from the mainwindow and sidebar
     */
    virtual ~ToolView ();

  signals:
    /**
     * toolview hidden or shown
     * @param bool is this toolview made visible?
     */
    void visibleChanged (bool visible);

  /**
   * some internal methodes needed by the main window and the sidebars
   */
  protected:
    MainWindow *mainWindow () { return m_mainWin; }

    Sidebar *sidebar () { return m_sidebar; }

    void setVisible (bool vis);

  public:
    bool visible () const;

  protected:
    void childEvent ( TQChildEvent *ev );

  private:
    MainWindow *m_mainWin;
    Sidebar *m_sidebar;

    /**
     * unique id
     */
    TQString id;

    /**
     * is visible in sidebar
     */
    bool m_visible;

    /**
     * is this view persistent?
     */
    bool persistent;

    TQPixmap icon;
    TQString text;
};

class Sidebar : public KMultiTabBar
{
  Q_OBJECT
  

  public:
    Sidebar (KMultiTabBar::KMultiTabBarPosition pos, class MainWindow *mainwin, TQWidget *parent);
    virtual ~Sidebar ();

    void setSplitter (Splitter *sp);

  public:
    ToolView *addWidget (const TQPixmap &icon, const TQString &text, ToolView *widget);
    bool removeWidget (ToolView *widget);

    bool showWidget (ToolView *widget);
    bool hideWidget (ToolView *widget);

    void setLastSize (int s) { m_lastSize = s; }
    int lastSize () const { return m_lastSize; }
    void updateLastSize ();

    bool splitterVisible () const { return m_ownSplit->isVisible(); }

    void restoreSession ();

     /**
     * restore the current session config from given object, use current group
     * @param config config object to use
     */
    void restoreSession (TDEConfig *config);

     /**
     * save the current session config to given object, use current group
     * @param config config object to use
     */
    void saveSession (TDEConfig *config);

  public slots:
    // reimplemented, to block a show() call if we have no children or if
    // all sidebars are forced hidden.
    virtual void show();

  private slots:
    void tabClicked(int);

  protected:
    bool eventFilter(TQObject *obj, TQEvent *ev);

  private slots:
    void buttonPopupActivate (int id);

  private:
    MainWindow *m_mainWin;

    KMultiTabBar::KMultiTabBarPosition m_pos;
    Splitter *m_splitter;
    KMultiTabBar *m_tabBar;
    Splitter *m_ownSplit;

    TQIntDict<ToolView> m_idToWidget;
    TQMap<ToolView*, int> m_widgetToId;

    /**
     * list of all toolviews around in this sidebar
     */
    TQValueList<ToolView*> m_toolviews;

    int m_lastSize;

    int m_popupButton;
};

class MainWindow : public KParts::MainWindow
{
  Q_OBJECT
  

  friend class ToolView;

  //
  // Constructor area
  //
  public:
    /**
     * Constructor
     */
    MainWindow (TQWidget* parentWidget = 0, const char* name = 0);

    /**
     * Destructor
     */
    virtual ~MainWindow ();

  //
  // public interfaces
  //
  public:
    /**
     * central widget ;)
     * use this as parent for your content
     * this widget will get focus if a toolview is hidden
     * @return central widget
     */
    TQWidget *centralWidget () const;

    /**
     * add a given widget to the given sidebar if possible, name is very important
     * @param identifier unique identifier for this toolview
     * @param pos position for the toolview, if we are in session restore, this is only a preference
     * @param icon icon to use for the toolview
     * @param text text to use in addition to icon
     * @return created toolview on success or 0
     */
    ToolView *createToolView (const TQString &identifier, KMultiTabBar::KMultiTabBarPosition pos, const TQPixmap &icon, const TQString &text);

    /**
     * give you handle to toolview for the given name, 0 if no toolview around
     * @param identifier toolview name
     * @return toolview if existing, else 0
     */
    ToolView *toolView (const TQString &identifier) const;

    /**
     * set the toolview's tabbar style.
     * @param style the tabbar style.
     */
    void setToolViewStyle (KMultiTabBar::KMultiTabBarStyle style);

    /**
     * get the toolview's tabbar style. Call this before @p startRestore(),
     * otherwise you overwrite the usersettings.
     * @return toolview's tabbar style
     */
    KMultiTabBar::KMultiTabBarStyle toolViewStyle () const;

    /**
     * get the sidebars' visibility.
     * @return false, if the sidebars' visibility is forced hidden, otherwise true
     */
    bool sidebarsVisible() const;

  public slots:
    /**
     * set the sidebars' visibility to @p visible. If false, the sidebars
     * are @e always hidden. Usually you do not have to call this because
     * the user can set this in the menu.
     * @param visible sidebars visibility
     */
    void setSidebarsVisible( bool visible );

  protected:
    /**
     * called by toolview destructor
     * @param widget toolview which is destroyed
     */
    void toolViewDeleted (ToolView *widget);

  /**
   * modifiers for existing toolviews
   */
  public:
    /**
     * move a toolview around
     * @param widget toolview to move
     * @param pos position to move too, during session restore, only preference
     * @return success
     */
    bool moveToolView (ToolView *widget, KMultiTabBar::KMultiTabBarPosition pos);

    /**
     * show given toolview, discarded while session restore
     * @param widget toolview to show
     * @return success
     */
    bool showToolView (ToolView *widget);

    /**
     * hide given toolview, discarded while session restore
     * @param widget toolview to hide
     * @return success
     */
    bool hideToolView (ToolView *widget);

  /**
   * session saving and restore stuff
   */
  public:
    /**
     * start the restore
     * @param config config object to use
     * @param group config group to use
     */
    void startRestore (TDEConfig *config, const TQString &group);

    /**
     * finish the restore
     */
    void finishRestore ();

    /**
     * save the current session config to given object and group
     * @param config config object to use
     * @param group config group to use
     */
    void saveSession (TDEConfig *config, const TQString &group);

  /**
   * internal data ;)
   */
  private:
    /**
     * map identifiers to widgets
     */
    TQDict<ToolView> m_idToWidget;

    /**
     * list of all toolviews around
     */
    TQValueList<ToolView*> m_toolviews;

    /**
     * widget, which is the central part of the
     * main window ;)
     */
    TQWidget *m_centralWidget;

    /**
     * horizontal splitter
     */
    Splitter *m_hSplitter;

    /**
     * vertical splitter
     */
    Splitter *m_vSplitter;

    /**
     * sidebars for the four sides
     */
    Sidebar *m_sidebars[4];

    /**
     * sidebars state.
     */
    bool m_sidebarsVisible;

    /**
     * config object for session restore, only valid between
     * start and finish restore calls
     */
    TDEConfig *m_restoreConfig;

    /**
     * restore group
     */
    TQString m_restoreGroup;

    /**
     * out guiclient
     */
    GUIClient *m_guiClient;
};

}

#endif

// kate: space-indent on; indent-width 2;
