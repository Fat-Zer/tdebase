/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>
Copyright (c) 2004-2005 Aaron Seigo <aseigo@kde.org>

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

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include <tdeaboutdata.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeio/netaccess.h>
#include <tdelocale.h>
#include <tdemenubar.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <twinmodule.h>
#include <dcopref.h>

#include "container_extension.h"
#include "global.h"
#include "kicker.h"
#include "panelextension.h"
#include "pluginmanager.h"

#include "extensionmanager.h"

ExtensionManager* ExtensionManager::m_self = 0;

ExtensionManager* ExtensionManager::the()
{
    if (!m_self)
    {
        m_self = new ExtensionManager;
    }

    return m_self;
}

ExtensionManager::ExtensionManager()
    : TQObject(0, "ExtensionManager"),
      m_menubarPanel(0),
      m_mainPanel(0),
      m_panelCounter(-1),
      m_loadingContainers(false)
{
}

ExtensionManager::~ExtensionManager()
{
    if (this == m_self)
    {
        m_self = 0;
    }

    ExtensionList::iterator itEnd = _containers.end();
    for (ExtensionList::iterator it = _containers.begin(); it != itEnd; ++it)
    {
        delete *it;
    }
    _containers.clear();

    delete m_menubarPanel;
    delete m_mainPanel;
}

void ExtensionManager::initialize()
{
    m_loadingContainers = true;

//    kdDebug(1210) << "ExtensionManager::loadContainerConfig()" << endl;
    TDEConfig* config = TDEGlobal::config();
    PluginManager* pm = PluginManager::the();

    // set up the "main" panel
    if (config->hasGroup("Main Panel"))
    {
        config->setGroup("Main Panel");
        if (config->hasKey("DesktopFile"))
        {
            m_mainPanel = pm->createExtensionContainer(config->readPathEntry("DesktopFile"),
                                                       true, config->readPathEntry("ConfigFile"),
                                                       "Main Panel");
        }
    }

    if (!m_mainPanel)
    {
        // fall back to a regular ol' PanelExtension
        m_mainPanel = pm->createExtensionContainer(
                            "childpanelextension.desktop",
                            true,
                            TQString(kapp->aboutData()->appName()) + "rc",
                            "Main Panel");
    }

    if (!m_mainPanel)
    {
        KMessageBox::error(0, i18n("The TDE panel (kicker) could not load the main panel "
                                   "due to a problem with your installation. "),
                           i18n("Fatal Error!"));
        exit(1);
    }

    configureMenubar(true);

    Kicker::the()->setMainWidget(m_mainPanel);

    m_mainPanel->readConfig();
    m_mainPanel->show();
    kapp->processEvents();

    // read extension list
    config->setGroup("General");
    TQStringList elist = config->readListEntry("Extensions2");

    // now restore the extensions
    TQStringList::iterator itEnd = elist.end();
    for (TQStringList::iterator it = elist.begin(); it !=  elist.end(); ++it)
    {
        // last container?
        TQStringList::iterator lastcheck(it);
        lastcheck++;
        if (lastcheck == elist.end()) {
            m_loadingContainers = false;
        }

        // extension id
        TQString extensionId(*it);

        // create a matching applet container
        if (extensionId.find("Extension") == -1)
        {
            continue;
        }

        // is there a config group for this extension?
        if (!config->hasGroup(extensionId))
        {
            continue;
        }

        // set config group
        config->setGroup(extensionId);

        ExtensionContainer* e = pm->createExtensionContainer(config->readPathEntry("DesktopFile"),
                                                             true, // is startup
                                                             config->readPathEntry("ConfigFile"),
                                                             extensionId);
        if (e)
        {
            addContainer(e);
            e->readConfig();
            e->show();
            kapp->processEvents();
        }
    }
    m_loadingContainers = false;

    pm->clearUntrustedLists();
    connect(Kicker::the(), TQT_SIGNAL(configurationChanged()), TQT_SLOT(configurationChanged()));
    DCOPRef r( "ksmserver", "ksmserver" );
    r.send( "resumeStartup", TQCString( "kicker" ));
}

