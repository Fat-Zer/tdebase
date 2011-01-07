#ifndef __RULES_H__
#define __RULES_H__

#include <tqstring.h>
#include <tqdict.h>
#include <tqmap.h>


class XkbRules
{
public:

  XkbRules(bool layoutsOnly=false);

  const TQDict<char> &models() const { return m_models; };
  const TQDict<char> &layouts() const { return m_layouts; };
  const TQDict<char> &options() const { return m_options; };
  
  TQStringList getAvailableVariants(const TQString& layout);
  unsigned int getDefaultGroup(const TQString& layout, const TQString& includeGroup);

  bool isSingleGroup(const TQString& layout);

protected:

  void loadRules(TQString filename, bool layoutsOnly=false);
  void loadGroups(TQString filename);
  void loadOldLayouts(TQString filename);

private:

  TQDict<char> m_models;
  TQDict<char> m_layouts;
  TQDict<char> m_options;
  TQMap<TQString, unsigned int> m_initialGroups;
  TQDict<TQStringList> m_varLists;
  TQStringList m_oldLayouts;
  TQStringList m_nonLatinLayouts;
  
  TQString X11_DIR;	// pseudo-constant
  
//  void fixLayouts();
};


#endif
