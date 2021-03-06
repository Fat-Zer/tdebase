/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <tqdir.h>
#include <tqfileinfo.h>

#include <tdeapplication.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdeio/global.h>
#include <tdelocale.h>
#include <krun.h>
#include <kshell.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "konsole_mnu.h"

K_EXPORT_KICKER_MENUEXT(konsole, KonsoleMenu)

KonsoleMenu::KonsoleMenu(TQWidget *parent, const char *name, const TQStringList& /* args */)
    : KPanelMenu("", parent, name),
      m_profileMenu(0),
      m_bookmarksSession(0),
      m_bookmarkHandlerSession(0)
{
}

KonsoleMenu::~KonsoleMenu()
{
    TDEGlobal::locale()->removeCatalogue("libkickermenu_konsole");
}

static void insertItemSorted(TDEPopupMenu *menu,
                             const TQIconSet &iconSet,
                             const TQString &txt, int id)
{
  const int defaultId = 1; // The id of the 'new' item.
  int index = menu->indexOf(defaultId);
  int count = menu->count();
  if (index >= 0)
  {
     index++; // Skip separator
     while(true)
     {
        index++;
        if (index >= count)
        {
           index = -1; // Insert at end
           break;
        }
        if (menu->text(menu->idAt(index)) > txt)
           break; // Insert before this item
     }
  }
  menu->insertItem(iconSet, txt, id, index);
}


void KonsoleMenu::initialize()
{
    if (initialized())
    {
        clear();
    }
    else
    {
        kapp->iconLoader()->addAppDir("konsole");
    }

    setInitialized(true);

    TQStringList list = TDEGlobal::dirs()->findAllResources("data",
                                                         "konsole/*.desktop",
                                                          false, true);

    TQString defaultShell = locate("data", "konsole/shell.desktop");
    list.prepend(defaultShell);

    int id = 1;

    sessionList.clear();
    for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it == defaultShell) && (id != 1))
        {
           continue;
        }

        KSimpleConfig conf(*it, true /* read only */);
        conf.setDesktopGroup();
        TQString text = conf.readEntry("Name");

        // try to locate the binary
        TQString exec= conf.readPathEntry("Exec");
        if (exec.startsWith("su -c \'"))
        {
             exec = exec.mid(7,exec.length()-8);
        }

        exec = KRun::binaryName(exec, false);
        exec = KShell::tildeExpand(exec);
        TQString pexec = TDEGlobal::dirs()->findExe(exec);
        if (text.isEmpty() ||
            conf.readEntry("Type") != "KonsoleApplication" ||
            (!exec.isEmpty() && pexec.isEmpty()))
        {
            continue;
        }
        insertItemSorted(this, SmallIconSet(conf.readEntry("Icon", "konsole")),
                                            text, id++);
        TQFileInfo fi(*it);
        sessionList.append(fi.baseName(true));

        if (id == 2)
        {
           insertSeparator();
        }
    }

    m_bookmarkHandlerSession = new KonsoleBookmarkHandler(this, false);
    m_bookmarksSession = m_bookmarkHandlerSession->menu();
    insertSeparator();
    insertItem(SmallIconSet("keditbookmarks"),
               i18n("New Session at Bookmark"), m_bookmarksSession);
    connect(m_bookmarkHandlerSession,
            TQT_SIGNAL(openURL(const TQString&, const TQString&)),
            TQT_SLOT(newSession(const TQString&, const TQString&)));


    screenList.clear();
    TQCString screenDir = getenv("SCREENDIR");

    if (screenDir.isEmpty())
    {
        screenDir = TQFile::encodeName(TQDir::homeDirPath()) + "/.screen/";
    }

    TQStringList sessions;
    // Can't use TQDir as it doesn't support FIFOs :(
    DIR *dir = opendir(screenDir);
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)))
        {
            TQCString path = screenDir + "/" + entry->d_name;
            struct stat st;
            if (stat(path, &st) != 0)
            {
                continue;
            }

            int fd;
            if (S_ISFIFO(st.st_mode) && !(st.st_mode & 0111) && // xbit == attached
                (fd = open(path, O_WRONLY | O_NONBLOCK)) != -1)
            {
                ::close(fd);
                screenList.append(TQFile::decodeName(entry->d_name));
                insertItem(SmallIconSet("konsole"),
                           i18n("Screen is a program controlling screens!",
                                "Screen at %1").arg(entry->d_name), id);
                id++;
            }
        }
        closedir(dir);
    }

    // reset id as we are now going to populate the profiles submenu
    id = 0;

    delete m_profileMenu;
    m_profileMenu = new TDEPopupMenu(this);
    TQStringList profiles = TDEGlobal::dirs()->findAllResources("data",
                                                           "konsole/profiles/*",
                                                           false, true );
    m_profiles.resize(profiles.count());
    TQStringList::ConstIterator pEnd = profiles.end();
    for (TQStringList::ConstIterator pIt = profiles.begin(); pIt != pEnd; ++pIt)
    {
        TQFileInfo info(*pIt);
        TQString profileName = TDEIO::decodeFileName(info.baseName());
        TQString niceName = profileName;
        KSimpleConfig cfg(*pIt, true);
        if (cfg.hasGroup("Profile"))
        {
            cfg.setGroup("Profile");
            if (cfg.hasKey("Name"))
            {
                niceName = cfg.readEntry("Name");
            }
        }

        m_profiles[id] = profileName;
        ++id;
        m_profileMenu->insertItem(niceName, id);
    }

    int profileID = insertItem(i18n("New Session Using Profile"),
                               m_profileMenu);
    if (id == 1)
    {
        // we don't have any profiles, disable the menu
        setItemEnabled(profileID, false);
    }
    connect(m_profileMenu, TQT_SIGNAL(activated(int)), TQT_SLOT(launchProfile(int)));

    insertSeparator();
    insertItem(SmallIconSet("reload"),
               i18n("Reload Sessions"), this, TQT_SLOT(reinitialize()));
}

