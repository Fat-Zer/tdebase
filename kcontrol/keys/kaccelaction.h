// THIS FILE IS A COPY OF kdelibs/kdecore/kaccelaction.h AND MUST BE KEPT
//  IN SYNC WITH THAT FILE.

/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KACCELACTION_H
#define _KACCELACTION_H

#include <tqmap.h>
#include <tqptrvector.h>
#include <tqstring.h>
#include <tqvaluevector.h>

#include <kshortcut.h>

class KAccelBase;

class TQObject;
class KConfig;
class KConfigBase;

/*
	KAccelAction holds information an a given action, such as "Run Command"

	1) KAccelAction = "Run Command"
		Default3 = "Alt+F2"
		Default4 = "Meta+Enter;Alt+F2"
		1) KShortcut = "Meta+Enter"
			1) KKeySequence = "Meta+Enter"
				1) KKey = "Meta+Enter"
					1) Meta+Enter
					2) Meta+Keypad_Enter
		2) KShortcut = "Alt+F2"
			1) KKeySequence = "Alt+F2"
				1) Alt+F2
	2) KAccelAction = "Something"
		Default3 = ""
		Default4 = ""
		1) KShortcut = "Meta+X,Asterisk"
			1) KKeySequence = "Meta+X,Asterisk"
				1) KKey = "Meta+X"
					1) Meta+X
				2) KKey = "Asterisk"
					1) Shift+8 (English tqlayout)
					2) Keypad_Asterisk
*/

//---------------------------------------------------------------------
// KAccelAction
//---------------------------------------------------------------------

class KAccelAction
{
 public:
	KAccelAction();
	KAccelAction( const KAccelAction& );
	KAccelAction( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const KShortcut& cutDef3, const KShortcut& cutDef4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );
	~KAccelAction();

	void clear();
	bool init( const TQString& sName, const TQString& sLabel, const TQString& sWhatsThis,
			const KShortcut& cutDef3, const KShortcut& cutDef4,
			const TQObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );

	KAccelAction& operator=( const KAccelAction& );

	const TQString& name() const                { return m_sName; }
	const TQString& label() const               { return m_sLabel; }
	const TQString& whatsThis() const           { return m_sWhatsThis; }
	const KShortcut& shortcut() const          { return m_cut; }
	const KShortcut& shortcutDefault() const;
	const KShortcut& shortcutDefault3() const  { return m_cutDefault3; }
	const KShortcut& shortcutDefault4() const  { return m_cutDefault4; }
	const TQObject* objSlotPtr() const          { return m_pObjSlot; }
	const char* methodSlotPtr() const          { return m_psMethodSlot; }
	bool isConfigurable() const                { return m_bConfigurable; }
	bool isEnabled() const                     { return m_bEnabled; }

	void setName( const TQString& );
	void setLabel( const TQString& );
	void setWhatsThis( const TQString& );
	bool setShortcut( const KShortcut& rgCuts );
	void setSlot( const TQObject* pObjSlot, const char* psMethodSlot );
	void setConfigurable( bool );
	void setEnabled( bool );

	int getID() const   { return m_nIDAccel; }
	void setID( int n ) { m_nIDAccel = n; }
	bool isConnected() const;

	bool setKeySequence( uint i, const KKeySequence& );
	void clearShortcut();
	bool tqcontains( const KKeySequence& );

	TQString toString() const;
	TQString toStringInternal() const;

	static bool useFourModifierKeys();
	static void useFourModifierKeys( bool );

 protected:
	TQString m_sName,
	        m_sLabel,
	        m_sWhatsThis;
	KShortcut m_cut;
	KShortcut m_cutDefault3, m_cutDefault4;
	const TQObject* m_pObjSlot;
	const char* m_psMethodSlot;
	bool m_bConfigurable,
	     m_bEnabled;
	int m_nIDAccel;
	uint m_nConnections;

	void incConnections();
	void decConnections();

 private:
	static int g_bUseFourModifierKeys;
	class KAccelActionPrivate* d;

	friend class KAccelActions;
	friend class KAccelBase;
};

//---------------------------------------------------------------------
// KAccelActions
//---------------------------------------------------------------------

class KAccelActions
{
 public:
	KAccelActions();
	KAccelActions( const KAccelActions& );
	virtual ~KAccelActions();

	void clear();
	bool init( const KAccelActions& );
	bool init( KConfigBase& config, const TQString& sGroup );

	void updateShortcuts( KAccelActions& );

	int actionIndex( const TQString& sAction ) const;
	KAccelAction* actionPtr( uint );
	const KAccelAction* actionPtr( uint ) const;
	KAccelAction* actionPtr( const TQString& sAction );
	const KAccelAction* actionPtr( const TQString& sAction ) const;
	KAccelAction* actionPtr( KKeySequence cut );
	KAccelAction& operator []( uint );
	const KAccelAction& operator []( uint ) const;

	KAccelAction* insert( const TQString& sAction, const TQString& sLabel, const TQString& sWhatsThis,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const TQObject* pObjSlot = 0, const char* psMethodSlot = 0,
			bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insert( const TQString& sName, const TQString& sLabel );
	bool remove( const TQString& sAction );

	bool readActions( const TQString& sConfigGroup = "Shortcuts", KConfigBase* pConfig = 0 );
	bool writeActions( const TQString& sConfigGroup = "Shortcuts", KConfigBase* pConfig = 0,
			bool bWriteAll = false, bool bGlobal = false ) const;

	void emitKeycodeChanged();

	uint count() const;

 protected:
	KAccelBase* m_pKAccelBase;
	KAccelAction** m_prgActions;
	uint m_nSizeAllocated, m_nSize;

	void resize( uint );
	void insertPtr( KAccelAction* );

 private:
	class KAccelActionsPrivate* d;

	KAccelActions( KAccelBase* );
	void initPrivate( KAccelBase* );
	KAccelActions& operator =( KAccelActions& );

	friend class KAccelBase;
};

#endif // _KACCELACTION_H
