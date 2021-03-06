/*

Shell for tdm conversation plugins

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
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

#include <config.h>

#include "kgverify.h"
#include "tdmconfig.h"
#include "tdm_greet.h"

#include "themer/tdmthemer.h"
#include "themer/tdmitem.h"
#include "themer/tdmlabel.h"

#include <ksslcertificate.h>

#include <tdehardwaredevices.h>
#include <tdecryptographiccarddevice.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <tqregexp.h>
#include <tqpopupmenu.h>
#include <tqlayout.h>
#include <tqfile.h>
#include <tqlabel.h>

#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h> // for updateLockStatus()
#include <fixx11h.h> // ... and make eventFilter() work again

#define FULL_GREET_TO 40 // normal inactivity timeout
#define TIMED_GREET_TO 20 // inactivity timeout when persisting timed login
#define MIN_TIMED_TO 5 // minimal timed login delay
#define DEAD_TIMED_TO 2 // <enter> dead time after re-activating timed login
#define SECONDS 1000 // reduce to 100 to speed up testing

void KGVerifyHandler::verifyClear()
{
}

void KGVerifyHandler::updateStatus( bool, bool, int )
{
}

KGVerify::KGVerify(KGVerifyHandler *_handler, KdmThemer *_themer,
			TQWidget *_parent, TQWidget *_predecessor,
			const TQString &_fixedUser,
			const PluginList &_pluginList,
			KGreeterPlugin::Function _func,
			KGreeterPlugin::Context _ctx)
	: inherited()
	, coreLock(0)
	, fixedEntity(_fixedUser)
	, pluginList(_pluginList)
	, handler(_handler)
	, themer(_themer)
	, parent(_parent)
	, predecessor(_predecessor)
	, plugMenu(0)
	, curPlugin(-1)
	, timedLeft(0)
	, func(_func)
	, ctx(_ctx)
	, enabled(true)
	, running(false)
	, suspended(false)
	, failed(false)
	, isClear(true)
	, inGreeterPlugin(false)
	, abortRequested(false)
	, cardLoginInProgress(false)
	, cardLoginDevice(NULL)
{
	connect( &timer, TQT_SIGNAL(timeout()), TQT_SLOT(slotTimeout()) );
	connect( kapp, TQT_SIGNAL(activity()), TQT_SLOT(slotActivity()) );

	_parent->installEventFilter( this );
}

KGVerify::~KGVerify()
{
	Debug( "delete %s\n", pName.data() );
	delete greet;
}

TQPopupMenu *
KGVerify::getPlugMenu()
{
	// assert( !cont );
	if (!plugMenu) {
		uint np = pluginList.count();
		if (np > 1) {
			plugMenu = new TQPopupMenu( parent );
			connect( plugMenu, TQT_SIGNAL(activated( int )),
			         TQT_SLOT(slotPluginSelected( int )) );
			for (uint i = 0; i < np; i++)
				plugMenu->insertItem( i18n(greetPlugins[pluginList[i]].info->name), pluginList[i] );
		}
	}
	return plugMenu;
}

bool // public
KGVerify::entitiesLocal() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Local;
}

bool // public
KGVerify::entitiesFielded() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Fielded;
}

bool // public
KGVerify::entityPresettable() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Presettable;
}

bool // public
KGVerify::isClassic() const
{
	return !strcmp( greetPlugins[pluginList[curPlugin]].info->method, "classic" );
}

TQString // public
KGVerify::pluginName() const
{
	TQString name( greetPlugins[pluginList[curPlugin]].library->fileName() );
	uint st = name.findRev( '/' ) + 1;
	uint en = name.find( '.', st );
	if (en - st > 7 && TQConstString( name.unicode() + st, 7 ).string() == "kgreet_")
		st += 7;
	return name.mid( st, en - st );
}

static void
showWidgets( TQLayoutItem *li )
{
	TQWidget *w;
	TQLayout *l;

	if ((w = li->widget()))
		w->show();
	else if ((l = li->layout())) {
		TQLayoutIterator it = l->iterator();
		for (TQLayoutItem *itm = it.current(); itm; itm = ++it)
			 showWidgets( itm );
	}
}

void // public
KGVerify::selectPlugin( int id )
{
	if (pluginList.isEmpty()) {
		MsgBox( errorbox, i18n("No greeter widget plugin loaded. Check the configuration.") );
		::exit( EX_UNMANAGE_DPY );
	}
	curPlugin = id;
	if (plugMenu)
		plugMenu->setItemChecked( id, true );
	pName = ("greet_" + pluginName()).latin1();
	Debug( "new %s\n", pName.data() );
	greet = greetPlugins[pluginList[id]].info->create( this, themer, parent, predecessor, fixedEntity, func, ctx );
	timeable = _autoLoginDelay && entityPresettable() && isClassic();
}

void // public
KGVerify::loadUsers( const TQStringList &users )
{
	Debug( "%s->loadUsers(...)\n", pName.data() );
	greet->loadUsers( users );
}

void // public
KGVerify::presetEntity( const TQString &entity, int field )
{
	presEnt = entity;
	presFld = field;
}

bool // private
KGVerify::applyPreset()
{
	if (!presEnt.isEmpty()) {
		Debug( "%s->presetEntity(%\"s, %d)\n", pName.data(),
		       presEnt.latin1(), presFld );
		greet->presetEntity( presEnt, presFld );
		if (entitiesLocal()) {
			curUser = presEnt;
			handler->verifySetUser( presEnt );
		}
		return true;
	}
	return false;
}

bool // private
KGVerify::scheduleAutoLogin( bool initial )
{
	if (timeable) {
		Debug( "%s->presetEntity(%\"s, -1)\n", pName.data(),
		       _autoLoginUser.latin1(), -1 );
		greet->presetEntity( _autoLoginUser, -1 );
		curUser = _autoLoginUser;
		handler->verifySetUser( _autoLoginUser );
		timer.start( 1000 );
		if (initial) {
			timedLeft = _autoLoginDelay;
			deadTicks = 0;
		} else {
			timedLeft = TQMAX( _autoLoginDelay - TIMED_GREET_TO, MIN_TIMED_TO );
			deadTicks = DEAD_TIMED_TO;
		}
		updateStatus();
		running = false;
		isClear = true;
		return true;
	}
	return false;
}

void // private
KGVerify::performAutoLogin()
{
//	timer.stop();
	GSendInt( G_AutoLogin );
	handleVerify();
}

TQString // public
KGVerify::getEntity() const
{
	Debug( "%s->getEntity()\n", pName.data() );
	TQString ent = greet->getEntity();
	Debug( "  entity: %s\n", ent.latin1() );
	return ent;
}

void
KGVerify::setUser( const TQString &user )
{
	// assert( fixedEntity.isEmpty() );
	curUser = user;
	Debug( "%s->setUser(%\"s)\n", pName.data(), user.latin1() );
	greet->setUser( user );
	gplugActivity();
}

void
KGVerify::lockUserEntry(const bool lock)
{
	// assert( fixedEntity.isEmpty() );
	Debug( "%s->lockUserEntry(%\"s)\n", pName.data(), lock );
	greet->lockUserEntry(lock);
}

void
KGVerify::setPassword( const TQString &pass )
{
	greet->setPassword( pass );
	gplugActivity();
}

void
KGVerify::setInfoMessageDisplay(bool on)
{
	// assert( fixedEntity.isEmpty() );
	Debug( "%s->setInfoMessageDisplay(%\"s)\n", pName.data(), on );
	greet->setInfoMessageDisplay(on);
}

void
KGVerify::setPasswordPrompt(const TQString &prompt)
{
	greet->setPasswordPrompt(prompt);
	if (prompt != TQString::null) {
		setPassPromptText(prompt, false);
	}
	else {
		setPassPromptText(TQString::null, true);
	}
}

void
KGVerify::start()
{
	authTok = (func == KGreeterPlugin::ChAuthTok);
	cont = false;
	if (func == KGreeterPlugin::Authenticate && ctx == KGreeterPlugin::Login) {
		if (scheduleAutoLogin( true )) {
			if (!_autoLoginAgain)
				_autoLoginDelay = 0, timeable = false;
			return;
		} else
			applyPreset();
	}
	running = true;
	Debug( "%s->start()\n", pName.data() );
	greet->start();
	if (!(func == KGreeterPlugin::Authenticate ||
	      ctx == KGreeterPlugin::ChangeTok ||
	      ctx == KGreeterPlugin::ExChangeTok))
	{
		cont = true;
		handleVerify();
	}
}

void
KGVerify::abort()
{
	Debug( "%s->abort()\n", pName.data() );
	greet->abort();
	running = false;
}

void
KGVerify::suspend()
{
	// assert( !cont );
	if (running) {
		Debug( "%s->abort()\n", pName.data() );
		greet->abort();
	}
	suspended = true;
	updateStatus();
	timer.suspend();
}

void
KGVerify::resume()
{
	timer.resume();
	suspended = false;
	updateLockStatus();
	if (running) {
		Debug( "%s->start()\n", pName.data() );
		greet->start();
	} else if (delayed) {
		delayed = false;
		running = true;
		Debug( "%s->start()\n", pName.data() );
		greet->start();
	}
}

void // not a slot - called manually by greeter
KGVerify::accept()
{
	Debug( "%s->next()\n", pName.data() );
	greet->next();
}

void // private
KGVerify::doReject( bool initial )
{
	// assert( !cont );
	if (running) {
		Debug("%s->abort()\n", pName.data());
		greet->abort();
	}
	handler->verifyClear();
	Debug("%s->clear()\n", pName.data());
	greet->clear();
	curUser = TQString::null;
	if (!scheduleAutoLogin(initial)) {
		isClear = !(isClear && applyPreset());
		if (running) {
			Debug( "%s->start()\n", pName.data() );
			greet->start();
		}
		if (!failed) {
			timer.stop();
		}
	}
}

void // not a slot - called manually by greeter
KGVerify::reject()
{
	inGreeterPlugin = false;
	doReject( true );
}

void // not a slot - called manually by greeter
KGVerify::requestAbort()
{
	abortRequested = true;
	if (inGreeterPlugin) {
		greet->next();
	}
}

void
KGVerify::setEnabled( bool on )
{
	Debug( "%s->setEnabled(%s)\n", pName.data(), on ? "true" : "false" );
	greet->setEnabled( on );
	enabled = on;
	updateStatus();
}

void // private
KGVerify::slotTimeout()
{
	if (failed) {
		failed = false;
		updateStatus();
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		handler->verifyRetry();
		if (suspended)
			delayed = true;
		else {
			running = true;
			Debug( "%s->start()\n", pName.data() );
			greet->start();
			slotActivity();
			gplugActivity();
			if (cont)
				handleVerify();
		}
	} else if (timedLeft) {
		deadTicks--;
		if (!--timedLeft)
			performAutoLogin();
		else
			timer.start( 1000 );
		updateStatus();
	} else {
		// assert( ctx == Login );
		isClear = true;
		doReject( false );
	}
}

void
KGVerify::slotActivity()
{
	if (timedLeft) {
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		Debug( "%s->start()\n", pName.data() );
		greet->start();
		running = true;
		timedLeft = 0;
		updateStatus();
		timer.start( TIMED_GREET_TO * SECONDS );
	} else if (timeable)
		timer.start( TIMED_GREET_TO * SECONDS );
}


void // private static
KGVerify::VMsgBox( TQWidget *parent, const TQString &user,
                   TQMessageBox::Icon type, const TQString &mesg )
{
	FDialog::box( parent, type, user.isEmpty() ?
	              mesg : i18n("Authenticating %1...\n\n").arg( user ) + mesg );
}

static const char *msgs[]= {
	I18N_NOOP( "You are required to change your password immediately (password aged)." ),
	I18N_NOOP( "You are required to change your password immediately (root enforced)." ),
	I18N_NOOP( "You are not allowed to login at the moment." ),
	I18N_NOOP( "Home folder not available." ),
	I18N_NOOP( "Logins are not allowed at the moment.\nTry again later." ),
	I18N_NOOP( "Your login shell is not listed in /etc/shells." ),
	I18N_NOOP( "Root logins are not allowed." ),
	I18N_NOOP( "Your account has expired; please contact your system administrator." )
};

void // private static
KGVerify::VErrBox( TQWidget *parent, const TQString &user, const char *msg )
{
	TQMessageBox::Icon icon;
	TQString mesg;

	if (!msg) {
		mesg = i18n("A critical error occurred.\n"
		            "Please look at TDM's logfile(s) for more information\n"
		            "or contact your system administrator.");
		icon = errorbox;
	} else {
		mesg = TQString::fromLocal8Bit( msg );
		TQString mesg1 = mesg + '.';
		for (uint i = 0; i < as(msgs); i++)
			if (mesg1 == msgs[i]) {
				mesg = i18n(msgs[i]);
				break;
			}
		icon = sorrybox;
	}
	VMsgBox( parent, user, icon, mesg );
}

void // private static
KGVerify::VInfoBox(TQWidget *parent, const TQString &user, const char *msg)
{
	TQString mesg = TQString::fromLocal8Bit( msg );
	TQRegExp rx( "^Warning: your account will expire in (\\d+) day" );
	if (rx.search(mesg) >= 0) {
		int expire = rx.cap(1).toInt();
		mesg = expire ?
			i18n("Your account expires tomorrow.",
			     "Your account expires in %n days.", expire) :
			i18n("Your account expires today.");
	}
	else {
		rx.setPattern( "^Warning: your password will expire in (\\d+) day" );
		if (rx.search(mesg) >= 0) {
			int expire = rx.cap(1).toInt();
			mesg = expire ?
				i18n("Your password expires tomorrow.",
				     "Your password expires in %n days.", expire) :
				i18n("Your password expires today.");
		}
	}
	VMsgBox(parent, user, infobox, mesg);
}

bool // public static
KGVerify::handleFailVerify( TQWidget *parent )
{
	Debug( "handleFailVerify ...\n" );
	char *msg = GRecvStr();
	TQString user = TQString::fromLocal8Bit( msg );
	free( msg );

	for (;;) {
		int ret = GRecvInt();

		// non-terminal status
		switch (ret) {
		/* case V_PUT_USER: cannot happen - we are in "classic" mode */
		/* case V_PRE_OK: cannot happen - not in ChTok dialog */
		/* case V_CHTOK: cannot happen - called by non-interactive verify */
		case V_CHTOK_AUTH:
			Debug( " V_CHTOK_AUTH\n" );
			{
				TQStringList pgs( _pluginsLogin );
				pgs += _pluginsShutdown;
				TQStringList::ConstIterator it;
				for (it = pgs.begin(); it != pgs.end(); ++it)
					if (*it == "classic" || *it == "modern") {
						pgs = *it;
						goto gotit;
					} else if (*it == "generic") {
						pgs = "modern";
						goto gotit;
					}
				pgs = "classic";
			  gotit:
				KGChTok chtok( parent, user, init( pgs ), 0,
				               KGreeterPlugin::AuthChAuthTok,
				               KGreeterPlugin::Login );
				return chtok.exec();
			}
		case V_MSG_ERR:
			Debug( " V_MSG_ERR\n" );
			msg = GRecvStr();
			Debug( "  message %\"s\n", msg );
			VErrBox( parent, user, msg );
			if (msg)
				free( msg );
			continue;
		case V_MSG_INFO:
			Debug( " V_MSG_INFO\n" );
			msg = GRecvStr();
			Debug( "  message %\"s\n", msg );
			VInfoBox( parent, user, msg );
			free( msg );
			continue;
		}

		// terminal status
		switch (ret) {
		case V_OK:
			Debug( " V_OK\n" );
			return true;
		case V_AUTH:
			Debug( " V_AUTH\n" );
			VMsgBox( parent, user, sorrybox, i18n("Authentication failed") );
			return false;
		case V_FAIL:
			Debug( " V_FAIL\n" );
			return false;
		default:
			LogPanic( "Unknown V_xxx code %d from core\n", ret );
		}
	}
}

