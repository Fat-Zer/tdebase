/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_WORKSPACE_H
#define KWIN_WORKSPACE_H

#include <tqtimer.h>
#include <tqvaluevector.h>
#include <tdeshortcut.h>
#include <tqcursor.h>
#include <netwm.h>
#include <kxmessages.h>

#include "KWinInterface.h"
#include "utils.h"
#include "kdecoration.h"
#include "sm.h"

#include <X11/Xlib.h>

class TQPopupMenu;
class TDEConfig;
class TDEGlobalAccel;
class TDEShortcutDialog;
class TDEStartupInfo;
class TDEStartupInfoId;
class TDEStartupInfoData;
class TQSlider;
class TQPushButton;
class TDEProcess;

namespace KWinInternal
{

class Client;
class TabBox;
class PopupInfo;
class RootInfo;
class PluginMgr;
class Placement;
class Rules;
class WindowRules;

class SystemTrayWindow
    {
    public:
        SystemTrayWindow()
            : win(0),winFor(0)
            {}
        SystemTrayWindow( WId w )
            : win(w),winFor(0)
            {}
        SystemTrayWindow( WId w, WId wf  )
            : win(w),winFor(wf)
            {}

        bool operator==( const SystemTrayWindow& other )
            { return win == other.win; }
        WId win;
        WId winFor;
    };

typedef TQValueList<SystemTrayWindow> SystemTrayWindowList;

class Workspace : public TQObject, public KWinInterface, public KDecorationDefines
    {
    Q_OBJECT
    public:
        Workspace( bool restore = FALSE );
        virtual ~Workspace();

        static Workspace * self() { return _self; }

        bool workspaceEvent( XEvent * );

        KDecoration* createDecoration( KDecorationBridge* bridge );

        bool hasClient( const Client * );

        template< typename T > Client* findClient( T predicate );
        template< typename T1, typename T2 > void forEachClient( T1 procedure, T2 predicate );
        template< typename T > void forEachClient( T procedure );

        TQRect clientArea( clientAreaOption, const TQPoint& p, int desktop ) const;
        TQRect clientArea( clientAreaOption, const Client* c ) const;
        TQRect clientArea( clientAreaOption, int screen, int desktop ) const;

        /**
         * @internal
         */
        void killWindowId( Window window );
        void suspendWindowId( Window window );
        void resumeWindowId( Window window );
        bool isResumeableWindowID( Window window );

        void killWindow() { slotKillWindow(); }
        void suspendWindow() { slotSuspendWindow(); }
        void resumeWindow() { slotResumeWindow(); }

        WId rootWin() const;

        bool initializing() const;

        /**
         * Returns the active client, i.e. the client that has the focus (or None
         * if no client has the focus)
         */
        Client* activeClient() const;
        // Client that was activated, but it's not yet really activeClient(), because
        // we didn't process yet the matching FocusIn event. Used mostly in focus
        // stealing prevention code.
        Client* mostRecentlyActivatedClient() const;

        void activateClient( Client*, bool force = FALSE  );
        void requestFocus( Client* c, bool force = FALSE );
        void takeActivity( Client* c, int flags, bool handled ); // flags are ActivityFlags
        void handleTakeActivity( Client* c, Time timestamp, int flags ); // flags are ActivityFlags
        bool allowClientActivation( const Client* c, Time time = -1U, bool focus_in = false );
        void restoreFocus();
        void gotFocusIn( const Client* );
        void setShouldGetFocus( Client* );
        bool fakeRequestedActivity( Client* c );
        void unfakeActivity( Client* c );
        bool activateNextClient( Client* c );
        bool focusChangeEnabled() { return block_focus == 0; }

        void updateColormap();

        /**
         * Indicates that the client c is being moved around by the user.
         */
        void setClientIsMoving( Client *c );

        void place( Client *c, TQRect& area );
        void placeSmart( Client* c, const TQRect& area );

