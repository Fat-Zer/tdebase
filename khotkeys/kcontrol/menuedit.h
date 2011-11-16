/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _MENUEDIT_H_
#define _MENUEDIT_H_

#include <tqstring.h>
#include <kdialogbase.h>
#include <kaccel.h>
#include <kservice.h>

// see also tdebase/kmenuedit/khotkeys.h
extern "C"
    {
// initializes khotkeys DSO - loads i18n catalogue
// handled automatically by KHotKeys wrapper class in kmenuedit
KDE_EXPORT void khotkeys_init( void );
// clean up khotkeys DSO
// handled automatically by KHotKeys wrapper class in kmenuedit
KDE_EXPORT void khotkeys_cleanup( void );
// return keyboard shortcut ( e.g. "ALT+T" ) for given menu entry ( e.g.
// "System/Konsole.desktop"
KDE_EXPORT TQString khotkeys_get_menu_entry_shortcut( const TQString& entry_P );
// changes assigned shortcut to menu entry a updates config file
KDE_EXPORT TQString khotkeys_change_menu_entry_shortcut( const TQString& entry_P,
    const TQString& shortcut_P );
// menu entry was moved in K Menu
KDE_EXPORT bool khotkeys_menu_entry_moved( const TQString& new_P, const TQString& old_P );
// menu entry was removed
KDE_EXPORT void khotkeys_menu_entry_deleted( const TQString& entry_P );    
// List of all hotkeys in use
KDE_EXPORT TQStringList khotkeys_get_all_shortcuts( );
// Find menu entry that uses shortcut
KDE_EXPORT KService::Ptr khotkeys_find_menu_entry( const TQString& shortcut_P );
    } // extern "C"

#endif
