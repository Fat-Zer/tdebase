/*
    Naughty applet - Runaway process monitor for the TDE panel

    Copyright 2000 Rik Hemsley (rikkus) <rik@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <keditlistbox.h>
#include <knuminput.h>
#include <tdelocale.h>
#include <tqvbox.h>

#include "NaughtyConfigDialog.h"
#include "NaughtyConfigDialog.moc"

NaughtyConfigDialog::NaughtyConfigDialog
(
 const TQStringList & items,
 uint updateInterval,
 uint threshold,
 TQWidget * parent,
 const char * name
)
  :
  KDialogBase
  (
   parent,
   name,
   true,
   i18n("Configuration"),
   KDialogBase::Ok | KDialogBase::Cancel,
   KDialogBase::Ok,
   true
  )
{
  TQVBox * v = new TQVBox(this);
  setMainWidget(v);

  kini_updateInterval_  = new KIntNumInput(updateInterval, v);
  kini_threshold_       = new KIntNumInput(kini_updateInterval_, threshold, v);

  kini_updateInterval_  ->setLabel(i18n("&Update interval:"));
  kini_threshold_       ->setLabel(i18n("CPU &load threshold:"));

  kini_updateInterval_  ->setRange(1, 20);
  kini_threshold_       ->setRange(10, 1000);

  listBox_ = new KEditListBox
    (i18n("&Programs to Ignore"),
     v,
     "naughty config dialog ignore listbox",
     false,
     KEditListBox::Add | KEditListBox::Remove
    );

  listBox_->insertStringList(items);
}

NaughtyConfigDialog::~NaughtyConfigDialog()
{
}

  uint
NaughtyConfigDialog::updateInterval() const
{
  return uint(kini_updateInterval_->value());
}

  uint
NaughtyConfigDialog::threshold() const
{
  return uint(kini_threshold_->value());
}

  TQStringList
NaughtyConfigDialog::ignoreList() const
{
  TQStringList retval;

  for (int i = 0; i < listBox_->count(); i++)
    retval << listBox_->text(i);

  return retval;
}

