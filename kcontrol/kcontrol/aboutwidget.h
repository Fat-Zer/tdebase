/*
  Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>
 
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

#ifndef __aboutwidget_h__
#define __aboutwidget_h__

#include <tqwidget.h>
#include <tqlistview.h>
#include <tqhbox.h>

class TDECModuleInfo;
class TQPixmap;
class KPixmap;
class ConfigModule;
class TDEHTMLPart;
class KURL;

class AboutWidget : public TQHBox
{  
  Q_OBJECT    
  
public:   
  AboutWidget(TQWidget *parent, const char *name=0, TQListViewItem* category=0, const TQString &caption=TQString::null);

    /**
     * Set a new category without creating a new AboutWidget if there is
     * one visible already (reduces flicker)
     */
    void setCategory( TQListViewItem* category, const TQString& icon, const TQString& caption);

signals:
    void moduleSelected(ConfigModule *);

private slots:
    void slotModuleLinkClicked( const KURL& );

private:
    /**
     * Update the pixmap to be shown. Called from resizeEvent and from
     * setCategory.
     */
    void updatePixmap();

    bool    _moduleList;
    TQListViewItem* _category;
    TQString _icon;
    TQString _caption;
    TDEHTMLPart *_viewer;
    TQMap<TQString,ConfigModule*> _moduleMap;
};

#endif
