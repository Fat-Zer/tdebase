/* This file is part of TDE
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __devicepropsdlg_h__
#define __devicepropsdlg_h__

#include <kdialogbase.h>

#include <tdehardwaredevices.h>

#include "devicepropsdlgbase.h"

/**
 *
 * Dialog to view and edit hardware device properties
 *
 * @version 0.1
 * @author Timothy Pearson <kb9vqf@pearsoncomputing.net>
 */

class TDEUI_EXPORT DevicePropertiesDialog : public KDialogBase
{
	Q_OBJECT
public:
	/**
	* Create a dialog that allows a user to view and edit hardware device properties
	* @param parent     Parent widget for the line edit dialog
	*/
	DevicePropertiesDialog(TDEGenericDevice* device, TQWidget *parent);
	virtual ~DevicePropertiesDialog();

protected:
	virtual void virtual_hook( int id, void* data );

private:
	TDEGenericDevice* m_device;
	DevicePropertiesDialogBase* base;

	class DevicePropertiesDialogPrivate;
	DevicePropertiesDialogPrivate* d;
};

#endif
