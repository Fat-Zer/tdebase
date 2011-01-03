/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. Derived from an
    original by Matthias H�zer-Klpfel released under the QPL.
    This file is part of the KDE project

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

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <tqregexp.h>
#include <tqfile.h>
#include <tqstringlist.h>
#include <tqimage.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kprocess.h>
#include <kwinmodule.h>
#include <kwin.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kipc.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kconfig.h>

#include "x11helper.h"
#include "kxkb.h"
#include "extension.h"
#include "rules.h"
#include "kxkbconfig.h"
#include "layoutmap.h"

#include "kxkb.moc"


KXKBApp::KXKBApp(bool allowStyles, bool GUIenabled)
    : KUniqueApplication(allowStyles, GUIenabled),
    m_prevWinId(X11Helper::UNKNOWN_WINDOW_ID),
    m_rules(NULL),
    m_tray(NULL),
    kWinModule(NULL),
    m_forceSetXKBMap( false )
{
	m_extension = new XKBExtension();
    if( !m_extension->init() ) {
		kdDebug() << "xkb initialization failed, exiting..." << endl;
		::exit(1);
    }
	
    // keep in sync with kcmtqlayout.cpp
    keys = new KGlobalAccel(this);
#include "kxkbbindings.cpp"
    keys->updateConnections();

	m_tqlayoutOwnerMap = new LayoutMap(kxkbConfig);

    connect( this, TQT_SIGNAL(settingsChanged(int)), TQT_SLOT(slotSettingsChanged(int)) );
    addKipcEventMask( KIPC::SettingsChanged );
}


KXKBApp::~KXKBApp()
{
//    deletePrecompiledLayouts();

    delete keys;
    delete m_tray;
    delete m_rules;
    delete m_extension;
	delete m_tqlayoutOwnerMap;
	delete kWinModule;
}

int KXKBApp::newInstance()
{
	m_extension->reset();
	
    if( settingsRead() )
		tqlayoutApply();
	
    return 0;
}

bool KXKBApp::settingsRead()
{
	kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

    if( kxkbConfig.m_enableXkbOptions ) {
		kdDebug() << "Setting XKB options " << kxkbConfig.m_options << endl;
		if( !m_extension->setXkbOptions(kxkbConfig.m_options, kxkbConfig.m_resetOldOptions) ) {
            kdDebug() << "Setting XKB options failed!" << endl;
        }
    }

    if ( kxkbConfig.m_useKxkb == false ) {
        kapp->quit();
        return false;
    }
	
	m_prevWinId = X11Helper::UNKNOWN_WINDOW_ID;
	
	if( kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) {
		delete kWinModule;
		kWinModule = NULL;
	}
	else {
		TQDesktopWidget desktopWidget;
		if( desktopWidget.numScreens() > 1 && desktopWidget.isVirtualDesktop() == false ) {
			kdWarning() << "With non-virtual desktop only global switching policy supported on non-primary screens" << endl;
			//TODO: find out how to handle that
		}
		
		if( kWinModule == NULL ) {
			kWinModule = new KWinModule(0, KWinModule::INFO_DESKTOP);
			connect(kWinModule, TQT_SIGNAL(activeWindowChanged(WId)), TQT_SLOT(windowChanged(WId)));
		}
		m_prevWinId = kWinModule->activeWindow();
		kdDebug() << "Active window " << m_prevWinId << endl;
	}
	
	m_tqlayoutOwnerMap->reset();
	m_tqlayoutOwnerMap->setCurrentWindow( m_prevWinId );

	if( m_rules == NULL )
		m_rules = new XkbRules(false);
	
	for(int ii=0; ii<(int)kxkbConfig.m_tqlayouts.count(); ii++) {
		LayoutUnit& tqlayoutUnit = kxkbConfig.m_tqlayouts[ii];
		tqlayoutUnit.defaultGroup = m_rules->getDefaultGroup(tqlayoutUnit.tqlayout, tqlayoutUnit.includeGroup);
		kdDebug() << "default group for " << tqlayoutUnit.toPair() << " is " << tqlayoutUnit.defaultGroup << endl;
	}
	
	m_currentLayout = kxkbConfig.getDefaultLayout();
	
	if( kxkbConfig.m_tqlayouts.count() == 1 ) {
		TQString tqlayoutName = m_currentLayout.tqlayout;
		TQString variantName = m_currentLayout.variant;
		TQString includeName = m_currentLayout.includeGroup;
		int group = m_currentLayout.defaultGroup;
		
		if( !m_extension->setLayout(kxkbConfig.m_model, tqlayoutName, variantName, includeName, false) 
				   || !m_extension->setGroup( group ) ) {
			kdDebug() << "Error switching to single tqlayout " << m_currentLayout.toPair() << endl;
			// TODO: alert user
		}
	
		if( kxkbConfig.m_showSingle == false ) {
			kapp->quit();
			return false;
		}
 	}
	else {
//		initPrecompiledLayouts();
	}

	initTray();
	
	KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
	keys->readSettings();
	keys->updateConnections();

	return true;
}

