/*
   Copyright (C) 2004 Oswald Buddenhagen <ossi@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "dmctl.h"

#ifdef Q_WS_X11

#include <klocale.h>
#include <dcopclient.h>

#include <tqregexp.h>

#include <X11/Xauth.h>
#include <X11/Xlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

static enum { Dunno, NoDM, NewTDM, OldTDM, GDM } DMType = Dunno;
static const char *ctl, *dpy;

DM::DM() : fd( -1 )
{
	char *ptr;
	struct sockaddr_un sa;

	if (DMType == Dunno) {
		if (!(dpy = ::getenv( "DISPLAY" )))
			DMType = NoDM;
		else if ((ctl = ::getenv( "DM_CONTROL" )))
			DMType = NewTDM;
		else if ((ctl = ::getenv( "XDM_MANAGED" )) && ctl[0] == '/')
			DMType = OldTDM;
		else if (::getenv( "GDMSESSION" ))
			DMType = GDM;
		else
			DMType = NoDM;
	}
	switch (DMType) {
	default:
		return;
	case NewTDM:
	case GDM:
		if ((fd = ::socket( PF_UNIX, SOCK_STREAM, 0 )) < 0)
			return;
		sa.sun_family = AF_UNIX;
		if (DMType == GDM) {
			strcpy( sa.sun_path, "/var/run/gdm_socket" );
			if (::connect( fd, (struct sockaddr *)&sa, sizeof(sa) )) {
				strcpy( sa.sun_path, "/tmp/.gdm_socket" );
				if (::connect( fd, (struct sockaddr *)&sa, sizeof(sa) )) {
					::close( fd );
					fd = -1;
					break;
				}
			}
			GDMAuthenticate();
		} else {
			if ((ptr = const_cast<char*>(strchr( dpy, ':' ))))
				ptr = strchr( ptr, '.' );
			snprintf( sa.sun_path, sizeof(sa.sun_path),
			          "%s/dmctl-%.*s/socket",
			          ctl, ptr ? int(ptr - dpy) : 512, dpy );
			if (::connect( fd, (struct sockaddr *)&sa, sizeof(sa) )) {
				::close( fd );
				fd = -1;
			}
		}
		break;
	case OldTDM:
		{
			TQString tf( ctl );
			tf.truncate( tf.find( ',' ) );
			fd = ::open( tf.latin1(), O_WRONLY );
		}
		break;
	}
}

DM::~DM()
{
	if (fd >= 0)
		close( fd );
}

bool
DM::exec( const char *cmd )
{
	TQCString buf;

	return exec( cmd, buf );
}

/**
 * Execute a TDM/GDM remote control command.
 * @param cmd the command to execute. FIXME: undocumented yet.
 * @param buf the result buffer.
 * @return result:
 *  @li If true, the command was successfully executed.
 *   @p ret might contain addional results.
 *  @li If false and @p ret is empty, a communication error occurred
 *   (most probably TDM is not running).
 *  @li If false and @p ret is non-empty, it contains the error message
 *   from TDM.
 */
bool
DM::exec( const char *cmd, TQCString &buf )
{
	bool ret = false;
	int tl;
	unsigned len = 0;

	if (fd < 0)
		goto busted;

	tl = strlen( cmd );
	if (::write( fd, cmd, tl ) != tl) {
	    bust:
		::close( fd );
		fd = -1;
	    busted:
		buf.resize( 0 );
		return false;
	}
	if (DMType == OldTDM) {
		buf.resize( 0 );
		return true;
	}
	for (;;) {
		if (buf.size() < 128)
			buf.resize( 128 );
		else if (buf.size() < len * 2)
			buf.resize( len * 2 );
		if ((tl = ::read( fd, buf.data() + len, buf.size() - len)) <= 0) {
			if (tl < 0 && errno == EINTR)
				continue;
			goto bust;
		}
		len += tl;
		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;
			if (len > 2 && (buf[0] == 'o' || buf[0] == 'O') &&
			    (buf[1] == 'k' || buf[1] == 'K') && buf[2] <= 32)
				ret = true;
			break;
		}
	}
	return ret;
}

