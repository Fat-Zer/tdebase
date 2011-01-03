#ifndef __KIO_FONTS_H__
#define __KIO_FONTS_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKioFonts
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/03/2003
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

#include <fontconfig/fontconfig.h>
#include <time.h>
#include <kio/slavebase.h>
#include <kurl.h>
#include <klocale.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqmap.h>
#include <tqvaluelist.h>
#include "Misc.h"
#include "KfiConstants.h"

namespace KFI
{

class CKioFonts : public KIO::SlaveBase
{
    private:

    enum EConstants
    {
        KFI_PARAMS = 8
    };

    enum EDest
    {
        DEST_UNCHANGED,
        DEST_SYS,
        DEST_USER
    };

    enum EFolder
    {
        FOLDER_SYS,
        FOLDER_USER,

        FOLDER_COUNT
    };

    enum EOp
    {
        OP_COPY,
        OP_MOVE,
        OP_DELETE
    };

    class CDirList : public QStringList
    {
        public:

        CDirList()                                      { }
        CDirList(const TQString &str) : TQStringList(str) { }

        void add(const TQString &d)                      { if (!tqcontains(d)) append(d); }
    };

    struct TFolder
    {
        TQString                                 location;
        CDirList                                modified;
        TQMap<TQString, TQValueList<FcPattern *> > fontMap;   // Maps from "Times New Roman" -> $HOME/.fonts/times.ttf
    };

    public:

    CKioFonts(const TQCString &pool, const TQCString &app);
    virtual ~CKioFonts();

    static TQString getSect(const TQString &f) { return f.section('/', 1, 1); }

    void listDir(const KURL &url);
    void stat(const KURL &url);
    bool createStatEntry(KIO::UDSEntry &entry, const KURL &url, EFolder folder);
    void get(const KURL &url);
    void put(const KURL &url, int mode, bool overwrite, bool resume);
    void copy(const KURL &src, const KURL &dest, int mode, bool overwrite);
    void rename(const KURL &src, const KURL &dest, bool overwrite);
    void del(const KURL &url, bool isFile);

    private:

    bool     putReal(const TQString &destOrig, const TQCString &destOrigC, bool origExists, int mode, bool resume);
    void     modified(EFolder folder, bool clearList=true, const CDirList &dirs=CDirList());
    void     special(const TQByteArray &a);
    void     createRootRefreshCmd(TQCString &cmd, const CDirList &dirs=CDirList(), bool reparseCfg=true);
    void     doModified();
    TQString  getRootPasswd(bool askPasswd=true);
    bool     doRootCmd(const char *cmd, const TQString &passwd);
    bool     doRootCmd(const char *cmd, bool askPasswd=true) { return doRootCmd(cmd, getRootPasswd(askPasswd)); }
    bool     confirmUrl(KURL &url);
    void     clearFontList();
    bool     updateFontList();
    EFolder  getFolder(const KURL &url);
    TQMap<TQString, TQValueList<FcPattern *> >::Iterator getMap(const KURL &url);
    TQValueList<FcPattern *> * getEntries(const KURL &url);
    FcPattern * getEntry(EFolder folder, const TQString &file, bool full=false);
    bool     checkFile(const TQString &file);
    bool     getSourceFiles(const KURL &src, TQStringList &files);
    bool     checkDestFile(const KURL &src, const KURL &dest, EFolder destFolder, bool overwrite);
    bool     checkDestFiles(const KURL &src, TQMap<TQString, TQString> &map, const KURL &dest, EFolder destFolder, bool overwrite);
    bool     confirmMultiple(const KURL &url, const TQStringList &files, EFolder folder, EOp op);
    bool     confirmMultiple(const KURL &url, TQValueList<FcPattern *> *patterns, EFolder folder, EOp op);
    bool     checkUrl(const KURL &u, bool rootOk=false);
    bool     checkAllowed(const KURL &u);
    void     createAfm(const TQString &file, bool nrs=false, const TQString &passwd=TQString::null);
    void     reparseConfig();

    private:

    bool         itsRoot,
                 itsCanStorePasswd,
                 itsUsingFcFpe,
                 itsUsingXfsFpe,
                 itsHasSys,
                 itsAddToSysFc;
    TQString      itsPasswd;
    unsigned int itsFontChanges;
    EDest        itsLastDest;
    time_t       itsLastDestTime,
                 itsLastFcCheckTime;
    FcFontSet    *itsFontList;
    TFolder      itsFolders[FOLDER_COUNT];
    char         itsNrsKfiParams[KFI_PARAMS],
                 itsNrsNonMainKfiParams[KFI_PARAMS],
                 itsKfiParams[KFI_PARAMS];
};

}

#endif
