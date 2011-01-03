

#include <tqfile.h>


#include "template.h"


bool CSSTemplate::expand(TQString destname, const TQMap<TQString,TQString> &dict)
{
  TQFile inf(_filename);
  if (!inf.open(IO_ReadOnly))
    return false;
  TQTextStream is(&inf);
  
  TQFile outf(destname);
  if (!outf.open(IO_WriteOnly))
    return false;
  TQTextStream os(&outf);

  TQString line;
  while (!is.eof())
    {
      line = is.readLine();

      int start = line.find('$');
      if (start >= 0)
	{
	  int end = line.find('$', start+1);
	  if (end >= 0)
            {
	      TQString expr = line.mid(start+1, end-start-1);
	      TQString res = dict[expr];

	      line.tqreplace(start, end-start+1, res);
	    }
	}
      os << line << endl;
    }  

  inf.close();
  outf.close();

  return true;
}
