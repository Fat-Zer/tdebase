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
#include <tqgroupbox.h>
#include <tqlayout.h>
#undef Unsorted // Required for --enable-final (tqdir.h)
#include <tqfiledialog.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kstdguiitem.h>

#include "devicepropsdlg.h"

SensorDisplayWidget::SensorDisplayWidget(TQWidget *parent)
	: TQWidget(parent)
{
	m_nameLabel = new TQLabel(this);
	m_valueLabel = new TQLabel(this);

	TQGridLayout *mainGrid = new TQGridLayout(this, 1, 2, 0, 1);
	mainGrid->setRowStretch(1, 0);
	mainGrid->addWidget(m_nameLabel, 0, 0);
	mainGrid->addWidget(m_valueLabel, 0, 1);
}

SensorDisplayWidget::~SensorDisplayWidget()
{
}

void SensorDisplayWidget::setSensorName(TQString name) {
	m_nameLabel->setText(name);
}

void SensorDisplayWidget::setSensorValue(TQString value) {
	m_valueLabel->setText(value);
}

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
		if ((m_device->type() != TDEGenericDeviceType::OtherSensor) && (m_device->type() != TDEGenericDeviceType::ThermalSensor)) {
			base->tabBarWidget->removePage(base->tabSensor);
		}
		if (m_device->type() != TDEGenericDeviceType::Battery) {
			base->tabBarWidget->removePage(base->tabBattery);
		}
		if (m_device->type() != TDEGenericDeviceType::PowerSupply) {
			base->tabBarWidget->removePage(base->tabPowerSupply);
		}
		if (m_device->type() != TDEGenericDeviceType::Network) {
			base->tabBarWidget->removePage(base->tabNetwork);
		}

		if ((m_device->type() == TDEGenericDeviceType::OtherSensor) || (m_device->type() == TDEGenericDeviceType::ThermalSensor)) {
			base->groupSensors->setColumnLayout(0, TQt::Vertical );
			base->groupSensors->layout()->setSpacing( KDialog::spacingHint() );
			base->groupSensors->layout()->setMargin( KDialog::marginHint() );
			m_sensorDataGrid = new TQGridLayout( base->groupSensors->layout() );
			m_sensorDataGrid->setAlignment( TQt::AlignTop );
			m_sensorDataGridWidgets.setAutoDelete(true);
		}

		TQGridLayout *mainGrid = new TQGridLayout(plainPage(), 1, 1, 0, spacingHint());
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
		base->labelModalias->setText((m_device->moduleAlias().isNull())?i18n("<none>"):m_device->moduleAlias());

		// These might be redundant
		#if 0
		base->labelVendorName->setText((m_device->vendorName().isNull())?i18n("<unknown>"):m_device->vendorName());
		base->labelVendorModel->setText((m_device->vendorModel().isNull())?i18n("<unknown>"):m_device->vendorModel());
		#else
		base->labelVendorName->hide();
		base->stocklabelVendorName->hide();
		base->labelVendorModel->hide();
		base->stocklabelVendorModel->hide();
		#endif
		base->labelSerialNumber->setText((m_device->serialNumber().isNull())?i18n("<unknown>"):m_device->serialNumber());

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

		if ((m_device->type() == TDEGenericDeviceType::OtherSensor) || (m_device->type() == TDEGenericDeviceType::ThermalSensor)) {
			TDESensorDevice* sdevice = static_cast<TDESensorDevice*>(m_device);

			TDESensorClusterMap map = sdevice->values();
			TDESensorClusterMap::Iterator it;
			unsigned int i;

			if (m_sensorDataGridWidgets.count() != map.count()) {
				m_sensorDataGridWidgets.clear();
				for (i=0;i<map.count();i++) {
					SensorDisplayWidget* sensorWidget = new SensorDisplayWidget(base->groupSensors);
					m_sensorDataGrid->addWidget(sensorWidget, i, 0);
					m_sensorDataGridWidgets.append(sensorWidget);
				}
			}

			i=0;
			for ( it = map.begin(); it != map.end(); ++it ) {
				TQString sensorlabel = it.key();
				TQString sensordatastring;
				TDESensorCluster values = it.data();

				if (!values.label.isNull()) {
					sensorlabel = values.label;
				}
				if (sensorlabel.isNull()) {
					sensorlabel = i18n("<unnamed>");
				}

				if (values.minimum > 0) {
					sensordatastring += TQString("Minimum Value: %1, ").arg(values.minimum);
				}
				sensordatastring += TQString("Current Value: %1, ").arg(values.current);
				if (values.maximum > 0) {
					sensordatastring += TQString("Maximum Value: %1, ").arg(values.maximum);
				}
				if (values.warning > 0) {
					sensordatastring += TQString("Warning Value: %1, ").arg(values.warning);
				}
				if (values.critical > 0) {
					sensordatastring += TQString("Critical Value: %1, ").arg(values.critical);
				}

				if (sensordatastring.endsWith(", ")) {
					sensordatastring.truncate(sensordatastring.length()-2);
				}

				m_sensorDataGridWidgets.at(i)->setSensorName(sensorlabel);
				m_sensorDataGridWidgets.at(i)->setSensorValue(sensordatastring);

				i++;
			}
		}

		if (m_device->type() == TDEGenericDeviceType::Battery) {
			TDEBatteryDevice* bdevice = static_cast<TDEBatteryDevice*>(m_device);

			base->labelCurrentBatteryEnergy->setText((bdevice->energy()<0)?i18n("<unknown>"):TQString("%1 Wh").arg(bdevice->energy()));
			base->labelMaximumBatteryEnergy->setText((bdevice->maximumEnergy()<0)?i18n("<unknown>"):TQString("%1 Wh").arg(bdevice->maximumEnergy()));
			base->labelMaximumBatteryDesignEnergy->setText((bdevice->maximumDesignEnergy()<0)?i18n("<unknown>"):TQString("%1 Wh").arg(bdevice->maximumDesignEnergy()));
			base->labelMinimumBatteryVoltage->setText((bdevice->minimumVoltage()<0)?i18n("<unknown>"):TQString("%1 V").arg(bdevice->minimumVoltage()));
			base->labelCurrentBatteryVoltage->setText((bdevice->voltage()<0)?i18n("<unknown>"):TQString("%1 V").arg(bdevice->voltage()));
			base->labelCurrentBatteryDischargeRate->setText((bdevice->dischargeRate()<0)?i18n("<unknown>"):TQString("%1 Vh").arg(bdevice->dischargeRate()));
			base->labelCurrentBatteryStatus->setText((bdevice->status().isNull())?i18n("<unknown>"):bdevice->status());
			base->labelBatteryTechnology->setText((bdevice->technology().isNull())?i18n("<unknown>"):bdevice->technology());
			base->labelBatteryInstalled->setText((bdevice->installed()==0)?i18n("No"):i18n("Yes"));
			base->labelBatteryCharge->setText((bdevice->chargePercent()<0)?i18n("<unknown>"):TQString("%1 %").arg(bdevice->chargePercent()));
		}

		if (m_device->type() == TDEGenericDeviceType::PowerSupply) {
			TDEMainsPowerDevice* pdevice = static_cast<TDEMainsPowerDevice*>(m_device);

			base->labelPowerSupplyOnline->setText((pdevice->online()==0)?i18n("No"):i18n("Yes"));
		}

		if (m_device->type() == TDEGenericDeviceType::Network) {
			TDENetworkDevice* ndevice = static_cast<TDENetworkDevice*>(m_device);

			base->labelNetworkMAC->setText((ndevice->macAddress().isNull())?i18n("<unknown>"):ndevice->macAddress());
			base->labelNetworkState->setText((ndevice->state().isNull())?i18n("<unknown>"):ndevice->state());
			base->labelNetworkCarrierPresent->setText((ndevice->carrierPresent()==0)?i18n("No"):i18n("Yes"));
			base->labelNetworkDormant->setText((ndevice->dormant()==0)?i18n("No"):i18n("Yes"));

			base->labelNetworkIPV4Address->setText((ndevice->ipV4Address().isNull())?i18n("<none>"):ndevice->ipV4Address());
			base->labelNetworkIPV4Netmask->setText((ndevice->ipV4Netmask().isNull())?i18n("<none>"):ndevice->ipV4Netmask());
			base->labelNetworkIPV4Broadcast->setText((ndevice->ipV4Broadcast().isNull())?i18n("<none>"):ndevice->ipV4Broadcast());
			base->labelNetworkIPV4Destination->setText((ndevice->ipV4Destination().isNull())?i18n("<none>"):ndevice->ipV4Destination());

			base->labelNetworkIPV6Address->setText((ndevice->ipV6Address().isNull())?i18n("<none>"):ndevice->ipV6Address());
			base->labelNetworkIPV6Netmask->setText((ndevice->ipV6Netmask().isNull())?i18n("<none>"):ndevice->ipV6Netmask());
			base->labelNetworkIPV6Broadcast->setText((ndevice->ipV6Broadcast().isNull())?i18n("<none>"):ndevice->ipV6Broadcast());
			base->labelNetworkIPV6Destination->setText((ndevice->ipV6Destination().isNull())?i18n("<none>"):ndevice->ipV6Destination());

			base->labelNetworkRXBytes->setText((ndevice->rxBytes()<0)?i18n("<unknown>"):TDEHardwareDevices::bytesToFriendlySizeString(ndevice->rxBytes()));
			base->labelNetworkTXBytes->setText((ndevice->txBytes()<0)?i18n("<unknown>"):TDEHardwareDevices::bytesToFriendlySizeString(ndevice->txBytes()));
			base->labelNetworkRXPackets->setText((ndevice->rxPackets()<0)?i18n("<unknown>"):TQString("%1").arg(ndevice->rxPackets()));
			base->labelNetworkTXPackets->setText((ndevice->txPackets()<0)?i18n("<unknown>"):TQString("%1").arg(ndevice->txPackets()));
		}
	}
}

void DevicePropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "devicepropsdlg.moc"