void // private
KGVerify::handleVerify()
{
	TQString user;

	Debug( "handleVerify ...\n" );
	for (;;) {
		char *msg;
		int ret, echo, ndelay;
		KGreeterPlugin::Function nfunc;

		ret = GRecvInt();

		// requests
		coreLock = 1;
		switch (ret) {
		case V_GET_TEXT:
			Debug( " V_GET_TEXT\n" );
			msg = GRecvStr();
			Debug( "  prompt %\"s\n", msg );
			echo = GRecvInt();
			Debug( "  echo = %d\n", echo );
			ndelay = GRecvInt();
			Debug( "  ndelay = %d\n%s->textPrompt(...)\n", ndelay, pName.data() );
			if (abortRequested) {
				inGreeterPlugin = true;
				greet->textPrompt("", echo, ndelay);
				inGreeterPlugin = !ndelay;
				abortRequested = false;
			}
			else {
				if (msg && (msg[0] != 0)) {
					// Reset password entry and change text
					setPassPromptText(msg);
					greet->start();
					inGreeterPlugin = true;
					greet->textPrompt(msg, echo, ndelay);
					inGreeterPlugin = !ndelay;

					if (cardLoginInProgress) {
						TQString autoPIN = cardLoginDevice->autoPIN(); 
						if (autoPIN != TQString::null) {
							// Initiate login
							setPassword(autoPIN);
							accept();
						}
						cardLoginInProgress = false;
					}
				}
				else {
					inGreeterPlugin = true;
					greet->textPrompt(msg, echo, ndelay);
					inGreeterPlugin = !ndelay;
				}
			}
			if (msg) {
				free(msg);
			}
			return;
		case V_GET_BINARY:
			Debug( " V_GET_BINARY\n" );
			msg = GRecvArr( &ret );
			Debug( "  %d bytes prompt\n", ret );
			ndelay = GRecvInt();
			Debug( "  ndelay = %d\n%s->binaryPrompt(...)\n", ndelay, pName.data() );
			if (abortRequested) {
				gplugReturnBinary(NULL);
			}
			else {
				inGreeterPlugin = true;
				greet->binaryPrompt( msg, ndelay );
				inGreeterPlugin = !ndelay;
			}
			if (msg) {
				free(msg);
			}
			return;
		}

		// non-terminal status
		coreLock = 2;
		switch (ret) {
		case V_PUT_USER:
			Debug( " V_PUT_USER\n" );
			msg = GRecvStr();
			curUser = user = TQString::fromLocal8Bit( msg );
			// greet needs this to be able to return something useful from
			// getEntity(). but the backend is still unable to tell a domain ...
			Debug("  %s->setUser(%\"s)\n", pName.data(), user.latin1());
			greet->setUser( curUser );
			handler->verifySetUser(curUser);
			if (msg) {
				free(msg);
			}
			continue;
		case V_PRE_OK: // this is only for func == AuthChAuthTok
			Debug( " V_PRE_OK\n" );
			// With the "classic" method, the wrong user simply cannot be
			// authenticated, even with the generic plugin. Other methods
			// could do so, but this applies only to ctx == ChangeTok, which
			// is not implemented yet.
			authTok = true;
			cont = true;
			Debug("%s->succeeded()\n", pName.data());
			greet->succeeded();
			abortRequested = false;
			inGreeterPlugin = false;
			continue;
		case V_CHTOK_AUTH:
			Debug( " V_CHTOK_AUTH\n" );
			nfunc = KGreeterPlugin::AuthChAuthTok;
			user = curUser;
			goto dchtok;
		case V_CHTOK:
			Debug( " V_CHTOK\n" );
			nfunc = KGreeterPlugin::ChAuthTok;
			user = TQString::null;
		dchtok:
			{
				timer.stop();
				Debug( "%s->succeeded()\n", pName.data() );
				greet->succeeded();
				abortRequested = false;
				inGreeterPlugin = false;
				KGChTok chtok( parent, user, pluginList, curPlugin, nfunc, KGreeterPlugin::Login );
				if (!chtok.exec()) {
					goto retry;
				}
				handler->verifyOk();
				return;
			}
		case V_MSG_ERR:
			Debug( " V_MSG_ERR\n" );
			msg = GRecvStr();
			Debug( "  %s->textMessage(%\"s, true)\n", pName.data(), msg );
			inGreeterPlugin = true;
			if (!greet->textMessage( msg, true )) {
				inGreeterPlugin = false;
				Debug( "  message passed\n" );
				if (!abortRequested) {
					VErrBox( parent, user, msg );
				}
			}
			else {
				inGreeterPlugin = false;
				Debug( "  message swallowed\n" );
			}
			if (msg) {
				free(msg);
			}
			continue;
		case V_MSG_INFO:
			Debug( " V_MSG_INFO\n" );
			msg = GRecvStr();
			Debug( "  %s->textMessage(%\"s, false)\n", pName.data(), msg );
			inGreeterPlugin = true;
			if (!greet->textMessage( msg, false )) {
				inGreeterPlugin = false;
				Debug( "  message passed\n" );
				if (!abortRequested) {
					VInfoBox(parent, user, msg);
				}
			}
			else {
				inGreeterPlugin = false;
				Debug("  message swallowed\n");
			}
			free(msg);
			continue;
		}

		// terminal status
		coreLock = 0;
		running = false;
		timer.stop();

		if (ret == V_OK) {
			Debug( " V_OK\n" );
			if (!fixedEntity.isEmpty()) {
				Debug( "  %s->getEntity()\n", pName.data() );
				TQString ent = greet->getEntity();
				Debug( "  entity %\"s\n", ent.latin1() );
				if (ent != fixedEntity) {
					Debug( "%s->failed()\n", pName.data() );
					greet->failed();
					abortRequested = false;
					inGreeterPlugin = false;
					MsgBox( sorrybox,
					        i18n("Authenticated user (%1) does not match requested user (%2).\n")
					        .arg( ent ).arg( fixedEntity ) );
					goto retry;
				}
			}
			Debug( "%s->succeeded()\n", pName.data() );
			greet->succeeded();
			abortRequested = false;
			inGreeterPlugin = false;
			handler->verifyOk();
			return;
		}

		Debug( "%s->failed()\n", pName.data() );
		greet->failed();
		abortRequested = false;
		inGreeterPlugin = false;

		// Reset password prompt text
		setPassPromptText(TQString::null, true);

		if (ret == V_AUTH) {
			Debug( " V_AUTH\n" );
			failed = true;
			updateStatus();
			handler->verifyFailed();
			timer.start( 1500 + kapp->random()/(RAND_MAX/1000) );
			return;
		}
		if (ret != V_FAIL)
			LogPanic( "Unknown V_xxx code %d from core\n", ret );
		Debug( " V_FAIL\n" );
	  retry:
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		running = true;
		Debug( "%s->start()\n", pName.data() );
		greet->start();
		if (!cont) {
			return;
		}
		user = TQString::null;
	}
}

