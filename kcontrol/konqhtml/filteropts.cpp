/*
   Copyright (C) 2005 Ivor Hewitt <ivor@ivor.org>

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
#include <dcopclient.h>

#include <tqlayout.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqregexp.h>

#include "filteropts.h"
#include "filteropts.moc"

KCMFilter::KCMFilter(KConfig *config, TQString group,
                     TQWidget *parent, const char * )
    : KCModule( parent, "kcmkonqhtml" ),
      mConfig( config ),
      mGroupname( group ),
      mSelCount(0)
{
    setButtons(Default|Apply);

    TQVBoxLayout *topLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

    mEnableCheck = new TQCheckBox(i18n("Enable filters"), this);
    topLayout->addWidget( mEnableCheck );

    mKillCheck = new TQCheckBox(i18n("Hide filtered images"), this);
    topLayout->addWidget( mKillCheck );

    TQGroupBox *topBox = new TQGroupBox( 1, Horizontal, i18n("URL Expressions to Filter"), this );
    topLayout->addWidget( topBox );

    mListBox = new TQListBox( topBox );
    mListBox->setSelectionMode(TQListBox::Extended);
    new TQLabel( i18n("Expression (e.g. http://www.site.com/ad/*):"), topBox);
    mString = new TQLineEdit( topBox );

    TQHBox *buttonBox = new TQHBox( topBox );
    buttonBox->setSpacing( KDialog::spacingHint() );

    mInsertButton = new TQPushButton( i18n("Insert"), buttonBox );
    connect( mInsertButton, TQT_SIGNAL( clicked() ), TQT_SLOT( insertFilter() ) );
    mUpdateButton = new TQPushButton( i18n("Update"), buttonBox );
    connect( mUpdateButton, TQT_SIGNAL( clicked() ), TQT_SLOT( updateFilter() ) );
    mRemoveButton = new TQPushButton( i18n("Remove"), buttonBox );
    connect( mRemoveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removeFilter() ) );

    mImportButton = new TQPushButton(i18n("Import..."),buttonBox);
    connect( mImportButton, TQT_SIGNAL( clicked() ), TQT_SLOT( importFilters() ) );
    mExportButton = new TQPushButton(i18n("Export..."),buttonBox);
    connect( mExportButton, TQT_SIGNAL( clicked() ), TQT_SLOT( exportFilters() ) );

    connect( mEnableCheck, TQT_SIGNAL( clicked()), this, TQT_SLOT( slotEnableChecked()));
    connect( mKillCheck, TQT_SIGNAL( clicked()), this, TQT_SLOT( slotKillChecked()));
    connect( mListBox, TQT_SIGNAL( selectionChanged ()),this, TQT_SLOT( slotItemSelected()));

/*
 * Whats this items
 */
    TQWhatsThis::add( mEnableCheck, i18n("Enable or disable AdBlocK filters. When enabled a set of expressions "
                                        "to be blocked should be defined in the filter list for blocking to "
                                        "take effect."));
    TQWhatsThis::add( mKillCheck, i18n("When enabled blocked images will be removed from the page completely "
                                      "otherwise a placeholder 'blocked' image will be used."));
    TQWhatsThis::add( mListBox, i18n("This is the list of URL filters that will be applied to all linked "
                                    "images and frames. The filters are processed in order so place "
                                    "more generic filters towards the top of the list."));
    TQWhatsThis::add( mString, i18n("Enter an expression to filter. Expressions can be defined as either "
                                   "a filename style wildcard e.g. http://www.site.com/ads* or as a full "
                                   "regular expression by surrounding the string with '/' e.g. "
                                   " //(ad|banner)\\./"));
    load();
    updateButton();
}

KCMFilter::~KCMFilter()
{
    delete mConfig;
}

void KCMFilter::slotKillChecked()
{
    emit changed( true );
}

void KCMFilter::slotEnableChecked()
{
    updateButton();
    emit changed( true );
}

void KCMFilter::slotItemSelected()
{
    int currentId=-1;
    unsigned int i;
    for( i=0,mSelCount=0; i < mListBox->count() && mSelCount<2; ++i )
    {
        if (mListBox->isSelected(i))
        {
            currentId=i;
            mSelCount++;
        }
    }

    if ( currentId >= 0 )
    {
        mString->setText(mListBox->text(currentId));
    }
    updateButton();
}

void KCMFilter::updateButton()
{
    bool state = mEnableCheck->isChecked();

    mUpdateButton->setEnabled(state && (mSelCount == 1));
    mRemoveButton->setEnabled(state && (mSelCount > 0));
    mInsertButton->setEnabled(state);
    mImportButton->setEnabled(state);
    mExportButton->setEnabled(state);

    mListBox->setEnabled(state);
    mString->setEnabled(state);
    mKillCheck->setEnabled(state);
}

