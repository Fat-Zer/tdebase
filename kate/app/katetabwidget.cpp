/* This file is part of the KDE project
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

#include "katetabwidget.h"
#include "katetabwidget.moc"

#include <tqtabbar.h>

//BEGIN KateTabWidget

KateTabWidget::KateTabWidget(TQWidget* parent, const char* name)
 : KTabWidget(parent,name)
 , m_visibility (ShowWhenMoreThanOneTab)
{
  tabBar()->hide();

  setHoverCloseButton(true);

  connect(this, TQT_SIGNAL(closeRequest(TQWidget*)), this, TQT_SLOT(closeTab(TQWidget*)));
}

KateTabWidget::~KateTabWidget()
{
}

void KateTabWidget::closeTab(TQWidget* w)
{
  w->close();
}

void KateTabWidget::addTab ( TQWidget * child, const TQString & label )
{
  KTabWidget::addTab(child,label);
  showPage(child);
  maybeShow();
}

void KateTabWidget::addTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label )
{
  KTabWidget::addTab(child,iconset,label);
  showPage(child);
  maybeShow();
}

void KateTabWidget::addTab ( TQWidget * child, TQTab * tab )
{
  KTabWidget::addTab(child,tab);
  showPage(child);
  maybeShow();
}

void KateTabWidget::insertTab ( TQWidget * child, const TQString & label, int index)
{
  KTabWidget::insertTab(child,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void KateTabWidget::insertTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index )
{
  KTabWidget::insertTab(child,iconset,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void KateTabWidget::insertTab ( TQWidget * child, TQTab * tab, int index)
{
  KTabWidget::insertTab(child,tab,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void KateTabWidget::removePage ( TQWidget * w )
{
  KTabWidget::removePage(w);
  maybeShow();
}

void KateTabWidget::maybeShow()
{
  switch (m_visibility)
  {
    case AlwaysShowTabs:
      tabBar()->show();

      // show/hide corner widgets
      if (count() == 0)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case ShowWhenMoreThanOneTab:
      if (count()<2) tabBar()->hide();
      else tabBar()->show();

      // show/hide corner widgets
      if (count() < 2)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case NeverShowTabs:
      tabBar()->hide();
      break;
  }
}

void KateTabWidget::setCornerWidgetVisibility(bool visible)
{
  // there are two corner widgets: on TopLeft and on TopTight!

  if (cornerWidget(TQt::TopLeft) ) {
    if (visible)
      cornerWidget(TQt::TopLeft)->show();
    else
      cornerWidget(TQt::TopLeft)->hide();
  }

  if (cornerWidget(TQt::TopRight) ) {
    if (visible)
      cornerWidget(TQt::TopRight)->show();
    else
      cornerWidget(TQt::TopRight)->hide();
  }
}

void KateTabWidget::setTabWidgetVisibility( TabWidgetVisibility visibility )
{
  m_visibility = visibility;
  maybeShow();
}

KateTabWidget::TabWidgetVisibility KateTabWidget::tabWidgetVisibility( ) const
{
  return m_visibility;
}

//END KateTabWidget
