/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef __tasklmbmenu_h__
#define __tasklmbmenu_h__

#include <tqpopupmenu.h>
#include <tqtimer.h>

#include "taskmanager.h"

class TaskMenuItem : public QCustomMenuItem
{
public:
    TaskMenuItem(const TQString &text,
                 bool active, bool minimized, bool attention);
    ~TaskMenuItem();

    void paint(TQPainter*, const TQColorGroup&, bool, bool, int, int, int, int);
    TQSize tqsizeHint();
    void setAttentionState(bool state) { m_attentionState = state; }

private:
    TQString m_text;
    bool m_isActive;
    bool m_isMinimized;
    bool m_demandsAttention;
    bool m_attentionState;
};

/*****************************************************************************/

class KDE_EXPORT TaskLMBMenu : public QPopupMenu
{
    Q_OBJECT

public:
    TaskLMBMenu(const Task::List& list, TQWidget *parent = 0, const char *name = 0);

protected slots:
    void dragSwitch();
    void attentionTimeout();

protected:
    void dragEnterEvent(TQDragEnterEvent*);
    void dragLeaveEvent(TQDragLeaveEvent*);
    void dragMoveEvent(TQDragMoveEvent*);
    void mousePressEvent(TQMouseEvent*);
    void mouseMoveEvent(TQMouseEvent*);
    void mouseReleaseEvent(TQMouseEvent*);

private:
    void fillMenu();

    Task::List m_tasks;
    int        m_lastDragId;
    bool       m_attentionState;
    TQTimer*    m_attentionTimer;
    TQTimer*    m_dragSwitchTimer;
    TQPoint     m_dragStartPos;
    TQValueList<TaskMenuItem*> m_attentionMap;
};

#endif