void ExtensionManager::configureMenubar(bool duringInit)
{
    TDEConfig menuConfig( "kdesktoprc", true );
    if( TDEConfigGroup( &menuConfig, "KDE" ).readBoolEntry("macStyle", false)
        || TDEConfigGroup( &menuConfig, "Menubar" ).readBoolEntry( "ShowMenubar", false ))
    {
        if (TDEGlobal::dirs()->findResource("applets", "menuapplet.desktop").isEmpty() ||
            m_menubarPanel)
        {
            return;
        }

        if (duringInit)
        {
            AppletInfo menubarInfo("menuapplet.desktop", TQString::null, AppletInfo::Applet);
            if (PluginManager::the()->hasInstance(menubarInfo))
            {
                // it's already there, in the main panel!
                return;
            }
            migrateMenubar();
        }

        AppletInfo info("childpanelextension.desktop",
                        "kicker_menubarpanelrc",
                        AppletInfo::Extension);
        KPanelExtension* menubar = new MenubarExtension(info);
        m_menubarPanel = new ExtensionContainer(menubar, info, "Menubar Panel");
        m_menubarPanel->setPanelOrder(-1);
        m_menubarPanel->readConfig();
        m_menubarPanel->setPosition(KPanelExtension::Top);
        m_menubarPanel->setXineramaScreen(XineramaAllScreens);
        m_menubarPanel->setHideButtons(false, false);

        // this takes care of resizing the panel so it shows with the right height
        updateMenubar();

        m_menubarPanel->show();
        connect(kapp, TQT_SIGNAL(tdedisplayFontChanged()), TQT_SLOT(updateMenubar()));
    }
    else if (m_menubarPanel)
    {
        int screen = m_menubarPanel->xineramaScreen();
        delete m_menubarPanel;
        m_menubarPanel = 0;

        if (!m_loadingContainers) {
            emit desktopIconsAreaChanged(desktopIconsArea(screen), screen);
        }
    }
}

void ExtensionManager::migrateMenubar()
{
    // lame, lame, lame.
    // the menubar applet was just plunked into kicker and not much
    // thought was put into how it should be used. great idea, but no
    // integration. >:-(
    // so now we have to check to see if we HAVE another extension that
    // will have a menubar in it, and if so, abort creating one of our
    // own.
    //
    // the reason i didn't do this as a tdeconfig_update script is that
    // most people don't use this feature, so no reason to penalize
    // everyone, and moreover the user may added this to their main
    // panel, meaning kickerrc itself would have to be vastly modified
    // with lots of complications. not work it IMHO.

    TDEConfig* config = TDEGlobal::config();
    config->setGroup("General");

    if (config->readBoolEntry("CheckedForMenubar", false))
    {
        return;
    }

    if (!locate("config", "kicker_menubarpanelrc").isEmpty())
    {
        // don't overwrite/override something that's already there
        return;
    }

    TQStringList elist = config->readListEntry("Extensions2");
    TQStringList::iterator itEnd = elist.end();
    for (TQStringList::iterator it = elist.begin(); it !=  elist.end(); ++it)
    {
        TQString extensionId(*it);

        if (extensionId.find("Extension") == -1)
        {
            continue;
        }

        // is there a config group for this extension?
        if (!config->hasGroup(extensionId))
        {
            continue;
        }

        config->setGroup(extensionId);
        TQString extension = config->readPathEntry("ConfigFile");
        TDEConfig extensionConfig(locate("config", extension));
        extensionConfig.setGroup("General");

        if (extensionConfig.hasKey("Applets2"))
        {
            TQStringList containers = extensionConfig.readListEntry("Applets2");
            TQStringList::iterator cit = containers.begin();
            TQStringList::iterator citEnd = containers.end();
            for (; cit != citEnd; ++cit)
            {
                TQString appletId(*cit);

                // is there a config group for this applet?
                if (!extensionConfig.hasGroup(appletId))
                {
                    continue;
                }

                TDEConfigGroup group(&extensionConfig, appletId.latin1());
                TQString appletType = appletId.left(appletId.findRev('_'));

                if (appletType == "Applet")
                {
                    TQString appletFile = group.readPathEntry("DesktopFile");
                    if (appletFile.find("menuapplet.desktop") != -1)
                    {
                        TQString menubarConfig = locate("config", extension);
                        TDEIO::NetAccess::copy(menubarConfig,
                                             locateLocal("config",
                                             "kicker_menubarpanelrc"), 0);
                        elist.remove(it);
                        config->setGroup("General");
                        config->writeEntry("Extensions2", elist);
                        config->writeEntry("CheckedForMenubar", true);
                        config->sync();
                        return;
                    }
                }
            }
        }
    }

    config->setGroup("General");
    config->writeEntry("CheckedForMenubar", true);
}

