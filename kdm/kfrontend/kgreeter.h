/*

Greeter widget for kdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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


#ifndef KGREETER_H
#define KGREETER_H

#include "kgverify.h"
#include "kgdialog.h"

class KdmClock;
class UserListView;
class KdmThemer;
class KdmItem;

class KListView;
class KSimpleConfig;

class QLabel;
class QPushButton;
class QPopupMenu;
class QListViewItem;

struct SessType {
	TQString name, type;
	bool hid;
	int prio;

	SessType() {}
	SessType( const TQString &n, const TQString &t, bool h, int p ) :
		name( n ), type( t ), hid( h ), prio( p ) {}
	bool operator<( const SessType &st ) {
		return hid != st.hid ? hid < st.hid :
		       prio != st.prio ? prio < st.prio :
		       name < st.name;
	}
};

class KGreeter : public KGDialog, public KGVerifyHandler {
	Q_OBJECT
	typedef KGDialog inherited;

  public:
	KGreeter( bool themed = false );
	~KGreeter();

  public slots:
	void accept();
	void reject();
	void slotUserClicked( TQListViewItem * );
	void slotSessionSelected( int );
	void slotUserEntered();

  protected:
	void installUserList();
	void insertUser( const TQImage &, const TQString &, struct passwd * );
	void insertUsers();
	void putSession( const TQString &, const TQString &, bool, const char * );
	void insertSessions();
	virtual void pluginSetup();
	void setPrevWM( int );

	TQString curUser, dName;
	KSimpleConfig *stsFile;
	UserListView *userView;
	TQStringList *userList;
	TQPopupMenu *sessMenu;
	TQValueVector<SessType> sessionTypes;
	int nNormals, nSpecials;
	int curPrev, curSel;
	bool prevValid;
	bool needLoad;

	static int curPlugin;
	static PluginList pluginList;

  private slots:
	void slotLoadPrevWM();

  public: // from KGVerifyHandler
	virtual void verifyPluginChanged( int id );
	virtual void verifyClear();
	virtual void verifyOk();
	virtual void verifyFailed();
//	virtual void verifyRetry();
	virtual void verifySetUser( const TQString &user );
};

class KStdGreeter : public KGreeter {
	Q_OBJECT
	typedef KGreeter inherited;

  public:
	KStdGreeter();

  protected:
	virtual void pluginSetup();

  private:
	KdmClock *clock;
	TQLabel *pixLabel;
	TQPushButton *goButton;
	TQPushButton *menuButton;

  public: // from KGVerifyHandler
	virtual void verifyFailed();
	virtual void verifyRetry();
};

class KThemedGreeter : public KGreeter {
	Q_OBJECT
	typedef KGreeter inherited;

  public:
	KThemedGreeter();
	bool isOK() { return themer != 0; }
	static TQString timedUser;
	static int timedDelay;

  public slots:
	void slotThemeActivated( const TQString &id );
	void slotSessMenu();
	void slotActionMenu();

  protected:
	virtual void updateStatus( bool fail, bool caps, int timedleft );
	virtual void pluginSetup();
	virtual void keyPressEvent( TQKeyEvent * );
	virtual bool event( TQEvent *e );

  private:
//	KdmClock *clock;
	KdmThemer *themer;
	KdmItem *caps_warning, *xauth_warning, *pam_error, *timed_label,
	        *console_rect, *userlist_rect,
	        *session_button, *system_button;

  public: // from KGVerifyHandler
	virtual void verifyFailed();
	virtual void verifyRetry();
};

#endif /* KGREETER_H */
