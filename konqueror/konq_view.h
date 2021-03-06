/* -*- c-basic-offset: 2 -*-
 *  This file is part of the KDE project
 *  Copyright (C) 1998-2005 David Faure <faure@kde.org>
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
 **/

#ifndef __konq_view_h__
#define __konq_view_h__

#include "konq_mainwindow.h"
#include "konq_factory.h"

#include <tqptrlist.h>
#include <tqstring.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>
#include <tqcstring.h>

#include <ktrader.h>

class KonqRun;
class KonqFrame;
class KonqViewIface;
class KonqBrowserInterface;
namespace KParts
{
  class BrowserExtension;
  class StatusBarExtension;
}

struct HistoryEntry
{
  KURL url;
  TQString locationBarURL; // can be different from url when showing a index.html
  TQString title;
  TQByteArray buffer;
  TQString strServiceType;
  TQString strServiceName;
  TQByteArray postData;
  TQString postContentType;
  bool doPost;
  TQString pageReferrer;
  KonqMainWindow::PageSecurity pageSecurity;
};

/* This class represents a child of the main view. The main view maintains
 * the list of children. A KonqView contains a Browser::View and
 * handles it. It's more or less the backend structure for the views.
 * The widget handling stuff is done by the KonqFrame.
 */
class KonqView : public TQObject
{
  Q_OBJECT
public:

  /**
   * Create a konqueror view
   * @param viewFactory the factory to be used to create the part
   * @param viewFrame the frame where to create the view
   * @param mainWindow is the main window :-)
   * @param service the service implementing the part
   * @param partServiceOffers list of part offers found by the factory
   * @param appServiceOffers list of app offers found by the factory
   * @param serviceType the serviceType implemented by the part
   * @param passiveMode whether to initially make the view passive
   */
  KonqView( KonqViewFactory &viewFactory,
            KonqFrame* viewFrame,
            KonqMainWindow * mainWindow,
            const KService::Ptr &service,
            const TDETrader::OfferList &partServiceOffers,
            const TDETrader::OfferList &appServiceOffers,
            const TQString &serviceType,
            bool passiveMode);

  ~KonqView();

  /**
   * Displays another URL, but without changing the view mode (caller has to
   * ensure that the call makes sense)
   * @param url the URL to open
   * @param locationBarURL the URL to set in the location bar (see @ref setLocationBarURL)
   * @param nameFilter e.g. *.cpp
   * @param tempFile whether to delete the file after use
   */
  void openURL( const KURL &url,
                const TQString & locationBarURL,
                const TQString &nameFilter = TQString::null,
                bool tempFile = false );

  /**
   * Change the type of view (i.e. loads a new konqueror view)
   * Contract: the caller should call stop() first,
   *
   * @param serviceType the service type we want to show
   * @param serviceName allows to enforce a particular service to be chosen,
   *        @see KonqFactory.
   */
  bool changeViewMode( const TQString &serviceType,
                       const TQString &serviceName = TQString::null,
                       bool forceAutoEmbed = false );

  /**
   * Call this to prevent next openURL() call from changing history lists
   * Used when the same URL is reloaded (for instance with another view mode)
   *
   * Calling with lock=false is a hack reserved to the "find" feature.
   */
  void lockHistory( bool lock = true ) { m_bLockHistory = lock; }

  /**
   * @return true if view can go back
   */
  bool canGoBack()const { return m_lstHistory.at() > 0; }

  /**
   * @return true if view can go forward
   */
  bool canGoForward()const { return m_lstHistory.at() != ((int)m_lstHistory.count())-1; }

  /**
   * @return the position in the history
   */
  int historyPos() const { return m_lstHistory.at(); }

  uint historyLength() { return m_lstHistory.count(); }

  /**
   * Move in history. +1 is "forward", -1 is "back", you can guess the rest.
   */
  void go( int steps );

  /**
   * Helper function for go() and KonqViewManager
   */
  void restoreHistory();

  void setHistoryPos(int newPos) { m_lstHistory.at( newPos ); }

  /**
   * @return the history of this view
   */
  const TQPtrList<HistoryEntry> & history() { return m_lstHistory; }

  /**
   * @return the HistoryEntry at postion @p pos
   */
  const HistoryEntry* historyAt(const int pos);

  /**
   * Creates a deep copy of the @p other view's history buffers.
   */
  void copyHistory( KonqView *other );

  /**
   * Set the KonqRun instance that is running something for this view
   * The main window uses this to store the KonqRun for each child view.
   */
  void setRun( KonqRun * run  );

  KonqRun *run() const { return m_pRun; }

  /**
   * Stop loading
   */
  void stop();

