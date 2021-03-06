/***************************************************************************
 *   Copyright (C) 2001 by Matthias Hoelzer-Kluepfel <mhk@caldera.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <tqfile.h>
#include <tqdir.h>
#include <tqregexp.h>

#include <tdelocale.h>
#include <tdemessagebox.h>

#include "usbdb.h"
#include "usbdevices.h"

#include <math.h>

#ifdef Q_OS_FREEBSD
#include <sys/ioctl.h>
#include <sys/param.h>
#endif

TQPtrList<USBDevice> USBDevice::_devices;
USBDB *USBDevice::_db;


USBDevice::USBDevice()
  : _bus(0), _level(0), _parent(0), _port(0), _count(0), _device(0),
    _channels(0), _power(0), _speed(0.0),
    _bwTotal(0), _bwUsed(0), _bwPercent(0), _bwIntr(0), _bwIso(0), _hasBW(false),
    _verMajor(0), _verMinor(0), _class(0), _sub(0), _prot(0), _maxPacketSize(0), _configs(0),
    _vendorID(0), _prodID(0), _revMajor(0), _revMinor(0)
{
  _devices.append(this);
  _devices.setAutoDelete(true);

  if (!_db)
    _db = new USBDB;
}

static TQString catFile(TQString fname)
{
  char buffer[256];
  TQString result;
  int fd = ::open(TQFile::encodeName(fname), O_RDONLY);
  if (fd<0)
	return TQString::null;

  if (fd >= 0)
    {
      ssize_t count;
      while ((count = ::read(fd, buffer, 256)) > 0)
	result.append(TQString(buffer).left(count));

      ::close(fd);
    }
  return result.stripWhiteSpace();
}

void USBDevice::parseSysDir(int bus, int parent, int level, TQString dname)
{
  _level = level;
  _parent = parent;
  _manufacturer = catFile(dname + "/manufacturer");
  _product = catFile(dname + "/product");

  _bus = bus;
  _device = catFile(dname + "/devnum").toUInt();

  if (_device == 1)
    _product += TQString(" (%1)").arg(_bus);

  _vendorID = catFile(dname + "/idVendor").toUInt(0, 16);
  _prodID = catFile(dname + "/idProduct").toUInt(0, 16);

  _class = catFile(dname + "/bDeviceClass").toUInt(0, 16);
  _sub = catFile(dname + "/bDeviceSubClass").toUInt(0, 16);
  _maxPacketSize = catFile(dname + "/bMaxPacketSize0").toUInt();

  _speed = catFile(dname + "/speed").toDouble();
  _serial = catFile(dname + "/serial");
  _channels = catFile(dname + "/maxchild").toUInt();

  double version = catFile(dname + "/version").toDouble();
  _verMajor = int(version);
  _verMinor = int(10*(version - floor(version)));

  TQDir dir(dname);
  dir.setNameFilter(TQString("%1-*").arg(bus));
  dir.setFilter(TQDir::Dirs);
  TQStringList list = dir.entryList();

  for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
    if ((*it).contains(':'))
      continue;

    USBDevice* dev = new USBDevice();
    dev->parseSysDir(bus, ++level, _device, dname + "/" + *it);
  }
}

void USBDevice::parseLine(TQString line)
{
  if (line.startsWith("T:"))
    sscanf(line.local8Bit().data(),
	   "T:  Bus=%2d Lev=%2d Prnt=%2d Port=%d Cnt=%2d Dev#=%3d Spd=%3f MxCh=%2d",
	   &_bus, &_level, &_parent, &_port, &_count, &_device, &_speed, &_channels);
  else if (line.startsWith("S:  Manufacturer"))
    _manufacturer = line.mid(17);
  else if (line.startsWith("S:  Product")) {
    _product = line.mid(12);
    /* add bus number to root devices */
    if (_device==1)
	_product += TQString(" (%1)").arg(_bus);
  }
  else if (line.startsWith("S:  SerialNumber"))
    _serial = line.mid(17);
  else if (line.startsWith("B:"))
    {
      sscanf(line.local8Bit().data(),
	     "B:  Alloc=%3d/%3d us (%2d%%), #Int=%3d, #Iso=%3d",
	     &_bwUsed, &_bwTotal, &_bwPercent, &_bwIntr, &_bwIso);
      _hasBW = true;
    }
  else if (line.startsWith("D:"))
    {
      char buffer[11];
      sscanf(line.local8Bit().data(),
	     "D:  Ver=%x.%x Cls=%x(%10s) Sub=%x Prot=%x MxPS=%d #Cfgs=%d",
	     &_verMajor, &_verMinor, &_class, buffer, &_sub, &_prot, &_maxPacketSize, &_configs);
      _className = buffer;
    }
  else if (line.startsWith("P:"))
    sscanf(line.local8Bit().data(),
	   "P:  Vendor=%x ProdID=%x Rev=%x.%x",
	   &_vendorID, &_prodID, &_revMajor, &_revMinor);
}


