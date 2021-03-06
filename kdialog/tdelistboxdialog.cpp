//
//  Copyright (C) 1998 Matthias Hoelzer
//  email:  hoelzer@physik.uni-wuerzburg.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the7 implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include <tqlabel.h>
#include <tqlistbox.h>
#include <tqvbox.h>

#include "tdelistboxdialog.h"
#include "tdelistboxdialog.moc"

#include "tdelocale.h"

TDEListBoxDialog::TDEListBoxDialog(TQString text, TQWidget *parent)
    : KDialogBase( parent, 0, true, TQString::null, Ok|Cancel, Ok, true )
{
  TQVBox *page = makeVBoxMainWidget();

  label = new TQLabel(text, page);
  label->setAlignment(AlignCenter);

  table = new TQListBox(page);
  table->setFocus();
}

void TDEListBoxDialog::insertItem(const TQString& item)
{
  table->insertItem(item);
  table->setCurrentItem(0);
}

void TDEListBoxDialog::setCurrentItem(const TQString& item)
{
  for ( int i=0; i < (int) table->count(); i++ ) {
    if ( table->text(i) == item ) {
      table->setCurrentItem(i);
      break;
    }
  }
}

int TDEListBoxDialog::currentItem()
{
  return table->currentItem();
}
