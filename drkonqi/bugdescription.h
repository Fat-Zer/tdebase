/* This file is part of the TDE project
   Copyright (C) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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

#ifndef BUGDESCRIPTION_H
#define BUGDESCRIPTION_H

class TDEAboutData;

#include "bugdescriptiondialog.h"

class BugDescription : public BugDescriptionDialog
{
	TQ_OBJECT
	
	public:
		/**
		* Constructor.
		*/
		BugDescription(TQWidget *parent = 0, bool modal = true, const TDEAboutData *aboutData = 0);
	
	public:
		/**
		* Allows the debugger to set the default text in the editor.
		*/
		void setText(const TQString &str);

		TQString emailAddress();
		TQString crashDescription();

		void fullReportViewMode( bool enabled );
	
	protected slots:
		/**
		* OK has been clicked
		*/
		virtual void slotOk( void );
	
	private:
		TQString m_startstring;
};

#endif
