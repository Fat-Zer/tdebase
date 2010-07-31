#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__


#include <tqstring.h>
#include <tqmap.h>

class CSSTemplate
{
public:

  CSSTemplate(TQString fname) : _filename(fname) {};
  bool expand(TQString destname, const TQMap<TQString,TQString> &dict);

protected:
  TQString _filename;

};


#endif