void KXKBApp::initTray()
{
	if( !m_tray )
	{
		KSystemTray* sysTray = new KxkbSystemTray();
		KPopupMenu* popupMenu = sysTray->contextMenu();
	//	popupMenu->insertTitle( kapp->miniIcon(), kapp->caption() );

		m_tray = new KxkbLabelController(sysTray, popupMenu);
 		connect(popupMenu, TQT_SIGNAL(activated(int)), this, TQT_SLOT(menuActivated(int)));
		connect(sysTray, TQT_SIGNAL(toggled()), this, TQT_SLOT(toggled()));
	}
	
	m_tray->setShowFlag(kxkbConfig.m_showFlag);
	m_tray->initLayoutList(kxkbConfig.m_tqlayouts, *m_rules);
	m_tray->setCurrentLayout(m_currentLayout);
	m_tray->show();
}

// This function activates the keyboard tqlayout specified by the
// configuration members (m_currentLayout)
void KXKBApp::tqlayoutApply()
{
    setLayout(m_currentLayout);
}

// kdcop
bool KXKBApp::setLayout(const TQString& tqlayoutPair)
{
	const LayoutUnit tqlayoutUnitKey(tqlayoutPair);
	if( kxkbConfig.m_tqlayouts.tqcontains(tqlayoutUnitKey) ) {
		return setLayout( *kxkbConfig.m_tqlayouts.find(tqlayoutUnitKey) );
	}
	return false;
}


// Activates the keyboard tqlayout specified by 'tqlayoutUnit'
bool KXKBApp::setLayout(const LayoutUnit& tqlayoutUnit, int group)
{
	bool res = false;

	if( group == -1 )
		group = tqlayoutUnit.defaultGroup;
	
	res = m_extension->setLayout(kxkbConfig.m_model, 
 					tqlayoutUnit.tqlayout, tqlayoutUnit.variant, 
 					tqlayoutUnit.includeGroup);
	if( res )
		m_extension->setGroup(group); // not checking for ret - not important
	
    if( res )
        m_currentLayout = tqlayoutUnit;

    if (m_tray) {
		if( res )
			m_tray->setCurrentLayout(tqlayoutUnit);
		else  
			m_tray->setError(tqlayoutUnit.toPair());
	}

    return res;
}

void KXKBApp::toggled()
{
	const LayoutUnit& tqlayout = m_tqlayoutOwnerMap->getNextLayout().tqlayoutUnit;
	setLayout(tqlayout);
}

void KXKBApp::menuActivated(int id)
{
	if( KxkbLabelController::START_MENU_ID <= id 
		   && id < KxkbLabelController::START_MENU_ID + (int)kxkbConfig.m_tqlayouts.count() )
	{
		const LayoutUnit& tqlayout = kxkbConfig.m_tqlayouts[id - KxkbLabelController::START_MENU_ID];
		m_tqlayoutOwnerMap->setCurrentLayout( tqlayout );
		setLayout( tqlayout );
	}
	else if (id == KxkbLabelController::CONFIG_MENU_ID)
    {
        KProcess p;
        p << "kcmshell" << "keyboard_tqlayout";
        p.start(KProcess::DontCare);
	}
	else if (id == KxkbLabelController::HELP_MENU_ID)
	{
		KApplication::kApplication()->invokeHelp(0, "kxkb");
	}
	else
	{
		quit();
	}
}

