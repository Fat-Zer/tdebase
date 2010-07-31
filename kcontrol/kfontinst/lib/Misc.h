#ifndef __MISC_H__
#define __MISC_H__

////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Misc
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 01/05/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <kurl.h>

class QWidget;

namespace KFI
{

namespace Misc
{
    enum EConstants
    {
        FILE_PERMS   = 0644,
        DIR_PERMS    = 0755
    };

    extern KDE_EXPORT bool    check(const TQString &path, unsigned int fmt, bool checkW=false);
    inline KDE_EXPORT bool    fExists(const TQString &p)     { return check(p, S_IFREG, false); }
    inline KDE_EXPORT bool    dExists(const TQString &p)     { return check(p, S_IFDIR, false); }
    inline KDE_EXPORT bool    fWritable(const TQString &p)   { return check(p, S_IFREG, true); }
    inline KDE_EXPORT bool    dWritable(const TQString &p)   { return check(p, S_IFDIR, true); }
    inline KDE_EXPORT bool    isLink(const TQString &i)      { return check(i, S_IFLNK, false); }
    extern KDE_EXPORT TQString linkedTo(const TQString &i);
    extern KDE_EXPORT TQString dirSyntax(const TQString &d);  // Has trailing slash:  /file/path/
    extern KDE_EXPORT TQString xDirSyntax(const TQString &d); // No trailing slash:   /file/path
    inline KDE_EXPORT TQString fileSyntax(const TQString &f)  { return xDirSyntax(f); }
    extern KDE_EXPORT TQString getDir(const TQString &f);
    extern KDE_EXPORT TQString getFile(const TQString &f);
    extern KDE_EXPORT bool    createDir(const TQString &dir);
    extern KDE_EXPORT TQString changeExt(const TQString &f, const TQString &newExt);
    extern KDE_EXPORT bool    doCmd(const TQString &cmd, const TQString &p1=TQString::null, const TQString &p2=TQString::null, const TQString &p3=TQString::null);
    inline KDE_EXPORT bool    root() { return 0==getuid(); }
    extern KDE_EXPORT void    getAssociatedUrls(const KURL &url, KURL::List &list, bool afmAndPfm=true, TQWidget *widget=NULL);
    extern KDE_EXPORT void    createBackup(const TQString &f);
    extern KDE_EXPORT time_t  getTimeStamp(const TQString &item);
}

}

#endif