void KGVerify::setPassPromptText(TQString text, bool use_default_text) {
	if (themer) {
		KdmItem* password_label = themer->findNode("password-label");
		if (password_label) {
			KdmLabel* pass_label = static_cast<KdmLabel*>(password_label);
			if (use_default_text) {
				pass_label->setText(pass_label->lookupStock("password-label"));
			}
			else {
				pass_label->setText(text);
			}
			pass_label->update();
			themer->updateGeometry(true);
			static_cast<TQWidget *>(themer->parent())->repaint(true);
		}
	}
}

void
KGVerify::gplugReturnText( const char *text, int tag )
{
	Debug("%s: gplugReturnText(%\"s, %d)\n", pName.data(), tag & V_IS_SECRET ? "<masked>" : text, tag);
	GSendStr(text);
	if (text) {
		GSendInt(tag);
		handleVerify();
	}
	else {
		coreLock = 0;
	}
}

void
KGVerify::gplugReturnBinary( const char *data )
{
	if (data) {
		unsigned const char *up = (unsigned const char *)data;
		int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
		Debug("%s: gplugReturnBinary(%d bytes)\n", pName.data(), len);
		GSendArr(len, data);
		handleVerify();
	}
	else {
		Debug("%s: gplugReturnBinary(NULL)\n", pName.data());
		GSendArr(0, 0);
		coreLock = 0;
	}
}