bool
DM::canShutdown()
{
	if (DMType == OldTDM)
		return strstr( ctl, ",maysd" ) != 0;

	TQCString re;

	if (DMType == GDM)
		return exec( "QUERY_LOGOUT_ACTION\n", re ) && re.find("HALT") >= 0;

	return exec( "caps\n", re ) && re.find( "\tshutdown" ) >= 0;
}

void
DM::shutdown( KApplication::ShutdownType shutdownType,
              KApplication::ShutdownMode shutdownMode, /* NOT Default */
              const TQString &bootOption )
{
	if (shutdownType == KApplication::ShutdownTypeNone)
		return;

	bool cap_ask;
	if (DMType == NewTDM) {
		TQCString re;
		cap_ask = exec( "caps\n", re ) && re.find( "\tshutdown ask" ) >= 0;
	} else {
		if (!bootOption.isEmpty())
			return;
		cap_ask = false;
	}
	if (!cap_ask && shutdownMode == KApplication::ShutdownModeInteractive)
		shutdownMode = KApplication::ShutdownModeForceNow;

	TQCString cmd;
	if (DMType == GDM) {
		cmd.append( shutdownMode == KApplication::ShutdownModeForceNow ?
		            "SET_LOGOUT_ACTION " : "SET_SAFE_LOGOUT_ACTION " );
		cmd.append( shutdownType == KApplication::ShutdownTypeReboot ?
		            "REBOOT\n" : "HALT\n" );
	} else {
		cmd.append( "shutdown\t" );
		cmd.append( shutdownType == KApplication::ShutdownTypeReboot ?
		            "reboot\t" : "halt\t" );
		if (!bootOption.isEmpty())
			cmd.append( "=" ).append( bootOption.local8Bit() ).append( "\t" );
		cmd.append( shutdownMode == KApplication::ShutdownModeInteractive ?
		            "ask\n" :
		            shutdownMode == KApplication::ShutdownModeForceNow ?
		            "forcenow\n" :
		            shutdownMode == KApplication::ShutdownModeTryNow ?
		            "trynow\n" : "schedule\n" );
	}
	exec( cmd.data() );
}

bool
DM::bootOptions( TQStringList &opts, int &defopt, int &current )
{
	if (DMType != NewTDM)
		return false;

	TQCString re;
	if (!exec( "listbootoptions\n", re ))
		return false;

	opts = TQStringList::split( '\t', TQString::fromLocal8Bit( re.data() ) );
	if (opts.size() < 4)
		return false;

	bool ok;
	defopt = opts[2].toInt( &ok );
	if (!ok)
		return false;
	current = opts[3].toInt( &ok );
	if (!ok)
		return false;

	opts = TQStringList::split( ' ', opts[1] );
	for (TQStringList::Iterator it = opts.begin(); it != opts.end(); ++it)
		(*it).replace( "\\s", " " );

	return true;
}

void
DM::setLock( bool on )
{
	if (DMType != GDM)
		exec( on ? "lock\n" : "unlock\n" );
}

bool
DM::isSwitchable()
{
	if (DMType == OldTDM)
		return dpy[0] == ':';

	if (DMType == GDM)
		return exec( "QUERY_VT\n" );

	TQCString re;

	return exec( "caps\n", re ) && re.find( "\tlocal" ) >= 0;
}

int
DM::numReserve()
{
	if (DMType == GDM)
		return 1; /* Bleh */

	if (DMType == OldTDM)
		return strstr( ctl, ",rsvd" ) ? 1 : -1;

	TQCString re;
	int p;

	if (!(exec( "caps\n", re ) && (p = re.find( "\treserve " )) >= 0))
		return -1;
	return atoi( re.data() + p + 9 );
}

void
DM::startReserve()
{
	if (DMType == GDM)
		exec("FLEXI_XSERVER\n");
	else
		exec("reserve\n");
}

