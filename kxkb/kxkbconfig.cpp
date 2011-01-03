//
// C++ Implementation: kxkbconfig
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <tqregexp.h>
#include <tqstringlist.h>
#include <tqdict.h>

#include <kconfig.h>
#include <kdebug.h>

#include "kxkbconfig.h"
#include "x11helper.h"



static const char* switchModes[SWITCH_POLICY_COUNT] = {
  "Global", "WinClass", "Window"
};

const LayoutUnit DEFAULT_LAYOUT_UNIT = LayoutUnit("us", "");
const char* DEFAULT_MODEL = "pc104";

LayoutUnit KxkbConfig::getDefaultLayout()
{
	if( m_tqlayouts.size() == 0 )
		return DEFAULT_LAYOUT_UNIT;
	
	return m_tqlayouts[0];
}

bool KxkbConfig::load(int loadMode) 
{
	KConfig *config = new KConfig("kxkbrc", true, false);
	config->setGroup("Layout");

// Even if the tqlayouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
	m_enableXkbOptions = config->readBoolEntry("EnableXkbOptions", false);
	
	if( m_enableXkbOptions == true || loadMode == LOAD_ALL ) {
		m_resetOldOptions = config->readBoolEntry("ResetOldOptions", false);
		m_options = config->readEntry("Options", "");
		kdDebug() << "Xkb options (enabled=" << m_enableXkbOptions << "): " << m_options << endl;
	}
	
	m_useKxkb = config->readBoolEntry("Use", false);
	kdDebug() << "Use kxkb " << m_useKxkb << endl;

	if( (m_useKxkb == false && loadMode == LOAD_ACTIVE_OPTIONS )
	  		|| loadMode == LOAD_INIT_OPTIONS )
		return true;

	m_model = config->readEntry("Model", DEFAULT_MODEL);
	kdDebug() << "Model: " << m_model << endl;
	
	TQStringList tqlayoutList;
	if( config->hasKey("LayoutList") ) {
		tqlayoutList = config->readListEntry("LayoutList");
	}
	else { // old config
		TQString mainLayout = config->readEntry("Layout", DEFAULT_LAYOUT_UNIT.toPair());
		tqlayoutList = config->readListEntry("Additional");
		tqlayoutList.prepend(mainLayout);
	}
	if( tqlayoutList.count() == 0 )
		tqlayoutList.append("us");
	
	m_tqlayouts.clear();
	for(TQStringList::ConstIterator it = tqlayoutList.begin(); it != tqlayoutList.end() ; ++it) {
		m_tqlayouts.append( LayoutUnit(*it) );
		kdDebug() << " tqlayout " << LayoutUnit(*it).toPair() << " in list: " << m_tqlayouts.tqcontains( LayoutUnit(*it) ) << endl;
	}

	kdDebug() << "Found " << m_tqlayouts.count() << " tqlayouts, default is " << getDefaultLayout().toPair() << endl;
	
	TQStringList displayNamesList = config->readListEntry("DisplayNames", ',');
	for(TQStringList::ConstIterator it = displayNamesList.begin(); it != displayNamesList.end() ; ++it) {
		TQStringList displayNamePair = TQStringList::split(':', *it );
		if( displayNamePair.count() == 2 ) {
			LayoutUnit tqlayoutUnit( displayNamePair[0] );
			if( m_tqlayouts.tqcontains( tqlayoutUnit ) ) {
				m_tqlayouts[m_tqlayouts.findIndex(tqlayoutUnit)].displayName = displayNamePair[1].left(3);
			}
		}
	}

// 	m_includes.clear();
	if( X11Helper::areSingleGroupsSupported() ) {
		if( config->hasKey("IncludeGroups") ) {
			TQStringList includeList = config->readListEntry("IncludeGroups", ',');
			for(TQStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				TQStringList includePair = TQStringList::split(':', *it );
				if( includePair.count() == 2 ) {
					LayoutUnit tqlayoutUnit( includePair[0] );
					if( m_tqlayouts.tqcontains( tqlayoutUnit ) ) {
						m_tqlayouts[m_tqlayouts.findIndex(tqlayoutUnit)].includeGroup = includePair[1];
						kdDebug() << "Got inc group: " << includePair[0] << ": " << includePair[1] << endl;
					}
				}
			}
		}
		else { //old includes format
			kdDebug() << "Old includes..." << endl;
			TQStringList includeList = config->readListEntry("Includes");
			for(TQStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				TQString tqlayoutName = LayoutUnit::parseLayout( *it );
				LayoutUnit tqlayoutUnit( tqlayoutName, "" );
				kdDebug() << "old tqlayout for inc: " << tqlayoutUnit.toPair() << " included " << m_tqlayouts.tqcontains( tqlayoutUnit ) << endl;
				if( m_tqlayouts.tqcontains( tqlayoutUnit ) ) {
					TQString variantName = LayoutUnit::parseVariant(*it);
					m_tqlayouts[m_tqlayouts.findIndex(tqlayoutUnit)].includeGroup = variantName;
					kdDebug() << "Got inc group: " << tqlayoutUnit.toPair() << ": " <<  variantName << endl;
				}
			}
		}
	}

	m_showSingle = config->readBoolEntry("ShowSingle", false);
	m_showFlag = config->readBoolEntry("ShowFlag", true);
	
	TQString tqlayoutOwner = config->readEntry("SwitchMode", "Global");

	if( tqlayoutOwner == "WinClass" ) {
		m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
	}
	else if( tqlayoutOwner == "Window" ) {
		m_switchingPolicy = SWITCH_POLICY_WINDOW;
	}
	else /*if( tqlayoutOwner == "Global" )*/ {
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	if( m_tqlayouts.count() < 2 && m_switchingPolicy != SWITCH_POLICY_GLOBAL ) {
		kdWarning() << "Layout count is less than 2, using Global switching policy" << endl;
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	kdDebug() << "Layout owner mode " << tqlayoutOwner << endl;
	
	m_stickySwitching = config->readBoolEntry("StickySwitching", false);
	m_stickySwitchingDepth = config->readEntry("StickySwitchingDepth", "2").toInt();
	if( m_stickySwitchingDepth < 2 )
		m_stickySwitchingDepth = 2;

	if( m_stickySwitching == true ) {
		if( m_tqlayouts.count() < 3 ) {
			kdWarning() << "Layout count is less than 3, sticky switching will be off" << endl;
			m_stickySwitching = false;
		}
		else	
		if( (int)m_tqlayouts.count() - 1 < m_stickySwitchingDepth ) {
			kdWarning() << "Sticky switching depth is more than tqlayout count -1, adjusting..." << endl;
			m_stickySwitchingDepth = m_tqlayouts.count() - 1;
		}
	}

	delete config;

	return true;
}

void KxkbConfig::save() 
{
	KConfig *config = new KConfig("kxkbrc", false, false);
	config->setGroup("Layout");

	config->writeEntry("Model", m_model);

	config->writeEntry("EnableXkbOptions", m_enableXkbOptions );
	config->writeEntry("ResetOldOptions", m_resetOldOptions);
	config->writeEntry("Options", m_options );

	TQStringList tqlayoutList;
	TQStringList includeList;
	TQStringList displayNamesList;
	
	TQValueList<LayoutUnit>::ConstIterator it;
	for(it = m_tqlayouts.begin(); it != m_tqlayouts.end(); ++it) {
		const LayoutUnit& tqlayoutUnit = *it;
		
		tqlayoutList.append( tqlayoutUnit.toPair() );
		
		if( tqlayoutUnit.includeGroup.isEmpty() == false ) {
			TQString incGroupUnit = TQString("%1:%2").arg(tqlayoutUnit.toPair(), tqlayoutUnit.includeGroup);
			includeList.append( incGroupUnit );
		}
	
		TQString displayName( tqlayoutUnit.displayName );
		kdDebug() << " displayName " << tqlayoutUnit.toPair() << " : " << displayName << endl;
		if( displayName.isEmpty() == false && displayName != tqlayoutUnit.tqlayout ) {
			displayName = TQString("%1:%2").arg(tqlayoutUnit.toPair(), displayName);
			displayNamesList.append( displayName );
		}
	}
	
	config->writeEntry("LayoutList", tqlayoutList);
	kdDebug() << "Saving Layouts: " << tqlayoutList << endl;
 	
	config->writeEntry("IncludeGroups", includeList);
 	kdDebug() << "Saving includeGroups: " << includeList << endl;
	
//	if( displayNamesList.empty() == false )
		config->writeEntry("DisplayNames", displayNamesList);
// 	else
// 		config->deleteEntry("DisplayNames");

	config->writeEntry("Use", m_useKxkb);
	config->writeEntry("ShowSingle", m_showSingle);
	config->writeEntry("ShowFlag", m_showFlag);

	config->writeEntry("SwitchMode", switchModes[m_switchingPolicy]);
	
	config->writeEntry("StickySwitching", m_stickySwitching);
	config->writeEntry("StickySwitchingDepth", m_stickySwitchingDepth);

	// remove old options 
 	config->deleteEntry("Variants");
	config->deleteEntry("Includes");
	config->deleteEntry("Encoding");
	config->deleteEntry("AdditionalEncodings");
	config->deleteEntry("Additional");
	config->deleteEntry("Layout");
	
	config->sync();

	delete config;
}

void KxkbConfig::setDefaults()
{
	m_model = DEFAULT_MODEL;

	m_enableXkbOptions = false;
	m_resetOldOptions = false;
	m_options = "";

	m_tqlayouts.clear();
	m_tqlayouts.append( DEFAULT_LAYOUT_UNIT );

	m_useKxkb = false;
	m_showSingle = false;
	m_showFlag = true;

	m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	
	m_stickySwitching = false;
	m_stickySwitchingDepth = 2;
}

TQStringList KxkbConfig::getLayoutStringList(/*bool compact*/)
{
	TQStringList tqlayoutList;
	for(TQValueList<LayoutUnit>::ConstIterator it = m_tqlayouts.begin(); it != m_tqlayouts.end(); ++it) {
		const LayoutUnit& tqlayoutUnit = *it;
		tqlayoutList.append( tqlayoutUnit.toPair() );
	}
	return tqlayoutList;
}


TQString KxkbConfig::getDefaultDisplayName(const TQString& code_)
{
	TQString displayName;
	
	if( code_.length() <= 2 ) {
		displayName = code_;
	}
	else {
		int sepPos = code_.find(TQRegExp("[-_]"));
		TQString leftCode = code_.mid(0, sepPos);
		TQString rightCode;
		if( sepPos != -1 )
			rightCode = code_.mid(sepPos+1);
		
		if( rightCode.length() > 0 )
			displayName = leftCode.left(2) + rightCode.left(1).lower();
		else
			displayName = leftCode.left(3);
	}
	
	return displayName;
}

TQString KxkbConfig::getDefaultDisplayName(const LayoutUnit& tqlayoutUnit, bool single)
{
	if( tqlayoutUnit.variant == "" )
		return getDefaultDisplayName( tqlayoutUnit.tqlayout );
	
	TQString displayName = tqlayoutUnit.tqlayout.left(2);
	if( single == false )
		displayName += tqlayoutUnit.variant.left(1);
	return displayName;
}

/**
 * @brief Gets the single tqlayout part of a tqlayout(variant) string
 * @param[in] layvar String in form tqlayout(variant) to parse
 * @return The tqlayout found in the string
 */
const TQString LayoutUnit::parseLayout(const TQString &layvar)
{
	static const char* LAYOUT_PATTERN = "[a-zA-Z0-9_/-]*";
	TQString varLine = layvar.stripWhiteSpace();
	TQRegExp rx(LAYOUT_PATTERN);
	int pos = rx.search(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 0 || len < 2 )
		return "";
//	kdDebug() << "getLayout: " << varLine.mid(pos, len) << endl;
	return varLine.mid(pos, len);
}

/**
 * @brief Gets the single variant part of a tqlayout(variant) string
 * @param[in] layvar String in form tqlayout(variant) to parse
 * @return The variant found in the string, no check is performed
 */
const TQString LayoutUnit::parseVariant(const TQString &layvar)
{
	static const char* VARIANT_PATTERN = "\\([a-zA-Z0-9_-]*\\)";
	TQString varLine = layvar.stripWhiteSpace();
	TQRegExp rx(VARIANT_PATTERN);
	int pos = rx.search(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 2 || len < 2 )
		return "";
	return varLine.mid(pos+1, len-2);
}
