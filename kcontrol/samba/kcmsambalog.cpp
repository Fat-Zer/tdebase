/*
 * kcmsambalog.cpp
 *
 * Copyright (c) 2000 Alexander Neundorf <neundorf@kde.org>
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
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <tdemessagebox.h>
#include <kdialog.h>
#include <tdelocale.h>

#include "kcmsambalog.h"
#include "kcmsambalog.moc"

#define LOG_SCREEN_XY_OFFSET 10

LogView::LogView(TQWidget *parent,TDEConfig *config, const char *name)
: TQWidget (parent, name)
,configFile(config)
,filesCount(0)
,connectionsCount(0)
,logFileName("/var/log/samba.log",this)
,label(&logFileName,i18n("Samba log file: "),this)
,viewHistory(this)
,showConnOpen(i18n("Show opened connections"),this)
,showConnClose(i18n("Show closed connections"),this)
,showFileOpen(i18n("Show opened files"),this)
,showFileClose(i18n("Show closed files"),this)
,updateButton(i18n("&Update"),this)
{
   TQVBoxLayout *mainLayout=new TQVBoxLayout(this, KDialog::marginHint(),
       KDialog::spacingHint());
   TQHBoxLayout *leLayout=new TQHBoxLayout(mainLayout);
   leLayout->addWidget(&label);
   leLayout->addWidget(&logFileName,1);
   mainLayout->addWidget(&viewHistory,1);
   TQGridLayout *subLayout=new TQGridLayout(mainLayout,2,2);
   subLayout->addWidget(&showConnOpen,0,0);
   subLayout->addWidget(&showConnClose,1,0);
   subLayout->addWidget(&showFileOpen,0,1);
   subLayout->addWidget(&showFileClose,1,1);
   mainLayout->addWidget(&updateButton,0,Qt::AlignLeft);

   TQWhatsThis::add( &logFileName, i18n("This page presents the contents of"
     " your samba log file in a friendly layout. Check that the correct log"
     " file for your computer is listed here. If you need to, correct the name"
     " or location of the log file, and then click the \"Update\" button.") );

   TQWhatsThis::add( &showConnOpen, i18n("Check this option if you want to"
     " view the details for connections opened to your computer.") );

   TQWhatsThis::add( &showConnClose, i18n("Check this option if you want to"
     " view the events when connections to your computer were closed.") );

   TQWhatsThis::add( &showFileOpen, i18n("Check this option if you want to"
     " see the files which were opened on your computer by remote users."
     " Note that file open/close events are not logged unless the samba"
     " log level is set to at least 2 (you cannot set the log level"
     " using this module).") );

   TQWhatsThis::add( &showFileClose, i18n("Check this option if you want to"
     " see the events when files opened by remote users were closed."
     " Note that file open/close events are not logged unless the samba"
     " log level is set to at least 2 (you cannot set the log level"
     " using this module).") );

   TQWhatsThis::add( &updateButton, i18n("Click here to refresh the information"
     " on this page. The log file (shown above) will be read to obtain the"
     " events logged by samba.") );

   logFileName.setURL("/var/log/samba.log");

   viewHistory.setAllColumnsShowFocus(TRUE);
   viewHistory.setFocusPolicy(TQ_ClickFocus);
   viewHistory.setShowSortIndicator(true);

   viewHistory.addColumn(i18n("Date & Time"),130);
   viewHistory.addColumn(i18n("Event"),150);
   viewHistory.addColumn(i18n("Service/File"),210);
   viewHistory.addColumn(i18n("Host/User"),150);

   TQWhatsThis::add( &viewHistory, i18n("This list shows details of the events"
     " logged by samba. Note that events at the file level are not logged"
     " unless you have configured the log level for samba to 2 or greater.<p>"
     " As with many other lists in TDE, you can click on a column heading"
     " to sort on that column. Click again to change the sorting direction"
     " from ascending to descending or vice versa.<p>"
     " If the list is empty, try clicking the \"Update\" button. The samba"
     " log file will be read and the list refreshed.") );

   showConnOpen.setChecked(TRUE);
   showConnClose.setChecked(TRUE);
   showFileOpen.setChecked(FALSE);
   showFileClose.setChecked(FALSE);

   connect(&updateButton,TQT_SIGNAL(clicked()),this,TQT_SLOT(updateList()));
   emit contentsChanged(&viewHistory,0,0);

   label.setMinimumSize(label.sizeHint());
   logFileName.setMinimumSize(250,logFileName.sizeHint().height());
   viewHistory.setMinimumSize(425,200);
   showConnOpen.setMinimumSize(showConnOpen.sizeHint());
   showConnClose.setMinimumSize(showConnClose.sizeHint());
   showFileOpen.setMinimumSize(showFileOpen.sizeHint());
   showFileClose.setMinimumSize(showFileClose.sizeHint());
   updateButton.setFixedSize(updateButton.sizeHint());
}

void LogView::loadSettings()
{
   if (configFile==0) return;
   configFile->setGroup(LOGGROUPNAME);
   logFileName.setURL(configFile->readPathEntry( "SambaLogFile", "/var/log/samba.log"));

   showConnOpen.setChecked(configFile->readBoolEntry( "ShowConnectionOpen", TRUE));
   showConnClose.setChecked(configFile->readBoolEntry( "ShowConnectionClose", FALSE));
   showFileOpen.setChecked(configFile->readBoolEntry( "ShowFileOpen", TRUE));
   showFileClose.setChecked(configFile->readBoolEntry( "ShowFileClose", FALSE));
}

void LogView::saveSettings()
{
   if (configFile==0) return;
   configFile->setGroup(LOGGROUPNAME);
   configFile->writePathEntry( "SambaLogFile", logFileName.url());

   configFile->writeEntry( "ShowConnectionOpen", showConnOpen.isChecked());
   configFile->writeEntry( "ShowConnectionClose", showConnClose.isChecked());
   configFile->writeEntry( "ShowFileOpen", showFileOpen.isChecked());
   configFile->writeEntry( "ShowFileClose", showFileClose.isChecked());
}

#define CONN_OPEN " connect to service "
#define CONN_CLOSE " closed connection to service "
#define FILE_OPEN " opened file "
#define FILE_CLOSE " closed file "

//caution ! high optimized code :-)
void LogView::updateList()
{
   TQFile logFile(logFileName.url());
   if (logFile.open(IO_ReadOnly))
   {
      TQApplication::setOverrideCursor(tqwaitCursor);
      viewHistory.clear();
      filesCount=0;
      connectionsCount=0;

      int connOpenLen(strlen(CONN_OPEN));
      int connCloseLen(strlen(CONN_CLOSE));
      int fileOpenLen(strlen(FILE_OPEN));
      int fileCloseLen(strlen(FILE_CLOSE));

      char buf[400];
      char *c1, *c2, *c3, *c4, *c, time[25];
      int timeRead(0);

      while (!logFile.atEnd())
      {
         logFile.readLine(buf,sizeof(buf));
         timeRead=0;
         if (buf[0]=='[')
         {
            if (strlen(buf)>11)
               if (buf[5]=='/')
               {
                  buf[20]='\0';
                  strncpy(time,buf+1, sizeof(time));
                  time[sizeof(time)-1] = '\0';
                  timeRead=1;
               };
         };
         if (timeRead==0)
         {
            c1=0;
            c2=0;
            c3=0;
            c4=0;
            if (showConnOpen.isChecked()) c1=strstr(buf,CONN_OPEN);
            if (c1==0)
            {
               if (showConnClose.isChecked()) c2=strstr(buf,CONN_CLOSE);
               if (c2==0)
               {
                  if (showFileOpen.isChecked()) c3=strstr(buf,FILE_OPEN);
                  if (c3==0)
                  {
                     if (showFileClose.isChecked()) c4=strstr(buf,FILE_CLOSE);
                     if (c4==0) continue;
                  };
               };
            };
            if (c1!=0)
            {
               c=strstr(buf," as user");
               *c='\0';
               *c1='\0';
               new QListViewItemX(&viewHistory,time,I18N_NOOP("CONNECTION OPENED"),c1+connOpenLen,buf+2);
               connectionsCount++;
            }
            else if (c2!=0)
            {
               *c2='\0';
               new QListViewItemX(&viewHistory,time,I18N_NOOP("CONNECTION CLOSED"),c2+connCloseLen,buf+2);
            }
            else if (c3!=0)
            {
               c=strstr(buf," read=");
               *c='\0';
               *c3='\0';
               new QListViewItemX(&viewHistory,time,I18N_NOOP("            FILE OPENED"),c3+fileOpenLen,buf+2);
               filesCount++;
            }
            else if (c4!=0)
            {
               c=strstr(buf," (numopen=");
               *c='\0';
               *c4='\0';
               new QListViewItemX(&viewHistory,time,I18N_NOOP("            FILE CLOSED"),c4+fileCloseLen,buf+2);
            };
         };
      };
      logFile.close();
      emit contentsChanged(&viewHistory, filesCount, connectionsCount);
      TQApplication::restoreOverrideCursor();
   }
   else
   {
      TQString tmp = i18n("Could not open file %1").arg(logFileName.url());
      KMessageBox::error(this,tmp);
   };
}

