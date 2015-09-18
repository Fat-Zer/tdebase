/*

Conversation widget for tdm greeter

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


#ifndef KGREET_WINBIND_H
#define KGREET_WINBIND_H

#include "kgreeterplugin.h"

#include <tqobject.h>
#include <tqtimer.h>

class KComboBox;
class KLineEdit;
class KPasswordEdit;
class KSimpleConfig;
class TQGridLayout;
class TQLabel;
class KdmThemer;
class KProcIO;

class KWinbindGreeter : public TQObject, public KGreeterPlugin {
	Q_OBJECT

  public:
	KWinbindGreeter( KGreeterPluginHandler *handler,
	                 KdmThemer *themer,
	                 TQWidget *parent, TQWidget *predecessor,
	                 const TQString &fixedEntitiy,
	                 Function func, Context ctx );
	~KWinbindGreeter();
	virtual void loadUsers( const TQStringList &users );
	virtual void presetEntity( const TQString &entity, int field );
	virtual TQString getEntity() const;
	virtual void setUser( const TQString &user );
	virtual void lockUserEntry( const bool lock );
	virtual void setPassword( const TQString &pass );
	virtual void setEnabled( bool on );
	virtual void setInfoMessageDisplay( bool on );
	virtual bool textMessage( const char *message, bool error );
	virtual void textPrompt( const char *prompt, bool echo, bool nonBlocking );
	virtual bool binaryPrompt( const char *prompt, bool nonBlocking );
	virtual void start();
	virtual void suspend();
	virtual void resume();
	virtual void next();
	virtual void abort();
	virtual void succeeded();
	virtual void failed();
	virtual void revive();
	virtual void clear();

  public slots:
	void slotLoginLostFocus();
	void slotChangedDomain( const TQString &dom );
	void slotActivity();
        void slotStartDomainList();
        void slotReadDomainList();
        void slotEndDomainList();

  private:
	void setActive( bool enable );
	void setActive2( bool enable );
	void returnData();

	TQLabel *domainLabel, *loginLabel, *passwdLabel, *passwd1Label, *passwd2Label;
	KComboBox *domainCombo;
	KLineEdit *loginEdit;
	KPasswordEdit *passwdEdit, *passwd1Edit, *passwd2Edit;
	KSimpleConfig *stsFile;
	TQString fixedDomain, fixedUser, curUser;
	TQStringList allUsers, mDomainListing;
	KProcIO* m_domainLister;
        TQTimer mDomainListTimer;

	Function func;
	Context ctx;
	int exp, pExp, has;
	bool running, authTok, suppressInfoMsg;
};

#endif /* KGREET_WINBIND_H */
