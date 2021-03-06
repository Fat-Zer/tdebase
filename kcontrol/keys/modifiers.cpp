#include "modifiers.h"

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kkeynative.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <ctype.h>
#undef NONE

/*Modifier Scheme
	PC: Shift/Ctrl/Alt/Win
	Mac: Shift/Command/Apple/Alt
	Custom

X11
Modifier	XMod		Label
Shift		Shift		[Shift]
Ctrl		Control		[Ctrl] Ctrl|Apple
Alt		[Mod1]		[Alt] Alt|Command
Win		[Mod4]		[Win] Win|Alt|Meta|Super|Hyper
Extra1		[]		[] User definable

	Shift
	Lock	Caps_Lock
	Control	Control_L, Control_R
	Mod1	Alt_L, Alt_R
	Mod2	Num_Lock
	Mod3	Mode_switch
	Mod4	Super_L, Super_R
	Mod5	Scroll_Lock
*/

//For Mac keyboards:
//1) labels: Shift | Ctrl | Option | Command
//2) swap Ctrl & Command

ModifiersModule::ModifiersModule( TQWidget *parent, const char *name )
:	TQWidget( parent, name )
{
	initGUI();
	load( false );
}

void ModifiersModule::load( bool useDefaults )
{
   TDEConfig *c = TDEGlobal::config();

   c->setReadDefaults( useDefaults );

	c->setGroup( "Keyboard" );

	m_sLabelCtrlOrig = c->readEntry( "Label Ctrl", "Ctrl" );
	m_sLabelAltOrig = c->readEntry( "Label Alt", "Alt" );
	m_sLabelWinOrig = c->readEntry( "Label Win", "Win" );

	m_bMacKeyboardOrig = c->readBoolEntry( "Mac Keyboard", false );
	m_bMacSwapOrig = m_bMacKeyboardOrig && c->readBoolEntry( "Mac Modifier Swap", false );
	
   updateWidgetData();
}

// When [Apply] or [OK] are clicked.
void ModifiersModule::save()
{
	kdDebug(125) << "ModifiersModule::save()" << endl;

	TDEConfigGroupSaver cgs( TDEGlobal::config(), "Keyboard" );

	if( m_plblCtrl->text() != "Ctrl" )
		TDEGlobal::config()->writeEntry( "Label Ctrl", m_plblCtrl->text(), true, true );
	else
		TDEGlobal::config()->deleteEntry( "Label Ctrl", false, true );

	if( m_plblAlt->text() != "Alt" )
		TDEGlobal::config()->writeEntry( "Label Alt", m_plblAlt->text(), true, true );
	else
		TDEGlobal::config()->deleteEntry( "Label Alt", false, true );

	if( m_plblWin->text() != "Win" )
		TDEGlobal::config()->writeEntry( "Label Win", m_plblWin->text(), true, true );
	else
		TDEGlobal::config()->deleteEntry( "Label Win", false, true );

	if( m_pchkMacKeyboard->isChecked() )
		TDEGlobal::config()->writeEntry( "Mac Keyboard", true, true, true );
	else
		TDEGlobal::config()->deleteEntry( "Mac Keyboard", false, true );

	bool bMacSwap = m_pchkMacKeyboard->isChecked() && m_pchkMacSwap->isChecked();
	if( bMacSwap )
		TDEGlobal::config()->writeEntry( "Mac Modifier Swap", true, true, true );
	else
		TDEGlobal::config()->deleteEntry( "Mac Modifier Swap", false, true );

	TDEGlobal::config()->sync();

	if( m_bMacSwapOrig != bMacSwap ) {
		if( bMacSwap )
			setupMacModifierKeys();
		else
			kapp->tdeinitExec("kxkb");
		m_bMacSwapOrig = bMacSwap;
		updateWidgets();
	}
}

void ModifiersModule::defaults()
{
   load( true );
}

