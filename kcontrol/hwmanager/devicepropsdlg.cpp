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
		base = new DevicePropertiesDialogBase(plainPage());

		// Remove all non-applicable tabs
		if (m_device->type() != TDEGenericDeviceType::Disk) {
			base->tabBarWidget->removePage(base->tabDisk);
		}
		if (m_device->type() != TDEGenericDeviceType::CPU) {
			base->tabBarWidget->removePage(base->tabCPU);
		}

		TQGridLayout *mainGrid = new TQGridLayout(plainPage(), 1, 1, 0, spacingHint());
		mainGrid->setRowStretch(1, 1);
		mainGrid->setRowStretch(1, 1);
		mainGrid->addWidget(base, 0, 0);
	}

	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	connect(hwdevices, TQT_SIGNAL(hardwareRemoved(TDEGenericDevice*)), this, TQT_SLOT(processHardwareRemoved(TDEGenericDevice*)));
	connect(hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(processHardwareUpdated(TDEGenericDevice*)));

	populateDeviceInformation();
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
}

void DevicePropertiesDialog::processHardwareRemoved(TDEGenericDevice* dev) {
	if (dev == m_device) {
		close();
	}
}

void DevicePropertiesDialog::processHardwareUpdated(TDEGenericDevice* dev) {
	if (dev == m_device) {
		populateDeviceInformation();
	}
}

void DevicePropertiesDialog::populateDeviceInformation() {
	if (m_device) {
		base->labelDeviceType->setText(m_device->friendlyDeviceType());
		base->iconDeviceType->setPixmap(m_device->icon(KIcon::SizeSmall));
		base->labelDeviceName->setText(m_device->friendlyName());
		base->labelDeviceNode->setText((m_device->deviceNode().isNull())?i18n("<none>"):m_device->deviceNode());
		base->labelSystemPath->setText(m_device->systemPath());
		base->labelSubsytemType->setText(m_device->subsystem());
		base->labelDeviceDriver->setText((m_device->deviceDriver().isNull())?i18n("<none>"):m_device->deviceDriver());
		base->labelDeviceClass->setText((m_device->PCIClass().isNull())?i18n("<n/a>"):m_device->PCIClass());
		if (m_device->subsystem() == "pci") {
			base->labelBusID->setText(m_device->busID());
			base->labelBusID->show();
			base->stocklabelBusID->show();
		}
		else {
			base->labelBusID->hide();
			base->stocklabelBusID->hide();
		}

		if (m_device->type() == TDEGenericDeviceType::Disk) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(m_device);
			base->labelDiskMountpoint->setText(sdevice->mountPath());

			// Show status
			TQString status_text = "<qt>";
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Mountable)) {
				status_text += "Mountable<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Removable)) {
				status_text += "Removable<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Inserted)) {
				status_text += "Inserted<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::UsedByDevice)) {
				status_text += "In use<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::UsesDevice)) {
				status_text += "Uses other device<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem)) {
				status_text += "Contains a filesystem<br>";
			}
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)) {
				status_text += "Hotpluggable<br>";
			}
			if (status_text == "<qt>") {
				status_text += "<i>Unavailable</i>";
			}
			status_text += "</qt>";
			base->labelDiskStatus->setText(status_text);

			// TODO
			// Add mount/unmount buttons
		}

		if (m_device->type() == TDEGenericDeviceType::CPU) {
			TDECPUDevice* cdevice = static_cast<TDECPUDevice*>(m_device);

			// Show information
			base->labelCPUVendor->setText(cdevice->vendorEncoded());
			base->labelCPUFrequency->setText((cdevice->frequency()<0)?i18n("<unsupported>"):TQString("%1 MHz").arg(cdevice->frequency()));
			base->labelMinCPUFrequency->setText((cdevice->minFrequency()<0)?i18n("<unknown>"):TQString("%1 MHz").arg(cdevice->minFrequency()));
			base->labelMaxCPUFrequency->setText((cdevice->maxFrequency()<0)?i18n("<unsupported>"):TQString("%1 MHz").arg(cdevice->maxFrequency()));
			base->labelScalingDriver->setText((cdevice->scalingDriver().isNull())?i18n("<none>"):cdevice->scalingDriver());
			TQStringList scalingfreqs = cdevice->availableFrequencies();
			if (scalingfreqs.count() > 0) {
				TQString scalingfreqsstring = "<qt>";
				for ( TQStringList::Iterator it = scalingfreqs.begin(); it != scalingfreqs.end(); ++it ) {
					TQString freq = (*it);
					scalingfreqsstring.append(TQString("%1 MHz<br>").arg(freq.toDouble()/1000));
				}
				scalingfreqsstring.append("</qt>");
				base->labelScalingFrequencies->setText(scalingfreqsstring);
			}
			else {
				base->labelScalingFrequencies->setText(i18n("<none>"));
			}
			TQStringList dependentcpus = cdevice->dependentProcessors();
			if (dependentcpus.count() > 0) {
				TQString dependentcpusstring = "<qt>";
				for ( TQStringList::Iterator it = dependentcpus.begin(); it != dependentcpus.end(); ++it ) {
					TQString proc = (*it);
					dependentcpusstring.append(TQString("CPU %1<br>").arg(proc));
				}
				dependentcpusstring.append("</qt>");
				base->labelDependentCPUs->setText(dependentcpusstring);
			}
			else {
				base->labelDependentCPUs->setText(i18n("<none>"));
			}

		}
	}
}

void DevicePropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "devicepropsdlg.moc"
