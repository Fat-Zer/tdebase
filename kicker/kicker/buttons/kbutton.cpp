/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <tqtooltip.h>
#include <tqstyle.h>

#include <tdelocale.h>
#include <tdeapplication.h>
#include <kdebug.h>

#include "kickerSettings.h"

#include "config.h"

#include "menumanager.h"
#include "k_mnu.h"
#include "k_mnu_stub.h"

#include "kbutton.h"
#include "kbutton.moc"

KButton::KButton( TQWidget* parent )
    : PanelPopupButton( parent, "KButton", KickerSettings::showDeepButtons() )
{
    TQToolTip::add(this, i18n("Applications, tasks and desktop sessions"));
    setTitle(i18n("TDE Menu"));

    setPopup(MenuManager::the()->kmenu()->widget());
    MenuManager::the()->registerKButton(this);

    setIcon("kmenu");
    setIcon(KickerSettings::customKMenuIcon());

    if (KickerSettings::showKMenuText())
    {
        setButtonText(KickerSettings::kMenuText());
        setFont(KickerSettings::buttonFont());
        setTextColor(KickerSettings::buttonTextColor());
        setMaximumHeight(KickerSettings::maximumTDEMenuButtonHeight());
        setMaximumWidth(widthForHeight(KickerSettings::maximumTDEMenuButtonHeight()));
        setCenterButtonInContainer(false);
    }
}

KButton::~KButton()
{
    MenuManager::the()->unregisterKButton(this);
}

int KButton::widthForHeight(int height) const
{
    if (KickerSettings::showKMenuText()) {
        return PanelPopupButton::widthForHeight((height>KickerSettings::maximumTDEMenuButtonHeight())?KickerSettings::maximumTDEMenuButtonHeight():height);
    }
    else {
        return PanelPopupButton::widthForHeight(height);
    }
}

int KButton::heightForWidth(int width) const
{
    if (KickerSettings::showKMenuText()) {
        int recommendation = PanelPopupButton::heightForWidth(width);
        if (recommendation > KickerSettings::maximumTDEMenuButtonHeight()) recommendation = KickerSettings::maximumTDEMenuButtonHeight();
        return recommendation;
    }
    else {
        return PanelPopupButton::heightForWidth(width);
    }
}

void KButton::properties()
{
    TDEApplication::startServiceByDesktopName("kmenuedit", TQStringList(),
                                            0, 0, 0, "", true);
}

void KButton::initPopup()
{
//    kdDebug(1210) << "KButton::initPopup()" << endl;

    // this hack is required to ensure the correct popup position
    // when the size of the recent application part of the menu changes
    // please don't remove this _again_
    MenuManager::the()->kmenu()->initialize();
}

void KButton::drawButton(TQPainter *p)
{
    if (KickerSettings::showDeepButtons())
        PanelPopupButton::drawDeepButton(p);
    else
        PanelPopupButton::drawButton(p);
}
