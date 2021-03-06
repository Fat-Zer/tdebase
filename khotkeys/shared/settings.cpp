/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _SETTINGS_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "settings.h"

#include <tdeconfig.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdeaccel.h>
#include <tdeglobal.h>
#include <tdemessagebox.h>

#include "triggers.h"
#include "conditions.h"
#include "action_data.h"

namespace KHotKeys
{

// Settings

Settings::Settings()
    : actions( NULL ), gestures_exclude( NULL )
    {
    }
    
bool Settings::read_settings( bool include_disabled_P )
    {
    TDEConfig cfg( KHOTKEYS_CONFIG_FILE, true );
    return read_settings( cfg, include_disabled_P, ImportNone );
    }

bool Settings::import( TDEConfig& cfg_P, bool ask_P )
    {
    return read_settings( cfg_P, true, ask_P ? ImportAsk : ImportSilent );
    }

bool Settings::read_settings( TDEConfig& cfg_P, bool include_disabled_P, ImportType import_P )
    {
    if( actions == NULL )
        actions = new Action_data_group( NULL, "should never see", "should never see",
            NULL, Action_data_group::SYSTEM_ROOT, true );
    if( cfg_P.groupList().count() == 0 ) // empty
        return false;
    cfg_P.setGroup( "Main" ); // main group
    if( import_P == ImportNone ) // reading main cfg file
        already_imported = cfg_P.readListEntry( "AlreadyImported" );
    else
        {
        TQString import_id = cfg_P.readEntry( "ImportId" );
        if( !import_id.isEmpty())
            {
            if( already_imported.contains( import_id ))
                {
                if( import_P == ImportSilent
                    || KMessageBox::warningContinueCancel( NULL,
                        i18n( "This \"actions\" file has already been imported before. "
                              "Are you sure you want to import it again?" )) != KMessageBox::Continue )
                    return true; // import "successful"
                }
            else
                already_imported.append( import_id );
            }
        else
            {
            if( import_P != ImportSilent
                && KMessageBox::warningContinueCancel( NULL,
                    i18n( "This \"actions\" file has no ImportId field and therefore it cannot be determined "
                          "whether or not it has been imported already. Are you sure you want to import it?" ))
                    == KMessageBox::Cancel )
                return true;
            }
        }
    int version = cfg_P.readNumEntry( "Version", -1234576 );
    switch( version )
        {
        case 1:
            read_settings_v1( cfg_P );
          break;
        case 2:
            read_settings_v2( cfg_P, include_disabled_P );
          break;
        default:
            kdWarning( 1217 ) << "Unknown cfg. file version\n";
          return false;
        case -1234576: // no config file
            if( import_P ) // if importing, this is an error
                return false;
          break;
        }
    if( import_P != ImportNone )
        return true; // don't read global settings
    cfg_P.setGroup( "Main" ); // main group
    daemon_disabled = cfg_P.readBoolEntry( "Disabled", false );
    cfg_P.setGroup( "Gestures" );
    gestures_disabled_globally = cfg_P.readBoolEntry( "Disabled", true );
    gesture_mouse_button = cfg_P.readNumEntry( "MouseButton", 2 );
    gesture_mouse_button = KCLAMP( gesture_mouse_button, 2, 9 );
    gesture_timeout = cfg_P.readNumEntry( "Timeout", 300 );
    cfg_P.setGroup( "GesturesExclude" );
    delete gestures_exclude;
    gestures_exclude = new Windowdef_list( cfg_P );
	cfg_P.setGroup( "Voice" );
	voice_shortcut=TDEShortcut( cfg_P.readEntry("Shortcut" , "")  );
    return true;
    }

void Settings::write_settings()
    {
    TDEConfig cfg( KHOTKEYS_CONFIG_FILE, false );
// CHECKME    smazat stare sekce ?
    TQStringList groups = cfg.groupList();
    for( TQStringList::ConstIterator it = groups.begin();
         it != groups.end();
         ++it )
        cfg.deleteGroup( *it );
    cfg.setGroup( "Main" ); // main group
    cfg.writeEntry( "Version", 2 ); // now it's version 2 cfg. file
    cfg.writeEntry( "AlreadyImported", already_imported );
    cfg.setGroup( "Data" );
    int cnt = write_actions_recursively_v2( cfg, actions, true );
    cfg.setGroup( "Main" );
    cfg.writeEntry( "Autostart", cnt != 0 && !daemon_disabled );
    cfg.writeEntry( "Disabled", daemon_disabled );
    cfg.setGroup( "Gestures" );
    cfg.writeEntry( "Disabled", gestures_disabled_globally );
    cfg.writeEntry( "MouseButton", gesture_mouse_button );
    cfg.writeEntry( "Timeout", gesture_timeout );
    if( gestures_exclude != NULL )
        {
        cfg.setGroup( "GesturesExclude" );
        gestures_exclude->cfg_write( cfg );
        }
    else
        cfg.deleteGroup( "GesturesExclude" );
	cfg.setGroup( "Voice" );
	cfg.writeEntry("Shortcut" , voice_shortcut.toStringInternal() );

    }


// return value means the number of enabled actions written in the cfg file
// i.e. 'Autostart' for value > 0 should be on
int Settings::write_actions_recursively_v2( TDEConfig& cfg_P, Action_data_group* parent_P, bool enabled_P )
    {
    int enabled_cnt = 0;
    TQString save_cfg_group = cfg_P.group();
    int cnt = 0;
    for( Action_data_group::Iterator it = parent_P->first_child();
         it;
         ++it )
        {
        ++cnt;
        if( enabled_P && (*it)->enabled( true ))
            ++enabled_cnt;
        cfg_P.setGroup( save_cfg_group + "_" + TQString::number( cnt ));
        ( *it )->cfg_write( cfg_P );
        Action_data_group* grp = dynamic_cast< Action_data_group* >( *it );
        if( grp != NULL )
            enabled_cnt += write_actions_recursively_v2( cfg_P, grp, enabled_P && (*it)->enabled( true ));
        }
    cfg_P.setGroup( save_cfg_group );
    cfg_P.writeEntry( "DataCount", cnt );
    return enabled_cnt;
    }

void Settings::read_settings_v2( TDEConfig& cfg_P, bool include_disabled_P  )
    {
    cfg_P.setGroup( "Data" );
    read_actions_recursively_v2( cfg_P, actions, include_disabled_P );    
    }
    
void Settings::read_actions_recursively_v2( TDEConfig& cfg_P, Action_data_group* parent_P,
    bool include_disabled_P )
    {
    TQString save_cfg_group = cfg_P.group();
    int cnt = cfg_P.readNumEntry( "DataCount" );
    for( int i = 1;
         i <= cnt;
         ++i )
        {
        cfg_P.setGroup( save_cfg_group + "_" + TQString::number( i ));
        if( include_disabled_P || Action_data_base::cfg_is_enabled( cfg_P ))
            {
            Action_data_base* new_action = Action_data_base::create_cfg_read( cfg_P, parent_P );
            Action_data_group* grp = dynamic_cast< Action_data_group* >( new_action );
            if( grp != NULL )
                read_actions_recursively_v2( cfg_P, grp, include_disabled_P );
            }
        }
    cfg_P.setGroup( save_cfg_group );
    }

// backward compatibility
void Settings::read_settings_v1( TDEConfig& cfg_P )
    {    
    int sections = cfg_P.readNumEntry( "Num_Sections", 0 );
    Action_data_group* menuentries = NULL;
    for( Action_data_group::Iterator it( actions->first_child());
         *it;
         ++it )
        {
        Action_data_group* tmp = dynamic_cast< Action_data_group* >( *it );
        if( tmp == NULL )
            continue;
        if( tmp->system_group() == Action_data_group::SYSTEM_MENUENTRIES )
            {
            menuentries = tmp;
            break;
            }
        }
    for( int sect = 1;
         sect <= sections;
         ++sect )
        {
        TQString group = TQString( "Section%1" ).arg( sect );
        if( !cfg_P.hasGroup( group ))
            continue;
        cfg_P.setGroup( group );
        TQString name = cfg_P.readEntry( "Name" );
        if( name.isNull() )
            continue;
        TQString shortcut = cfg_P.readEntry( "Shortcut" );
        if( shortcut.isNull() )
            continue;
        TQString run = cfg_P.readEntry( "Run" );
        if( run.isNull() )
            continue;
        bool menuentry = cfg_P.readBoolEntry( "MenuEntry", false );
        // CHECKME tohle pridavani az pak je trosku HACK
        if( menuentry )
            {
            if( menuentries == NULL )
                {
                menuentries = new Action_data_group( actions,
                    i18n( MENU_EDITOR_ENTRIES_GROUP_NAME ),
                    i18n( "These entries were created using Menu Editor." ), NULL,
                    Action_data_group::SYSTEM_MENUENTRIES, true );
                menuentries->set_conditions( new Condition_list( "", menuentries ));
                }
            ( void ) new Menuentry_shortcut_action_data( menuentries, name, "",
                TDEShortcut( shortcut ), run );
            }
        else
            {
            ( void ) new Command_url_shortcut_action_data( actions, name, "",
                TDEShortcut( shortcut ), run );
            }
        }
    }

} // namespace KHotKeys
