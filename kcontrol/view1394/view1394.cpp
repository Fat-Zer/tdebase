/*
 * view1394.cpp
 *
 *  Copyright (C) 2003 Alexander Neundorf <neundorf@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdialog.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

#include "view1394.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

/* Get PRIX64 to output octlet_t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <libraw1394/csr.h>
#define CONFIGROM_BASE 0x00
#define CONFIGROM_CAP  0x08
#define CONFIGROM_GUID_HI 0x0c
#define CONFIGROM_GUID_LO 0x10
#define MAX_1394_PORTS 16

int my_reset_handler(raw1394handle_t handle, unsigned int )
{
   View1394* view1394=(View1394*)raw1394_get_userdata(handle);
   if (view1394!=0)
      view1394->rescanBus();
   return 0;
}


View1394::View1394(TQWidget *parent, const char *name)
:TDECModule(parent,name)
,m_insideRescanBus(false)
{
   setQuickHelp( i18n("On the right hand side you can see some information about "
               "your IEEE 1394 configuration.<br>"
               "The meaning of the columns:<br>"
               "<b>Name</b>: port or node name, the number can change with each bus reset<br>"
               "<b>GUID</b>: the 64 bit GUID of the node<br>"
               "<b>Local</b>: checked if the node is an IEEE 1394 port of your computer<br>"
               "<b>IRM</b>: checked if the node is isochronous resource manager capable<br>"
               "<b>CRM</b>: checked if the node is cycle master capable<br>"
               "<b>ISO</b>: checked if the node supports isochronous transfers<br>"
               "<b>BM</b>: checked if the node is bus manager capable<br>"
               "<b>PM</b>: checked if the node is power management capable<br>"
               "<b>Acc</b>: the cycle clock accuracy of the node, valid from 0 to 100<br>"
               "<b>Speed</b>: the speed of the node<br>"));

   setButtons( TDECModule::Help );

   m_ouiDb=new OuiDb();
   TQVBoxLayout *box=new TQVBoxLayout(this, 0, KDialog::spacingHint());
   m_view=new View1394Widget(this);
   for (int i=2; i<8; i++)
      m_view->m_listview->setColumnAlignment(i, AlignHCenter);
   m_view->m_listview->setColumnAlignment(8, AlignRight);
   m_view->m_listview->setColumnAlignment(9, AlignRight);
   box->addWidget(m_view);
   connect(m_view->m_busResetPb, TQT_SIGNAL(clicked()), this, TQT_SLOT(generateBusReset()));
   connect(&m_rescanTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(rescanBus()));
   m_notifiers.setAutoDelete(true);
   rescanBus();
}

View1394::~View1394()
{
}

bool View1394::readConfigRom(raw1394handle_t handle, nodeid_t nodeid, quadlet_t& firstQuad, quadlet_t& cap, octlet_t& guid)
{
   quadlet_t q=0;
   firstQuad=0;
   cap=0;
   guid=0;
   nodeaddr_t addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_BASE;
   for (int count=0; count<5; count++)
   {
      struct timeval tv;
      q=0;
      int res=raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q);
      if (res==0)
      {
         firstQuad=ntohl(q);
         break;
      }
      tv.tv_sec=0;
      tv.tv_usec=10*1000;
      select(0,0,0,0,&tv);
   }
   if (firstQuad==0)
      return false;

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_CAP;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   cap=ntohl(q);

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_GUID_HI;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   guid=octlet_t(ntohl(q))<<32;

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_GUID_LO;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   guid=guid|ntohl(q);

   return true;
}

void View1394::callRaw1394EventLoop(int fd)
{
   for (TQValueList<raw1394handle_t>::iterator it= m_handles.begin(); it!=m_handles.end(); ++it)
      if (raw1394_get_fd(*it)==fd)
      {
         raw1394_loop_iterate(*it);
         break;
      }
}

void View1394::rescanBus()
{
   if (m_insideRescanBus)
   {
      m_rescanTimer.start(100, true);
      return;
   }
   m_insideRescanBus=true;
//   static int depth=0;
//   depth++;
   m_notifiers.clear();
   for (TQValueList<raw1394handle_t>::iterator it=m_handles.begin(); it!=m_handles.end(); ++it)
      raw1394_destroy_handle(*it);
   m_handles.clear();
   m_view->m_listview->clear();
   
   raw1394handle_t handle=raw1394_new_handle();
   if (handle==NULL)
   {
      m_insideRescanBus=false;
      return;
   }
   //now search for new stuff
   struct raw1394_portinfo p_info[MAX_1394_PORTS];
   int num_of_cards=raw1394_get_port_info(handle, p_info, MAX_1394_PORTS);
   raw1394_destroy_handle(handle);
   //iterate over all cards
   for (int i=0; i<num_of_cards; i++)
   {
      handle=raw1394_new_handle();
      if (raw1394_set_port(handle, i)!=0)
      {
         raw1394_destroy_handle(handle);
         continue;
      }
      raw1394_set_userdata(handle, this);
      raw1394_set_bus_reset_handler(handle, my_reset_handler);
      TQSocketNotifier *notif=new TQSocketNotifier(raw1394_get_fd(handle),TQSocketNotifier::Read);
      connect(notif, TQT_SIGNAL(activated(int)), this, TQT_SLOT(callRaw1394EventLoop(int)));
      m_notifiers.append(notif);
      m_handles.append(handle);
      
      TQListViewItem *card=new TQListViewItem(m_view->m_listview,i18n("Port %1:\"%2\"").arg(i).arg(p_info[i].name));
      int num_of_nodes=raw1394_get_nodecount(handle);

      int localNodeId=raw1394_get_local_id(handle);
      //iterate over all nodes connected to this card
      for (int j=0; j<num_of_nodes; j++)
      {
         //get the guid of the node
         octlet_t guid=0;
         quadlet_t firstQuad=0;
         quadlet_t cap=0;
         bool success=readConfigRom(handle, j, firstQuad, cap, guid);

         TQString nodeStr=i18n("Node %1").arg(j);
         if (!success)
         {
            new TQListViewItem(card,nodeStr, i18n("Not ready"));
            continue;
         }
         if (((firstQuad>>24) & 0xff)==1) //minimal config rom
         {
            TQString guidStr=TQString::number(firstQuad,16);
            guidStr="0x"+guidStr.rightJustify(6,'0');
            new TQListViewItem(card,nodeStr, guidStr);
         }
         else  //general config rom
         {
            TQString guidStr;
            char buf[32];
            snprintf(buf, 32, "%"PRIX64, guid);
            guidStr=buf;
            guidStr="0x"+guidStr.rightJustify(16,'0');
            TQString local=((j | 0xffc0) == localNodeId)?"X":"";
            TQString irmStr=(cap & 0x80000000) ? "X":"";
            TQString cmStr=(cap & 0x40000000) ? "X":"";
            TQString isStr=(cap & 0x20000000) ? "X":"";
            TQString bmStr=(cap & 0x10000000) ? "X":"";
            TQString pmStr=(cap & 0x08000000) ? "X":"";
            TQString accStr=TQString::number((cap &0x00ff0000)>>16);
            int speed=( cap & 0x00000007);
            TQString speedStr;
            switch(speed)
            {
            case(3):
               speedStr="800";
               break;
            case(2):
               speedStr="400";
               break;
            case(1):
               speedStr="200";
               break;
            case(0):
            default:
               speedStr="100";
               break;
            }
            TQListViewItem* node=new TQListViewItem(card);
            node->setText(0,nodeStr);
            node->setText(1, guidStr);
            node->setText(2,local);
            node->setText(3, irmStr);
            node->setText(4, cmStr);
            node->setText(5,isStr);
            node->setText(6,bmStr);
            node->setText(7,pmStr);
            node->setText(8, accStr);
            node->setText(9, speedStr);
            node->setText(10, m_ouiDb->vendor(guid));
         }
      }
      card->setOpen(true);
   }
//   depth--;
   m_insideRescanBus=false;
}

void View1394::generateBusReset()
{
   for (TQValueList<raw1394handle_t>::iterator it=m_handles.begin(); it!=m_handles.end(); ++it)
      raw1394_reset_bus(*it);
}


OuiDb::OuiDb()
{
   TQString filename=locate("data","kcmview1394/oui.db");
   if (filename.isEmpty())
      return;
   TQFile f(filename);
   if (!f.open(IO_ReadOnly))
      return;

   TQByteArray ba=f.readAll();
   int bytesLeft=ba.size();
   char* data=ba.data();
   while(bytesLeft>8)
   {
      char *eol=(char*)memchr((const void*)data, '\n',bytesLeft);
      if (eol==0)
         break;
      if ((eol-data)<8)
         break;
      data[6]='\0';
      *eol='\0';
      m_vendorIds.insert(data, data+7);
      bytesLeft-=(eol+1-data);
      data=eol+1;
   }

   f.close();
}

TQString OuiDb::vendor(octlet_t guid)
{
   guid=(guid & 0xffffff0000000000LL)>>40;
   TQString key=TQString::number((unsigned int)(guid),16);
   key=key.rightJustify(6, '0').upper();
   TQString v=m_vendorIds[key];
   if (v.isEmpty())
      v=i18n("Unknown");
   return v;
}



// ------------------------------------------------------------------------

extern "C"
{

   KDE_EXPORT TDECModule *create_view1394(TQWidget *parent, const char *name)
   {
      TDEGlobal::locale()->insertCatalogue("kcmview1394");
      return new View1394(parent, name);
   }
}

#include "view1394.moc"
