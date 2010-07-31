#include "filetypesview.h"

extern "C"
{
	  KDE_EXPORT KCModule *create_filetypes(TQWidget *parent, const char *)
          {
        return new FileTypesView(parent, "filetypes");
	  }

}

