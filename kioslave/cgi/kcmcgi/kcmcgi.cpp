/*
   Copyright (C) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kfiledialog.h>

#include <tqlayout.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqgroupbox.h>
#include <tqhbox.h>

#include "kcmcgi.h"
#include "kcmcgi.moc"

extern "C"
{
  KDE_EXPORT TDECModule *create_cgi( TQWidget *parent, const char * )
  {
    TDEGlobal::locale()->insertCatalogue("kcmcgi");
    return new KCMCgi( parent, "kcmcgi" );
  }
}


KCMCgi::KCMCgi(TQWidget *parent, const char *name)
  : TDECModule(parent, name)
{
  setButtons(Default|Apply);

  TQVBoxLayout *topLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

  TQGroupBox *topBox = new TQGroupBox( 1, Qt::Horizontal, i18n("Paths to Local CGI Programs"), this );
  topLayout->addWidget( topBox );

  mListBox = new TQListBox( topBox );

  TQHBox *buttonBox = new TQHBox( topBox );
  buttonBox->setSpacing( KDialog::spacingHint() );

  mAddButton = new TQPushButton( i18n("Add..."), buttonBox );
  connect( mAddButton, TQT_SIGNAL( clicked() ), TQT_SLOT( addPath() ) );

  mRemoveButton = new TQPushButton( i18n("Remove"), buttonBox );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removePath() ) );
  connect( mListBox, TQT_SIGNAL( clicked ( TQListBoxItem * )),this, TQT_SLOT( slotItemSelected( TQListBoxItem *)));

  mConfig = new TDEConfig("kcmcgirc");

  load();
  updateButton();
  TDEAboutData *about =
    new TDEAboutData( I18N_NOOP("kcmcgi"),
                    I18N_NOOP("CGI KIO Slave Control Module"),
                    0, 0, TDEAboutData::License_GPL,
                    I18N_NOOP("(c) 2002 Cornelius Schumacher") );

  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  setAboutData(about);
}

KCMCgi::~KCMCgi()
{
  delete mConfig;
}

void KCMCgi::slotItemSelected( TQListBoxItem * )
{
    updateButton();
}

void KCMCgi::updateButton()
{
    mRemoveButton->setEnabled( mListBox->selectedItem ());
}

void KCMCgi::defaults()
{
  mListBox->clear();
  updateButton();
}

void KCMCgi::save()
{
  TQStringList paths;

  uint i;
  for( i = 0; i < mListBox->count(); ++i ) {
    paths.append( mListBox->text( i ) );
  }

  mConfig->setGroup( "General" );
  mConfig->writeEntry( "Paths", paths );

  mConfig->sync();
}

void KCMCgi::load()
{
  mConfig->setGroup( "General" );
  TQStringList paths = mConfig->readListEntry( "Paths" );

  mListBox->insertStringList( paths );
}

void KCMCgi::addPath()
{
  TQString path = KFileDialog::getExistingDirectory( TQString::null, this );

  if ( !path.isEmpty() ) {
    mListBox->insertItem( path );
    emit changed( true );
  }
  updateButton();
}

void KCMCgi::removePath()
{
  int index = mListBox->currentItem();
  if ( index >= 0 ) {
    mListBox->removeItem( index );
    emit changed( true );
  }
  updateButton();
}

TQString KCMCgi::quickHelp() const
{
  return i18n("<h1>CGI Scripts</h1> The CGI KIO slave lets you execute "
              "local CGI programs without the need to run a web server. "
              "In this control module you can configure the paths that "
              "are searched for CGI scripts.");
}
