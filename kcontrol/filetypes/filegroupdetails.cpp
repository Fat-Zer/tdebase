/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "filegroupdetails.h"
#include "typeslistitem.h"
#include <tqlayout.h>
#include <tqvbuttongroup.h>
#include <tqradiobutton.h>
#include <tqwhatsthis.h>
#include <kdialog.h>
#include <tdelocale.h>

FileGroupDetails::FileGroupDetails(TQWidget *parent, const char *name )
    : TQWidget( parent, name )
{
  TQWidget * parentWidget = this;
  TQVBoxLayout *secondLayout = new TQVBoxLayout(parentWidget,
      0, KDialog::spacingHint());

  m_autoEmbed = new TQVButtonGroup( i18n("Left Click Action"), parentWidget );
  m_autoEmbed->layout()->setSpacing( KDialog::spacingHint() );
  secondLayout->addWidget( m_autoEmbed );
  // The order of those two items is very important. If you change it, fix typeslistitem.cpp !
  new TQRadioButton( i18n("Show file in embedded viewer"), m_autoEmbed );
  new TQRadioButton( i18n("Show file in separate viewer"), m_autoEmbed );
  connect(m_autoEmbed, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( slotAutoEmbedClicked( int ) ));

  TQWhatsThis::add( m_autoEmbed, i18n("Here you can configure what the Konqueror file manager"
    " will do when you click on a file belonging to this group. Konqueror can display the file in"
    " an embedded viewer or start up a separate application. You can change this setting for a"
    " specific file type in the 'Embedding' tab of the file type configuration.") );

  secondLayout->addStretch();
}

void FileGroupDetails::setTypeItem( TypesListItem * item )
{
  Q_ASSERT( item->isMeta() );
  m_item = item;
  m_autoEmbed->setButton( item ? item->autoEmbed() : -1 );
}

void FileGroupDetails::slotAutoEmbedClicked(int button)
{
  if ( !m_item )
    return;
  m_item->setAutoEmbed( button );
  emit changed(true);
}

#include "filegroupdetails.moc"