        TQPoint adjustClientPosition( Client* c, TQPoint pos );
        TQRect adjustClientSize( Client* c, TQRect moveResizeGeom, int mode );
        void raiseClient( Client* c );
        void lowerClient( Client* c );
        void raiseClientRequest( Client* c, NET::RequestSource src, Time timestamp );
        void lowerClientRequest( Client* c, NET::RequestSource src, Time timestamp );
        void restackClientUnderActive( Client* );
        void updateClientLayer( Client* c );
        void raiseOrLowerClient( Client * );
        void reconfigure();

        void clientHidden( Client*  );
        void clientAttentionChanged( Client* c, bool set );

        void clientMoved(const TQPoint &pos, Time time);

        /**
         * Returns the current virtual desktop of this workspace
         */
        int currentDesktop() const;
        /**
         * Returns the number of virtual desktops of this workspace
         */
        int numberOfDesktops() const;
        void setNumberOfDesktops( int n );
        
        int activeScreen() const;
        int numScreens() const;
        void checkActiveScreen( const Client* c );
        void setActiveScreenMouse( TQPoint mousepos );
        TQRect screenGeometry( int screen ) const;
        int screenNumber( TQPoint pos ) const;

        TQWidget* desktopWidget();

    // for TabBox
        Client* nextFocusChainClient(Client*) const;
        Client* previousFocusChainClient(Client*) const;
        Client* nextStaticClient(Client*) const;
        Client* previousStaticClient(Client*) const;
        int nextDesktopFocusChain( int iDesktop ) const;
        int previousDesktopFocusChain( int iDesktop ) const;
        void closeTabBox();

         /**
         * Returns the list of clients sorted in stacking order, with topmost client
         * at the last position
         */
        const ClientList& stackingOrder() const;

        ClientList ensureStackingOrder( const ClientList& clients ) const;

        Client* topClientOnDesktop( int desktop, bool unconstrained = false, bool only_normal = true ) const;
        Client* findDesktop( bool topmost, int desktop ) const;
        void sendClientToDesktop( Client* c, int desktop, bool dont_activate );
        void windowToPreviousDesktop( Client* c );
        void windowToNextDesktop( Client* c );
        void sendClientToScreen( Client* c, int screen );

    // KDE4 remove me - and it's also in the DCOP interface :(
        void showWindowMenuAt( unsigned long id, int x, int y );
        void kDestopResized();

        /**
	 * Shows the menu operations menu for the client and makes it active if
	 * it's not already.
         */
        void showWindowMenu( const TQRect &pos, Client* cl );
	/**
	 * Backwards compatibility.
	 */
        void showWindowMenu( int x, int y, Client* cl );
        void showWindowMenu( TQPoint pos, Client* cl );

        void updateMinimizedOfTransients( Client* );
        void updateOnAllDesktopsOfTransients( Client* );
        void checkTransients( Window w );

        void performWindowOperation( Client* c, WindowOperation op );

        void storeSession( TDEConfig* config, SMSavePhase phase );

        SessionInfo* takeSessionInfo( Client* );
        WindowRules findWindowRules( const Client*, bool );
        void rulesUpdated();
        void discardUsedWindowRules( Client* c, bool withdraw );
        void disableRulesUpdates( bool disable );
        bool rulesUpdatesDisabled() const;

    // dcop interface
        void cascadeDesktop();
        void unclutterDesktop();
        void doNotManage(TQString);
        bool setCurrentDesktop( int new_desktop );
        void updateOverlappingShadows(WId window);
        void setShadowed(WId window, bool shadowed);
        void nextDesktop();
        void previousDesktop();
        void circulateDesktopApplications();
        void setCurrentScreen( int new_screen );

        TQString desktopName( int desk ) const;
        virtual void setDesktopLayout(int , int , int );
        void updateDesktopLayout();
        void setShowingDesktop( bool showing );
        void resetShowingDesktop( bool keep_hidden );
        bool showingDesktop() const;

        bool isNotManaged( const TQString& title );  // ### setter or getter ?

