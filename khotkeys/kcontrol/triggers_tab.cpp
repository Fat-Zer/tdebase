/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _TRIGGERS_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "triggers_tab.h"

#include <assert.h>
#include <tqpushbutton.h>
#include <tqlineedit.h>
#include <tqpopupmenu.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqheader.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeshortcut.h>
#include <tdeconfig.h>
#include <tdeshortcutlist.h>
#include <kkeybutton.h>
#include <kkeydialog.h>

#include "kcmkhotkeys.h"
#include "windowdef_list_widget.h"
#include "window_trigger_widget.h"
#include "gesturerecordpage.h"
#include "voicerecordpage.h"

namespace KHotKeys
{

// Triggers_tab

Triggers_tab::Triggers_tab( TQWidget* parent_P, const char* name_P )
    : Triggers_tab_ui( parent_P, name_P ), selected_item( NULL )
    {
    TQPopupMenu* popup = new TQPopupMenu; // CHECKME looks like setting parent doesn't work
    popup->insertItem( i18n( "Shortcut Trigger..." ), TYPE_SHORTCUT_TRIGGER );
    popup->insertItem( i18n( "Gesture Trigger..." ), TYPE_GESTURE_TRIGGER );
    popup->insertItem( i18n( "Window Trigger..." ), TYPE_WINDOW_TRIGGER );
#ifdef HAVE_ARTS
    if( haveArts())
        popup->insertItem( i18n( "Voice Trigger..." ), TYPE_VOICE_TRIGGER );
#endif
    connect( popup, TQT_SIGNAL( activated( int )), TQT_SLOT( new_selected( int )));
    connect( triggers_listview, TQT_SIGNAL( doubleClicked ( TQListViewItem *, const TQPoint &, int ) ),
             this, TQT_SLOT( modify_pressed() ) );

    new_button->setPopup( popup );
    copy_button->setEnabled( false );
    modify_button->setEnabled( false );
    delete_button->setEnabled( false );
    triggers_listview->header()->hide();
    triggers_listview->addColumn( "" );
    triggers_listview->setSorting( -1 );
    triggers_listview->setForceSelect( true );
    clear_data();
    // KHotKeys::Module::changed()
    connect( new_button, TQT_SIGNAL( clicked()),
        module, TQT_SLOT( changed()));
    connect( copy_button, TQT_SIGNAL( clicked()),
        module, TQT_SLOT( changed()));
    connect( modify_button, TQT_SIGNAL( clicked()),
        module, TQT_SLOT( changed()));
    connect( delete_button, TQT_SIGNAL( clicked()),
        module, TQT_SLOT( changed()));
    connect( comment_lineedit, TQT_SIGNAL( textChanged( const TQString& )),
        module, TQT_SLOT( changed()));
    }

Triggers_tab::~Triggers_tab()
    {
    delete new_button->popup(); // CHECKME
    }

void Triggers_tab::clear_data()
    {
    comment_lineedit->clear();
    triggers_listview->clear();
    }

void Triggers_tab::set_data( const Trigger_list* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    comment_lineedit->setText( data_P->comment());
    Trigger_list_item* after = NULL;
    triggers_listview->clear();
    for( Trigger_list::Iterator it( *data_P );
         *it;
         ++it )
        after = create_listview_item( *it, triggers_listview, after, true );
    }

Trigger_list* Triggers_tab::get_data( Action_data* data_P ) const
    {
    Trigger_list* list = new Trigger_list( comment_lineedit->text());
    for( TQListViewItem* pos = triggers_listview->firstChild();
         pos != NULL;
         pos = pos->nextSibling())
        list->append( static_cast< Trigger_list_item* >( pos )->trigger()->copy( data_P ));
    return list;
    }

void Triggers_tab::new_selected( int type_P )
    {
    Trigger_dialog* dlg = NULL;
    switch( type_P )
        {
        case TYPE_SHORTCUT_TRIGGER: // Shortcut_trigger
            dlg = new Shortcut_trigger_dialog(
                new Shortcut_trigger( NULL, TDEShortcut())); // CHECKME NULL ?
          break;
        case TYPE_GESTURE_TRIGGER: // Gesture trigger
            dlg = new Gesture_trigger_dialog(
                new Gesture_trigger( NULL, TQString::null )); // CHECKME NULL ?
          break;
        case TYPE_WINDOW_TRIGGER: // Window trigger
            dlg = new Window_trigger_dialog( new Window_trigger( NULL, new Windowdef_list( "" ),
                0 )); // CHECKME NULL ?
          break;
        case TYPE_VOICE_TRIGGER: // Voice trigger
			dlg = new Voice_trigger_dialog( new Voice_trigger(NULL,TQString::null,VoiceSignature(),VoiceSignature())); // CHECKME NULL ?
			break;
        }
    if( dlg != NULL )
        {
        Trigger* trg = dlg->edit_trigger();
        if( trg != NULL )
            triggers_listview->setSelected( create_listview_item( trg, triggers_listview,
                selected_item, false ), true );
        delete dlg;
        }
    }

void Triggers_tab::copy_pressed()
    {
        if ( selected_item )
        {
            triggers_listview->setSelected( create_listview_item( selected_item->trigger(),triggers_listview, selected_item, true ), true );
        }
    }

void Triggers_tab::delete_pressed()
    {
    delete selected_item; // CHECKME snad vyvola signaly pro enable()
    selected_item = NULL;
    }

void Triggers_tab::modify_pressed()
{
    if ( selected_item )
        edit_listview_item( selected_item );
}

void Triggers_tab::current_changed( TQListViewItem* item_P )
    {
//    if( item_P == selected_item )
//        return;
    selected_item = static_cast< Trigger_list_item* >( item_P );
//    triggers_listview->setSelected( item_P, true );
    copy_button->setEnabled( item_P != NULL );
    modify_button->setEnabled( item_P != NULL );
    delete_button->setEnabled( item_P != NULL );
    }

Trigger_list_item* Triggers_tab::create_listview_item( Trigger* trigger_P,
    TQListView* parent_P, TQListViewItem* after_P, bool copy_P )
    {
    Trigger* new_trg = copy_P ? trigger_P->copy( NULL ) : trigger_P; // CHECKME NULL ?
// CHECKME uz by nemelo byt treba    if( after_P == NULL )
//        return new Trigger_list_item( parent_P, new_trg );
//    else
        return new Trigger_list_item( parent_P, after_P, new_trg );
    }

void Triggers_tab::edit_listview_item( Trigger_list_item* item_P )
    {
    Trigger_dialog* dlg = NULL;
    if( Shortcut_trigger* trg = dynamic_cast< Shortcut_trigger* >( item_P->trigger()))
        dlg = new Shortcut_trigger_dialog( trg );
    else if( Gesture_trigger* trg = dynamic_cast< Gesture_trigger* >( item_P->trigger()))
        dlg = new Gesture_trigger_dialog( trg );
    else if( Window_trigger* trg = dynamic_cast< Window_trigger* >( item_P->trigger()))
        dlg = new Window_trigger_dialog( trg );
    else if( Voice_trigger* trg = dynamic_cast< Voice_trigger* >( item_P->trigger()))
        dlg = new Voice_trigger_dialog( trg );
// CHECKME TODO dalsi
    else
        assert( false );
    Trigger* new_trigger = dlg->edit_trigger();
    if( new_trigger != NULL )
        item_P->set_trigger( new_trigger );
    delete dlg;
    }

// Trigger_list_item

TQString Trigger_list_item::text( int column_P ) const
    {
    return column_P == 0 ? trigger()->description() : TQString::null;
    }

// Shortcut_trigger_widget

Shortcut_trigger_widget::Shortcut_trigger_widget( TQWidget* parent_P, const char* )
    : TQWidget( parent_P )
    {
    TQVBoxLayout* lay = new TQVBoxLayout( this, 11, 6 );
    TQLabel* lbl = new TQLabel( i18n( "Select keyboard shortcut:" ), this );
    lay->addWidget( lbl );
    lay->addSpacing( 10 );
    bt = new KKeyButton( this );
    lay->addWidget( bt, 0 , Qt::AlignHCenter );
    lay->addStretch();
    clear_data();
    connect( bt, TQT_SIGNAL( capturedShortcut( const TDEShortcut& )),
        this, TQT_SLOT( capturedShortcut( const TDEShortcut& )));
    }

void Shortcut_trigger_widget::clear_data()
    {
    bt->setShortcut( TDEShortcut(), false );
    }

void Shortcut_trigger_widget::capturedShortcut( const TDEShortcut& s_P )
    {
    if( KKeyChooser::checkGlobalShortcutsConflict( s_P, true, topLevelWidget())
        || KKeyChooser::checkStandardShortcutsConflict( s_P, true, topLevelWidget()))
        return;
    // KHotKeys::Module::changed()
    module->changed();
    bt->setShortcut( s_P, false );
    }

void Shortcut_trigger_widget::set_data( const Shortcut_trigger* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    bt->setShortcut( data_P->shortcut(), false );
    }

Shortcut_trigger* Shortcut_trigger_widget::get_data( Action_data* data_P ) const
    {
    return !bt->shortcut().isNull()
        ? new Shortcut_trigger( data_P, bt->shortcut()) : NULL;
    }

// Shortcut_trigger_dialog

Shortcut_trigger_dialog::Shortcut_trigger_dialog( Shortcut_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        trigger( NULL )
    {
    widget = new Shortcut_trigger_widget( this );
    widget->set_data( trigger_P );
    setMainWidget( widget );
    }

Trigger* Shortcut_trigger_dialog::edit_trigger()
    {
    exec();
    return trigger;
    }

void Shortcut_trigger_dialog::accept()
    {
    KDialogBase::accept();
    trigger = widget->get_data( NULL ); // CHECKME NULL ?
    }

// Window_trigger_dialog

Window_trigger_dialog::Window_trigger_dialog( Window_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        trigger( NULL )
    {
    widget = new Window_trigger_widget( this );
    widget->set_data( trigger_P );
    setMainWidget( widget );
    }

Trigger* Window_trigger_dialog::edit_trigger()
    {
    exec();
    return trigger;
    }

void Window_trigger_dialog::accept()
    {
    KDialogBase::accept();
    trigger = widget->get_data( NULL ); // CHECKME NULL ?
    }

// Gesture_trigger_dialog

Gesture_trigger_dialog::Gesture_trigger_dialog( Gesture_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        _trigger( trigger_P ), _page( NULL )
    {
    _page = new GestureRecordPage( _trigger->gesturecode(),
                                  this, "GestureRecordPage");

    connect(_page, TQT_SIGNAL(gestureRecorded(bool)),
            this, TQT_SLOT(enableButtonOK(bool)));

    setMainWidget( _page );
    }

Trigger* Gesture_trigger_dialog::edit_trigger()
    {
    if( exec())
        return new Gesture_trigger( NULL, _page->getGesture()); // CHECKME NULL?
    else
        return NULL;
    }


// Voice_trigger_dialog

Voice_trigger_dialog::Voice_trigger_dialog( Voice_trigger* trigger_P )
: KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
_trigger( trigger_P ), _page( NULL )
{
	_page = new VoiceRecordPage( _trigger ? _trigger->voicecode() : TQString::null ,  this, "VoiceRecordPage");

	connect(_page, TQT_SIGNAL(voiceRecorded(bool)), this, TQT_SLOT(enableButtonOK(bool)));

	setMainWidget( _page );
}

Trigger* Voice_trigger_dialog::edit_trigger()
{
	if( exec())
		return new Voice_trigger(NULL, _page->getVoiceId(),
								 (_page->isModifiedSignature(1) || !_trigger) ?  _page->getVoiceSignature(1) : _trigger->voicesignature(1) ,
								 (_page->isModifiedSignature(2) || !_trigger) ?  _page->getVoiceSignature(2) : _trigger->voicesignature(2) ); 
	else
		return NULL;
}


	

} // namespace KHotKeys

#include "triggers_tab.moc"
