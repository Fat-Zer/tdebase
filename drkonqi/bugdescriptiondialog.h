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
#ifndef _BUGDESCRIPTIONDIALOG_H__
#define _BUGDESCRIPTIONDIALOG_H__

#include <kdialogbase.h>

class TQMultiLineEdit;
class TQLineEdit;
class TQHButtonGroup;
class TDEProcess;
class TDEAboutData;
class BugDescriptionDialogPrivate;

class TDEUI_EXPORT BugDescriptionDialog : public KDialogBase
{
	TQ_OBJECT

	public:
		/**
		* Creates a bug-report dialog.
		* Note that you shouldn't have to do this manually,
		* since KHelpMenu takes care of the menu item
		* for "Report Bug..." and of creating a BugDescriptionDialog dialog.
		*/
		BugDescriptionDialog( TQWidget * parent = 0L, bool modal=true, const TDEAboutData *aboutData = 0L );
		/**
		* Destructor
		*/
		virtual ~BugDescriptionDialog();
	
	protected slots:
		/**
		* OK has been clicked
		*/
		virtual void slotOk( void );
		/**
		* Cancel has been clicked
		*/
		virtual void slotCancel();
	
	protected:
		TQLabel * m_emailLabel;
		TQLabel * m_descriptionLabel;
		TQMultiLineEdit * m_lineedit;
		TQLineEdit * m_email;
	
	protected:
		virtual void virtual_hook( int id, void* data );

	private:
		BugDescriptionDialogPrivate *d;
};

#endif