        void sendPingToWindow( Window w, Time timestamp ); // called from Client::pingWindow()
        void sendTakeActivity( Client* c, Time timestamp, long flags ); // called from Client::takeActivity()
        
        bool kompmgrIsRunning();
        void setOpacity(unsigned long winId, unsigned int opacityPercent);
        void setShadowSize(unsigned long winId, unsigned int shadowSizePercent);
        void setUnshadowed(unsigned long winId); // redundant, equals setShadowSize(inId, 0)

    // only called from Client::destroyClient() or Client::releaseWindow()
        void removeClient( Client*, allowed_t );
        void setActiveClient( Client*, allowed_t );
        Group* findGroup( Window leader ) const;
        void addGroup( Group* group, allowed_t );
        void removeGroup( Group* group, allowed_t );
        Group* findClientLeaderGroup( const Client* c ) const;

        bool checkStartupNotification( Window w, TDEStartupInfoId& id, TDEStartupInfoData& data );

        void focusToNull(); // SELI public?
        enum FocusChainChange { FocusChainMakeFirst, FocusChainMakeLast, FocusChainUpdate };
        void updateFocusChains( Client* c, FocusChainChange change );
        
        bool forcedGlobalMouseGrab() const;
        void clientShortcutUpdated( Client* c );
        bool shortcutAvailable( const TDEShortcut& cut, Client* ignore = NULL ) const;
        bool globalShortcutsDisabled() const;
        void disableGlobalShortcuts( bool disable );
        void disableGlobalShortcutsForClient( bool disable );

        void sessionSaveStarted();
        void sessionSaveDone();
        void setWasUserInteraction();
        bool wasUserInteraction() const;
        bool sessionSaving() const;

        bool managingTopMenus() const;
        int topMenuHeight() const;
        void updateCurrentTopMenu();

        int packPositionLeft( const Client* cl, int oldx, bool left_edge ) const;
        int packPositionRight( const Client* cl, int oldx, bool right_edge ) const;
        int packPositionUp( const Client* cl, int oldy, bool top_edge ) const;
        int packPositionDown( const Client* cl, int oldy, bool bottom_edge ) const;

        static TQStringList configModules(bool controlCenter);

        void cancelDelayFocus();
        void requestDelayFocus( Client* );
        void updateFocusMousePosition( const TQPoint& pos );
        TQPoint focusMousePosition() const;
        
        void toggleTopDockShadows(bool on);

    public slots:
        void refresh();
    // keybindings
        void slotSwitchDesktopNext();
        void slotSwitchDesktopPrevious();
        void slotSwitchDesktopRight();
        void slotSwitchDesktopLeft();
        void slotSwitchDesktopUp();
        void slotSwitchDesktopDown();

        void slotSwitchToDesktop( int );
    //void slotSwitchToWindow( int );
        void slotWindowToDesktop( int );
    //void slotWindowToListPosition( int );
        void slotSwitchToScreen( int );
        void slotWindowToScreen( int );
        void slotSwitchToNextScreen();
        void slotWindowToNextScreen();

        void slotWindowMaximize();
        void slotWindowMaximizeVertical();
        void slotWindowMaximizeHorizontal();
        void slotWindowMinimize();
        void slotWindowShade();
        void slotWindowRaise();
        void slotWindowLower();
        void slotWindowRaiseOrLower();
        void slotActivateAttentionWindow();
        void slotWindowPackLeft();
        void slotWindowPackRight();
        void slotWindowPackUp();
        void slotWindowPackDown();
        void slotWindowGrowHorizontal();
        void slotWindowGrowVertical();
        void slotWindowShrinkHorizontal();
        void slotWindowShrinkVertical();

        void slotWalkThroughDesktops();
        void slotWalkBackThroughDesktops();
        void slotWalkThroughApps();
        void slotWalkBackThroughApps();

        void slotWalkThroughDesktopList();
        void slotWalkBackThroughDesktopList();
        void slotWalkThroughWindows();
        void slotWalkBackThroughWindows();