bool
DM::localSessions( SessList &list )
{
	if (DMType == OldTDM)
		return false;

	TQCString re;

	if (DMType == GDM) {
		if (!exec( "CONSOLE_SERVERS\n", re ))
			return false;
		TQStringList sess = TQStringList::split( TQChar(';'), re.data() + 3 );
		for (TQStringList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
			TQStringList ts = TQStringList::split( TQChar(','), *it, true );
			SessEnt se;
			se.display = ts[0];
			se.user = ts[1];
			se.vt = ts[2].toInt();
			se.session = "<unknown>";
			se.self = ts[0] == ::getenv( "DISPLAY" ); /* Bleh */
			se.tty = false;
			list.append( se );
		}
	} else {
		if (!exec( "list\talllocal\n", re ))
			return false;
		TQStringList sess = TQStringList::split( TQChar('\t'), re.data() + 3 );
		for (TQStringList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
			TQStringList ts = TQStringList::split( TQChar(','), *it, true );
			SessEnt se;
			se.display = ts[0];
			if (ts[1][0] == '@')
				se.from = ts[1].mid( 1 ), se.vt = 0;
			else
				se.vt = ts[1].mid( 2 ).toInt();
			se.user = ts[2];
			se.session = ts[3];
			se.self = (ts[4].find( '*' ) >= 0);
			se.tty = (ts[4].find( 't' ) >= 0);
			list.append( se );
		}
	}
	return true;
}

void
DM::sess2Str2( const SessEnt &se, TQString &user, TQString &loc )
{
	if (se.tty) {
		user = i18n("user: ...", "%1: TTY login").arg( se.user );
		loc = se.vt ? TQString(TQString("vt%1").arg( se.vt )) : se.display ;
	} else {
		user =
			se.user.isEmpty() ?
				se.session.isEmpty() ?
					i18n("Unused") :
					se.session == "<remote>" ?
						i18n("X login on remote host") :
						TQString(i18n("... host", "X login on %1").arg( se.session )) :
				se.session == "<unknown>" ?
					se.user :
					TQString(i18n("user: session type", "%1: %2")
						.arg( se.user ).arg( se.session ));
		loc =
			se.vt ?
				TQString(TQString("%1, vt%2").arg( se.display ).arg( se.vt )) :
				se.display;
	}
}

TQString
DM::sess2Str( const SessEnt &se )
{
	TQString user, loc;

	sess2Str2( se, user, loc );
	return i18n("session (location)", "%1 (%2)").arg( user ).arg( loc );
}

bool
DM::switchVT( int vt )
{
	if (DMType == GDM)
		return exec( TQString(TQString("SET_VT %1\n").arg(vt)).latin1() );

	return exec( TQString(TQString("activate\tvt%1\n").arg(vt)).latin1() );
}

void
DM::lockSwitchVT( int vt )
{
	if (switchVT( vt ))
		kapp->dcopClient()->send( "kdesktop", "KScreensaverIface", "lock()", TQString("") );
}

void
DM::GDMAuthenticate()
{
	FILE *fp;
	const char *dpy, *dnum, *dne;
	int dnl;
	Xauth *xau;

	dpy = DisplayString( TQPaintDevice::x11AppDisplay() );
	if (!dpy) {
		dpy = ::getenv( "DISPLAY" );
		if (!dpy)
			return;
	}
	dnum = strchr( dpy, ':' ) + 1;
	dne = strchr( dpy, '.' );
	dnl = dne ? dne - dnum : strlen( dnum );

	/* XXX should do locking */
	if (!(fp = fopen( XauFileName(), "r" )))
		return;

	while ((xau = XauReadAuth( fp ))) {
		if (xau->family == FamilyLocal &&
		    xau->number_length == dnl && !memcmp( xau->number, dnum, dnl ) &&
		    xau->data_length == 16 &&
		    xau->name_length == 18 && !memcmp( xau->name, "MIT-MAGIC-COOKIE-1", 18 ))
		{
			TQString cmd( "AUTH_LOCAL " );
			for (int i = 0; i < 16; i++)
				cmd += TQString::number( (uchar)xau->data[i], 16 ).rightJustify( 2, '0');
			cmd += "\n";
			if (exec( cmd.latin1() )) {
				XauDisposeAuth( xau );
				break;
			}
		}
		XauDisposeAuth( xau );
	}

	fclose (fp);
}

#endif // Q_WS_X11