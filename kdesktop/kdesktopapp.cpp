/* This file is part of the KDE project
   Copyright (C) 2007 Dennis Kasprzyk <onestone@opencompositing.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdesktopapp.h>

KDesktopApp::KDesktopApp():
KUniqueApplication()
{
#ifdef COMPOSITE
    initCmBackground();
#endif
}

KDesktopApp::KDesktopApp(Display * dpy, Qt::HANDLE visual, Qt::HANDLE colormap):
KUniqueApplication(dpy, visual, colormap)
{
#ifdef COMPOSITE
    initCmBackground();
#endif
}

#ifdef COMPOSITE
void KDesktopApp::initCmBackground()
{
    Atom type;
    int format;
    unsigned long num, rest;
    unsigned char *data;

    m_bgSupported = false;
    m_cmBackground =
            XInternAtom (qt_xdisplay(), "_COMPIZ_WALLPAPER_SUPPORTED", false);

    XSelectInput (qt_xdisplay(), qt_xrootwin(), PropertyChangeMask);

    if (XGetWindowProperty (qt_xdisplay(), qt_xrootwin(), m_cmBackground,
                            0, 1, FALSE,  XA_CARDINAL, &type, &format, &num,
                            &rest, &data) == Success && num)
    {
        if (type == XA_CARDINAL)
            m_bgSupported = (*data == 1);
        XFree (data);
    }
}

bool KDesktopApp::x11EventFilter (XEvent * xevent)
{
    if (xevent->type == PropertyNotify &&
        xevent->xproperty.window == qt_xrootwin() &&
        xevent->xproperty.atom == m_cmBackground)
    {
        Atom type;
        int format;
        unsigned long num, rest;
        unsigned char *data;
    
        Bool supported = false;
        
        if (XGetWindowProperty (qt_xdisplay(), qt_xrootwin(), m_cmBackground,
                            0, 1, FALSE,  XA_CARDINAL, &type, &format, &num,
                            &rest, &data) == Success && num)
        {
            if (type == XA_CARDINAL)
                supported = (*data == 1);
            XFree (data);
        }

        if (m_bgSupported != supported)
        {
            m_bgSupported = supported;
            emit cmBackgroundChanged(supported);
        }
    }
    return KUniqueApplication::x11EventFilter (xevent);
}

#endif

#include "kdesktopapp.moc"