#define SET_CODE_SYM( iCode, sym ) \
	if( iCode >= keyCodeMin && iCode <= keyCodeMax ) \
		rgKeySyms[(iCode-keyCodeMin) * nSymsPerCode] = sym;
#define SET_MOD_CODE( iMod, code1, code2 ) \
	xmk->modifiermap[iMod * xmk->max_keypermod + 0] = code1; \
	xmk->modifiermap[iMod * xmk->max_keypermod + 1] = code2;
void ModifiersModule::setupMacModifierKeys()
{
	const int CODE_Ctrl_L = 0x25, CODE_Ctrl_R = 0x6d;
	const int CODE_Win_L  = 0x73, CODE_Win_R  = 0x74;
	//const int CODE_Alt_L  = 0x40, CODE_Alt_R  = 0x71;
	int keyCodeMin, keyCodeMax, nKeyCodes, nSymsPerCode;

	XDisplayKeycodes( tqt_xdisplay(), &keyCodeMin, &keyCodeMax );
	nKeyCodes = keyCodeMax - keyCodeMin + 1;
	KeySym* rgKeySyms = XGetKeyboardMapping( tqt_xdisplay(), keyCodeMin, nKeyCodes, &nSymsPerCode );
	XModifierKeymap* xmk = XGetModifierMapping( tqt_xdisplay() );

	SET_CODE_SYM( CODE_Ctrl_L, XK_Super_L )
	SET_CODE_SYM( CODE_Ctrl_R, XK_Super_R )
	SET_CODE_SYM( CODE_Win_L,  XK_Control_L )
	SET_CODE_SYM( CODE_Win_R,  XK_Control_R )
	//SET_CODE_SYM( CODE_Win_L,  XK_Alt_L )
	//SET_CODE_SYM( CODE_Win_R,  XK_Alt_R )
	//SET_CODE_SYM( CODE_Alt_L,  XK_Control_L )
	//SET_CODE_SYM( CODE_Alt_R,  XK_Control_R )

	SET_MOD_CODE( ControlMapIndex, CODE_Win_L, CODE_Win_R );
	SET_MOD_CODE( Mod4MapIndex,    CODE_Ctrl_L, CODE_Ctrl_R );
	//SET_MOD_CODE( ControlMapIndex, CODE_Alt_L, CODE_Alt_R );
	//SET_MOD_CODE( Mod1MapIndex,    CODE_Win_L, CODE_Win_R );
	//SET_MOD_CODE( Mod4MapIndex,    CODE_Ctrl_L, CODE_Ctrl_R );

	XSetModifierMapping( tqt_xdisplay(), xmk );
	XChangeKeyboardMapping( tqt_xdisplay(), keyCodeMin, nSymsPerCode, rgKeySyms, nKeyCodes );
	XFree( rgKeySyms );
	XFreeModifiermap( xmk );
}
#undef SET_CODE_SYM

