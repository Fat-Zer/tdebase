/*****************************************************************

Copyright (c) 2006 Stephan Binner <binner@kde.org>
                   Stephan Kulow <coolo@kde.org>
                   Dirk Mueller <mueller@kde.org>

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

#ifndef __knewbutton_h__
#define __knewbutton_h__

#include "kbutton.h"

#include <tqmovie.h>
#include <tqpoint.h>

/**
 * Button that tqcontains the PanelKMenu and client menu manager.
 */
class KNewButton : public KButton
{
    Q_OBJECT

public:
    KNewButton( TQWidget *parent );
    ~KNewButton();

    static KNewButton *self() { return m_self; }

    void loadConfig( const KConfigGroup& config );

    virtual const TQPixmap& labelIcon() const;

    virtual int widthForHeight(int height) const;
    virtual int preferredDimension(int panelDim) const;
    virtual int heightForWidth(int width) const;

    TQColor borderColor() const;

    virtual void setPopupDirection(KPanelApplet::Direction d);

private slots:
    void slottqStatus(int);
    void slotSetSize(const TQSize&);
    void slotStopAnimation();
    void rewindMovie();
    void updateMovie();

protected:
    virtual void show();
    virtual void slotExecMenu();
    virtual TQString tileName() { return "KMenu"; }
    virtual TQString defaultIcon() const { return "go"; }

    virtual void enterEvent(TQEvent* e);
    virtual void leaveEvent(TQEvent* e);
    virtual void mouseMoveEvent(TQMouseEvent* e);
    virtual void dragEnterEvent(TQDragEnterEvent*);
    virtual void dragLeaveEvent(TQDragLeaveEvent*);
    virtual bool eventFilter(TQObject *, TQEvent *);
    void timerEvent(TQTimerEvent*);

private:
    void drawEye();
    double buttonScaleFactor(const TQSize& s) const;

    TQMovie* m_movie;
    TQPixmap m_active_pixmap;
    TQPoint m_oldPos;
    TQSize m_iconSize;
    TQRect m_sloppyRegion;
    int m_hoverTimer;
    int m_openTimer;
    bool m_active;
    bool m_mouseInside;
    bool m_drag;

    static KNewButton *m_self;
};

#endif
