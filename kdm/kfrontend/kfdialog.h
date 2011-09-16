/*

Dialog class that handles input focus in absence of a wm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#define KDM_LOGIN_SCREEN_BASE_TITLE i18n("Login to TDE")

#ifndef FDIALOG_H
#define FDIALOG_H

#include <tqdialog.h>
#include <tqmessagebox.h>

class TQFrame;

class FDialog : public TQDialog {
	typedef TQDialog inherited;

  public:
	FDialog( TQWidget *parent = 0, bool framed = true );
	virtual int exec();

	static void box( TQWidget *parent, TQMessageBox::Icon type,
	                 const TQString &text );
#define errorbox TQMessageBox::Critical
#define sorrybox TQMessageBox::Warning
#define infobox TQMessageBox::Information
	void MsgBox( TQMessageBox::Icon typ, const TQString &msg ) { box( this, typ, msg ); }

  protected:
	virtual void resizeEvent( TQResizeEvent *e );
	void adjustGeometry();

  private:
	TQFrame *winFrame;
	bool m_wmTitle;
};

class KFMsgBox : public FDialog {
	typedef FDialog inherited;

  public:
	KFMsgBox( TQWidget *parent, TQMessageBox::Icon type, const TQString &text );
};

#endif /* FDIALOG_H */
