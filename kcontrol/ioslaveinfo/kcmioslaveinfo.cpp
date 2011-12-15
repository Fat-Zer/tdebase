/*
 * kcmioslaveinfo.cpp
 *
 * Copyright 2001 Alexander Neundorf <neundorf@kde.org>
 * Copyright 2001 George Staikos  <staikos@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <tqfile.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <layout.h>
#include <tqspinbox.h>
#include <tqtabwidget.h>
#include <textcodec.h>
#include <tqvbox.h>
#include <tqwhatsthis.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>

#include "kcmioslaveinfo.h"

typedef KGenericFactory<KCMIOSlaveInfo, TQWidget> SlaveFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ioslaveinfo, SlaveFactory("kcmioslaveinfo") )

KCMIOSlaveInfo::KCMIOSlaveInfo(TQWidget *parent, const char *name, const TQStringList &)
               :KCModule(SlaveFactory::instance(), parent,name),m_ioslavesLb(0),m_tfj(0)
{
   TQVBoxLayout *layout=new TQVBoxLayout(this, 0, KDialog::spacingHint());

   setQuickHelp( i18n("<h1>IO slaves</h1> Gives you an overview of the installed ioslaves."));
   setButtons( KCModule::Help );

   TQLabel* label=new TQLabel(i18n("Available IO slaves:"),this);
   TQHBox *hbox=new TQHBox(this);
   m_ioslavesLb=new KListBox(hbox);
   m_ioslavesLb->setMinimumSize(fontMetrics().width("blahfaselwhatever----"),10);
   connect( m_ioslavesLb, TQT_SIGNAL( selectionChanged( TQListBoxItem * ) ), TQT_SLOT( showInfo( TQListBoxItem * ) ) );
   //TODO make something useful after 2.1 is released
   m_info=new KTextBrowser(hbox);
   hbox->setSpacing(KDialog::spacingHint());

   layout->addWidget(label);
   layout->addWidget(hbox);
   hbox->setStretchFactor(m_ioslavesLb,1);
   hbox->setStretchFactor(m_info,5);

   TQStringList protocols=KProtocolInfo::protocols();
   for (TQStringList::Iterator it=protocols.begin(); it!=protocols.end(); ++it)
   {
      TQString proto = *it;
      m_ioslavesLb->insertItem( SmallIcon( KProtocolInfo::icon( proto )), 
                                proto );
   };
   m_ioslavesLb->sort();
   m_ioslavesLb->setSelected(0, true);

   setButtons(KCModule::Help);

   KAboutData *about =
   new KAboutData(I18N_NOOP("kcmioslaveinfo"),
	I18N_NOOP("KDE Panel System Information Control Module"),
	0, 0, KAboutData::License_GPL,
    	I18N_NOOP("(c) 2001 - 2002 Alexander Neundorf"));

   about->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
   about->addAuthor("George Staikos", 0, "staikos@kde.org");
   setAboutData( about );

}

void KCMIOSlaveInfo::slaveHelp( KIO::Job *, const TQByteArray &data)
{
    if ( data.size() == 0 ) { // EOF
        int index = helpData.find( "<meta http-equiv=\"Content-Type\"" );
        index = helpData.find( "charset=", index ) + 8;
        TQString charset = helpData.mid( index, helpData.find( '\"', index ) - index );
        TQString text = TQTextCodec::codecForName(charset.latin1())->toUnicode( helpData );
        index = text.find( "<div class=\"titlepage\">" );
        text = text.mid( index );
        index = text.find( "<table width=\"100%\" class=\"bottom-nav\"" );
        text = text.left( index );
        m_info->setText(text);
        return;
    }
    helpData += data;
}

void KCMIOSlaveInfo::slotResult(KIO::Job *)
{
   m_tfj = 0;
}

void KCMIOSlaveInfo::showInfo(const TQString& protocol)
{
   TQString file = TQString("kioslave/%1.docbook").arg( protocol );
   file = KGlobal::locale()->langLookup( file );
   if (m_tfj)
   {
      m_tfj->kill();
      m_tfj = 0;
   }

   if (!file.isEmpty())
   {
       helpData.truncate( 0 );
       m_tfj = KIO::get( KURL( TQString("help:/kioslave/%1.html").arg( protocol ) ), true, false );
       connect( m_tfj, TQT_SIGNAL( data( KIO::Job *, const TQByteArray &) ), TQT_SLOT( slaveHelp( KIO::Job *, const TQByteArray &) ) );
       connect( m_tfj, TQT_SIGNAL( result( KIO::Job * ) ), TQT_SLOT( slotResult( KIO::Job * ) ) );
       return;
   }
   m_info->setText(i18n("Some info about protocol %1:/ ...").arg(protocol));
}

void KCMIOSlaveInfo::showInfo(TQListBoxItem *item)
{
   if (item==0)
      return;
   showInfo( item->text() );
}

#include "kcmioslaveinfo.moc"

