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

#include "remotemenu.h"

#include <kdebug.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kservice.h>

#include <tqpixmap.h>
#include <tqdir.h>
#include <tqtimer.h>

#include "kickerSettings.h"

#define WIZARD_SERVICE "knetattach"

K_EXPORT_KICKER_MENUEXT(remotemenu, RemoteMenu)


RemoteMenu::RemoteMenu(TQWidget *parent, const char *name,
                       const TQStringList &/*args*/)
  : KPanelMenu(parent, name), KDirNotify()
{
    TDEGlobal::dirs()->addResourceType("remote_entries",
    TDEStandardDirs::kde_default("data") + "remoteview");

    TQString path = TDEGlobal::dirs()->saveLocation("remote_entries");

    TQDir dir = path;
    if (!dir.exists())
    {
        dir.cdUp();
        dir.mkdir("remoteview");
    }
}

RemoteMenu::~RemoteMenu()
{
}

void RemoteMenu::initialize()
{
    int id = 0;
    if (KickerSettings::showMenuTitles())
    {
        insertTitle(i18n("Network Folders"));
    }

    id = insertItem(SmallIcon("wizard"), i18n("Add Network Folder"));
    connectItem(id, this, TQT_SLOT(startWizard()));
    id = insertItem(SmallIcon("kfm"), i18n("Manage Network Folders"));
    connectItem(id, this, TQT_SLOT(openRemoteDir()));

    insertSeparator();

    m_desktopMap.clear();
    TQStringList names_found;
    TQStringList dirList = TDEGlobal::dirs()->resourceDirs("remote_entries");

    TQStringList::ConstIterator dirpath = dirList.begin();
    TQStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        TQDir dir = *dirpath;
        if (!dir.exists()) continue;

        TQStringList filenames
            = dir.entryList( TQDir::Files | TQDir::Readable );

        TQStringList::ConstIterator name = filenames.begin();
        TQStringList::ConstIterator endf = filenames.end();

        for(; name!=endf; ++name)
        {
            if (!names_found.contains(*name))
            {
                names_found.append(*name);
                TQString filename = *dirpath+*name;
                KDesktopFile desktop(filename);
                id = insertItem(SmallIcon(desktop.readIcon()), desktop.readName());
                m_desktopMap[id] = filename;
            }
        }
    }
}

void RemoteMenu::startWizard()
{
    KURL url;
    KService::Ptr service = KService::serviceByDesktopName(WIZARD_SERVICE);

    if (service && service->isValid())
    {
        url.setPath(locate("apps", service->desktopEntryPath()));
        new KRun(url, 0, true); // will delete itself
    }
}

void RemoteMenu::openRemoteDir()
{
    new KRun(KURL("remote:/"));
}

void RemoteMenu::slotExec(int id)
{
    if (m_desktopMap.contains(id))
    {
        new KRun(m_desktopMap[id]);
    }
}

ASYNC RemoteMenu::FilesAdded(const KURL &directory)
{
    if (directory.protocol()=="remote") reinitialize();
}

ASYNC RemoteMenu::FilesRemoved(const KURL::List &fileList)
{
    KURL::List::ConstIterator it = fileList.begin();
    KURL::List::ConstIterator end = fileList.end();
    
    for (; it!=end; ++it)
    {
        if ((*it).protocol()=="remote")
        {
            reinitialize();
            return;
        }
    }
}

ASYNC RemoteMenu::FilesChanged(const KURL::List &fileList)
{
    FilesRemoved(fileList);
}

ASYNC RemoteMenu::FilesRenamed(const KURL &src, const KURL &dest)
{
    if (src.protocol()=="remote" || dest.protocol()=="remote")
        reinitialize();
}

#include "remotemenu.moc"