void ModifiersModule::initGUI()
{
	TQGridLayout* pLayoutTop = new TQGridLayout( this, 6, 2, KDialog::marginHint() );
	pLayoutTop->setColStretch( 1, 1 );

	TQGroupBox* pGroup = new TQGroupBox( 2, Qt::Horizontal, i18n("TDE Modifiers"), this );
	pLayoutTop->addWidget( pGroup, 0, 0 );

	TQLabel* plbl = new TQLabel( i18n("Modifier"), pGroup );
	TQFont font = plbl->font();
	font.setUnderline( true );
	font.setWeight( TQFont::Bold );
	plbl->setFont( font );
	plbl = new TQLabel( i18n("X11-Mod"), pGroup );
	plbl->setFont( font );

	new TQLabel( i18n(TQACCEL_OBJECT_NAME_STRING, "Shift"), pGroup );
	new TQLabel( "shift", pGroup );

	m_plblCtrl = new TQLabel( i18n(TQACCEL_OBJECT_NAME_STRING, "Ctrl"), pGroup );
	new TQLabel( "control", pGroup );

	m_plblAlt = new TQLabel( i18n(TQACCEL_OBJECT_NAME_STRING, "Alt"), pGroup );
	new TQLabel( "mod1", pGroup );

	m_plblWin = new TQLabel( i18n("Win"), pGroup );
	m_plblWinModX = new TQLabel( "", pGroup );
	/*m_pcbWinX = newModXComboBox( pGroup );
	int i;
	switch( KKeyNative::modX(KKey::WIN) ) {
		case Mod2Mask: i = 1; break;
		case Mod3Mask: i = 2; break;
		case Mod4Mask: i = 3; break;
		case Mod5Mask: i = 5; break;
		default:       i = 0;
	}
	m_pcbWinX->setCurrentItem( i );*/

	m_pchkMacKeyboard = new TQCheckBox( i18n("Macintosh keyboard"), this );
	m_pchkMacKeyboard->setChecked( m_bMacKeyboardOrig );
	connect( m_pchkMacKeyboard, TQT_SIGNAL(clicked()), TQT_SLOT(slotMacKeyboardClicked()) );
	pLayoutTop->addWidget( m_pchkMacKeyboard, 1, 0 );

	m_pchkMacSwap = new TQCheckBox( i18n("MacOS-style modifier usage"), this );
	m_pchkMacSwap->setChecked( m_bMacSwapOrig );
	TQWhatsThis::add( m_pchkMacSwap,
		i18n("Checking this box will change your X Modifier Mapping to "
			"better reflect the standard MacOS modifier key usage. "
			"It allows you to use <i>Command+C</i> for <i>Copy</i>, for instance, "
			"instead of the PC standard of <i>Ctrl+C</I>. "
			"<b>Command</b> will be used for application and console commands, "
			"<b>Option</b> as a command modifier and for navigating menus and dialogs, "
			"and <b>Control</b> for window manager commands.") );
	connect( m_pchkMacSwap, TQT_SIGNAL(clicked()), TQT_SLOT(slotMacSwapClicked()) );
	pLayoutTop->addWidget( m_pchkMacSwap, 2, 0 );

	//------------------
	pLayoutTop->addRowSpacing( 3, KDialog::spacingHint() * 3 );

	pGroup = new TQGroupBox( 1, Qt::Horizontal, i18n("X Modifier Mapping"), this );
	pLayoutTop->addWidget( pGroup, 4, 0 );

	m_plstXMods = new TDEListView( pGroup );
	m_plstXMods->setSorting( -1 );
	m_plstXMods->setSelectionMode( TQListView::NoSelection );
	m_plstXMods->setAllColumnsShowFocus( true );
	m_plstXMods->addColumn( i18n("X11-Mod") );

	new TDEListViewItem( m_plstXMods, "mod5" );
	new TDEListViewItem( m_plstXMods, "mod4" );
	new TDEListViewItem( m_plstXMods, "mod3" );
	new TDEListViewItem( m_plstXMods, "mod2" );
	new TDEListViewItem( m_plstXMods, "mod1" );
	new TDEListViewItem( m_plstXMods, "control" );
	new TDEListViewItem( m_plstXMods, "lock" );
	new TDEListViewItem( m_plstXMods, "shift" );

	//------------------
	pLayoutTop->setRowStretch( 5, 1 );

	updateWidgets();
}

/*KComboBox* ModifiersModule::newModXComboBox( TQWidget* parent )
{
	KComboBox* pcb = new KComboBox( parent );
	pcb->insertItem( "" );
	pcb->insertItem( "mod2" );
	pcb->insertItem( "mod3" );
	pcb->insertItem( "mod4" );
	pcb->insertItem( "mod5" );
	return pcb;
}*/

