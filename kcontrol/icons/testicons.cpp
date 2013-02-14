/* Test programme for icons setup module. */

#include <tdeapplication.h>
#include "icons.h"

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, "testicons");
    TDEIconConfig *w = new TDEIconConfig(0L, "testicons");
    app.setMainWidget(w);
    w->show();
    return app.exec();
}
