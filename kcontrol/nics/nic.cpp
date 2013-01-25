/*
 * nic.cpp
 *
 *  Copyright (C) 2001 Alexander Neundorf <neundorf@kde.org>
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

#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <config.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#include <kaboutdata.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kglobal.h>

#include <tqlayout.h>
#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqtimer.h>

#include "nic.h"

#ifdef USE_SOLARIS
/* net/if.h is incompatible with STL on Solaris 2.6 - 2.8, redefine
   map in the header file because we don't need it. -- Simon Josefsson */
#define map junkmap
#endif
#  include <net/if.h>
#ifdef USE_SOLARIS
#undef map
#endif

#include <sys/ioctl.h>

#ifndef	HAVE_STRUCT_SOCKADDR_SA_LEN
	#undef HAVE_GETNAMEINFO
	#undef HAVE_GETIFADDRS
#endif

#if defined(HAVE_GETNAMEINFO) && defined(HAVE_GETIFADDRS)
	#include <ifaddrs.h>
	#include <netdb.h>

	TQString flags_tos (unsigned int flags);
#endif

typedef KGenericFactory<KCMNic, TQWidget> KCMNicFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_nic, KCMNicFactory("kcmnic"))

struct MyNIC
{
   TQString name;
   TQString addr;
   TQString netmask;
   TQString state;
   TQString type;
   TQString HWaddr;
};

typedef TQPtrList<MyNIC> NICList;

NICList* findNICs();

KCMNic::KCMNic(TQWidget *parent, const char * name, const TQStringList &)
   :TDECModule(KCMNicFactory::instance(), parent,name)
{
   TQVBoxLayout *box=new TQVBoxLayout(this, 0, KDialog::spacingHint());
   m_list=new TQListView(this);
   box->addWidget(m_list);
   m_list->addColumn(i18n("Name"));
   m_list->addColumn(i18n("IP Address"));
   m_list->addColumn(i18n("Network Mask"));
   m_list->addColumn(i18n("Type"));
   m_list->addColumn(i18n("State"));
   m_list->addColumn(i18n("HWaddr"));
   m_list->setAllColumnsShowFocus(true);
   TQHBoxLayout *hbox=new TQHBoxLayout(box);
   m_updateButton=new TQPushButton(i18n("&Update"),this);
   hbox->addWidget(m_updateButton);
   hbox->addStretch(1);
   TQTimer* timer=new TQTimer(this);
   timer->start(60000);
   connect(m_updateButton,TQT_SIGNAL(clicked()),this,TQT_SLOT(update()));
   connect(timer,TQT_SIGNAL(timeout()),this,TQT_SLOT(update()));
   update();
   TDEAboutData *about =
   new TDEAboutData(I18N_NOOP("kcminfo"),
	I18N_NOOP("TDE Panel System Information Control Module"),
	0, 0, TDEAboutData::License_GPL,
	I18N_NOOP("(c) 2001 - 2002 Alexander Neundorf"));

   about->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
   setAboutData( about );

}

void KCMNic::update()
{
   m_list->clear();
   NICList *nics=findNICs();
   nics->setAutoDelete(true);
   for (MyNIC* tmp=nics->first(); tmp!=0; tmp=nics->next())
      new TQListViewItem(m_list,tmp->name, tmp->addr, tmp->netmask, tmp->type, tmp->state, tmp->HWaddr);
   delete nics;
}

static TQString HWaddr2String( char *hwaddr )
{
   TQString ret;
   int i;
   for (i=0; i<6; i++, hwaddr++) {
	int v = (*hwaddr & 0xff);
	TQString num = TQString("%1").arg(v,0,16);
	if (num.length() < 2)
		num.prepend("0");
	if (i>0)
		ret.append(":");
	ret.append(num);
   }
   return ret;
}

