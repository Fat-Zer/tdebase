/* $XFree86: xc/programs/xterm/wcwidth.h,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

/* Markus Kuhn -- 2001-01-12 -- public domain */
/* Adaptions for KDE by Waldo Bastian <bastian@kde.org> */

#ifndef	_KONSOLE_WCWIDTH_H_
#define	_KONSOLE_WCWIDTH_H_

#include <tqglobal.h>
#include <tqstring.h>

int konsole_wcwidth(TQ_UINT16 ucs);
//int konsole_wcwidth_cjk(TQ_UINT16 ucs);

int string_width( const TQString &txt );

#endif
