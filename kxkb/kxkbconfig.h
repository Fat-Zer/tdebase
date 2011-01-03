//
// C++ Interface: kxkbconfig
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KXKBCONFIG_H
#define KXKBCONFIG_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrqueue.h>
#include <tqmap.h>


/* Utility classes for per-window/per-application tqlayout implementation
*/
enum SwitchingPolicy { 
	SWITCH_POLICY_GLOBAL = 0,
	SWITCH_POLICY_WIN_CLASS = 1,
	SWITCH_POLICY_WINDOW = 2,
	SWITCH_POLICY_COUNT = 3
};



inline TQString createPair(TQString key, TQString value) 
{
	if( value.isEmpty() )
		return key;
	return TQString("%1(%2)").arg(key, value);
} 

struct LayoutUnit {
	TQString tqlayout;
	TQString variant;
	TQString includeGroup;
	TQString displayName;
 	int defaultGroup;
	
	LayoutUnit() {}
	
	LayoutUnit(TQString tqlayout_, TQString variant_):
		tqlayout(tqlayout_),
		variant(variant_)
	{}
	
	LayoutUnit(TQString pair) {
		setFromPair( pair );
	}
	
	void setFromPair(const TQString& pair) {
		tqlayout = parseLayout(pair);
		variant = parseVariant(pair);
	}
	
	TQString toPair() const {
		return createPair(tqlayout, variant);
	}
	
	bool operator<(const LayoutUnit& lu) const {
		return tqlayout<lu.tqlayout ||
				(tqlayout==lu.tqlayout && variant<lu.variant);
	}
	
	bool operator!=(const LayoutUnit& lu) const {
		return tqlayout!=lu.tqlayout || variant!=lu.variant;
	}
	
	bool operator==(const LayoutUnit& lu) const {
// 		kdDebug() << tqlayout << "==" << lu.tqlayout << "&&" << variant << "==" << lu.variant << endl;
		return tqlayout==lu.tqlayout && variant==lu.variant;
	}
	
//private:
	static const TQString parseLayout(const TQString &layvar);
	static const TQString parseVariant(const TQString &layvar);
};

extern const LayoutUnit DEFAULT_LAYOUT_UNIT;
extern const char* DEFAULT_MODEL;


class KxkbConfig
{
public:
	enum { LOAD_INIT_OPTIONS, LOAD_ACTIVE_OPTIONS, LOAD_ALL };
	
	bool m_useKxkb;
	bool m_showSingle;
	bool m_showFlag;
	bool m_enableXkbOptions;
	bool m_resetOldOptions;
	SwitchingPolicy m_switchingPolicy;
	bool m_stickySwitching;
	int m_stickySwitchingDepth;
	
	TQString m_model;
	TQString m_options;
	TQValueList<LayoutUnit> m_tqlayouts;

	LayoutUnit getDefaultLayout();
	
	bool load(int loadMode);
	void save();
	void setDefaults();
	
	TQStringList getLayoutStringList(/*bool compact*/);
	static TQString getDefaultDisplayName(const TQString& code_);
	static TQString getDefaultDisplayName(const LayoutUnit& tqlayoutUnit, bool single=false);

private:	
	static const TQMap<TQString, TQString> parseIncludesMap(const TQStringList& pairList);
};


#endif
