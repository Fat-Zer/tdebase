/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _CONDITIONS_LIST_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "condition_list_widget.h"

#include <assert.h>
#include <tqpushbutton.h>
#include <tqheader.h>
#include <tqlineedit.h>
#include <tqpopupmenu.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#include <khlistview.h>

#include <conditions.h>

#include "windowdef_list_widget.h"

#include "kcmkhotkeys.h"

namespace KHotKeys
{

// Condition_list_widget

Condition_list_widget::Condition_list_widget( TQWidget* parent_P, const char* name_P )
    : Condition_list_widget_ui( parent_P, name_P ), selected_item( NULL )
    {
    conditions.setAutoDelete( true );
    TQPopupMenu* popup = new TQPopupMenu; // CHECKME looks like setting parent doesn't work
    popup->insertItem( i18n( "Active Window..." ), TYPE_ACTIVE_WINDOW );
    popup->insertItem( i18n( "Existing Window..." ), TYPE_EXISTING_WINDOW );
    popup->insertItem( i18n( "Not_condition", "Not" ), TYPE_NOT );
    popup->insertItem( i18n( "And_condition", "And" ), TYPE_AND );
    popup->insertItem( i18n( "Or_condition", "Or" ), TYPE_OR );
    connect( conditions_listview, TQT_SIGNAL( doubleClicked ( TQListViewItem *, const TQPoint &, int ) ),
             this, TQT_SLOT( modify_pressed() ) );

    connect( popup, TQT_SIGNAL( activated( int )), TQT_SLOT( new_selected( int )));
    new_button->setPopup( popup );
    conditions_listview->header()->hide();
    conditions_listview->addColumn( "" );
    conditions_listview->setSorting( -1 );
    conditions_listview->setRootIsDecorated( true ); // CHECKME
    conditions_listview->setForceSelect( true );
    copy_button->setEnabled( false );
    modify_button->setEnabled( false );
    delete_button->setEnabled( false );
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

Condition_list_widget::~Condition_list_widget()
    {
    delete new_button->popup();
    }

void Condition_list_widget::clear_data()
    {
    comment_lineedit->clear();
    conditions.clear();
    conditions_listview->clear();
    }

void Condition_list_widget::set_data( const Condition_list* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    comment_lineedit->setText( data_P->comment());
    conditions.clear();
    conditions_listview->clear();
    insert_listview_items( data_P, conditions_listview, NULL );
#ifdef KHOTKEYS_DEBUG
    kdDebug( 1217 ) << "Condition_list_widget::set_data():" << endl;
    Condition::debug_list( conditions );
#endif
    }

void Condition_list_widget::insert_listview_items( const Condition_list_base* parent_P,
    TQListView* parent1_P, Condition_list_item* parent2_P )
    {
    Condition_list_item* prev = NULL;
    for( Condition_list_base::Iterator it( *parent_P );
         *it;
         ++it )
        {
        prev = create_listview_item( *it, parent1_P, parent2_P, prev, true );
        if( Condition_list_base* group = dynamic_cast< Condition_list_base* >( *it ))
            insert_listview_items( group, NULL, prev );
        }
    }

Condition_list* Condition_list_widget::get_data( Action_data_base* data_P ) const
    {
#ifdef KHOTKEYS_DEBUG
    kdDebug( 1217 ) << "Condition_list_widget::get_data():" << endl;
    Condition::debug_list( conditions );
#endif
// CHECKME TODO hmm, tady to bude chtit asi i children :(
    Condition_list* list = new Condition_list( comment_lineedit->text(), data_P );
    get_listview_items( list, conditions_listview->firstChild());
    return list;
    }

void Condition_list_widget::get_listview_items( Condition_list_base* list_P,
    TQListViewItem* first_item_P ) const
    {
    list_P->clear();
    for( TQListViewItem* pos = first_item_P;
         pos != NULL;
         pos = pos->nextSibling())
        {
        Condition* cond = static_cast< Condition_list_item* >( pos )->condition()->copy( list_P );
        if( Condition_list_base* group = dynamic_cast< Condition_list_base* >( cond ))
            get_listview_items( group, pos->firstChild());
        }
    }

void Condition_list_widget::new_selected( int type_P )
    {
    Condition_list_item* parent = NULL;
    Condition_list_item* after = NULL;
    if( selected_item && selected_item->condition())
        {
        Condition_list_base* tmp = dynamic_cast< Condition_list_base* >
            ( selected_item->condition());
        if( tmp && tmp->accepts_children())
            {
            int ret = KMessageBox::questionYesNoCancel( NULL,
                i18n( "A group is selected.\nAdd the new condition in this selected group?" ), TQString::null, i18n("Add in Group"), i18n("Ignore Group"));
            if( ret == KMessageBox::Cancel )
                return;
            else if( ret == KMessageBox::Yes )
                parent = selected_item;
            else
                parent = NULL;
            }
        }
    if( parent == NULL && selected_item != NULL && selected_item->parent() != NULL )
        {
        parent = static_cast< Condition_list_item* >( selected_item->parent());
        after = selected_item;
        }
    Condition_list_base* parent_cond = parent
        ? static_cast< Condition_list_base* >( parent->condition()) : NULL;
    assert( !parent || dynamic_cast< Condition_list_base* >( parent->condition()));
    Condition_dialog* dlg = NULL;
    Condition* condition = NULL;
    switch( type_P )
        {
        case TYPE_ACTIVE_WINDOW: // Active_window_condition
            dlg = new Active_window_condition_dialog(
                new Active_window_condition( new Windowdef_list( "" ), parent_cond )); // CHECKME NULL
          break;
        case TYPE_EXISTING_WINDOW: // Existing_window_condition
            dlg = new Existing_window_condition_dialog(
                new Existing_window_condition( new Windowdef_list( "" ), parent_cond )); // CHECKME NULL
          break;
        case TYPE_NOT: // Not_condition
            condition = new Not_condition( parent_cond );
          break;
        case TYPE_AND: // And_condition
            condition = new And_condition( parent_cond );
          break;
        case TYPE_OR: // Or_condition
            condition = new Or_condition( parent_cond );
          break;
        }
    if( dlg != NULL )
        {
        condition = dlg->edit_condition();
        delete dlg;
        }
    if( condition != NULL )
        {
        if( parent != NULL )
            conditions_listview->setSelected( create_listview_item( condition,
                NULL, parent, after, false ), true );
        else
            conditions_listview->setSelected( create_listview_item( condition,
                conditions_listview, NULL, selected_item, false ), true );
        }
    }

void Condition_list_widget::copy_pressed()
    {
        if ( !selected_item )
            return;
    conditions_listview->setSelected( create_listview_item(
        selected_item->condition()->copy( selected_item->condition()->parent()),
        selected_item->parent() ? NULL : conditions_listview,
        static_cast< Condition_list_item* >( selected_item->parent()),
        selected_item, true ), true );
    }

void Condition_list_widget::delete_pressed()
{
    if ( selected_item )
    {
        conditions.remove( selected_item->condition()); // we own it
        delete selected_item; // CHECKME snad vyvola signaly pro enable()
        selected_item = NULL;
    }
}

void Condition_list_widget::modify_pressed()
    {
        if ( !selected_item )
            return;
    edit_listview_item( selected_item );
    }

void Condition_list_widget::current_changed( TQListViewItem* item_P )
    {
//    if( item_P == selected_item )
//        return;
    selected_item = static_cast< Condition_list_item* >( item_P );
//    conditions_listview->setSelected( selected_item, true );
    copy_button->setEnabled( selected_item != NULL );
    delete_button->setEnabled( selected_item != NULL );
    if( selected_item != NULL )
        { // not,and,or can't be modified
        if( dynamic_cast< Not_condition* >( selected_item->condition()) == NULL
            && dynamic_cast< And_condition* >( selected_item->condition()) == NULL
            && dynamic_cast< Or_condition* >( selected_item->condition()) == NULL )
            {
            modify_button->setEnabled( true );
            }
        else
            modify_button->setEnabled( false );
        }
    else
        modify_button->setEnabled( false );
    }

Condition_list_item* Condition_list_widget::create_listview_item( Condition* condition_P,
    TQListView* parent1_P, Condition_list_item* parent2_P, TQListViewItem* after_P, bool copy_P )
    {
#ifdef KHOTKEYS_DEBUG
    kdDebug( 1217 ) << "Condition_list_widget::create_listview_item():" << endl;
    Condition::debug_list( conditions );
	kdDebug( 1217 ) << kdBacktrace() << endl;
#endif
    Condition* new_cond = copy_P ? condition_P->copy( parent2_P
        ? static_cast< Condition_list_base* >( parent2_P->condition()) : NULL ) : condition_P;
    assert( !copy_P || !parent2_P || dynamic_cast< Condition_list_base* >( parent2_P->condition()));
// CHECKME uz by nemelo byt treba
/*    if( after_P == NULL )
        {
        if( parent1_P == NULL )
            return new Condition_list_item( parent2_P, new_win );
        else
            return new Condition_list_item( parent1_P, new_win );
        }
    else*/
        {
        if( parent1_P == NULL )
            {
            parent2_P->setOpen( true );
            if( new_cond->parent() == NULL ) // own only toplevels, they own the rest
                conditions.append( new_cond ); // we own it, not the listview
            return new Condition_list_item( parent2_P, after_P, new_cond );
            }
        else
            {
            if( new_cond->parent() == NULL )
                conditions.append( new_cond ); // we own it, not the listview
            return new Condition_list_item( parent1_P, after_P, new_cond );
            }
        }
    }

void Condition_list_widget::edit_listview_item( Condition_list_item* item_P )
    {
    Condition_dialog* dlg = NULL;
    if( Active_window_condition* condition
        = dynamic_cast< Active_window_condition* >( item_P->condition()))
        dlg = new Active_window_condition_dialog( condition );
    else if( Existing_window_condition* condition
        = dynamic_cast< Existing_window_condition* >( item_P->condition()))
        dlg = new Existing_window_condition_dialog( condition );
    else if( dynamic_cast< Not_condition* >( item_P->condition()) != NULL )
        return;
    else if( dynamic_cast< And_condition* >( item_P->condition()) != NULL )
        return;
    else if( dynamic_cast< Or_condition* >( item_P->condition()) != NULL )
        return;
    else // CHECKME TODO pridat dalsi
        assert( false );
    Condition* new_condition = dlg->edit_condition();
    if( new_condition != NULL )
        {
        Condition* old_cond = item_P->condition();
        item_P->set_condition( new_condition );
        int pos = conditions.find( old_cond );
        if( pos >= 0 )
            {
            conditions.remove( pos ); // we own it
            conditions.insert( pos, new_condition );
            }
        item_P->widthChanged( 0 );
        conditions_listview->repaintItem( item_P );
        }
#ifdef KHOTKEYS_DEBUG
    kdDebug( 1217 ) << "Condition_list_widget::edit_listview_item():" << endl;
    Condition::debug_list( conditions );
#endif
    delete dlg;
    }

// Condition_list_item

TQString Condition_list_item::text( int column_P ) const
    {
    return column_P == 0 ? condition()->description() : TQString::null;
    }

// Active_window_condition_dialog

Active_window_condition_dialog::Active_window_condition_dialog(
    Active_window_condition* condition_P )
    : KDialogBase( NULL, NULL, true, i18n( "Window Details" ), Ok | Cancel ), condition( NULL )
    {
    widget = new Windowdef_list_widget( this );
    widget->set_data( condition_P->window());
    setMainWidget( widget );
    }

Condition* Active_window_condition_dialog::edit_condition()
    {
    exec();
    return condition;
    }

void Active_window_condition_dialog::accept()
    {
    KDialogBase::accept();
    condition = new Active_window_condition( widget->get_data(), NULL ); // CHECKME NULL ?
    }

// Existing_window_condition_dialog

Existing_window_condition_dialog::Existing_window_condition_dialog(
    Existing_window_condition* condition_P )
    : KDialogBase( NULL, NULL, true, i18n( "Window Details" ), Ok | Cancel ), condition( NULL )
    {
    widget = new Windowdef_list_widget( this );
    widget->set_data( condition_P->window());
    setMainWidget( widget );
    }

Condition* Existing_window_condition_dialog::edit_condition()
    {
    exec();
    return condition;
    }

void Existing_window_condition_dialog::accept()
    {
    KDialogBase::accept();
    condition = new Existing_window_condition( widget->get_data(), NULL ); // CHECKME NULL ?
    }

} // namespace KHotKeys

#include "condition_list_widget.moc"
