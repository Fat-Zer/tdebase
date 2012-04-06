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
#include <config.h>

#include <tqvalidator.h>
#include <tqpushbutton.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqtabwidget.h>
#include <tqlayout.h>
#undef Unsorted // Required for --enable-final (tqdir.h)
#include <tqfiledialog.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kstdguiitem.h>

#include "devicepropsdlg.h"

DevicePropertiesDialog::DevicePropertiesDialog(TDEGenericDevice* device, TQWidget *parent)
	: KDialogBase(Plain, TQString::null, Ok|Cancel, Ok, parent, 0L, true, true)
{
	m_device = device;
	enableButtonOK( false );

	if (m_device) {
		TQGridLayout *mainGrid = new TQGridLayout(plainPage(), 1, 1, 0, spacingHint());
		mainGrid->setRowStretch(1, 1);
		mainGrid->setRowStretch(1, 1);
	
		TQTabWidget *mainTabs = new TQTabWidget(plainPage());
	
		TQWidget *genericPropertiesTab = new TQWidget(this);
	
		TQGridLayout *generalTabLayout = new TQGridLayout(genericPropertiesTab, 4, 2, 0, spacingHint() );

		int row = 0;
		TQLabel *label;
		label = new TQLabel(i18n("Device Name:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel(m_device->friendlyName(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		label = new TQLabel(i18n("Device Node:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel(m_device->deviceNode(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		label = new TQLabel(i18n("System Path:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel(m_device->systemPath(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		label = new TQLabel(i18n("Subsystem Type:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel(m_device->subsystem(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		label = new TQLabel(i18n("Device Driver:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel((m_device->deviceDriver().isNull())?i18n("<none>"):m_device->deviceDriver(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		label = new TQLabel(i18n("Device Class:"), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 0);
		label = new TQLabel((m_device->PCIClass().isNull())?i18n("<n/a>"):m_device->PCIClass(), genericPropertiesTab);
		generalTabLayout->addWidget(label, row, 1);
		row++;
		if (m_device->subsystem() == "pci") {
			TQString busid = m_device->systemPath();
			busid = busid.remove(0, busid.findRev("/")+1);
			busid = busid.remove(0, busid.find(":")+1);
			label = new TQLabel(i18n("Bus ID:"), genericPropertiesTab);
			generalTabLayout->addWidget(label, row, 0);
			label = new TQLabel(busid, genericPropertiesTab);
			generalTabLayout->addWidget(label, row, 1);
			row++;
		}
	
		mainTabs->addTab(genericPropertiesTab, i18n("&General"));
	
		mainGrid->addWidget(mainTabs, 0, 0);
	}
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
}

void DevicePropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "devicepropsdlg.moc"