void ExtensionManager::saveContainerConfig()
{
//    kdDebug(1210) << "ExtensionManager::saveContainerConfig()" << endl;

    TDEConfig *config = TDEGlobal::config();

    // build the extension list
    TQStringList elist;
    ExtensionList::iterator itEnd = _containers.end();
    for (ExtensionList::iterator it = _containers.begin(); it != itEnd; ++it)
    {
        elist.append((*it)->extensionId());
    }

    // write extension list
    config->setGroup("General");
    config->writeEntry("Extensions2", elist);

    config->sync();
}

void ExtensionManager::configurationChanged()
{
    if (m_mainPanel)
    {
        m_mainPanel->readConfig();
    }

    if (m_menubarPanel)
    {
        m_menubarPanel->readConfig();
    }

    ExtensionList::iterator itEnd = _containers.end();
    for (ExtensionList::iterator it = _containers.begin(); it != itEnd; ++it)
    {
        (*it)->readConfig();
    }
}

void ExtensionManager::updateMenubar()
{
    if (!m_menubarPanel)
    {
        return;
    }

    //kdDebug(0) << "ExtensionManager::updateMenubar()" << endl;
    // we need to make sure the panel is tall enough to accomodate the
    // menubar! an easy way to find out the height of a menu: make one ;)
    KMenuBar tmpmenu;
    tmpmenu.insertItem("TDE Rocks!");
    m_menubarPanel->setSize(KPanelExtension::SizeCustom,
                            tmpmenu.sizeHint().height());
    m_menubarPanel->writeConfig();

    if (!m_loadingContainers) {
        emit desktopIconsAreaChanged(desktopIconsArea(m_menubarPanel->xineramaScreen()),
                                     m_menubarPanel->xineramaScreen());
    }
}

bool ExtensionManager::isMainPanel(const TQWidget* panel) const
{
    return m_mainPanel == panel;
}

bool ExtensionManager::isMenuBar(const TQWidget* panel) const
{
    return m_menubarPanel == panel;
}

void ExtensionManager::addExtension( const TQString& desktopFile )
{
    PluginManager* pm = PluginManager::the();
    ExtensionContainer *e = pm->createExtensionContainer(desktopFile,
                                                         false, // is not startup
                                                         TQString::null, // no config
                                                         uniqueId());
    

    kdDebug(1210) << "ExtensionManager::addExtension" << endl;

    if (e)
    {
        e->readConfig();
        // as a new panel, the position will be set to the preferred position
        // we just need to make sure this works with the rest of the panel layout
        e->setPosition(initialPanelPosition(e->position()));
        kdDebug(1210)<<"after e->readConfig(): pos="<<e->position()<<endl;
        addContainer(e);
        e->show();
        e->writeConfig();
        saveContainerConfig();
    }
}

void ExtensionManager::addContainer(ExtensionContainer* e)
{
    if (!e)
    {
        return;
    }

    _containers.append(e);

    connect(e, TQT_SIGNAL(removeme(ExtensionContainer*)),
            this, TQT_SLOT(removeContainer(ExtensionContainer*)));

    if (!m_loadingContainers) {
        emit desktopIconsAreaChanged(desktopIconsArea(e->xineramaScreen()),
                                     e->xineramaScreen());
    }
}

void ExtensionManager::removeContainer(ExtensionContainer* e)
{
    if (!e)
    {
        return;
    }

    e->removeSessionConfigFile();
    _containers.remove(e);
    e->deleteLater(); // Wait till we return to the main event loop
    saveContainerConfig();

    if (!m_loadingContainers) {
        emit desktopIconsAreaChanged(desktopIconsArea(e->xineramaScreen()),
                                     e->xineramaScreen());
    }
}