        void slotWindowOperations();
        void slotWindowClose();
        void slotWindowMove();
        void slotWindowResize();
        void slotWindowAbove();
        void slotWindowBelow();
        void slotWindowOnAllDesktops();
        void slotWindowFullScreen();
        void slotWindowNoBorder();

        void slotWindowToNextDesktop();
        void slotWindowToPreviousDesktop();
        void slotWindowToDesktopRight();
        void slotWindowToDesktopLeft();
        void slotWindowToDesktopUp();
        void slotWindowToDesktopDown();

        void slotMouseEmulation();
        void slotDisableGlobalShortcuts();

        void slotSettingsChanged( int category );

        void slotReconfigure();

        void slotKillWindow();
        void slotSuspendWindow();
        void slotResumeWindow();

        void slotGrabWindow();
        void slotGrabDesktop();

        void slotSetupWindowShortcut();
        void setupWindowShortcutDone( bool );

        void updateClientArea();
        
        // kompmgr, also dcop
        void startKompmgr();

    private slots:
        void desktopPopupAboutToShow();
        void clientPopupAboutToShow();
        void slotSendToDesktop( int );
        void clientPopupActivated( int );
        void configureWM();
        void desktopResized();
        void slotUpdateToolWindows();
        void lostTopMenuSelection();
        void lostTopMenuOwner();
        void delayFocus();
        void gotTemporaryRulesMessage( const TQString& );
        void cleanupTemporaryRules();
        void writeWindowRules();
        void kipcMessage( int id, int data );
        // kompmgr
        void setPopupClientOpacity(int v);
        void resetClientOpacity();
        void setTransButtonText(int value);
        void unblockKompmgrRestart();
        void restartKompmgr( TDEProcess *proc );
        void handleKompmgrOutput( TDEProcess *proc, char *buffer, int buflen);
        void stopKompmgr();
        void kompmgrReloadSettings();
        // end

    protected:
        bool keyPressMouseEmulation( XKeyEvent& ev );

    private:
        void init();
        void initShortcuts();
        void readShortcuts();
        void initDesktopPopup();
        void setupWindowShortcut( Client* c );

        bool startKDEWalkThroughWindows();
        bool startWalkThroughDesktops( int mode ); // TabBox::Mode::DesktopMode | DesktopListMode
        bool startWalkThroughDesktops();
        bool startWalkThroughDesktopList();
        void KDEWalkThroughWindows( bool forward );
        void CDEWalkThroughWindows( bool forward );
        void walkThroughDesktops( bool forward );
        void KDEOneStepThroughWindows( bool forward );
        void oneStepThroughDesktops( bool forward, int mode ); // TabBox::Mode::DesktopMode | DesktopListMode
        void oneStepThroughDesktops( bool forward );
        void oneStepThroughDesktopList( bool forward );
        bool establishTabBoxGrab();
        void removeTabBoxGrab();
        int desktopToRight( int desktop ) const;
        int desktopToLeft( int desktop ) const;
        int desktopUp( int desktop ) const;
        int desktopDown( int desktop ) const;

        void updateStackingOrder( bool propagate_new_clients = false );
        void propagateClients( bool propagate_new_clients ); // called only from updateStackingOrder
        ClientList constrainedStackingOrder();
        void raiseClientWithinApplication( Client* c );
        void lowerClientWithinApplication( Client* c );
        bool allowFullClientRaising( const Client* c, Time timestamp );
        bool keepTransientAbove( const Client* mainwindow, const Client* transient );
        void blockStackingUpdates( bool block );
        void addTopMenu( Client* c );
        void removeTopMenu( Client* c );
        void setupTopMenuHandling();
        void updateTopMenuGeometry( Client* c = NULL );
        void updateToolWindows( bool also_hide );

    // this is the right way to create a new client
        Client* createClient( Window w, bool is_mapped );
        void addClient( Client* c, allowed_t );

        Window findSpecialEventWindow( XEvent* e );

        void randomPlacement(Client* c);
        void smartPlacement(Client* c);
        void cascadePlacement(Client* c, bool re_init = false);