void
KGVerify::gplugSetUser( const TQString &user )
{
	Debug( "%s: gplugSetUser(%\"s)\n", pName.data(), user.latin1() );
	curUser = user;
	handler->verifySetUser( user );
}

void
KGVerify::gplugStart()
{
	// XXX handle func != Authenticate
	Debug( "%s: gplugStart()\n", pName.data() );
	GSendInt( ctx == KGreeterPlugin::Shutdown ? G_VerifyRootOK : G_Verify );
	GSendStr( greetPlugins[pluginList[curPlugin]].info->method );
	handleVerify();
}

void
KGVerify::gplugActivity()
{
	Debug( "%s: gplugActivity()\n", pName.data() );
	if (func == KGreeterPlugin::Authenticate &&
	    ctx == KGreeterPlugin::Login)
	{
		isClear = false;
		if (!timeable)
			timer.start( FULL_GREET_TO * SECONDS );
	}
}

void
KGVerify::gplugMsgBox( TQMessageBox::Icon type, const TQString &text )
{
	Debug( "%s: gplugMsgBox(%d, %\"s)\n", pName.data(), type, text.latin1() );
	MsgBox( type, text );
}

bool
KGVerify::eventFilter( TQObject *o, TQEvent *e )
{
	switch (e->type()) {
	case TQEvent::KeyPress:
		if (timedLeft) {
			TQKeyEvent *ke = (TQKeyEvent *)e;
			if (ke->key() == Key_Return || ke->key() == Key_Enter) {
				if (deadTicks <= 0) {
					timedLeft = 0;
					performAutoLogin();
				}
				return true;
			}
		}
		/* fall through */
	case TQEvent::KeyRelease:
		updateLockStatus();
		/* fall through */
	default:
		break;
	}
	return inherited::eventFilter( o, e );
}