void KCMFilter::importFilters()
{
    TQString inFile = KFileDialog::getOpenFileName();
    if (inFile.length() > 0)
    {
        TQFile f(inFile);
        if ( f.open( IO_ReadOnly ) )
        {
            TQTextStream ts( &f );
            TQStringList paths;
            TQString line;
            while (!ts.atEnd())
            {
                line = ts.readLine();
                if (line.lower().compare("[adblock]") == 0)
                    continue;
                
                // Treat leading ! as filter comment, otherwise check expressions
                // are valid.
                if (!line.startsWith("!"))
                {
                    if (line.length()>2 && line[0]=='/' && line[line.length()-1] == '/')
                    {
                        TQString inside = line.mid(1, line.length()-2);
                        TQRegExp rx(inside);
                        if (!rx.isValid())
                            continue;
                    }
                    else
                    {
                        TQRegExp rx(line);
                        rx.setWildcard(true);
                        if (!rx.isValid())
                            continue;
                    }
                }

                if (!line.isEmpty() && mListBox->findItem(line, Qt::CaseSensitive|Qt::ExactMatch) == 0)
                    paths.append(line);
            }
            f.close();
            
            mListBox->insertStringList( paths );
            emit changed(true);
        }
    }
}

void KCMFilter::exportFilters()
{
  TQString outFile = KFileDialog::getSaveFileName();
  if (outFile.length() > 0)
  {
    TQFile f(outFile);
    if ( f.open( IO_WriteOnly ) )
    {
      TQTextStream ts( &f );
      ts.setEncoding( TQTextStream::UnicodeUTF8 );
      ts << "[AdBlock]" << endl;

      uint i;
      for( i = 0; i < mListBox->count(); ++i )
        ts << mListBox->text(i) << endl;

      f.close();
    }
  }
}

void KCMFilter::defaults()
{
	load( true );
}

void KCMFilter::save()
{
    mConfig->deleteGroup(mGroupname);
    mConfig->setGroup(mGroupname);

    mConfig->writeEntry("Enabled",mEnableCheck->isChecked());
    mConfig->writeEntry("Shrink",mKillCheck->isChecked());

    uint i;
    for( i = 0; i < mListBox->count(); ++i )
    {
        TQString key = "Filter-" + TQString::number(i);
        mConfig->writeEntry(key, mListBox->text(i));
    }
    mConfig->writeEntry("Count",mListBox->count());

    mConfig->sync();
    DCOPClient::mainClient()->send("konqueror*","KonquerorIface","reparseConfiguration()",TQByteArray());

}
void KCMFilter::load()
{
	load( false );
}

void KCMFilter::load( bool useDefaults )
{
    TQStringList paths;

	 mConfig->setReadDefaults( useDefaults );

    mConfig->setGroup( mGroupname );
    mEnableCheck->setChecked( mConfig->readBoolEntry("Enabled",false));
    mKillCheck->setChecked( mConfig->readBoolEntry("Shrink",false));

    TQMap<TQString,TQString> entryMap = mConfig->entryMap( mGroupname );
    TQMap<TQString,TQString>::ConstIterator it;
    int num = mConfig->readNumEntry("Count",0);
    for (int i=0; i<num; ++i)
    {
        TQString key = "Filter-" + TQString::number(i);
        it = entryMap.find(key);
        if (it != entryMap.end())
            paths.append(it.data());
    }

    mListBox->insertStringList( paths );

	 emit changed( useDefaults );
}

void KCMFilter::insertFilter()
{
    if ( !mString->text().isEmpty() )
    {
        mListBox->insertItem( mString->text() );
        int id=mListBox->count()-1;
        mListBox->clearSelection();
        mListBox->setSelected(id,true);
        mListBox->setCurrentItem(id);
        mListBox->ensureCurrentVisible();
        mString->clear();
        emit changed( true );
    }
    updateButton();
}

void KCMFilter::removeFilter()
{
    for( int i = mListBox->count(); i >= 0; --i )
    {
        if (mListBox->isSelected(i))
            mListBox->removeItem(i);
    }
    mString->clear();
    emit changed( true );
    updateButton();
}

void KCMFilter::updateFilter()
{
    if ( !mString->text().isEmpty() )
    {
        int index = mListBox->currentItem();
        if ( index >= 0 )
        {
            mListBox->changeItem( mString->text(), index );
            emit changed( true );
        }
    }
    updateButton();
}

TQString KCMFilter::quickHelp() const
{
    return i18n("<h1>Konqueror AdBlocK</h1> Konqueror AdBlocK allows you to create a list of filters"
                " that are checked against linked images and frames. URL's that match are either discarded or"
                " tqreplaced with a placeholder image. ");
}
