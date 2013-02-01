#ifndef __FONTMAP_H__
#define __FONTMAP_H__

////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Fontmap
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 06/06/2003
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
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <tqstring.h>
#include <tqstringlist.h>

namespace KFI
{

class CFontEngine;

namespace Fontmap
{
    class CFile
    {
        private:

        struct TEntry
        {
            TEntry(const TQString &fname) : filename(fname) {}

            TQString     filename,
                        psName;
            TQStringList entries;
        };

        public:

        CFile(const TQString &dir);

        const TQStringList * getEntries(const TQString &fname);
        unsigned int        getLineCount() { return itsLineCount; }

        private:

        TEntry * findEntry(const TQString &fname, bool isAlias=false);
        TEntry * getEntry(TEntry **current, const TQString &fname, bool isAlias=false);

        private:

        TQString          itsDir;
        TQPtrList<TEntry> itsEntries;
        unsigned int     itsLineCount;
    };

    extern bool create(const TQString &dir, CFontEngine &fe);
}

}

#endif