void
KGVerify::updateLockStatus()
{
	unsigned int lmask;
	Window dummy1, dummy2;
	int dummy3, dummy4, dummy5, dummy6;
	XQueryPointer( tqt_xdisplay(), DefaultRootWindow( tqt_xdisplay() ),
	               &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
	               &lmask );
	capsLocked = lmask & LockMask;
	updateStatus();
}

void
KGVerify::MsgBox( TQMessageBox::Icon typ, const TQString &msg )
{
	timer.suspend();
	FDialog::box( parent, typ, msg );
	timer.resume();
}


TQVariant // public static
KGVerify::getConf( void *, const char *key, const TQVariant &dflt )
{
	if (!qstrcmp( key, "EchoMode" ))
		return TQVariant( _echoMode );
	else {
		TQString fkey = TQString::fromLatin1( key ) + '=';
		for (TQStringList::ConstIterator it = _pluginOptions.begin();
		     it != _pluginOptions.end(); ++it)
			if ((*it).startsWith( fkey ))
				return (*it).mid( fkey.length() );
		return dflt;
	}
}

TQValueVector<GreeterPluginHandle> KGVerify::greetPlugins;

PluginList
KGVerify::init( const TQStringList &plugins )
{
	PluginList pluginList;

	for (TQStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it) {
		GreeterPluginHandle plugin;
		TQString path = KLibLoader::self()->findLibrary(
			((*it)[0] == '/' ? *it : "kgreet_" + *it ).latin1() );
		if (path.isEmpty()) {
			LogError( "GreeterPlugin %s does not exist\n", (*it).latin1() );
			continue;
		}
		uint i, np = greetPlugins.count();
		for (i = 0; i < np; i++)
			if (greetPlugins[i].library->fileName() == path)
				goto next;
		if (!(plugin.library = KLibLoader::self()->library( path.latin1() ))) {
			LogError( "Cannot load GreeterPlugin %s (%s)\n", (*it).latin1(), path.latin1() );
			continue;
		}
		if (!plugin.library->hasSymbol( "kgreeterplugin_info" )) {
			LogError( "GreeterPlugin %s (%s) is no valid greet widget plugin\n",
			          (*it).latin1(), path.latin1() );
			plugin.library->unload();
			continue;
		}
		plugin.info = (kgreeterplugin_info*)plugin.library->symbol( "kgreeterplugin_info" );
		if (!plugin.info->init( TQString::null, getConf, 0 )) {
			LogError( "GreeterPlugin %s (%s) refuses to serve\n",
			          (*it).latin1(), path.latin1() );
			plugin.library->unload();
			continue;
		}
		Debug( "GreeterPlugin %s (%s) loaded\n", (*it).latin1(), plugin.info->name );
		greetPlugins.append( plugin );
	  next:
		pluginList.append( i );
	}
	return pluginList;
}

