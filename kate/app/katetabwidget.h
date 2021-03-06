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

#ifndef __KATE_TABWIDGET_H__
#define __KATE_TABWIDGET_H__

#include <ktabwidget.h>

class KateTabWidget : public KTabWidget
{
  Q_OBJECT

  public:
    enum TabWidgetVisibility {
    AlwaysShowTabs         = 0,
    ShowWhenMoreThanOneTab = 1,
    NeverShowTabs          = 2
    };

  public:
    KateTabWidget(TQWidget* parent, const char* name=0);
    virtual ~KateTabWidget();

    virtual void addTab ( TQWidget * child, const TQString & label );

    virtual void addTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label );

    virtual void addTab ( TQWidget * child, TQTab * tab );

    virtual void insertTab ( TQWidget * child, const TQString & label, int index = -1 );

    virtual void insertTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index = -1 );

    virtual void insertTab ( TQWidget * child, TQTab * tab, int index = -1 );

    virtual void removePage ( TQWidget * w );

    TabWidgetVisibility tabWidgetVisibility() const;

    void setTabWidgetVisibility( TabWidgetVisibility );

  private slots:
    void closeTab(TQWidget* w);

  private:
    void maybeShow();
    void setCornerWidgetVisibility(bool visible);

  private:
    TabWidgetVisibility m_visibility;
};

#endif
