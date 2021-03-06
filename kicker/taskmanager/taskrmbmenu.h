/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef __taskrmbmenu_h__
#define __taskrmbmenu_h__

#include <tqpopupmenu.h>

class KDE_EXPORT TaskRMBMenu : public TQPopupMenu
{
	Q_OBJECT

public:
	TaskRMBMenu(const Task::List&, bool showAll = true, TQPopupMenu* moveMenu = NULL, TQWidget *parent = 0, const char *name = 0);
	TaskRMBMenu(Task::Ptr, bool showAll = true, TQWidget *parent = 0, const char *name = 0);

private:
	void fillMenu(Task::Ptr);
	void fillMenu();
    TQPopupMenu* makeAdvancedMenu(Task::Ptr);
	TQPopupMenu* makeDesktopsMenu(Task::Ptr);
	TQPopupMenu* makeDesktopsMenu();

private slots:
	void slotMinimizeAll();
	void slotMaximizeAll();
	void slotRestoreAll();
	void slotShadeAll();
	void slotCloseAll();
	void slotAllToDesktop( int desktop );
	void slotAllToCurrentDesktop();

private:
	Task::List tasks;
	bool showAll;
	TQPopupMenu* taskMoveMenu;
};

#endif