void
KGVerify::done()
{
	for (uint i = 0; i < greetPlugins.count(); i++) {
		if (greetPlugins[i].info->done)
			greetPlugins[i].info->done();
		greetPlugins[i].library->unload();
	}
}


KGStdVerify::KGStdVerify( KGVerifyHandler *_handler, TQWidget *_parent,
                          TQWidget *_predecessor, const TQString &_fixedUser,
                          const PluginList &_pluginList,
                          KGreeterPlugin::Function _func,
                          KGreeterPlugin::Context _ctx )
	: inherited( _handler, 0, _parent, _predecessor, _fixedUser,
	             _pluginList, _func, _ctx )
	, failedLabelState( 0 )
{
	grid = new TQGridLayout;
	grid->setAlignment( AlignCenter );

	failedLabel = new TQLabel( parent );
	failedLabel->setFont( _failFont );
	grid->addWidget( failedLabel, 1, 0, Qt::AlignCenter );

	updateLockStatus();
}

KGStdVerify::~KGStdVerify()
{
	grid->removeItem( greet->getLayoutItem() );
}

void // public
KGStdVerify::selectPlugin( int id )
{
	inherited::selectPlugin( id );
	grid->addItem( greet->getLayoutItem(), 0, 0 );
	showWidgets( greet->getLayoutItem() );
}