USBDevice *USBDevice::find(int bus, int device)
{
  TQPtrListIterator<USBDevice> it(_devices);
  for ( ; it.current(); ++it)
    if (it.current()->bus() == bus && it.current()->device() == device)
      return it.current();
  return 0;
}

TQString USBDevice::product()
{
  if (!_product.isEmpty())
    return _product;
  TQString pname = _db->device(_vendorID, _prodID);
  if (!pname.isEmpty())
    return pname;
  return i18n("Unknown");
}


TQString USBDevice::dump()
{
  TQString r;

  r = "<qml><h2><center>" + product() + "</center></h2><br/><hl/>";

  if (!_manufacturer.isEmpty())
    r += i18n("<b>Manufacturer:</b> ") + _manufacturer + "<br/>";
  if (!_serial.isEmpty())
    r += i18n("<b>Serial #:</b> ") + _serial + "<br/>";

  r += "<br/><table>";

  TQString c = TQString("<td>%1</td>").arg(_class);
  TQString cname = _db->cls(_class);
  if (!cname.isEmpty())
    c += "<td>(" + i18n(cname.latin1()) +")</td>";
  r += i18n("<tr><td><i>Class</i></td>%1</tr>").arg(c);
  TQString sc = TQString("<td>%1</td>").arg(_sub);
  TQString scname = _db->subclass(_class, _sub);
  if (!scname.isEmpty())
    sc += "<td>(" + i18n(scname.latin1()) +")</td>";
  r += i18n("<tr><td><i>Subclass</i></td>%1</tr>").arg(sc);
  TQString pr = TQString("<td>%1</td>").arg(_prot);
  TQString prname = _db->protocol(_class, _sub, _prot);
  if (!prname.isEmpty())
    pr += "<td>(" + prname +")</td>";
  r += i18n("<tr><td><i>Protocol</i></td>%1</tr>").arg(pr);
#ifndef Q_OS_FREEBSD
  r += i18n("<tr><td><i>USB Version</i></td><td>%1.%2</td></tr>")
    .arg(_verMajor,0,16)
    .arg(TQString::number(_verMinor,16).prepend('0').right(2));
#endif
  r += "<tr><td></td></tr>";

  TQString v = TQString::number(_vendorID,16);
  TQString name = _db->vendor(_vendorID);
  if (!name.isEmpty())
    v += "<td>(" + name +")</td>";
  r += i18n("<tr><td><i>Vendor ID</i></td><td>0x%1</td></tr>").arg(v);
  TQString p = TQString::number(_prodID,16);
  TQString pname = _db->device(_vendorID, _prodID);
  if (!pname.isEmpty())
    p += "<td>(" + pname +")</td>";
  r += i18n("<tr><td><i>Product ID</i></td><td>0x%1</td></tr>").arg(p);
  r += i18n("<tr><td><i>Revision</i></td><td>%1.%2</td></tr>")
    .arg(_revMajor,0,16)
    .arg(TQString::number(_revMinor,16).prepend('0').right(2));
  r += "<tr><td></td></tr>";

  r += i18n("<tr><td><i>Speed</i></td><td>%1 Mbit/s</td></tr>").arg(_speed);
  r += i18n("<tr><td><i>Channels</i></td><td>%1</td></tr>").arg(_channels);
#ifdef Q_OS_FREEBSD
	if ( _power )
		r += i18n("<tr><td><i>Power Consumption</i></td><td>%1 mA</td></tr>").arg(_power);
	else
		r += i18n("<tr><td><i>Power Consumption</i></td><td>self powered</td></tr>");
	r += i18n("<tr><td><i>Attached Devicenodes</i></td><td>%1</td></tr>").arg(*_devnodes.at(0));
	if ( _devnodes.count() > 1 )
		for ( TQStringList::Iterator it = _devnodes.at(1); it != _devnodes.end(); ++it )
			r += "<tr><td></td><td>" + *it + "</td></tr>";
#else
  r += i18n("<tr><td><i>Max. Packet Size</i></td><td>%1</td></tr>").arg(_maxPacketSize);
#endif
  r += "<tr><td></td></tr>";

  if (_hasBW)
    {
      r += i18n("<tr><td><i>Bandwidth</i></td><td>%1 of %2 (%3%)</td></tr>").arg(_bwUsed).arg(_bwTotal).arg(_bwPercent);
      r += i18n("<tr><td><i>Intr. requests</i></td><td>%1</td></tr>").arg(_bwIntr);
      r += i18n("<tr><td><i>Isochr. requests</i></td><td>%1</td></tr>").arg(_bwIso);
      r += "<tr><td></td></tr>";
    }

  r += "</table>";

  return r;
}


