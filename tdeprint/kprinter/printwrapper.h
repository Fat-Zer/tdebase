/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef PRINTWRAPPER_H
#define PRINTWRAPPER_H

#include <tqwidget.h>

class KPrinter;
class KPrintDialog;
class TQSocketNotifier;

class PrintWrapper : public TQWidget
{
	Q_OBJECT
public:
	PrintWrapper();

public slots:
	void slotPrint();
	void slotPrintRequested(KPrinter*);

private slots:
	void slotGotStdin();
		
private:
	bool force_stdin;
	bool check_stdin;
	KPrintDialog* dlg;
	TQSocketNotifier* notif;
};

#endif
