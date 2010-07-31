/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __appletproxy_h__
#define __appletproxy_h__

#include <tqcstring.h>
#include <tqobject.h>

#include <dcopobject.h>

#include "appletinfo.h"

class KPanelApplet;
class KickerPluginManager;

class AppletProxy : public TQObject, DCOPObject
{
    Q_OBJECT

public:
    AppletProxy(TQObject* parent, const char* name = 0);
    ~AppletProxy();

    void loadApplet(const TQString& desktopFile, const TQString& configFile);
    KPanelApplet* loadApplet(const AppletInfo& info);
    void dock(const TQCString& callbackID);
    void showStandalone();

    bool process(const TQCString &fun, const TQByteArray &data,
		 TQCString& replyType, TQByteArray &replyData);

protected slots:
    void slotUpdateLayout();
    void slotRequestFocus();
    void slotApplicationRemoved(const TQCString&);

private:
    void repaintApplet(TQWidget* widget);

    AppletInfo          *_info;
    KPanelApplet        *_applet;
    TQCString             _callbackID;
    TQPixmap              _bg;
};

#endif