#ifndef Q_OS_FREEBSD
bool USBDevice::parse(TQString fname)
{
  _devices.clear();

  TQString result;

  // read in the complete file
  //
  // Note: we can't use a TQTextStream, as the files in /proc
  // are pseudo files with zero length
  char buffer[256];
  int fd = ::open(TQFile::encodeName(fname), O_RDONLY);
  if (fd<0)
	return false;

  if (fd >= 0)
    {
      ssize_t count;
      while ((count = ::read(fd, buffer, 256)) > 0)
	result.append(TQString(buffer).left(count));

      ::close(fd);
    }

  // read in the device infos
  USBDevice *device = 0;
  int start=0, end;
  result.replace(TQRegExp("^\n"),"");
  while ((end = result.find('\n', start)) > 0)
    {
      TQString line = result.mid(start, end-start);

      if (line.startsWith("T:"))
	device = new USBDevice();

      if (device)
	device->parseLine(line);

      start = end+1;
    }
  return true;
}

bool USBDevice::parseSys(TQString dname)
{
   TQDir d(dname);
   d.setNameFilter("usb*");
   TQStringList list = d.entryList();

   for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it) {
     USBDevice* device = new USBDevice();

     int bus = 0;
     TQRegExp bus_reg("[a-z]*([0-9]+)");
     if (bus_reg.search(*it) != -1)
         bus = bus_reg.cap(1).toInt();


     device->parseSysDir(bus, 0, 0, d.absPath() + "/" + *it);
  }

  return d.count();
}

#else

/*
 * FreeBSD support by Markus Brueffer <markus@brueffer.de>
 * libusb20 support by Hans Petter Selasky <hselasky@freebsd.org>
 *
 * Basic idea and some code fragments were taken from FreeBSD's usbdevs(8), 
 * originally developed for NetBSD, so this code should work with no or 
 * only little modification on NetBSD.
 */

void USBDevice::collectData(struct libusb20_backend *pbe,
    struct libusb20_device *pdev)
{
	char tempbuf[32];
	struct usb_device_info di;

	if (libusb20_dev_get_info(pdev, &di))
		memset(&di, 0, sizeof(di));

	// determine data for this device
	_level        = 0;
	_parent       = 0;

	_bus          = di.udi_bus;
	_device       = di.udi_addr;
	_product      = TQString::fromLatin1(di.udi_product);
	if ( _device == 1 )
		_product += " " + TQString::number( _bus );
	_manufacturer = TQString::fromLatin1(di.udi_vendor);
	_prodID       = di.udi_productNo;
	_vendorID     = di.udi_vendorNo;
	_class        = di.udi_class;
	_sub          = di.udi_subclass;
	_prot         = di.udi_protocol;
	_power        = di.udi_power;
	_channels     = di.udi_nports;

	// determine the speed
	switch (di.udi_speed) {
		case LIBUSB20_SPEED_LOW:  _speed = 1.5;   break;
		case LIBUSB20_SPEED_FULL: _speed = 12.0;  break;
		case LIBUSB20_SPEED_HIGH: _speed = 480.0; break;
		case LIBUSB20_SPEED_VARIABLE: _speed = 480.0; break;
		case LIBUSB20_SPEED_SUPER: _speed = 4800.0; break;
		default: _speed = 480.0; break;
	}

	// Get all attached devicenodes
	for (int i = 0; i < 32; ++i) {
	    if (libusb20_dev_get_iface_desc(pdev, i, tempbuf, sizeof(tempbuf)) == 0) {
		_devnodes << tempbuf;
	    } else {
		break;
	    }
	}

	// For compatibility, split the revision number
	sscanf( di.udi_release, "%x.%x", &_revMajor, &_revMinor );

}

bool USBDevice::parse(TQString fname)
{
	struct libusb20_backend *pbe;
	struct libusb20_device *pdev;

	_devices.clear();

	pbe = libusb20_be_alloc_default();
	if (pbe == NULL)
	    return (false);

	pdev = NULL;

	while ((pdev = libusb20_be_device_foreach(pbe, pdev))) {
	    USBDevice *device = new USBDevice();
	    device->collectData(pbe, pdev);
	}

	libusb20_be_free(pbe);

	return true;
}

bool USBDevice::parseSys(TQString)
{
	// sysfs is not available on FreeBSD
	return 0;
}
#endif
