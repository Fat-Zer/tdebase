/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#ifndef REMOTEIMPL_H
#define REMOTEIMPL_H

#include <tdeio/global.h>
#include <tdeio/job.h>
#include <kurl.h>

#include <tqstring.h>

class RemoteImpl
{
public:
	RemoteImpl();

	void createTopLevelEntry(TDEIO::UDSEntry &entry) const;
	bool createWizardEntry(TDEIO::UDSEntry &entry) const;
	bool isWizardURL(const KURL &url) const;
	bool statNetworkFolder(TDEIO::UDSEntry &entry, const TQString &filename) const;

	void listRoot(TQValueList<TDEIO::UDSEntry> &list) const;

	KURL findBaseURL(const TQString &filename) const;
	TQString findDesktopFile(const TQString &filename) const;
	
	bool deleteNetworkFolder(const TQString &filename) const;
	bool renameFolders(const TQString &src, const TQString &dest,
	                   bool overwrite) const;

private:
	bool findDirectory(const TQString &filename, TQString &directory) const;
	void createEntry(TDEIO::UDSEntry& entry, const TQString &directory,
	                 const TQString &file) const;
};

#endif
