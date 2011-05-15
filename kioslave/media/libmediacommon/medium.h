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

#ifndef _MEDIUM_H_
#define _MEDIUM_H_

#include <tqstring.h>
#include <tqstringlist.h>
#include <kurl.h>
#include <tqmap.h>

class Medium
{
public:
	typedef TQValueList<const Medium> MList;

	static const uint ID = 0;
	static const uint NAME = 1;
	static const uint LABEL = 2;
	static const uint USER_LABEL = 3;
	static const uint MOUNTABLE = 4;
	static const uint DEVICE_NODE = 5;
	static const uint MOUNT_POINT = 6;
	static const uint FS_TYPE = 7;
	static const uint MOUNTED = 8;
	static const uint BASE_URL = 9;
	static const uint MIME_TYPE = 10;
	static const uint ICON_NAME = 11;
	static const uint ENCRYPTED = 12;
	static const uint CLEAR_DEVICE_UDI = 13;
	static const uint PROPERTIES_COUNT = 14;
	static const TQString SEPARATOR;

	Medium(const TQString &id, const TQString &name);
	static const Medium create(const TQStringList &properties);
	static MList createList(const TQStringList &properties);

	const TQStringList &properties() const { return m_properties; }

	TQString id() const { return m_properties[ID]; }
	TQString name() const { return m_properties[NAME]; }
	TQString label() const { return m_properties[LABEL]; }
	TQString userLabel() const { return m_properties[USER_LABEL]; }
	bool isMountable() const { return m_properties[MOUNTABLE]=="true"; }
	TQString deviceNode() const { return m_properties[DEVICE_NODE]; }
	TQString mountPoint() const { return m_properties[MOUNT_POINT]; }
	TQString fsType() const { return m_properties[FS_TYPE]; }
	bool isMounted() const { return m_properties[MOUNTED]=="true"; }
	TQString baseURL() const { return m_properties[BASE_URL]; }
	TQString mimeType() const { return m_properties[MIME_TYPE]; }
	TQString iconName() const { return m_properties[ICON_NAME]; }
 	bool isEncrypted() const { return m_properties[ENCRYPTED]=="true"; };
 	TQString clearDeviceUdi() const { return m_properties[CLEAR_DEVICE_UDI]; };

	bool needMounting() const;
 	bool needDecryption() const;
	KURL prettyBaseURL() const;
	TQString prettyLabel() const;

	void setName(const TQString &name);
	void setLabel(const TQString &label);
	void setUserLabel(const TQString &label);
 	void setEncrypted(bool state);

	bool mountableState(bool mounted);
	void mountableState(const TQString &deviceNode,
	                    const TQString &mountPoint,
	                    const TQString &fsType, bool mounted);
 	void mountableState(const TQString &deviceNode,
 	                    const TQString &clearDeviceUdi,
 	                    const TQString &mountPoint,
 	                    const TQString &fsType, bool mounted);
	void unmountableState(const TQString &baseURL = TQString::null);

	void setMimeType(const TQString &mimeType);
	void setIconName(const TQString &iconName);
	void setHalMounted(bool flag) const { m_halmounted = flag; }
	bool halMounted() const { return m_halmounted; }

private:
	Medium();
	void loadUserLabel();

	TQStringList m_properties;
	mutable bool m_halmounted;
	
friend class TQValueListNode<const Medium>;
};

namespace MediaManagerUtils {
  static inline TQMap<TQString,TQString> splitOptions(const TQStringList & options) 
    {
      TQMap<TQString,TQString> valids;

      for (TQStringList::ConstIterator it = options.begin(); it != options.end(); ++it)
	{
	  TQString key = (*it).left((*it).tqfind('='));
	  TQString value = (*it).mid((*it).tqfind('=') + 1);
	  valids[key] = value;
	}
      return valids;
    }
}

#endif
