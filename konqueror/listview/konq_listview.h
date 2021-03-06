/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __konq_listview_h__
#define __konq_listview_h__

#include <tdeparts/browserextension.h>
#include <tdeglobalsettings.h>
#include <konq_operations.h>
#include <tdeparts/factory.h>
#include <konq_dirpart.h>
#include <kmimetyperesolver.h>

#include <tqvaluelist.h>
#include <tqlistview.h>
#include <tqstringlist.h>

#include <konq_propsview.h>
#include "konq_listviewwidget.h"

class TDEAction;
class TDEToggleAction;
class ListViewBrowserExtension;

class KonqListViewFactory : public KParts::Factory
{
public:
  KonqListViewFactory();
  virtual ~KonqListViewFactory();

  virtual KParts::Part* createPartObject( TQWidget *parentWidget, const char *, TQObject *parent, const char *name, const char*, const TQStringList &args );

  static TDEInstance *instance();
  static KonqPropsView *defaultViewProps();

private:
  static TDEInstance *s_instance;
  static KonqPropsView *s_defaultViewProps;
};

/**
 * The part for the tree view. It does quite nothing, just the
 * konqueror interface. Most of the functionality is in the
 * widget, KonqListViewWidget.
 */
class KonqListView : public KonqDirPart
{
  friend class KonqBaseListViewWidget;
  friend class ListViewBrowserExtension;
  Q_OBJECT
  TQ_PROPERTY( bool supportsUndo READ supportsUndo )
public:
  KonqListView( TQWidget *parentWidget, TQObject *parent, const char *name, const TQString& mode );
  virtual ~KonqListView();

  virtual const KFileItem * currentItem();
  virtual KFileItemList selectedFileItems() {return m_pListView->selectedFileItems();};

  KonqBaseListViewWidget *listViewWidget() const { return m_pListView; }

  bool supportsUndo() const { return true; }

  virtual void saveState( TQDataStream &stream );
  virtual void restoreState( TQDataStream &stream );

  // "Cut" icons : disable those whose URL is in lst, enable the others
  virtual void disableIcons( const KURL::List & lst );

  // See KMimeTypeResolver
  void mimeTypeDeterminationFinished() {}
  //int iconSize() { return m_pListView->iconSize(); }
  void determineIcon( KonqBaseListViewItem * item );

  TQPtrList<KonqBaseListViewItem> & lstPendingMimeIconItems() { return m_mimeTypeResolver->m_lstPendingMimeIconItems; }
  void listingComplete();

  virtual void newIconSize( int );

protected:
  virtual bool doOpenURL( const KURL &url );
  virtual bool doCloseURL();
  virtual bool openFile() { return true; }

  void setupActions();
  void guiActivateEvent( KParts::GUIActivateEvent *event );

protected slots:
  void slotSelect();
  void slotUnselect();
  void slotSelectAll();
  void slotUnselectAll();
  void slotInvertSelection();
  void slotCaseInsensitive();
  void slotSelectionChanged();

  void slotShowDot();
  //this is called if a item in the submenu is toggled
  //it saves the new configuration according to the menu items
  //and calls createColumns()
  //it adjusts the indece of the remaining columns
  void slotColumnToggled();
  //this is called when the user changes the order of the
  //columns by dragging them
  //at this moment the columns haven't changed their order yet, so
  //it starts a singleshottimer, after which the columns changed their order
  //and then slotSaveAfterHeaderDrag is called
  void headerDragged(int sec, int from, int to);
  //saves the new order of the columns
  void slotSaveAfterHeaderDrag();
  // column width changed
  void slotHeaderSizeChanged();
  void slotSaveColumnWidths();  // delayed
  void slotHeaderClicked(int sec);

  // This comes from KonqDirPart, it's for the "Find" feature
  virtual void slotStarted() { m_pListView->slotStarted(); }
  virtual void slotCanceled() { m_pListView->slotCanceled(); }
  virtual void slotCompleted() { m_pListView->slotCompleted(); }
  virtual void slotNewItems( const KFileItemList& lst ) { m_pListView->slotNewItems( lst ); }
  virtual void slotDeleteItem( KFileItem * item ) { m_pListView->slotDeleteItem( item ); }
  virtual void slotRefreshItems( const KFileItemList& lst ) { m_pListView->slotRefreshItems( lst ); }
  virtual void slotClear() { m_pListView->slotClear(); }
  virtual void slotRedirection( const KURL & u ) { m_pListView->slotRedirection( u ); }

  // Connected to KonqDirPart
  void slotKFindOpened();
  void slotKFindClosed();

private:

  KonqBaseListViewWidget *m_pListView;
  KMimeTypeResolver<KonqBaseListViewItem,KonqListView> *m_mimeTypeResolver;
  TQTimer *m_headerTimer;

  TDEAction *m_paSelect;
  TDEAction *m_paUnselect;
  TDEAction *m_paSelectAll;
  TDEAction *m_paUnselectAll;
  TDEAction *m_paInvertSelection;

  // These 2 actions are 'fake' actions. They are defined so that the keyboard shortcuts
  // can be set from the 'Configure Shortcuts..." dialog.
  // The real actions are performed in the TDEListViewLineEdit::keyPressEvent() in tdeui
  TDEAction *m_paRenameMoveNext;
  TDEAction *m_paRenameMovePrev;

  TDEToggleAction *m_paCaseInsensitive;

  TDEToggleAction *m_paShowDot;
  TDEToggleAction *m_paShowTime;
  TDEToggleAction *m_paShowType;
  TDEToggleAction *m_paShowMimeType;
  TDEToggleAction *m_paShowAccessTime;
  TDEToggleAction *m_paShowCreateTime;
  TDEToggleAction *m_paShowLinkDest;
  TDEToggleAction *m_paShowSize;
  TDEToggleAction *m_paShowOwner;
  TDEToggleAction *m_paShowGroup;
  TDEToggleAction *m_paShowPermissions;
  TDEToggleAction *m_paShowURL;
};

class ListViewBrowserExtension : public KonqDirPartBrowserExtension
{
   Q_OBJECT
   friend class KonqListView;
   friend class KonqBaseListViewWidget;
   public:
      ListViewBrowserExtension( KonqListView *listView );

      virtual int xOffset();
      virtual int yOffset();

   protected slots:
      void updateActions();

      void copy() { copySelection( false ); }
      void cut() { copySelection( true ); }
      void paste();
      void pasteTo( const KURL & );
      void rename();
      void trash();
      void del() { KonqOperations::del(m_listView->listViewWidget(),
                                       KonqOperations::DEL,
                                       m_listView->listViewWidget()->selectedUrls()); }

      void reparseConfiguration();
      void setSaveViewPropertiesLocally( bool value );
      void setNameFilter( const TQString &nameFilter );
      // void refreshMimeTypes is missing

      void properties();
      void editMimeType();

   private:
      void copySelection( bool move );

      KonqListView *m_listView;
};

#endif
