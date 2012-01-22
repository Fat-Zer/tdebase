/*

Configuration for tdm

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
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


#ifndef TDMCONFIG_H
#define TDMCONFIG_H

#include <config.h>

#include "config.ci"

#ifdef __cplusplus

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqfont.h>
#include <sys/time.h>

extern TQString _stsFile;
extern bool _isLocal;
extern bool _authorized;

CONF_GREET_CPP_DECLS

// this file happens to be included everywhere, so just put it here
struct dpySpec;
void decodeSess( dpySpec *sess, TQString &user, TQString &loc );

extern struct timeval st;

inline TQString timestamp() {
	struct timeval nst;
	gettimeofday(&nst, 0);
	if (!st.tv_sec)
		gettimeofday(&st, 0);

	TQString ret;
	ret.sprintf("[%07ld]", (nst.tv_sec - st.tv_sec) * 1000 + (nst.tv_usec - st.tv_usec) / 1000);
	return ret;
}

extern "C"
#endif
void init_config( void );

CONF_GREET_C_DECLS

#endif /* TDMCONFIG_H */