void KonsoleMenu::slotExec(int id)
{
    if (id < 1)
    {
        return;
    }

    --id;
    kapp->propagateSessionManager();
    TQStringList args;
    if (static_cast<unsigned int>(id) < sessionList.count())
    {
        args << "--type";
        args << sessionList[id];
    }
    else
    {
        args << "-e";
        args << "screen";
        args << "-r";
        args << screenList[id - sessionList.count()];
    }
    TDEApplication::tdeinitExec("konsole", args);
    return;
}

void KonsoleMenu::launchProfile(int id)
{
    if (id < 1 || id > m_profiles.count())
    {
        return;
    }

    --id;
    // this is a session, not a bookmark, so execute that instead
   TQStringList args;
   args << "--profile" << m_profiles[id];
   kapp->tdeinitExec("konsole", args);
}

KURL KonsoleMenu::baseURL() const
{
    KURL url;
    /*url.setPath(se->getCwd()+"/");*/
    return url;
}

void KonsoleMenu::newSession(const TQString& sURL, const TQString& title)
{
    TQStringList args;

    KURL url = KURL(sURL);
    if ((url.protocol() == "file") && (url.hasPath()))
    {
        args << "-T" << title;
        args << "--workdir" << url.path();
        TDEApplication::tdeinitExec("konsole", args);
        return;
    }
    else if ((!url.protocol().isEmpty()) && (url.hasHost()))
    {
        TQString protocol = url.protocol();
        TQString host = url.host();
        args << "-T" << title;
        args << "-e" << protocol.latin1(); /* argv[0] == command to run. */
        if (url.hasUser()) {
            args << "-l" << url.user().latin1();
        }
        args << host.latin1();
        TDEApplication::tdeinitExec("konsole", args);
        return;
    }
    /*
     * We can't create a session without a protocol.
     * We should ideally popup a warning, about an invalid bookmark.
     */
}


#include "konsole_mnu.moc"
