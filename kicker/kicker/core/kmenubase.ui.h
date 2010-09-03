#include <X11/Xlib.h>

void KMenuBase::init()
{
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes( qt_xdisplay(), winId(), CWOverrideRedirect, &attrs );
    setWFlags( Qt::WType_Popup );
}
