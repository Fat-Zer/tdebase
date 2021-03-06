#include <tdeglobal.h>
#include <tdelocale.h>
#include <kgenericfactory.h>

#include "kcmstyle.h"

extern "C" {
    TDECModule *create_style(TQWidget *parent, const char *) {
      return new KCMStyle(parent, "kcmstyle");
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, TQWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_style, GeneralFactory )
*/