void ExtensionManager::removeAllContainers()
{
    while (!_containers.isEmpty())
    {
        ExtensionContainer* e = _containers.first();
        _containers.remove(e);
        e->deleteLater(); // Wait till we return to the main event loop
    }

    saveContainerConfig();
}

TQString ExtensionManager::uniqueId()
{
    TQString idBase = "Extension_%1";
    TQString newId;
    int i = 0;
    bool unique = false;

    while (!unique)
    {
        i++;
        newId = idBase.arg(i);

        unique = true;
        ExtensionList::iterator itEnd = _containers.end();
        for (ExtensionList::iterator it = _containers.begin(); it != itEnd; ++it)
        {
            if ((*it)->extensionId() == newId)
            {
                unique = false;
                break;
            }
        }
    }

    return newId;
}

KPanelExtension::Position ExtensionManager::initialPanelPosition(KPanelExtension::Position preferred)
{
    // Guess a good position
    bool positions[KPanelExtension::Bottom+1];
    for( int i = 0; i <= (int) KPanelExtension::Bottom; ++i )
    {
       positions[i] = true;
    }

    ExtensionList::iterator itEnd = _containers.end();
    for (ExtensionList::iterator it = _containers.begin(); it != itEnd; ++it)
    {
       positions[(int) (*it)->position()] = false;
    }

    KPanelExtension::Position pos = preferred;
    if (positions[(int)pos])
       return pos;

    pos = (KPanelExtension::Position) (pos ^ 1);
    if (positions[(int)pos])
       return pos;

    pos = (KPanelExtension::Position) (pos ^ 3);
    if (positions[(int)pos])
       return pos;

    pos = (KPanelExtension::Position) (pos ^ 1);
    if (positions[(int)pos])
       return pos;

    return preferred;
}

bool ExtensionManager::shouldExclude(int XineramaScreen,
                                     const ExtensionContainer* extension,
                                     const ExtensionContainer* exclude) const
{
    // Rules of Exclusion:
    // 0. Exclude ourselves
    // 1. Exclude panels not on our Xinerama screen
    // 2. Exclude panels on the same side of the screen as ourselves that are above us
    // 3. Exclude panels on the opposite side of the screen. Breaks down if the user
    //    dabbles in insane layouts where a top/bottom or left/right pair overlap?
    // 4. Exclude panels on adjacent sides of the screen that do not overlap with us

    if (exclude->winId() == extension->winId())
    {
        // Rule 0 Exclusion
        return true;
    }

    if (extension->xineramaScreen()!= XineramaAllScreens &&
        exclude->xineramaScreen() != XineramaAllScreens &&
        exclude->xineramaScreen() != XineramaScreen)
    {
        // Rule 1 exclusion
        return true;
    }

    if (!exclude->reserveStrut())
    {
        return true;
    }

    bool lowerInStack = extension->panelOrder() < exclude->panelOrder();
    if (exclude->position() == extension->position())
    {
        // Rule 2 Exclusion
        if (extension->position() == KPanelExtension::Bottom &&
            exclude->geometry().bottom() == extension->geometry().bottom() &&
            !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == KPanelExtension::Top &&
                 exclude->geometry().top() == extension->geometry().top() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == KPanelExtension::Left &&
                 exclude->geometry().left() == extension->geometry().left() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == KPanelExtension::Right &&
                 exclude->geometry().right() == extension->geometry().right() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }

        return lowerInStack;
    }

    // Rule 3 exclusion
    if (exclude->orientation() == extension->orientation())
    {
        // on the opposite side of the screen from us.
        return true;
    }

    // Rule 4 exclusion
    if (extension->position() == KPanelExtension::Bottom)
    {
        if (exclude->geometry().bottom() > extension->geometry().top())
        {
            return lowerInStack;
        }
    }
    else if (extension->position() == KPanelExtension::Top)
    {
        if (exclude->geometry().top() < extension->geometry().bottom())
        {
            return lowerInStack;
        }
    }
    else if (extension->position() == KPanelExtension::Left)
    {
        if (exclude->geometry().left() < extension->geometry().right())
        {
            return lowerInStack;
        }
    }
    else /* if (extension->position() == KPanelExtension::Right) */
    {
        if (exclude->geometry().right() > extension->geometry().left())
        {
            return lowerInStack;
        }
    }

    return true;
}

