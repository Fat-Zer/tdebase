#ifndef X11HELPER_H_
#define X11HELPER_H_

#include <tqdict.h>
#include <tqstringlist.h>


struct RulesInfo {
	TQDict<char> models;
	TQDict<char> tqlayouts;
	TQDict<char> options;
};

struct OldLayouts {
	TQStringList oldLayouts;
	TQStringList nonLatinLayouts;
};

class X11Helper
{
	static bool m_tqlayoutsClean;

public:
	static const WId UNKNOWN_WINDOW_ID = (WId) 0;
	static const TQString X11_WIN_CLASS_ROOT;
	static const TQString X11_WIN_CLASS_UNKNOWN;
	/**
	 * Tries to find X11 xkb config dir
	 */
	static const TQString findX11Dir();
	static const TQString findXkbRulesFile(TQString x11Dir, Display* dpy);
	static TQString getWindowClass(WId winId, Display* dpy);
	static TQStringList* getVariants(const TQString& tqlayout, const TQString& x11Dir, bool oldLayouts=false);
	static RulesInfo* loadRules(const TQString& rulesFile, bool tqlayoutsOnly=false);
	static OldLayouts* loadOldLayouts(const TQString& rulesFile);
	
	static bool areLayoutsClean() { return m_tqlayoutsClean; }
	static bool areSingleGroupsSupported();
};

#endif /*X11HELPER_H_*/
