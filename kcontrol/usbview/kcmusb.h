/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _KCMUSB_H
#define _KCMUSB_H

#include <tqintdict.h>

#include <tdecmodule.h>

class TQListView;
class TQListViewItem;
class TQTextView;


class USBViewer : public TDECModule
{
  Q_OBJECT

public:

  USBViewer(TQWidget *parent = 0L, const char *name = 0L, const TQStringList &list=TQStringList() );

  void load();

protected slots:

  void selectionChanged(TQListViewItem *item);
  void refresh();

private:

  TQIntDict<TQListViewItem> _items;
  TQListView *_devices;
  TQTextView *_details;
};


#endif
