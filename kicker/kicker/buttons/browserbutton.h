/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __browserbutton_h__
#define __browserbutton_h__

#include "panelbutton.h"

class PanelBrowserMenu;

/**
 * Button that contains a Browser directory menu
 */
class BrowserButton : public PanelPopupButton
{
    Q_OBJECT

public:
    BrowserButton( const TQString& icon, const TQString& startDir, TQWidget* parent );
    BrowserButton( const TDEConfigGroup& config, TQWidget* parent );
    virtual ~BrowserButton();

    void saveConfig( TDEConfigGroup& config ) const;

    virtual void properties();

protected slots:
    virtual void slotDelayedPopup();
    virtual void startDrag();

protected:
    void initialize( const TQString& icon, const TQString& startDir );
    virtual TQString tileName() { return "Browser"; }
    virtual void initPopup();
    virtual void dropEvent(TQDropEvent *ev);
    virtual void dragEnterEvent(TQDragEnterEvent *ev);
    virtual void dragLeaveEvent(TQDragLeaveEvent *ev);
    virtual TQString defaultIcon() const { return "kdisknav"; };

    PanelBrowserMenu*    topMenu;
    TQString              _icon;
    TQTimer *_menuTimer;
};

#endif

