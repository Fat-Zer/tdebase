/*  This file is part of the KDE libraries

    Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __tdeio_about_h__
#define __tdeio_about_h__

#include <tqcstring.h>

#include <tdeio/global.h>
#include <tdeio/slavebase.h>


class AboutProtocol : public TDEIO::SlaveBase
{
public:
    AboutProtocol(const TQCString &pool_socket, const TQCString &app_socket);
    virtual ~AboutProtocol();

    virtual void get(const KURL& url);
    virtual void mimetype(const KURL& url);
};

#endif