TQRect ExtensionManager::workArea(int XineramaScreen, const ExtensionContainer* extension)
{
    if (!extension)
    {
        return Kicker::the()->twinModule()->workArea(XineramaScreen);
    }

    TQValueList<WId> list;

    ExtensionList::iterator itEnd = _containers.end();
    ExtensionList::iterator it = _containers.begin();

    // If the hide mode is Manual, exclude the struts of
    // panels below this one in the list. Else exclude the
    // struts of all panels.
    if (extension->reserveStrut() &&
        extension != m_menubarPanel &&
        extension->hideMode() == ExtensionContainer::ManualHide)
    {
        if (m_mainPanel && shouldExclude(XineramaScreen, extension, m_mainPanel))
        {
            list.append(m_mainPanel->winId());
        }

        for (; it != itEnd; ++it)
        {
            if (shouldExclude(XineramaScreen, extension, *it))
            {
                list.append((*it)->winId());
            }
        }
    }
    else
    {
        // auto hide panel? just ignore everything else for now.
        if (extension == m_menubarPanel)
        {
            list.append(m_menubarPanel->winId());
        }

        if (m_mainPanel)
        {
            list.append(m_mainPanel->winId());
        }

        for (; it != itEnd; ++it)
        {
            list.append((*it)->winId());
        }
    }

    TQRect workArea;
    if ((XineramaScreen == XineramaAllScreens) || (kapp->desktop()->numScreens() < 2))
    {
         /* special value for all screens */
         workArea = Kicker::the()->twinModule()->workArea(list);
    }
    else
    {
        workArea = Kicker::the()->twinModule()->workArea(list, XineramaScreen)
                   .intersect(TQApplication::desktop()->screenGeometry(XineramaScreen));
    }

    return workArea;
}

int ExtensionManager::nextPanelOrder()
{
    ++m_panelCounter;
    return m_panelCounter;
}

void ExtensionManager::reduceArea(TQRect &area, const ExtensionContainer *extension) const
{
    if (!extension ||
        extension->hideMode() == ExtensionContainer::AutomaticHide ||
        !extension->reserveStrut())
    {
        return;
    }

    TQRect geom = extension->initialGeometry(extension->position(), extension->alignment(),
                                            extension->xineramaScreen());

    // reduce given area (TQRect) to the space not covered by the given extension
    // As simplification: the length of the extension is not taken into account
    // which means that even a small extension e.g. on the left side of the desktop
    // will remove the available area with its with

    switch (extension->position())
    {
        case KPanelExtension::Left:
        {
            area.setLeft(TQMAX(area.left(), geom.right()));
            break;
        }
        case KPanelExtension::Right:
        {
            area.setRight(TQMIN(area.right(), geom.left()));
            break;
        }
        case KPanelExtension::Top:
        {
            area.setTop(TQMAX(area.top(), geom.bottom()));
            break;
        }
        case KPanelExtension::Bottom:
        {
            area.setBottom(TQMIN(area.bottom(), geom.top()));
            break;
        }
        default: ;  // ignore KPanelExtension::Floating ... at least for now
    }
}

TQRect ExtensionManager::desktopIconsArea(int screen) const
{
    // This is pretty broken, mixes Xinerama and non-Xinerama multihead
    // and generally doesn't seem to be required anyway => ignore screen.
//    TQRect area = TQApplication::desktop()->screenGeometry(screen);
    TQRect area = TQApplication::desktop()->geometry();

    reduceArea(area, m_mainPanel);
    reduceArea(area, m_menubarPanel);

    for (ExtensionList::const_iterator it = _containers.constBegin();
         it != _containers.constEnd();
         ++it)
    {
        reduceArea(area, (*it));
    }

    kdDebug(1210) << "ExtensionManager::desktopIconsArea() = " << area
                  << " screen = " << screen << endl;
    return area;
}

void ExtensionManager::extensionSizeChanged(ExtensionContainer *extension)
{
  // we have to recalc the available space for desktop icons
  if (!extension)
  {
      return;
  }

  if (!m_loadingContainers) {
      emit desktopIconsAreaChanged(desktopIconsArea(extension->xineramaScreen()),
                                   extension->xineramaScreen());
  }
}

#include "extensionmanager.moc"