        bool addSystemTrayWin( WId w );
        bool removeSystemTrayWin( WId w, bool check );
        void propagateSystemTrayWins();
        SystemTrayWindow findSystemTrayWin( WId w );

    // desktop names and number of desktops
        void loadDesktopSettings();
        void saveDesktopSettings();

    // mouse emulation
        WId getMouseEmulationWindow();
        enum MouseEmulation { EmuPress, EmuRelease, EmuMove };
        unsigned int sendFakedMouseEvent( TQPoint pos, WId win, MouseEmulation type, int button, unsigned int state ); // returns the new state

        void tabBoxKeyPress( const KKeyNative& keyX );
        void tabBoxKeyRelease( const XKeyEvent& ev );

    // electric borders
        void checkElectricBorders( bool force = false );
        void createBorderWindows();
        void destroyBorderWindows();
        bool electricBorder(XEvent * e);
        void raiseElectricBorders();

    // ------------------
    
        void helperDialog( const TQString& message, const Client* c );

        void calcDesktopLayout(int &x, int &y) const;

        TQPopupMenu* clientPopup();
        void closeActivePopup();

        void updateClientArea( bool force );

        SystemTrayWindowList systemTrayWins;

        int current_desktop;
        int number_of_desktops;
        TQMemArray<int> desktop_focus_chain;
        int active_screen;

        TQWidget* active_popup;
        Client* active_popup_client;

        TQWidget* desktop_widget;

        void loadSessionInfo();
        void loadWindowRules();
        void editWindowRules( Client* c, bool whole_app );

        TQPtrList<SessionInfo> session;
        TQValueList<Rules*> rules;
        KXMessages temporaryRulesMessages;
        TQTimer rulesUpdatedTimer;
        bool rules_updates_disabled;
        static const char* windowTypeToTxt( NET::WindowType type );
        static NET::WindowType txtToWindowType( const char* txt );
        static bool sessionInfoWindowTypeMatch( Client* c, SessionInfo* info );

        Client* active_client;
        Client* last_active_client;
        Client* next_active_client; // will be active after active_client deactivates
        Client* most_recently_raised; // used _only_ by raiseOrLowerClient()
        Client* movingClient;
        Client* pending_take_activity;

    // delay(ed) window focus timer and client
        TQTimer* delayFocusTimer;
        Client* delayfocus_client;
        TQPoint focusMousePos;

        ClientList clients;
        ClientList desktops;

        ClientList unconstrained_stacking_order; // topmost last
        ClientList stacking_order; // topmost last
        TQValueVector< ClientList > focus_chain; // currently active last
        ClientList global_focus_chain; // this one is only for things like tabbox's MRU
        ClientList should_get_focus; // last is most recent
        ClientList attention_chain;
        
        bool showing_desktop;
        ClientList showing_desktop_clients;
        int block_showing_desktop;

        GroupList groups;

        bool was_user_interaction;
        bool session_saving;
        int session_active_client;
        int session_desktop;

        bool control_grab;
        bool tab_grab;
    //KKeyNative walkThroughDesktopsKeycode, walkBackThroughDesktopsKeycode;
    //KKeyNative walkThroughDesktopListKeycode, walkBackThroughDesktopListKeycode;
    //KKeyNative walkThroughWindowsKeycode, walkBackThroughWindowsKeycode;
        TDEShortcut cutWalkThroughDesktops, cutWalkThroughDesktopsReverse;
        TDEShortcut cutWalkThroughDesktopList, cutWalkThroughDesktopListReverse;
        TDEShortcut cutWalkThroughWindows, cutWalkThroughWindowsReverse;
        TDEShortcut cutWalkThroughApps, cutWalkThroughAppsReverse;
        bool mouse_emulation;
        unsigned int mouse_emulation_state;
        WId mouse_emulation_window;
        int block_focus;

        TabBox* tab_box;
        PopupInfo* popupinfo;

        TQPopupMenu *popup;
        TQPopupMenu *advanced_popup;
        TQPopupMenu *desk_popup;
        int desk_popup_index;

