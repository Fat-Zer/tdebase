/* This file is part of the KDE project
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "toolviewmanager.h"
#include "toolviewmanager.moc"

#include "plugin.h"
#include "documentmanager.h"
#include "pluginmanager.h"

#include "../app/katemainwindow.h"

namespace Kate
{

class PrivateToolViewManager
  {
  public:
    PrivateToolViewManager ()
    {
    }

    ~PrivateToolViewManager ()
    {
    }

    KateMainWindow *toolViewMan;
  };

ToolViewManager::ToolViewManager (void *toolViewManager) : TQObject ((KateMainWindow*) toolViewManager)
{
  d = new PrivateToolViewManager ();
  d->toolViewMan = (KateMainWindow*) toolViewManager;
}

ToolViewManager::~ToolViewManager ()
{
  delete d;
}

TQWidget *ToolViewManager::createToolView (const TQString &identifier, ToolViewManager::Position pos, const TQPixmap &icon, const TQString &text)
{
  return d->toolViewMan->createToolView (identifier, (KMultiTabBar::KMultiTabBarPosition)pos, icon, text);
}

bool ToolViewManager::moveToolView (TQWidget *widget, ToolViewManager::Position  pos)
{
  if (!widget || !widget->tqqt_cast("KateMDI::ToolView"))
    return false;

  return d->toolViewMan->moveToolView (static_cast<KateMDI::ToolView*>(widget), (KMultiTabBar::KMultiTabBarPosition)pos);
}

bool ToolViewManager::showToolView(TQWidget *widget)
{
  if (!widget || !widget->tqqt_cast("KateMDI::ToolView"))
    return false;

  return d->toolViewMan->showToolView (static_cast<KateMDI::ToolView*>(widget));
}

bool ToolViewManager::hideToolView(TQWidget *widget)
{
  if (!widget || !widget->tqqt_cast("KateMDI::ToolView"))
    return false;

  return d->toolViewMan->hideToolView (static_cast<KateMDI::ToolView*>(widget));
}

}
