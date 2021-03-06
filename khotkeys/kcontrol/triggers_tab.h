/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _TRIGGERS_TAB_H_
#define _TRIGGERS_TAB_H_

#include <kdialogbase.h>

#include <khlistview.h>

#include <triggers.h>
#include <triggers_tab_ui.h>
#include <window_trigger_widget.h>

class KKeyButton;
class TDEShortcut;

namespace KHotKeys
{

class Windowdef_list;
class Action_data;
class Trigger_list_item;
class KHotKeysShortcutList;

// A listbox here would do too, but unlike TQListView, TQListBox now even cannot be subclassed
// to behave sanely WRT selecting and the current item
class Triggers_tab
    : public Triggers_tab_ui
    {
    Q_OBJECT
    public:
        Triggers_tab( TQWidget* parent_P = NULL, const char* name_P = NULL );
        virtual ~Triggers_tab();
        void set_data( const Trigger_list* data_P );
        Trigger_list* get_data( Action_data* data_P ) const;
    public slots:
        void clear_data();
    protected:
        Trigger_list_item* create_listview_item( Trigger* trigger_P, TQListView* parent_P,
            TQListViewItem* after_P, bool copy_P );
        void edit_listview_item( Trigger_list_item* item_P );
        Trigger_list_item* selected_item;
        enum type_t { TYPE_SHORTCUT_TRIGGER, TYPE_GESTURE_TRIGGER, TYPE_WINDOW_TRIGGER, TYPE_VOICE_TRIGGER };
    protected slots:
        void new_selected( int type_P );
        virtual void copy_pressed();
        virtual void delete_pressed();
        virtual void modify_pressed();
        virtual void current_changed( TQListViewItem* item_P );
    };

class Trigger_list_item
    : public TQListViewItem
    {
    public:
        Trigger_list_item( TQListView* parent_P, Trigger* trigger_P );
        Trigger_list_item( TQListView* parent_P, TQListViewItem* after_P, Trigger* trigger_P );
        virtual ~Trigger_list_item();
        virtual TQString text( int column_P ) const;
        Trigger* trigger() const;
        void set_trigger( Trigger* trigger_P );
    protected:
        Trigger* _trigger; // owns
    };
        
class Trigger_dialog
    {
    public:
        virtual Trigger* edit_trigger() = 0;
        virtual ~Trigger_dialog();
    };

// TODO no need for such extra class?    
class Shortcut_trigger_widget
    : public TQWidget
    {
    Q_OBJECT
    public:
        Shortcut_trigger_widget( TQWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Shortcut_trigger* trigger_P );
        Shortcut_trigger* get_data( Action_data* data_P ) const;
    public slots:
        void clear_data();
    private slots:
        void capturedShortcut( const TDEShortcut& );
    private:
        KKeyButton* bt;
    };

typedef Shortcut_trigger_widget Shortcut_trigger_tab;
    
class Shortcut_trigger_dialog
    : public KDialogBase, public Trigger_dialog
    {
    Q_OBJECT
    public:
        Shortcut_trigger_dialog( Shortcut_trigger* trigger_P );
        virtual Trigger* edit_trigger();
    protected:
        virtual void accept();
        Shortcut_trigger_widget* widget;
        Shortcut_trigger* trigger;
    };
    
class Window_trigger_dialog
    : public KDialogBase, public Trigger_dialog
    {
    Q_OBJECT
    public:
        Window_trigger_dialog( Window_trigger* trigger_P );
        virtual Trigger* edit_trigger();
    protected:
        virtual void accept();
        Window_trigger_widget* widget;
        Window_trigger* trigger;
    };
        
class GestureRecordPage;

class Gesture_trigger_dialog
    : public KDialogBase, public Trigger_dialog
    {
    Q_OBJECT
    public:
        Gesture_trigger_dialog( Gesture_trigger* trigger_P );
        virtual Trigger* edit_trigger();
    private:
        // CHECKME accept() ?
        Gesture_trigger* _trigger;
        GestureRecordPage *_page;
    };        


class VoiceRecordPage;

class Voice_trigger_dialog
	: public KDialogBase, public Trigger_dialog
{
	Q_OBJECT
	public:
		Voice_trigger_dialog( Voice_trigger* trigger_P );
		virtual Trigger* edit_trigger();
	private:
        // CHECKME accept() ?
		Voice_trigger* _trigger;
		VoiceRecordPage *_page;
};

	
            
//***************************************************************************
// Inline
//***************************************************************************

// Trigger_list_item

inline
Trigger_list_item::Trigger_list_item( TQListView* parent_P, Trigger* trigger_P )
    : TQListViewItem( parent_P ), _trigger( trigger_P )
    {
    }
    
inline
Trigger_list_item::Trigger_list_item( TQListView* parent_P, TQListViewItem* after_P,
    Trigger* trigger_P )
    : TQListViewItem( parent_P, after_P ), _trigger( trigger_P )
    {
    }
    
inline
Trigger_list_item::~Trigger_list_item()
    {                // CHECKME if the listview will ever be used hiearchically,
    delete _trigger; // this will be wrong, the triggers tree will have to be kept
    }                // and deleted separately

inline
Trigger* Trigger_list_item::trigger() const
    {
    return _trigger;
    }
    
inline
void Trigger_list_item::set_trigger( Trigger* trigger_P )
    {
    delete _trigger;
    _trigger = trigger_P;
    }

// Trigger_dialog

inline
Trigger_dialog::~Trigger_dialog()
    {
    }
    
} // namespace KHotKeys

#endif