void // private slot
KGStdVerify::slotPluginSelected( int id )
{
	if (failed)
		return;
	if (id != curPlugin) {
		plugMenu->setItemChecked( curPlugin, false );
		parent->setUpdatesEnabled( false );
		grid->removeItem( greet->getLayoutItem() );
		Debug( "delete %s\n", pName.data() );
		delete greet;
		selectPlugin( id );
		handler->verifyPluginChanged( id );
		if (running) {
			start();
		}
		parent->setUpdatesEnabled( true );
	}
}

void
KGStdVerify::updateStatus()
{
	int nfls;

	if (!enabled)
		nfls = 1;
	else if (failed)
		nfls = 2;
	else if (timedLeft)
		nfls = -timedLeft;
	else if (!suspended && capsLocked)
		nfls = 3;
	else
		nfls = 1;

	if (failedLabelState != nfls) {
		failedLabelState = nfls;
		if (nfls < 0) {
			failedLabel->setPaletteForegroundColor( Qt::black );
			failedLabel->setText( i18n( "Automatic login in 1 second...",
			                            "Automatic login in %n seconds...",
			                            timedLeft ) );
		} else {
			switch (nfls) {
			default:
				failedLabel->clear();
				break;
			case 3:
				failedLabel->setPaletteForegroundColor( Qt::red );
				failedLabel->setText( i18n("Warning: Caps Lock on") );
				break;
			case 2:
				failedLabel->setPaletteForegroundColor( Qt::black );
				failedLabel->setText( authTok ?
				                         i18n("Change failed") :
				                         fixedEntity.isEmpty() ?
				                            i18n("Login failed") :
				                            i18n("Authentication failed") );
				break;
			}
		}
	}
}

KGThemedVerify::KGThemedVerify( KGVerifyHandler *_handler,
                                KdmThemer *_themer,
                                TQWidget *_parent, TQWidget *_predecessor,
                                const TQString &_fixedUser,
                                const PluginList &_pluginList,
                                KGreeterPlugin::Function _func,
                                KGreeterPlugin::Context _ctx )
	: inherited( _handler, _themer, _parent, _predecessor, _fixedUser,
	             _pluginList, _func, _ctx )
{
	updateLockStatus();
}

