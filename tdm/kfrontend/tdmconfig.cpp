/*

Config for tdm

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

#include "tdmconfig.h"
#include "tdm_greet.h"

#include <tdeapplication.h>
#include <tdelocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

struct timeval st = {0, 0};

CONF_GREET_DEFS

TQString _stsFile;
bool _isLocal;
bool _authorized;

static TQString
GetCfgQStr( int id )
{
	char *tmp = GetCfgStr( id );
	TQString qs = TQString::fromUtf8( tmp );
	free( tmp );
	return qs;
}

static TQStringList
GetCfgQStrList( int id )
{
	int i, len;
	char **tmp = GetCfgStrArr( id, &len );
	TQStringList qsl;
	for (i = 0; i < len - 1; i++) {
		qsl.append( TQString::fromUtf8( tmp[i] ) );
		free( tmp[i] );
	}
	free( tmp );
	return qsl;
}

// Based on tdeconfigbase.cpp
static TQFont
Str2Font( const TQString &aValue )
{
	uint nFontBits;
	TQFont aRetFont;
	TQString chStr;

	TQStringList sl = TQStringList::split( TQString::fromLatin1(","), aValue );

	if (sl.count() == 1) {
		/* X11 font spec */
		aRetFont = TQFont( aValue );
		aRetFont.setRawMode( true );
	} else if (sl.count() == 10) {
		/* qt3 font spec */
		aRetFont.fromString( aValue );
	} else if (sl.count() == 6) {
		/* backward compatible kde2 font spec */
		aRetFont = TQFont( sl[0], sl[1].toInt(), sl[4].toUInt() );

		aRetFont.setStyleHint( (TQFont::StyleHint)sl[2].toUInt() );

		nFontBits = sl[5].toUInt();
		aRetFont.setItalic( (nFontBits & 0x01) != 0 );
		aRetFont.setUnderline( (nFontBits & 0x02) != 0 );
		aRetFont.setStrikeOut( (nFontBits & 0x04) != 0 );
		aRetFont.setFixedPitch( (nFontBits & 0x08) != 0 );
		aRetFont.setRawMode( (nFontBits & 0x20) != 0 );
	}
	aRetFont.setStyleStrategy( (TQFont::StyleStrategy)
	   (TQFont::PreferMatch |
	    (_antiAliasing ? TQFont::PreferAntialias : TQFont::NoAntialias)) );

	return aRetFont;
}

extern "C"
void init_config( void )
{
	CONF_GREET_INIT

	_isLocal = GetCfgInt( C_isLocal );
	_hasConsole = _hasConsole && _isLocal && GetCfgInt( C_hasConsole );
	_authorized = GetCfgInt( C_isAuthorized );

	_stsFile = _dataDir + "/tdmsts";

	// Greet String
	char hostname[256], *ptr;
	hostname[0] = '\0';
	if (!gethostname( hostname, sizeof(hostname) ))
		hostname[sizeof(hostname)-1] = '\0';
	struct utsname tuname;
	uname( &tuname );
	TQString gst = _greetString;
	_greetString = TQString::null;
	int i, j, l = gst.length();
	for (i = 0; i < l; i++) {
		if (gst[i] == '%') {
			switch (gst[++i].cell()) {
			case '%': _greetString += gst[i]; continue;
			case 'd': ptr = dname; break;
			case 'h': ptr = hostname; break;
			case 'n': ptr = tuname.nodename;
				for (j = 0; ptr[j]; j++)
					if (ptr[j] == '.') {
						ptr[j] = 0;
						break;
					}
				break;
			case 's': ptr = tuname.sysname; break;
			case 'r': ptr = tuname.release; break;
			case 'm': ptr = tuname.machine; break;
			default: _greetString += i18n("[fix tdmrc!]"); continue;
			}
			_greetString += TQString::fromLocal8Bit( ptr );
		} else
			_greetString += gst[i];
	}
}


/* out-of-place utility function */
void
decodeSess( dpySpec *sess, TQString &user, TQString &loc )
{
	if (sess->flags & isTTY) {
		user =
			i18n( "%1: TTY login", "%1: %n TTY logins", sess->count )
				.arg( sess->user );
		loc = 
#ifdef HAVE_VTS
			sess->vt ?
				TQString("vt%1").arg( sess->vt ) :
#endif
				TQString::fromLatin1( *sess->from ? sess->from : sess->display );
	} else {
		user =
			!sess->user ?
				i18n("Unused") :
				*sess->user ?
					i18n("user: session type", "%1: %2")
						.arg( sess->user ).arg( sess->session ) :
					i18n("... host", "X login on %1").arg( sess->session );
		loc =
#ifdef HAVE_VTS
			sess->vt ?
				TQString("%1, vt%2").arg( sess->display ).arg( sess->vt ) :
#endif
				TQString::fromLatin1( sess->display );
	}
}