        TDEGlobalAccel *keys;
        TDEGlobalAccel *client_keys;
        ShortcutDialog* client_keys_dialog;
        Client* client_keys_client;
        TDEGlobalAccel *disable_shortcuts_keys;
        bool global_shortcuts_disabled;
        bool global_shortcuts_disabled_for_client;

        WId root;

        PluginMgr *mgr;

        RootInfo *rootInfo;
        TQWidget* supportWindow;

    // swallowing
        TQStringList doNotManageList;

    // colormap handling
        Colormap default_colormap;
        Colormap installed_colormap;

    // Timer to collect requests for 'reconfigure'
        TQTimer reconfigureTimer;

        TQTimer updateToolWindowsTimer;

        static Workspace *_self;

        bool workspaceInit;

        TDEStartupInfo* startup;

        bool electric_have_borders;
        int electric_current_border;
        WId electric_top_border;
        WId electric_bottom_border;
        WId electric_left_border;
        WId electric_right_border;
        int electricLeft;
        int electricRight;
        int electricTop;
        int electricBottom;
        Time electric_time_first;
        Time electric_time_last;
        TQPoint electric_push_point;

        Qt::Orientation layoutOrientation;
        int layoutX;
        int layoutY;

        Placement *initPositioning;

        TQRect* workarea; //  array of workareas for virtual desktops
        TQRect** screenarea; // array of workareas per xinerama screen for all virtual desktops

        bool managing_topmenus;
        TDESelectionOwner* topmenu_selection;
        TDESelectionWatcher* topmenu_watcher;
        ClientList topmenus; // doesn't own them
        mutable int topmenu_height;
        TQWidget* topmenu_space;

        int set_active_client_recursion;
        int block_stacking_updates; // when >0, stacking updates are temporarily disabled
        bool blocked_propagating_new_clients; // propagate also new clients after enabling stacking updates?
        Window null_focus_window;
        bool forced_global_mouse_grab;
        friend class StackingUpdatesBlocker;
        friend class Client;
        
        //kompmgr
        TQSlider *transSlider;
        TQPushButton *transButton;
        // not used yet
        /*Client* topDock;
        int maximizedWindowCounter;
        int topDockShadowSize;*/
        //end
        
     signals:
        void kompmgrStarted();
        void kompmgrStopped();