KGThemedVerify::~KGThemedVerify()
{
}

void // public
KGThemedVerify::selectPlugin( int id )
{
	inherited::selectPlugin( id );
	TQLayoutItem *l;
	KdmItem *n;
	if (themer && (l = greet->getLayoutItem())) {
		if (!(n = themer->findNode( "talker" )))
			MsgBox( errorbox,
			        i18n("Theme not usable with authentication method '%1'.")
			        .arg( i18n(greetPlugins[pluginList[id]].info->name) ) );
		else {
			n->setLayoutItem( l );
			showWidgets( l );
		}
	}
	if (themer)
		themer->updateGeometry( true );
}

void // private slot
KGThemedVerify::slotPluginSelected( int id )
{
	if (failed)
		return;
	if (id != curPlugin) {
		plugMenu->setItemChecked( curPlugin, false );
		Debug( "delete %s\n", pName.data() );
		delete greet;
		selectPlugin( id );
		handler->verifyPluginChanged( id );
		if (running) {
			start();
		}
	}
}

void
KGThemedVerify::updateStatus()
{
	handler->updateStatus( enabled && failed,
	                       enabled && !suspended && capsLocked,
	                       timedLeft );
}


KGChTok::KGChTok( TQWidget *_parent, const TQString &user,
                  const PluginList &pluginList, int curPlugin,
                  KGreeterPlugin::Function func,
                  KGreeterPlugin::Context ctx )
	: inherited( _parent )
	, verify( 0 )
{
	TQSizePolicy fp( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
	okButton = new KPushButton( KStdGuiItem::ok(), this );
	okButton->setSizePolicy( fp );
	okButton->setDefault( true );
	cancelButton = new KPushButton( KStdGuiItem::cancel(), this );
	cancelButton->setSizePolicy( fp );

	verify = new KGStdVerify( this, this, cancelButton, user, pluginList, func, ctx );
	verify->selectPlugin( curPlugin );

	TQVBoxLayout *box = new TQVBoxLayout( this, 10 );

	box->addWidget( new TQLabel( i18n("Changing authentication token"), this ), 0, AlignHCenter );

	box->addLayout( verify->getLayout() );

	box->addWidget( new KSeparator( KSeparator::HLine, this ) );

	TQHBoxLayout *hlay = new TQHBoxLayout( box );
	hlay->addStretch( 1 );
	hlay->addWidget( okButton );
	hlay->addStretch( 1 );
	hlay->addWidget( cancelButton );
	hlay->addStretch( 1 );

	connect( okButton, TQT_SIGNAL(clicked()), TQT_SLOT(accept()) );
	connect( cancelButton, TQT_SIGNAL(clicked()), TQT_SLOT(reject()) );

	TQTimer::singleShot( 0, verify, TQT_SLOT(start()) );
}

KGChTok::~KGChTok()
{
	hide();
	delete verify;
}

void
KGChTok::accept()
{
	verify->accept();
}

void
KGChTok::verifyPluginChanged( int )
{
	// cannot happen
}

void
KGChTok::verifyOk()
{
	inherited::accept();
}

void
KGChTok::verifyFailed()
{
	okButton->setEnabled( false );
	cancelButton->setEnabled( false );
}

void
KGChTok::verifyRetry()
{
	okButton->setEnabled( true );
	cancelButton->setEnabled( true );
}

void
KGChTok::verifySetUser( const TQString & )
{
	// cannot happen
}


////// helper class, nuke when qtimer supports suspend()/resume()

QXTimer::QXTimer()
	: inherited( 0 )
	, left( -1 )
{
	connect( &timer, TQT_SIGNAL(timeout()), TQT_SLOT(slotTimeout()) );
}

void
QXTimer::start( int msec )
{
	left = msec;
	timer.start( left, true );
	gettimeofday( &stv, 0 );
}

void
QXTimer::stop()
{
	timer.stop();
	left = -1;
}

void
QXTimer::suspend()
{
	if (timer.isActive()) {
		timer.stop();
		struct timeval tv;
		gettimeofday( &tv, 0 );
		left -= (tv.tv_sec - stv.tv_sec) * 1000 + (tv.tv_usec - stv.tv_usec) / 1000;
		if (left < 0)
			left = 0;
	}
}

void
QXTimer::resume()
{
	if (left >= 0 && !timer.isActive()) {
		timer.start( left, true );
		gettimeofday( &stv, 0 );
	}
}

void
QXTimer::slotTimeout()
{
	left = 0;
	emit timeout();
}


#include "kgverify.moc"
