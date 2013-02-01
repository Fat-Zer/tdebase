/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _INPUT_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "input.h"

#include <assert.h>
#include <tqwidget.h>

#include <kglobalaccel.h>
#include <kdebug.h>
#include <kapplication.h>
#include <tdeversion.h>
#include <tqtimer.h>
#include <kkeynative.h>

#include "khotkeysglobal.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "windows.h"

namespace KHotKeys
{

// Kbd

Kbd::Kbd( bool grabbing_enabled_P, TQObject* parent_P )
    : TQObject( parent_P )
    {
    assert( keyboard_handler == NULL );
    keyboard_handler = this;
    kga = new TDEGlobalAccel( NULL );
    kga->setEnabled( grabbing_enabled_P );
    }
    
Kbd::~Kbd()
    {
    keyboard_handler = NULL;
    delete kga;
    }
    
void Kbd::insert_item( const TDEShortcut& shortcut_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.shortcuts.append( shortcut_P );
    if( rcv.active )
        grab_shortcut( shortcut_P );
    }

void Kbd::remove_item( const TDEShortcut& shortcut_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.shortcuts.remove( shortcut_P );
    if( rcv.active )
        ungrab_shortcut( shortcut_P );
    if( rcv.shortcuts.count() == 0 )
        receivers.remove( receiver_P );
    }
    
void Kbd::activate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( rcv.active )
        return;
    rcv.active = true;
    for( TQValueList< TDEShortcut >::ConstIterator it( rcv.shortcuts.begin());
         it != rcv.shortcuts.end();
         ++it )
        grab_shortcut( *it );
    }

void Kbd::deactivate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( !rcv.active )
        return;
    rcv.active = false;
    for( TQValueList< TDEShortcut >::ConstIterator it( rcv.shortcuts.begin());
         it != rcv.shortcuts.end();
         ++it )
        ungrab_shortcut( *it );
    }

void Kbd::grab_shortcut( const TDEShortcut& shortcut_P )
    {
    if( grabs.contains( shortcut_P ))
        ++grabs[ shortcut_P ];
    else
        {
        grabs[ shortcut_P ] = 1;
#if 0
        // CHECKME ugly ugly hack
        TQString name = ' ' + TQString::number( keycode_P );
        kga->insertItem( "", name, keycode_P );
        kga->connectItem( name, this, TQT_SLOT( key_slot( int )));
#endif
        TQString name = ' ' + shortcut_P.toStringInternal();
        kga->insert( name, name, TQString::null, shortcut_P, shortcut_P,
            this, TQT_SLOT( key_slot( TQString )));
        TQTimer::singleShot( 0, this, TQT_SLOT( update_connections()));
        }
    }
    
void Kbd::ungrab_shortcut( const TDEShortcut& shortcut_P )
    {
    if( !grabs.contains( shortcut_P ))
        return;
    if( --grabs[ shortcut_P ] == 0 )
        {
#if 0
        // CHECKME workaround for TDEGlobalAccel::disconnectItem() not working
        kga->setItemEnabled( ' ' + TQString::number( keycode_P ), false );
        // kga->disconnectItem( ' ' + TQString::number( keycode_P ), NULL, NULL );
        kga->removeItem( ' ' + TQString::number( keycode_P ));
#endif
        kga->remove( ' ' + shortcut_P.toStringInternal());
        grabs.remove( shortcut_P );
        TQTimer::singleShot( 0, this, TQT_SLOT( update_connections()));
        }
    }

void Kbd::update_connections()
    {
    kga->updateConnections();
    }
    
void Kbd::key_slot( TQString key_P )
    {
    kdDebug( 1217 ) << "Key pressed:" << key_P << endl;
    TDEShortcut shortcut( key_P );
    if( !grabs.contains( shortcut ))
        return;
    for( TQMap< Kbd_receiver*, Receiver_data >::ConstIterator it = receivers.begin();
         it != receivers.end();
         ++it )
        if( ( *it ).shortcuts.contains( shortcut ) && ( *it ).active
            && it.key()->handle_key( shortcut ))
            return;
    }
    