NICList* findNICs()
{
   TQString upMessage(   i18n("State of network card is connected",    "Up") );
   TQString downMessage( i18n("State of network card is disconnected", "Down") );

   NICList* nl=new NICList;
   nl->setAutoDelete(true);

#if !defined(HAVE_GETIFADDRS) || !defined(HAVE_GETNAMEINFO)

   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   char buf[8*1024];
   struct ifconf ifc;
   ifc.ifc_len = sizeof(buf);
   ifc.ifc_req = (struct ifreq *) buf;
   int result=ioctl(sockfd, SIOCGIFCONF, &ifc);

   for (char* ptr = buf; ptr < buf + ifc.ifc_len; )
   {
      struct ifreq *ifr =(struct ifreq *) ptr;
#ifdef	HAVE_STRUCT_SOCKADDR_SA_LEN
      int len = sizeof(struct sockaddr);
      if (ifr->ifr_addr.sa_len > len)
         len = ifr->ifr_addr.sa_len;		/* length > 16 */
      ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */
#else
      ptr += sizeof(*ifr);			/* for next one in buffer */
#endif

      int flags;
      struct sockaddr_in *sinptr;
      MyNIC *tmp=0;
      switch (ifr->ifr_addr.sa_family)
      {
      case AF_INET:
         sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
         flags=0;

         struct ifreq ifcopy;
         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFFLAGS,&ifcopy);
         flags=ifcopy.ifr_flags;

         tmp=new MyNIC;
         tmp->name=ifr->ifr_name;
         tmp->state= ((flags & IFF_UP) == IFF_UP) ? upMessage : downMessage;

         if ((flags & IFF_BROADCAST) == IFF_BROADCAST)
            tmp->type=i18n("Broadcast");
         else if ((flags & IFF_POINTOPOINT) == IFF_POINTOPOINT)
            tmp->type=i18n("Point to Point");
#ifndef _AIX
         else if ((flags & IFF_MULTICAST) == IFF_MULTICAST)
            tmp->type=i18n("Multicast");
#endif
         else if ((flags & IFF_LOOPBACK) == IFF_LOOPBACK)
            tmp->type=i18n("Loopback");
         else
            tmp->type=i18n("Unknown");

         tmp->addr=inet_ntoa(sinptr->sin_addr);

         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFNETMASK,&ifcopy);
         if (result==0)
         {
            sinptr = (struct sockaddr_in *) &ifcopy.ifr_addr;
            tmp->netmask=inet_ntoa(sinptr->sin_addr);
         }
         else
            tmp->netmask=i18n("Unknown");

         ifcopy=*ifr;
         result=-1;  // if none of the two #ifs below matches, ensure that result!=0 so that "Unknown" is returned as result
#ifdef SIOCGIFHWADDR
         result=ioctl(sockfd,SIOCGIFHWADDR,&ifcopy);
         if (result==0)
         {
            char *n = &ifcopy.ifr_ifru.ifru_hwaddr.sa_data[0];
            tmp->HWaddr = HWaddr2String(n);
         }
#elif defined SIOCGENADDR
         result=ioctl(sockfd,SIOCGENADDR,&ifcopy);
         if (result==0)
         {
            char *n = &ifcopy.ifr_ifru.ifru_enaddr[0];
            tmp->HWaddr = HWaddr2String(n);
         }
#endif
         if (result!=0)
         {
            tmp->HWaddr = i18n("Unknown");
         }

         nl->append(tmp);
         break;

      default:
         break;
      }
   }
#else
  struct ifaddrs *ifap, *ifa;
  if (getifaddrs(&ifap) != 0) {
    return nl;
  }

  MyNIC *tmp=0;
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    switch (ifa->ifa_addr->sa_family) {
    case AF_INET6:
    case AF_INET: {
      tmp = new MyNIC;
      tmp->name = ifa->ifa_name;

      char buf[128];

      bzero(buf, 128);
      getnameinfo(ifa->ifa_addr, ifa->ifa_addr->sa_len, buf, 127, 0, 0, NI_NUMERICHOST);
      tmp->addr = buf;

      if (ifa->ifa_netmask != NULL) {
	bzero(buf, 128);
	getnameinfo(ifa->ifa_netmask, ifa->ifa_netmask->sa_len, buf, 127, 0, 0, NI_NUMERICHOST);
	tmp->netmask = buf;
      }

      tmp->state= (ifa->ifa_flags & IFF_UP) ? upMessage : downMessage;
      tmp->type = flags_tos(ifa->ifa_flags);

      nl->append(tmp);
      break;
    }
    default:
      break;
    }
  }

  freeifaddrs(ifap);
#endif
   return nl;
}


#if defined(HAVE_GETNAMEINFO) && defined(HAVE_GETIFADDRS)
TQString flags_tos (unsigned int flags)
{
  TQString tmp;
  if (flags & IFF_POINTOPOINT) {
    tmp +=  i18n("Point to Point");
  }

  if (flags & IFF_BROADCAST) {
    if (tmp.length()) {
      tmp += TQString::fromLatin1(", ");
    }
    tmp += i18n("Broadcast");
  }
  
  if (flags & IFF_MULTICAST) {
    if (tmp.length()) {
      tmp += TQString::fromLatin1(", ");
    }
    tmp += i18n("Multicast");
  }
  
  if (flags & IFF_LOOPBACK) {
    if (tmp.length()) {
      tmp += TQString::fromLatin1(", ");
    }
    tmp += i18n("Loopback");
  }
  return tmp;
}
#endif

#include "nic.moc"
