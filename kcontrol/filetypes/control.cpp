#include "filetypesview.h"

extern "C"
{
	  KDE_EXPORT TDECModule *create_filetypes(TQWidget *parent, const char *)
          {
        return new FileTypesView(parent, "filetypes");
	  }

}