#ifdef HAVE_XTEST

} // namespace KHotKeys
#include <X11/extensions/XTest.h>
namespace KHotKeys
{

static bool xtest_available = false;
static bool xtest_inited = false;
static bool xtest()
    {
    if( xtest_inited )
        return xtest_available;
    xtest_inited = true;
    int dummy1, dummy2, dummy3, dummy4;
    xtest_available =
        ( XTestQueryExtension( tqt_xdisplay(), &dummy1, &dummy2, &dummy3, &dummy4 ) == True );
    return xtest_available;
    }
#endif

// CHECKME nevola XFlush(), musi se pak volat rucne
bool Kbd::send_macro_key( const KKey& key, Window window_P )
    {
    unsigned int keysym = KKeyNative( key ).sym();
    KeyCode x_keycode = XKeysymToKeycode( tqt_xdisplay(), keysym );
    if( x_keycode == NoSymbol )
	return false;
    unsigned int x_mod = KKeyNative( key ).mod();
#ifdef HAVE_XTEST
    if( xtest() && window_P == None )
        {
        // CHECKME tohle jeste potrebuje modifikatory
        bool ret = XTestFakeKeyEvent( tqt_xdisplay(), x_keycode, True, CurrentTime );
        ret = ret && XTestFakeKeyEvent( tqt_xdisplay(), x_keycode, False, CurrentTime );
        return ret;
        }
#endif
    if( window_P == None || window_P == InputFocus )
        window_P = windows_handler->active_window();
    if( window_P == None ) // CHECKME tohle cele je ponekud ...
        window_P = InputFocus;
    XEvent ev;
    ev.type = KeyPress;
    ev.xkey.display = tqt_xdisplay();
    ev.xkey.window = window_P;
    ev.xkey.root = tqt_xrootwin();   // I don't know whether these have to be set
    ev.xkey.subwindow = None;       // to these values, but it seems to work, hmm
    ev.xkey.time = CurrentTime;
    ev.xkey.x = 0;
    ev.xkey.y = 0;
    ev.xkey.x_root = 0;
    ev.xkey.y_root = 0;
    ev.xkey.keycode = x_keycode;
    ev.xkey.state = x_mod;
    ev.xkey.same_screen = True;
    bool ret = XSendEvent( tqt_xdisplay(), window_P, True, KeyPressMask, &ev );
#if 1
    ev.type = KeyRelease;  // is this actually really needed ??
    ev.xkey.display = tqt_xdisplay();
    ev.xkey.window = window_P;
    ev.xkey.root = tqt_xrootwin();
    ev.xkey.subwindow = None;
    ev.xkey.time = CurrentTime;
    ev.xkey.x = 0;
    ev.xkey.y = 0;
    ev.xkey.x_root = 0;
    ev.xkey.y_root = 0;
    ev.xkey.state = x_mod;
    ev.xkey.keycode = x_keycode;
    ev.xkey.same_screen = True;
    ret = ret && XSendEvent( tqt_xdisplay(), window_P, True, KeyReleaseMask, &ev );
#endif
    // Qt's autorepeat compression is broken and can create "aab" from "aba"
    // XSync() should create delay longer than Qt's max autorepeat interval
    XSync( tqt_xdisplay(), False );
    return ret;
    }

bool Mouse::send_mouse_button( int button_P, bool release_P )
    {
#ifdef HAVE_XTEST
    if( xtest())
        {
        // CHECKME tohle jeste potrebuje modifikatory
        // a asi i spravnou timestamp misto CurrentTime
        bool ret = XTestFakeButtonEvent( tqt_xdisplay(), button_P, True, CurrentTime );
        if( release_P )
            ret = ret && XTestFakeButtonEvent( tqt_xdisplay(), button_P, False, CurrentTime );
        return ret;
        }
#endif
    return false;
    }
    
} // namespace KHotKeys

#include "input.moc"
