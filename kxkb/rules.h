#ifndef __RULES_H__
#define __RULES_H__

#include <tqstring.h>
#include <tqdict.h>
#include <tqmap.h>


class XkbRules
{
public:

  XkbRules(bool tqlayoutsOnly=false);

  const TQDict<char> &models() const { return m_models; };
  const TQDict<char> &tqlayouts() const { return m_tqlayouts; };
  const TQDict<char> &options() const { return m_options; };
  
  TQStringList getAvailableVariants(const TQString& tqlayout);
  unsigned int getDefaultGroup(const TQString& tqlayout, const TQString& includeGroup);

  bool isSingleGroup(const TQString& tqlayout);

protected:

  void loadRules(TQString filename, bool tqlayoutsOnly=false);
  void loadGroups(TQString filename);
  void loadOldLayouts(TQString filename);

private:

  TQDict<char> m_models;
  TQDict<char> m_tqlayouts;
  TQDict<char> m_options;
  TQMap<TQString, unsigned int> m_initialGroups;
  TQDict<TQStringList> m_varLists;
  TQStringList m_oldLayouts;
  TQStringList m_nonLatinLayouts;
  
  TQString X11_DIR;	// pseudo-constant
  
//  void fixLayouts();
};


#endif