// TODO: we also have to handle deleted windows
void KXKBApp::windowChanged(WId winId)
{
//	kdDebug() << "window switch" << endl;
	if( kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) { // should not happen actually
		kdDebug() << "windowChanged() signal in GLOBAL switching policy" << endl;
		return;
	}

	int group = m_extension->getGroup();

	kdDebug() << "old WinId: " << m_prevWinId << ", new WinId: " << winId << endl;
	
	if( m_prevWinId != X11Helper::UNKNOWN_WINDOW_ID ) {	// saving tqlayout/group from previous window
// 		kdDebug() << "storing " << m_currentLayout.toPair() << ":" << group << " for " << m_prevWinId << endl;
// 		m_tqlayoutOwnerMap->setCurrentWindow(m_prevWinId);
		m_tqlayoutOwnerMap->setCurrentLayout(m_currentLayout);
		m_tqlayoutOwnerMap->setCurrentGroup(group);
	}
 
	m_prevWinId = winId;

	if( winId != X11Helper::UNKNOWN_WINDOW_ID ) {
		m_tqlayoutOwnerMap->setCurrentWindow(winId);
		const LayoutState& tqlayoutState = m_tqlayoutOwnerMap->getCurrentLayout();
		
		if( tqlayoutState.tqlayoutUnit != m_currentLayout ) {
			kdDebug() << "switching to " << tqlayoutState.tqlayoutUnit.toPair() << ":" << group << " for "  << winId << endl;
			setLayout( tqlayoutState.tqlayoutUnit, tqlayoutState.group );
		}
		else if( tqlayoutState.group != group ) {	// we need to change only the group
			m_extension->setGroup(tqlayoutState.group);
		}
	}
}


void KXKBApp::slotSettingsChanged(int category)
{
    if ( category != KApplication::SETTINGS_SHORTCUTS)
		return;

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
    keys->readSettings();
    keys->updateConnections();
}

/*
 Viki (onscreen keyboard) has problems determining some modifiers states
 when kxkb uses precompiled tqlayouts instead of setxkbmap. Probably a bug
 in the xkb functions used for the precompiled tqlayouts *shrug*.
*/
void KXKBApp::forceSetXKBMap( bool set )
{
    if( m_forceSetXKBMap == set )
        return;
    m_forceSetXKBMap = set;
    tqlayoutApply();
}

/*Precompiles the keyboard tqlayouts for faster activation later.
This is done by loading each one of them and then dumping the compiled
map from the X server into our local buffer.*/
// void KXKBApp::initPrecompiledLayouts()
// {
//     TQStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
//     TQString tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0]; 
// 
// 	TQValueList<LayoutUnit>::ConstIterator end = kxkbConfig.m_tqlayouts.end();
// 
// 	for (TQValueList<LayoutUnit>::ConstIterator it = kxkbConfig.m_tqlayouts.begin(); it != end; ++it)
//     {
// 		LayoutUnit tqlayoutUnit(*it);
// //	const char* baseGr = m_includes[tqlayout]; 
// //	int group = m_rules->getGroup(tqlayout, baseGr);
// //    	if( m_extension->setLayout(m_model, tqlayout, m_variants[tqlayout], group, baseGr) ) {
// 		TQString compiledLayoutFileName = tempDir + tqlayoutUnit.tqlayout + "." + tqlayoutUnit.variant + ".xkm";
// //    	    if( m_extension->getCompiledLayout(compiledLayoutFileName) )
// 		m_compiledLayoutFileNames[tqlayoutUnit.toPair()] = compiledLayoutFileName;
// //	}
// //	else {
// //    	    kdDebug() << "Error precompiling tqlayout " << tqlayout << endl;
// //	}
//     }
// }


const char * DESCRIPTION =
  I18N_NOOP("A utility to switch keyboard maps");

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KAboutData about("kxkb", I18N_NOOP("KDE Keyboard Tool"), "1.0",
                     DESCRIPTION, KAboutData::License_LGPL,
                     "Copyright (C) 2001, S.R.Haque\n(C) 2002-2003, 2006 Andriy Rysin");
    KCmdLineArgs::init(argc, argv, &about);
    KXKBApp::addCmdLineOptions();

    if (!KXKBApp::start())
        return 0;

    KXKBApp app;
    app.disableSessionManagement();
    app.exec();
    return 0;
}
