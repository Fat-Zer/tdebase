this file is currently not used.
this message breaks compilation.
that is intentional :-]

/*
  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <tqheader.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqpoint.h>
#include <tqcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kservicegroup.h>
#include <kdebug.h>

#include "modulemenu.h"
#include "modulemenu.moc"
#include "modules.h"
#include "global.h"


ModuleMenu::ModuleMenu(ConfigModuleList *list, TQWidget * parent, const char * name)
  : KPopupMenu(parent, name)
  , _modules(list)
{
  // use large id's to start with...
  id = 10000;

  fill(this, KCGlobal::baseGroup());

  connect(this, TQT_SIGNAL(activated(int)), this, TQT_SLOT(moduleSelected(int)));
}

void ModuleMenu::fill(KPopupMenu *parentMenu, const TQString &parentPath)
{
  TQStringList subMenus = _modules->submenus(parentPath);
  for(TQStringList::ConstIterator it = subMenus.begin();
      it != subMenus.end(); ++it)
  {
     TQString path = *it;
     KServiceGroup::Ptr group = KServiceGroup::group(path);
     if (!group)
        continue;
     
     // create new menu
     KPopupMenu *menu = new KPopupMenu(parentMenu);
     connect(menu, TQT_SIGNAL(activated(int)), this, TQT_SLOT(moduleSelected(int)));

     // Item names may contain ampersands. To avoid them being converted to 
     // accelators, replace them with two ampersands.
     TQString name = group->caption();
     name.replace("&", "&&");
  
     parentMenu->insertItem(KGlobal::iconLoader()->loadIcon(group->icon(), KIcon::Desktop, KIcon::SizeSmall)
                        , name, menu);

     fill(menu, path);
  }

  ConfigModule *module;
  TQPtrList<ConfigModule> moduleList = _modules->modules(parentPath);
  for (module=moduleList.first(); module != 0; module=moduleList.next())
  {
     // Item names may contain ampersands. To avoid them being converted to 
     // accelators, replace them with two ampersands.
     TQString name = module->moduleName();
     name.replace("&", "&&");

     int realid = parentMenu->insertItem(KGlobal::iconLoader()->loadIcon(module->icon(), KIcon::Desktop, KIcon::SizeSmall)
                                     , name, id);
     _moduleDict.insert(realid, module);

      id++;
  }
  
}

void ModuleMenu::moduleSelected(int id)
{
  kdDebug(1208) << "Item " << id << " selected" << endl;
  ConfigModule *module = _moduleDict[id];
  if (module)
    emit moduleActivated(module);
}
