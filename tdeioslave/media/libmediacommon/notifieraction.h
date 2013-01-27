/* This file is part of the KDE Project
   Copyright (c) 2005 Jean-Remy Falleri <jr.falleri@laposte.net>
   Copyright (c) 2005 Kévin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _NOTIFIERACTION_H_
#define _NOTIFIERACTION_H_

#include <tdefileitem.h>
#include <tqstring.h>
#include <tqpixmap.h>

class NotifierSettings;

class NotifierAction
{
public:
	NotifierAction();
	virtual ~NotifierAction();

	virtual TQString label() const;
	virtual TQString iconName() const;
	virtual void setLabel( const TQString &label );
	virtual void setIconName( const TQString &icon );

	TQPixmap pixmap() const;
	
	TQStringList autoMimetypes();

	virtual TQString id() const = 0;
	virtual bool isWritable() const;
	virtual bool supportsMimetype( const TQString &mimetype ) const;
	virtual void execute( KFileItem &medium ) = 0;

private:
	void addAutoMimetype( const TQString &mimetype );
	void removeAutoMimetype( const TQString &mimetype );

	TQString m_label;
	TQString m_iconName;
	TQStringList m_autoMimetypes;

	friend class NotifierSettings;
};

#endif
