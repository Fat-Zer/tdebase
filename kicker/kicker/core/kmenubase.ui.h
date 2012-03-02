#include <X11/Xlib.h>

void KMenuBase::init()
{
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes( tqt_xdisplay(), winId(), CWOverrideRedirect, &attrs );
    setWFlags( (WFlags)TQt::WType_Popup );
}
