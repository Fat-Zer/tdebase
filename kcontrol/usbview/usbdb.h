/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __USB_DB_H__
#define __USB_DB_H__


#include <tqdict.h>


class USBDB
{
public:

  USBDB();

  TQString vendor(int id);
  TQString device(int vendor, int id);

  TQString cls(int cls);
  TQString subclass(int cls, int sub);
  TQString protocol(int cls, int sub, int prot);

private:

  TQDict<TQString> _classes, _ids;

};


#endif