  /**
   * Retrieve view's URL
   */
  KURL url() const;

  KURL upURL() const;

  /**
   * Get view's location bar URL, i.e. the one that the view signals
   * It can be different from url(), for instance if we display a index.html
   */
  TQString locationBarURL() const { return m_sLocationBarURL; }

  /**
   * Get the URL that was typed to get the current URL.
   */
  TQString typedURL() const { return m_sTypedURL; }
  /**
   * Set the URL that was typed to get the current URL.
   */
  void setTypedURL( const TQString & u ) { m_sTypedURL = u; }

  /**
   * Return the security state of page in view
   */
  KonqMainWindow::PageSecurity pageSecurity() const { return m_pageSecurity; }

  /**
   * @return the part embedded into this view
   */
  KParts::ReadOnlyPart *part() const { return m_pPart; }

  /**
   * see KonqViewManager::removePart
   */
  void partDeleted() { m_pPart = 0L; }

  KParts::BrowserExtension *browserExtension() const;

  KParts::StatusBarExtension *statusBarExtension() const;

  /**
   * @return a pointer to the KonqFrame which the view lives in
   */
  KonqFrame* frame() const { return m_pKonqFrame; }

  /**
   * @return the servicetype this view is currently displaying
   */
  TQString serviceType() const { return m_serviceType; }

  /**
   * @return the servicetypes this view is capable to display
   */
  TQStringList serviceTypes() const { return m_service->serviceTypes(); }

  bool supportsServiceType( const TQString &serviceType ) const;

  // True if "Use index.html" is set (->the view doesn't necessarily show HTML!)
  bool allowHTML() const { return m_bAllowHTML; }
  void setAllowHTML( bool allow ) { m_bAllowHTML = allow; }

  // True if currently loading
  bool isLoading() const { return m_bLoading; }
  void setLoading( bool loading, bool hasPending = false );

  // True if "locked to current location" (and their view mode, in fact)
  bool isLockedLocation() const { return m_bLockedLocation; }
  void setLockedLocation( bool b );

  // True if can't be made active (e.g. dirtree).
  bool isPassiveMode() const { return m_bPassiveMode; }
  void setPassiveMode( bool mode );

  // True if is hierarchical view
  bool isHierarchicalView() const { return m_bHierarchicalView; }
  void setHierarchicalView( bool mode );

  // True if 'link' symbol set
  bool isLinkedView() const { return m_bLinkedView; }
  void setLinkedView( bool mode );

  // True if toggle view
  void setToggleView( bool b ) { m_bToggleView = b; }
  bool isToggleView() const { return m_bToggleView; }

  // True if it always follows the active view
  void setFollowActive(bool b) { m_bFollowActive = b; }
  bool isFollowActive() { return m_bFollowActive; }

  // True if locked to current view mode
  // Toggle views and passive views are locked to their view mode.
  bool isLockedViewMode() const { return m_bToggleView || m_bPassiveMode; }

  // True if "builtin" (see X-TDE-BrowserView-Built-Into)
  bool isBuiltinView() const { return m_bBuiltinView; }

  void setService( const KService::Ptr &s ) { m_service = s; }
  KService::Ptr service() { return m_service; }

  TQString caption() const { return m_caption; }

  TDETrader::OfferList partServiceOffers() { return m_partServiceOffers; }
  TDETrader::OfferList appServiceOffers() { return m_appServiceOffers; }

  KonqMainWindow *mainWindow() const { return m_pMainWindow; }

  // return true if the method was found, false if the execution failed
  bool callExtensionMethod( const char *methodName );
  bool callExtensionBoolMethod( const char *methodName, bool value );
  bool callExtensionStringMethod( const char *methodName, TQString value );
  bool callExtensionURLMethod( const char *methodName, const KURL& value );

  void setViewName( const TQString &name );
  TQString viewName() const;

  // True to enable the context popup menu
  void enablePopupMenu( bool b );
  bool isPopupMenuEnabled() const { return m_bPopupMenuEnabled; }

  void reparseConfiguration();

  void disableScrolling();

  TQStringList frameNames() const;

  KonqViewIface * dcopObject();

  void goHistory( int steps );

  // Set the TDEGlobal active instance (the one used by KBugReport)
  void setActiveInstance();

  // Called before reloading this view. Sets args.reload to true, and offers to repost form data.
  // Returns false in case the reload must be cancelled.
  bool prepareReload( KParts::URLArgs& args );

  // overload for the TQString version
  void setLocationBarURL( const KURL& locationBarURL );

  static TQStringList childFrameNames( KParts::ReadOnlyPart *part );

  static KParts::BrowserHostExtension *hostExtension( KParts::ReadOnlyPart *part, const TQString &name );

signals:

  /**
   * Signal the main window that the embedded part changed (e.g. because of changeViewMode)
   */
  void sigPartChanged( KonqView *childView, KParts::ReadOnlyPart *oldPart, KParts::ReadOnlyPart *newPart );

  /**
   * Emitted in slotCompleted
   */
  void viewCompleted( KonqView * view );

  /**
   * Emitted only if the option backRightClick is activated
   */
  void backRightClick();

public slots:
  /**
   * Store location-bar URL in the child view
   * and updates the main view if this view is the current one
   * May be different from url e.g. if using "allowHTML".
   */
  void setLocationBarURL( const TQString & locationBarURL );
  /**
   * get an icon for the URL from the BrowserExtension
   */
  void setIconURL( const KURL &iconURL );

  void setTabIcon( const KURL &url );

  void setCaption( const TQString & caption );

  void setPageSecurity( int );

  // connected to the KROP's TDEIO::Job
  // but also to KonqRun's job
  void slotInfoMessage( TDEIO::Job *, const TQString &msg );

protected slots:
  // connected to the KROP's TDEIO::Job
  void slotStarted( TDEIO::Job * job );
  void slotCompleted();
  void slotCompleted( bool );
  void slotCanceled( const TQString & errMsg );
  void slotPercent( TDEIO::Job *, unsigned long percent );
  void slotSpeed( TDEIO::Job *, unsigned long bytesPerSecond );

  /**
   * Connected to the BrowserExtension
   */
  void slotSelectionInfo( const KFileItemList &items );
  void slotMouseOverInfo( const KFileItem* item );
  void slotOpenURLNotify();
  void slotEnableAction( const char * name, bool enabled );
  void slotSetActionText( const char* name, const TQString& text );
  void slotMoveTopLevelWidget( int x, int y );
  void slotResizeTopLevelWidget( int w, int h );
  void slotRequestFocus(KParts::ReadOnlyPart*);

private:
  void enableBackRightClick( bool b );
  /**
   * Replace the current view with a new view, created by @p viewFactory.
   */
  void switchView( KonqViewFactory &viewFactory );

  /**
   * Connects the internal part to the main window.
   * Do this after creating it and before inserting it.
   */
  void connectPart();

  /**
   * Creates a new entry in the history.
   */
  void createHistoryEntry();

  /**
   * Updates the current entry in the history.
   * @param saveLocationBarURL whether to save the location bar URL as part of it
   * (not done in openURL, to be able to revert if aborting)
   */
  void updateHistoryEntry(bool saveLocationBarURL);

  void aboutToOpenURL( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  void setServiceTypeInExtension();

  void finishedWithCurrentURL();

  virtual bool eventFilter( TQObject *obj, TQEvent *e );

////////////////// private members ///////////////

  KParts::ReadOnlyPart *m_pPart;

  TQString m_sLocationBarURL;
  TQString m_sTypedURL;
  KonqMainWindow::PageSecurity m_pageSecurity;

  /**
   * The full history (back + current + forward)
   * The current position in the history is m_lstHistory.current()
   */
  TQPtrList<HistoryEntry> m_lstHistory;

  /**
   * The post data that _resulted_ in this page.
   * e.g. when submitting a form, and the result is an image, this data will be
   * set (and saved/restored) when the image is being viewed. Necessary for reload.
   */
  TQByteArray m_postData;
  TQString m_postContentType;
  bool m_doPost;

  /**
   * The referrer that was used to obtain this page.
   */
  TQString m_pageReferrer;

  KonqMainWindow *m_pMainWindow;
  KonqRun *m_pRun;
  KonqFrame *m_pKonqFrame;

  uint m_bAllowHTML:1;
  uint m_bLoading:1;
  uint m_bLockedLocation:1;
  uint m_bPassiveMode:1;
  uint m_bLinkedView:1;
  uint m_bToggleView:1;
  uint m_bLockHistory:1;
  uint m_bAborted:1;
  uint m_bGotIconURL:1;
  uint m_bPopupMenuEnabled:1;
  uint m_bFollowActive:1;
  uint m_bPendingRedirection:1;
  uint m_bBuiltinView:1;
  uint m_bURLDropHandling:1;
  uint m_bBackRightClick:1;
  uint m_bHierarchicalView:1;
  uint m_bDisableScrolling:1;
  TDETrader::OfferList m_partServiceOffers;
  TDETrader::OfferList m_appServiceOffers;
  KService::Ptr m_service;
  TQString m_serviceType;
  TQString m_caption;
  TQString m_tempFile;
  KonqViewIface * m_dcopObject;
  KonqBrowserInterface *m_browserIface;
  int m_randID;
};

#endif