    private:
        friend bool performTransiencyCheck();
    };

// helper for Workspace::blockStackingUpdates() being called in pairs (true/false)
class StackingUpdatesBlocker
    {
    public:
        StackingUpdatesBlocker( Workspace* w )
            : ws( w ) { ws->blockStackingUpdates( true ); }
        ~StackingUpdatesBlocker()
            { ws->blockStackingUpdates( false ); }
    private:
        Workspace* ws;
    };

// NET WM Protocol handler class
class RootInfo : public NETRootInfo4
    {
    private:
        typedef KWinInternal::Client Client;  // because of NET::Client
    public:
        RootInfo( Workspace* ws, Display *dpy, Window w, const char *name, unsigned long pr[], int pr_num, int scr= -1);
    protected:
        virtual void changeNumberOfDesktops(int n);
        virtual void changeCurrentDesktop(int d);
//    virtual void changeActiveWindow(Window w); the extended version is used
        virtual void changeActiveWindow(Window w,NET::RequestSource src, Time timestamp, Window active_window);
        virtual void closeWindow(Window w);
        virtual void moveResize(Window w, int x_root, int y_root, unsigned long direction);
        virtual void moveResizeWindow(Window w, int flags, int x, int y, int width, int height );
        virtual void gotPing(Window w, Time timestamp);
        virtual void restackWindow(Window w, RequestSource source, Window above, int detail, Time timestamp);
        virtual void gotTakeActivity(Window w, Time timestamp, long flags );
        virtual void changeShowingDesktop( bool showing );
    private:
        Workspace* workspace;
    };


inline WId Workspace::rootWin() const
    {
    return root;
    }

inline bool Workspace::initializing() const
    {
    return workspaceInit;
    }

inline Client* Workspace::activeClient() const
    {
    // next_active_client is a kludge for drop shadows. If a window that is
    // activated is not also raised (i.e. when focus follows mouse), then the
    // newly activated window and its shadow won't cover visual artifacts that
    // might exist in the inactive window's shadow. We work around this by
    // (re)drawing the inactive window's shadow after the active window's shadow
    // is drawn, but to do that the inactive window needs to know which window
    // will become active next. next_active_client is a Client pointer for that
    // purpose.
    return next_active_client != NULL ? next_active_client : active_client;
    }

inline Client* Workspace::mostRecentlyActivatedClient() const
    {
    return should_get_focus.count() > 0 ? should_get_focus.last() : active_client;
    }

inline int Workspace::currentDesktop() const
    {
    return current_desktop;
    }

inline int Workspace::numberOfDesktops() const
    {
    return number_of_desktops;
    }

inline void Workspace::addGroup( Group* group, allowed_t )
    {
    groups.append( group );
    }

inline void Workspace::removeGroup( Group* group, allowed_t )
    {
    groups.remove( group );
    }

inline const ClientList& Workspace::stackingOrder() const
    {
// TODO    Q_ASSERT( block_stacking_updates == 0 );
    return stacking_order;
    }

inline void Workspace::showWindowMenu(TQPoint pos, Client* cl)
    {
    showWindowMenu(TQRect(pos, pos), cl);
    }

inline void Workspace::showWindowMenu(int x, int y, Client* cl)
    {
    showWindowMenu(TQRect(TQPoint(x, y), TQPoint(x, y)), cl);
    }

inline
void Workspace::setWasUserInteraction()
    {
    was_user_interaction = true;
    }

inline
bool Workspace::wasUserInteraction() const
    {
    return was_user_interaction;
    }

inline
bool Workspace::managingTopMenus() const
    {
    return managing_topmenus;
    }

inline void Workspace::sessionSaveStarted()
    {
    session_saving = true;
    }

inline void Workspace::sessionSaveDone()
    {
    session_saving = false;
    }

inline bool Workspace::sessionSaving() const
    {
    return session_saving;
    }

inline bool Workspace::forcedGlobalMouseGrab() const
    {
    return forced_global_mouse_grab;
    }

inline bool Workspace::showingDesktop() const
    {
    return showing_desktop;
    }

inline bool Workspace::globalShortcutsDisabled() const
    {
    return global_shortcuts_disabled || global_shortcuts_disabled_for_client;
    }

inline
bool Workspace::rulesUpdatesDisabled() const
    {
    return rules_updates_disabled;
    }

inline
void Workspace::updateFocusMousePosition( const TQPoint& pos )
    {
    focusMousePos = pos;
    }

inline
TQPoint Workspace::focusMousePosition() const
    {
    return focusMousePos;
    }

template< typename T >
inline Client* Workspace::findClient( T predicate )
    {
    if( Client* ret = findClientInList( clients, predicate ))
        return ret;
    if( Client* ret = findClientInList( desktops, predicate ))
        return ret;
    return NULL;
    }

template< typename T1, typename T2 >
inline void Workspace::forEachClient( T1 procedure, T2 predicate )
    {
    for ( ClientList::ConstIterator it = clients.begin(); it != clients.end(); ++it)
        if ( predicate( const_cast< const Client* >( *it)))
            procedure( *it );
    for ( ClientList::ConstIterator it = desktops.begin(); it != desktops.end(); ++it)
        if ( predicate( const_cast< const Client* >( *it)))
            procedure( *it );
    }

template< typename T >
inline void Workspace::forEachClient( T procedure )
    {
    return forEachClient( procedure, TruePredicate());
    }

KWIN_COMPARE_PREDICATE( ClientMatchPredicate, const Client*, cl == value );
inline bool Workspace::hasClient( const Client* c )
    {
    return findClient( ClientMatchPredicate( c ));
    }

} // namespace

#endif
