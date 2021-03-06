/*

  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
                     Stefan Schimanski <1Stein@gmx.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/



#ifndef __NSPluginClassIface_h__
#define __NSPluginClassIface_h__


#include <tqstringlist.h>
#include <tqcstring.h>
#include <dcopobject.h>
#include <dcopref.h>


class NSPluginViewerIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  virtual void shutdown() = 0;
  virtual DCOPRef newClass(TQString plugin) = 0;
};


class NSPluginClassIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:

  virtual DCOPRef newInstance(TQString url, TQString mimeType, TQ_INT8 embed,
                              TQStringList argn, TQStringList argv,
                              TQString appId, TQString callbackId, TQ_INT8 reload,
                              TQ_INT8 doPost, TQByteArray postData, TQ_UINT32 xembed) = 0;
  virtual TQString getMIMEDescription() = 0;

};


class NSPluginInstanceIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:

  virtual void shutdown() = 0;

  virtual int winId() = 0;

  virtual int setWindow(TQ_INT8 remove=0) = 0;

  virtual void resizePlugin(TQ_INT32 w, TQ_INT32 h) = 0;

  virtual void javascriptResult(TQ_INT32 id, TQString result) = 0;

  virtual void displayPlugin() = 0;
  
  virtual void gotFocusIn() = 0;
  virtual void gotFocusOut() = 0;
};


#endif

