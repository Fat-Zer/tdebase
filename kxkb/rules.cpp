#include <tqwindowdefs.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqstringlist.h>
#include <tqdir.h>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <config.h>

#include "x11helper.h"
#include "rules.h"


XkbRules::XkbRules(bool layoutsOnly):
    m_layouts(90)
{
	X11_DIR = X11Helper::findX11Dir();

   	if( X11_DIR == NULL ) {
        kdError() << "Cannot find X11 directory!" << endl;
//        throw Exception();
		return;
   	}

	TQString rulesFile = X11Helper::findXkbRulesFile(X11_DIR, qt_xdisplay());
	
	if( rulesFile.isEmpty() ) {
  		kdError() << "Cannot find rules file in " << X11_DIR << endl;
//		throw Exception();
		return;
	}

	loadRules(rulesFile, layoutsOnly);
	loadOldLayouts(rulesFile);
	loadGroups(::locate("config", "kxkb_groups"));
}


void XkbRules::loadRules(TQString file, bool layoutsOnly)
{
	RulesInfo* rules = X11Helper::loadRules(file, layoutsOnly);

	if (rules == NULL) {
		kdDebug() << "Unable to load rules" << endl;
		return;
	}

	m_layouts= rules->layouts;
	if( layoutsOnly == false ) {
		m_models = rules->models;
		m_options = rules->options;
	}

	//  fixLayouts();
}

// void XkbRules::fixLayouts() {
// // THIS IS TEMPORARY!!!
// // This should be fixed in XFree86 (and actually is fixed in XFree 4.2)
// // some handcoded ones, because the X11 rule file doesn't get them correctly, or in case
// // the rule file wasn't found
// 	static struct {
// 		const char * locale;
// 		const char * layout;
// 	} fixedLayouts[] = {
// 		{ "ben", "Bengali" },
// 		{ "ar", "Arabic" },
// 		{ "ir", "Farsi" },
// 		{ 0, 0 }
// 	};
// 	
// 	for(int i=0; fixedLayouts[i].layout != 0; i++ ) {
// 		if( m_layouts.tqfind(fixedLayouts[i].locale) == 0 )
// 			m_layouts.insert(fixedLayouts[i].locale, fixedLayouts[i].layout);
// 	}
// }

bool XkbRules::isSingleGroup(const TQString& layout)
{
	  return X11Helper::areSingleGroupsSupported()
			  && !m_oldLayouts.tqcontains(layout)
			  && !m_nonLatinLayouts.tqcontains(layout);
}


// check $oldlayouts and $nonlatin groups for XFree 4.3 and later
void XkbRules::loadOldLayouts(TQString rulesFile)
{
	OldLayouts* oldLayoutsStruct = X11Helper::loadOldLayouts( rulesFile );
	m_oldLayouts = oldLayoutsStruct->oldLayouts;
	m_nonLatinLayouts = oldLayoutsStruct->nonLatinLayouts;
}

// for multi-group layouts in XFree 4.2 and older
//    or if layout is present in $oldlayout or $nonlatin groups
void XkbRules::loadGroups(TQString file)
{
  TQFile f(file);
  if (f.open(IO_ReadOnly))
    {
      TQTextStream ts(&f);
      TQString locale;
      unsigned int grp;

      while (!ts.eof()) {
         ts >> locale >> grp;
	 locale.simplifyWhiteSpace();

	 if (locale[0] == '#' || locale.left(2) == "//" || locale.isEmpty())
	    continue;

    	 m_initialGroups.insert(locale, grp);
      }
      
      f.close();
    }
}

unsigned int 
XkbRules::getDefaultGroup(const TQString& layout, const TQString& includeGroup)
{
// check for new one-group layouts in XFree 4.3 and older
    if( isSingleGroup(layout) ) {
		if( includeGroup.isEmpty() == false )
			return 1;
		else
			return 0;
    }
    
    TQMap<TQString, unsigned int>::iterator it = m_initialGroups.tqfind(layout);
    return it == m_initialGroups.end() ? 0 : it.data();
}


TQStringList
XkbRules::getAvailableVariants(const TQString& layout)
{
    if( layout.isEmpty() || !layouts().tqfind(layout) )
	return TQStringList();

    TQStringList* result1 = m_varLists[layout];
    if( result1 )
        return *result1;

    bool oldLayouts = m_oldLayouts.tqcontains(layout);
    TQStringList* result = X11Helper::getVariants(layout, X11_DIR, oldLayouts);

    m_varLists.insert(layout, result);

    return *result;
}

