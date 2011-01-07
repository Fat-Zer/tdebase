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

#ifndef __systemtrayapplet_h__
#define __systemtrayapplet_h__

#include <tqvaluevector.h>
#include <tqstringlist.h>
#include <tqevent.h>
#include <qxembed.h>

#include <dcopobject.h>
#include <kapplication.h>
#include <kpanelapplet.h>

#include "simplebutton.h"

class TQGridLayout;
class TQTimer;
class KWinModule;
class TrayEmbed;
class KDialogBase;
class KActionSelector;

class SystemTrayApplet : public KPanelApplet, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    typedef TQValueVector<TrayEmbed*> TrayEmbedList;

public:

    SystemTrayApplet(const TQString& configFile, Type t = Normal, int actions = 0,
                     TQWidget *parent = 0, const char *name = 0);
    ~SystemTrayApplet();

    int widthForHeight(int h) const;
    int heightForWidth(int w) const;
    int maxIconWidth() const;
    int maxIconHeight() const;

    bool eventFilter(TQObject* watched, TQEvent* e);

k_dcop:
    void loadSettings();
    void iconSizeChanged();

protected:
    void resizeEvent(TQResizeEvent*);
    void moveEvent(TQMoveEvent *);
    bool x11Event( XEvent *e );
    void preferences();
    void orientationChange( Orientation );

protected slots:
    void initialize();
    void systemTrayWindowAdded( WId );
    void updateTrayWindows();
    void layoutTray();
    void paletteChange(const TQPalette & /* oldPalette */);
    void toggleExpanded();
    void settingsDialogFinished();
    void applySettings();
    void checkAutoRetract();
    void configure() { preferences(); }
    void setBackground();

private:
    void embedWindow( WId w, bool kde_tray );
    bool isWinManaged( WId w);
    bool shouldHide( WId w);
    void updateVisibleWins();
    void expand();
    void retract();
    void showExpandButton(bool show);
    void refreshExpandButton();

    TrayEmbedList m_shownWins;
    TrayEmbedList m_hiddenWins;
    TQStringList m_hiddenIconList;
    TQStringList m_sortOrderIconList;
    KWinModule *kwin_module;
    Atom net_system_tray_selection;
    Atom net_system_tray_opcode;
    bool m_showFrame;
    bool m_showHidden;
    SimpleArrowButton *m_expandButton;
    KDialogBase* m_settingsDialog;
    KActionSelector* m_iconSelector;
    TQTimer* m_autoRetractTimer;
    bool m_autoRetract;
    int m_iconSize;
    TQGridLayout* m_layout;
};

class TrayEmbed : public QXEmbed
{
    Q_OBJECT
public:
    TrayEmbed( bool kdeTray, TQWidget* parent = NULL );
    bool kdeTray() const { return kde_tray; }
    void setBackground();
    void getIconSize(int defaultIconSize);
private:
    bool kde_tray;
    TQWidget *m_scaledWidget;
};

#endif
