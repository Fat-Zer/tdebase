/* This file is part of the KDE Project
   Copyright (c) 2004 Kévin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "medium.h"

#include <tdeconfig.h>
#include <klocale.h>

const TQString Medium::SEPARATOR = "---";

Medium::Medium(const TQString id, TQString uuid, const TQString name)
{
	m_properties+= id; /* ID */
	m_properties+= uuid; /* UUID */
	m_properties+= name; /* NAME */
	m_properties+= name; /* LABEL */
	m_properties+= TQString::null; /* USER_LABEL */

	m_properties+= "false"; /* MOUNTABLE */
	m_properties+= TQString::null; /* DEVICE_NODE */
	m_properties+= TQString::null; /* MOUNT_POINT */
	m_properties+= TQString::null; /* FS_TYPE */
	m_properties+= "false"; /* MOUNTED */
	m_properties+= TQString::null; /* BASE_URL */
	m_properties+= TQString::null; /* MIME_TYPE */
	m_properties+= TQString::null; /* ICON_NAME */
	m_properties+= "false"; /* ENCRYPTED */
	m_properties+= TQString::null; /* CLEAR_DEVICE_UDI */
	m_properties+= "false"; /* HIDDEN */

	loadUserLabel();

	m_halmounted = false;
}

Medium::Medium()
{
	m_properties+= TQString::null; /* ID */
	m_properties+= TQString::null; /* UUID */
	m_properties+= TQString::null; /* NAME */
	m_properties+= TQString::null; /* LABEL */
	m_properties+= TQString::null; /* USER_LABEL */

	m_properties+= TQString::null; /* MOUNTABLE */
	m_properties+= TQString::null; /* DEVICE_NODE */
	m_properties+= TQString::null; /* MOUNT_POINT */
	m_properties+= TQString::null; /* FS_TYPE */
	m_properties+= TQString::null; /* MOUNTED */
	m_properties+= TQString::null; /* BASE_URL */
	m_properties+= TQString::null; /* MIME_TYPE */
	m_properties+= TQString::null; /* ICON_NAME */
	m_properties+= TQString::null; /* ENCRYPTED */
	m_properties+= TQString::null; /* CLEAR_DEVICE_UDI */
	m_properties+= "false";        /* HIDDEN */
	
	m_halmounted = false;
}

const Medium Medium::create(const TQStringList &properties)
{
	Medium m;

	if ( properties.size() >= PROPERTIES_COUNT )
	{
		m.m_properties[ID] = properties[ID];
		m.m_properties[UUID] = properties[UUID];
		m.m_properties[NAME] = properties[NAME];
		m.m_properties[LABEL] = properties[LABEL];
		m.m_properties[USER_LABEL] = properties[USER_LABEL];

		m.m_properties[MOUNTABLE] = properties[MOUNTABLE];
		m.m_properties[DEVICE_NODE] = properties[DEVICE_NODE];
		m.m_properties[MOUNT_POINT] = properties[MOUNT_POINT];
		m.m_properties[FS_TYPE] = properties[FS_TYPE];
		m.m_properties[MOUNTED] = properties[MOUNTED];
		m.m_properties[BASE_URL] = properties[BASE_URL];
		m.m_properties[MIME_TYPE] = properties[MIME_TYPE];
		m.m_properties[ICON_NAME] = properties[ICON_NAME];
		m.m_properties[ENCRYPTED] = properties[ENCRYPTED];
		m.m_properties[CLEAR_DEVICE_UDI] = properties[CLEAR_DEVICE_UDI];
		m.m_properties[HIDDEN] = properties[HIDDEN];
	}

	return m;
}

Medium::MList Medium::createList(const TQStringList &properties)
{
	MList l;

	if ( properties.size() % (PROPERTIES_COUNT+1) == 0)
	{
		int media_count = properties.size()/(PROPERTIES_COUNT+1);

		TQStringList props = properties;

		for(int i=0; i<media_count; i++)
		{
			const Medium m = create(props);
			l.append(m);

			TQStringList::iterator first = props.begin();
			TQStringList::iterator last = props.find(SEPARATOR);
			++last;
			props.erase(first, last);
		}
	}

	return l;
}


void Medium::setName(const TQString &name)
{
	m_properties[NAME] = name;
}

void Medium::setLabel(const TQString &label)
{
	m_properties[LABEL] = label;
}

void Medium::setEncrypted(bool state)
{
	m_properties[ENCRYPTED] = ( state ? "true" : "false" );
}

void Medium::setHidden(bool state)
{
	m_properties[HIDDEN] = ( state ? "true" : "false" );
}

void Medium::setUserLabel(const TQString &label)
{
	TDEConfig cfg("mediamanagerrc");
	cfg.setGroup("UserLabels");

	TQString entry_name = m_properties[UUID];

	if ( label.isNull() )
	{
		cfg.deleteEntry(entry_name);
	}
	else
	{
		cfg.writeEntry(entry_name, label);
	}

	m_properties[USER_LABEL] = label;
}

void Medium::loadUserLabel()
{
	TDEConfig cfg("mediamanagerrc");
	cfg.setGroup("UserLabels");

	TQString entry_name = m_properties[UUID];

	if ( cfg.hasKey(entry_name) )
	{
		m_properties[USER_LABEL] = cfg.readEntry(entry_name);
	}
	else
	{
		m_properties[USER_LABEL] = TQString::null;
	}
}


bool Medium::mountableState(bool mounted)
{
	if ( m_properties[DEVICE_NODE].isEmpty()
	  || ( mounted && m_properties[MOUNT_POINT].isEmpty() ) )
	{
		return false;
	}

	m_properties[MOUNTABLE] = "true";
	m_properties[MOUNTED] = ( mounted ? "true" : "false" );

	return true;
}

void Medium::mountableState(const TQString &deviceNode,
                            const TQString &mountPoint,
                            const TQString &fsType, bool mounted)
{
	m_properties[MOUNTABLE] = "true";
	m_properties[DEVICE_NODE] = deviceNode;
	m_properties[MOUNT_POINT] = mountPoint;
	m_properties[FS_TYPE] = fsType;
	m_properties[MOUNTED] = ( mounted ? "true" : "false" );
}

void Medium::mountableState(const TQString &deviceNode,
	                    const TQString &clearDeviceUdi,
                            const TQString &mountPoint,
                            const TQString &fsType, bool mounted)
{
	m_properties[MOUNTABLE] = "true";
	m_properties[DEVICE_NODE] = deviceNode;
	m_properties[CLEAR_DEVICE_UDI] = clearDeviceUdi;
	m_properties[MOUNT_POINT] = mountPoint;
	m_properties[FS_TYPE] = fsType;
	m_properties[MOUNTED] = ( mounted ? "true" : "false" );
}

void Medium::unmountableState(const TQString &baseURL)
{
	m_properties[MOUNTABLE] = "false";
	m_properties[BASE_URL] = baseURL;
}

void Medium::setMimeType(const TQString &mimeType)
{
	m_properties[MIME_TYPE] = mimeType;
}

void Medium::setIconName(const TQString &iconName)
{
	m_properties[ICON_NAME] = iconName;
}

bool Medium::needMounting() const
{
	return isMountable() && !isMounted();
}

bool Medium::needDecryption() const
{
	return isEncrypted() && clearDeviceUdi().isEmpty();
}

KURL Medium::prettyBaseURL() const
{
        if ( !baseURL().isEmpty() )
            return baseURL();

		return KURL( mountPoint() );
}

TQString Medium::prettyLabel() const
{
	if ( !userLabel().isEmpty() )
	{
		return userLabel();
	}
	else
	{
		return label();
	}
}