void ModifiersModule::updateWidgetData()
{
	m_plblCtrl->setText( m_sLabelCtrlOrig );
	m_plblAlt->setText( m_sLabelAltOrig );
	m_plblWin->setText( m_sLabelWinOrig );
	m_pchkMacKeyboard->setChecked( m_bMacKeyboardOrig );
	m_pchkMacSwap->setChecked( m_bMacSwapOrig );
        m_pchkMacSwap->setEnabled( m_bMacKeyboardOrig );
}

void ModifiersModule::updateWidgets()
{
	if( m_pchkMacKeyboard->isChecked() ) {
		// If keys are swapped around to reflect MacOS norms:
		if( m_pchkMacSwap->isChecked() ) {
			m_plblCtrl->setText( i18n("Command") ); // Ctrl in Alt's place
			m_plblAlt->setText( i18n("Option") );   // Alt in Win's place
			m_plblWin->setText( i18n("Control") );  // Win in Ctrl's place
		} else {
			m_plblCtrl->setText( i18n("Control") ); // Ctrl labeled Control
			m_plblAlt->setText( i18n("Option") );  // Alt labeled Command
			m_plblWin->setText( i18n("Command") );   // Win labeled Option
		}
		m_pchkMacSwap->setEnabled( true );
	} else {
		m_plblCtrl->setText( i18n(TQACCEL_OBJECT_NAME_STRING, "Ctrl") );
		m_plblAlt->setText( i18n(TQACCEL_OBJECT_NAME_STRING, "Alt") );
		m_plblWin->setText( i18n("Win") );
		m_pchkMacSwap->setEnabled( false );
	}

	XModifierKeymap* xmk = XGetModifierMapping( tqt_xdisplay() );

	for( int iKey = m_plstXMods->columns()-1; iKey < xmk->max_keypermod; iKey++ )
		m_plstXMods->addColumn( i18n("Key %1").arg(iKey+1) );

	//int iModWinDef = -1;
	for( int iMod = 0; iMod < 8; iMod++ ) {
		// Find the default modifier index for the Win key.
		/*if( iMod > Mod2Index ) {
			uint symX = XkbKeycodeToKeysym( tqt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * iMod], 0, 0 );
			if( symX == XK_Super_L || symX == XK_Super_R )
				iModWinDef = iMod;
			else if( iModWinDef == -1 && (symX == XK_Meta_L || symX == XK_Meta_R) )
				iModWinDef = iMod;
		}*/

		// Insert items into X modifier map list
		for( int iKey = 0; iKey < xmk->max_keypermod; iKey++ ) {
			uint symX = XkbKeycodeToKeysym( tqt_xdisplay(), xmk->modifiermap[xmk->max_keypermod * iMod + iKey], 0, 0 );
			m_plstXMods->itemAtIndex( iMod )->setText( 1 + iKey, XKeysymToString( symX ) );
		}
	}

	XFreeModifiermap( xmk );

	int i;
	switch( KKeyNative::modX(KKey::WIN) ) {
		case Mod2Mask: i = 2; break;
		case Mod3Mask: i = 3; break;
		case Mod4Mask: i = 4; break;
		case Mod5Mask: i = 5; break;
		default:       i = 0;
	}
	if( i != 0 )
		m_plblWinModX->setText( "mod" + TQString::number(i) );
	else
		m_plblWinModX->setText( "<" + i18n("None") + ">" );
}

void ModifiersModule::slotMacKeyboardClicked()
{
	updateWidgets();
	emit changed( true );
}

void ModifiersModule::slotMacSwapClicked()
{
	if( m_pchkMacKeyboard->isChecked() && !KKeyNative::keyboardHasWinKey() ) {
		KMessageBox::sorry( this,
			i18n("You can only activate this option if your "
			"X keyboard layout has the 'Super' or 'Meta' keys "
			"properly configured as modifier keys."),
			"Incompatibility" );
		m_pchkMacSwap->setChecked( false );
	} else {
		updateWidgets();
		emit changed( true );
	}
}

#include "modifiers.moc"
