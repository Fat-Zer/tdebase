/*
 * Copyright 2015 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 * 
 * This file is part of hwdevicetray, the TDE Hardware Device Monitor System Tray Application
 * 
 * hwdevicetray is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * hwdevicetray is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with cryptocardwatcher. If not, see http://www.gnu.org/licenses/.
 */

#include <tqtimer.h>
#include <tqimage.h>
#include <tqtooltip.h>
#include <tqfileinfo.h>

#include <krun.h>
#include <tdeaction.h>
#include <tdeapplication.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <tdeglobal.h>
#include <tdemessagebox.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>

#include <dcopclient.h>

#include <cstdlib>
#include <unistd.h>

#include "hwdevicetray_configdialog.h"

#include "hwdevicetray.h"

HwDeviceSystemTray::HwDeviceSystemTray(TQWidget* parent, const char *name)
	: KSystemTray(parent, name) {

	// Create notifier
	m_hardwareNotifierContainer = new TDEPassivePopupStackContainer();
	connect(m_hardwareNotifierContainer, TQT_SIGNAL(popupClicked(KPassivePopup*, TQPoint, TQString)), this, TQT_SLOT(devicePopupClicked(KPassivePopup*, TQPoint, TQString)));

	// Create help submenu
	m_help = new KHelpMenu(this, TDEGlobal::instance()->aboutData(), false, actionCollection());
	TDEPopupMenu *help = m_help->menu();
	help->connectItem(KHelpMenu::menuHelpContents, this, TQT_SLOT(slotHelpContents()));

	setPixmap(KSystemTray::loadIcon("kcmdevices"));
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	connect(this, TQT_SIGNAL(quitSelected()), this, TQT_SLOT(_quit()));
	TQToolTip::add(this, i18n("Hardware device monitor"));
	m_parent = parent;

	globalKeys = new TDEGlobalAccel(TQT_TQOBJECT(this));
	TDEGlobalAccel* keys = globalKeys;
	#include "hwdevicetray_bindings.cpp"
	// the keys need to be read from tdeglobals, not kickerrc
	globalKeys->readSettings();
	globalKeys->setEnabled(true);
	globalKeys->updateConnections();

	connect(kapp, TQT_SIGNAL(settingsChanged(int)), TQT_SLOT(slotSettingsChanged(int)));

	new TDEActionMenu(i18n("Open Device"), SmallIcon("connect_creating",  TQIconSet::Automatic), actionCollection(), "mount_menu");
	new TDEActionMenu(i18n("Eject Device"), SmallIcon("connect_no",  TQIconSet::Automatic), actionCollection(), "unmount_menu");

#ifdef __TDE_HAVE_TDEHWLIB
	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	connect(hwdevices, TQT_SIGNAL(hardwareAdded(TDEGenericDevice*)), this, TQT_SLOT(deviceAdded(TDEGenericDevice*)));
	connect(hwdevices, TQT_SIGNAL(hardwareRemoved(TDEGenericDevice*)), this, TQT_SLOT(deviceRemoved(TDEGenericDevice*)));
	connect(hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(deviceChanged(TDEGenericDevice*)));
#endif
}

HwDeviceSystemTray::~HwDeviceSystemTray() {
	delete m_hardwareNotifierContainer;
}

/*!
 * \b TQT_SLOT which called if hwdevicetray is exited by the user. In this case the user
 * is asked through a yes/no box if "HwDeviceTray should start automatically on log in" and the
 * result is written to the KDE configfile.
 */
void HwDeviceSystemTray::_quit () {
	r_config = new KSimpleConfig("hwdevicetrayrc");

	TQString tmp1 = i18n ("Start hardware device tray automatically when you log in?");
	int tmp2 = KMessageBox::questionYesNo (0, tmp1, i18n("Question"), i18n("Start Automatically"), i18n("Do Not Start"));
	r_config->setGroup("General");
	r_config->writeEntry ("Autostart", tmp2 == KMessageBox::Yes);
	r_config->sync ();

	exit(0);
}

void HwDeviceSystemTray::resizeTrayIcon () {
	// Honor Free Desktop specifications that allow for arbitrary system tray icon sizes
	TQPixmap origpixmap;
	TQPixmap scaledpixmap;
	TQImage newIcon;
	origpixmap = KSystemTray::loadSizedIcon("kcmdevices", width());
	newIcon = origpixmap;
	newIcon = newIcon.smoothScale(width(), height());
	scaledpixmap = newIcon;
	setPixmap(scaledpixmap);
}

void HwDeviceSystemTray::resizeEvent (TQResizeEvent *) {
	// Honor Free Desktop specifications that allow for arbitrary system tray icon sizes
	resizeTrayIcon();
}

void HwDeviceSystemTray::showEvent (TQShowEvent *) {
	// Honor Free Desktop specifications that allow for arbitrary system tray icon sizes
	resizeTrayIcon();
}

void HwDeviceSystemTray::mousePressEvent(TQMouseEvent* e) {
	// Popup the context menu with left-click
	if (e->button() == Qt::LeftButton) {
		contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
		e->accept();
		return;
	}

	KSystemTray::mousePressEvent(e);
}

bool HwDeviceSystemTray::isMonitoredDevice(TDEStorageDevice* sdevice) {
	// Type selection logic largely duplicated from the media manager tdeioslave
	if (((sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)
		|| sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem)
		|| sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio)
		|| sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
		&& !sdevice->checkDiskStatus(TDEDiskDeviceStatus::UsedByDevice)
		&& !sdevice->checkDiskStatus(TDEDiskDeviceStatus::Hidden)
		&& (sdevice->isDiskOfType(TDEDiskDeviceType::HDD)
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDR))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMO))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMRRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDMRRWW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDR))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRDL))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRWDL))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSR))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRDL))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDPLUSRWDL))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDR))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDR))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::HDDVDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::Floppy))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::Zip))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::Jaz))))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::Camera))) {
		return true;
	}
	else {
		return false;
	}
}

void HwDeviceSystemTray::contextMenuAboutToShow(TDEPopupMenu* menu) {
	menu->clear();
	menu->setCheckable(true);

	populateMenu(menu);

	menu->insertTitle(SmallIcon("configure"), i18n("Global Configuration"));

	TDEAction *actHardwareConfig = new TDEAction(i18n("Configure Hardware..."), SmallIconSet("kcmpci"), TDEShortcut(), TQT_TQOBJECT(this), TQT_SLOT(slotHardwareConfig()), actionCollection());
	actHardwareConfig->plug(menu);

	TDEAction *actShortcutKeys = new TDEAction(i18n("Configure Shortcut Keys..."), SmallIconSet("configure"), TDEShortcut(), TQT_TQOBJECT(this), TQT_SLOT(slotEditShortcutKeys()), actionCollection());
	actShortcutKeys->plug(menu);

	menu->insertItem(SmallIcon("help"), KStdGuiItem::help().text(), m_help->menu());
	TDEAction *quitAction = actionCollection()->action(KStdAction::name(KStdAction::Quit));
	quitAction->plug(menu);

	m_menu = menu;
}

void HwDeviceSystemTray::configChanged() {
	//
}

void HwDeviceSystemTray::populateMenu(TDEPopupMenu* menu) {
	int lastMountIndex;
	int lastUnmountIndex;
	TDEGenericDevice *hwdevice;

	TDEActionMenu* mountDiskActionMenu = static_cast<TDEActionMenu*>(actionCollection()->action("mount_menu"));
	mountDiskActionMenu->popupMenu()->clear();
	m_mountMenuIndexMap.clear();
	TDEActionMenu* unmountDiskActionMenu = static_cast<TDEActionMenu*>(actionCollection()->action("unmount_menu"));
	unmountDiskActionMenu->popupMenu()->clear();
	m_mountMenuIndexMap.clear();

	menu->insertTitle(SmallIcon("drive-harddisk"), i18n("Storage Devices"));

	// Find all storage devices and add them to the popup menu
	lastMountIndex = 1;
	lastUnmountIndex = 1;
	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	TDEGenericHardwareList diskDeviceList = hwdevices->listByDeviceClass(TDEGenericDeviceType::Disk);
	for (hwdevice = diskDeviceList.first(); hwdevice; hwdevice = diskDeviceList.next()) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
		if (isMonitoredDevice(sdevice)) {
			lastMountIndex = mountDiskActionMenu->popupMenu()->insertItem(hwdevice->icon(TDEIcon::SizeSmall), i18n("%1 (%2)").arg(sdevice->friendlyName(), sdevice->deviceNode()));
			mountDiskActionMenu->popupMenu()->connectItem(lastMountIndex, this, TQT_SLOT(slotMountDevice(int)));
			m_mountMenuIndexMap[lastMountIndex] = sdevice->diskUUID();
			if (m_mountMenuIndexMap[lastMountIndex] == "") {
				m_mountMenuIndexMap[lastMountIndex] = sdevice->systemPath();
			}
			if (sdevice->mountPath() != TQString::null) {
				lastUnmountIndex = unmountDiskActionMenu->popupMenu()->insertItem(hwdevice->icon(TDEIcon::SizeSmall), i18n("%1 (%2)").arg(sdevice->friendlyName(), sdevice->deviceNode()));
				unmountDiskActionMenu->popupMenu()->connectItem(lastUnmountIndex, this, TQT_SLOT(slotUnmountDevice(int)));
				m_unmountMenuIndexMap[lastUnmountIndex] = sdevice->diskUUID();
				if (m_unmountMenuIndexMap[lastMountIndex] == "") {
					m_unmountMenuIndexMap[lastMountIndex] = sdevice->systemPath();
				}
			}
		}
	}

	if (lastMountIndex == 0) {
		mountDiskActionMenu->setEnabled(false);
	}
	else {
		mountDiskActionMenu->setEnabled(true);
	}
	if (lastUnmountIndex == 0) {
		unmountDiskActionMenu->setEnabled(false);
	}
	else {
		unmountDiskActionMenu->setEnabled(true);
	}

	mountDiskActionMenu->plug(menu);
	unmountDiskActionMenu->plug(menu);
}

void HwDeviceSystemTray::slotMountDevice(int parameter)
{
	TDEGenericDevice *hwdevice;
	TQString uuid = m_mountMenuIndexMap[parameter];
	if (uuid != "") {
		TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
		TDEGenericHardwareList diskDeviceList = hwdevices->listByDeviceClass(TDEGenericDeviceType::Disk);
		for (hwdevice = diskDeviceList.first(); hwdevice; hwdevice = diskDeviceList.next()) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			if ((sdevice->diskUUID() == uuid) || (sdevice->systemPath() == uuid)) {
				if (sdevice->isDiskOfType(TDEDiskDeviceType::Camera)) {
					new KRun(TQString("media:/%1").arg(sdevice->friendlyName()));
				}
				else {
					new KRun(TQString("system:/media/%1").arg(TQFileInfo(sdevice->deviceNode()).baseName(true)));
				}
				return;
			}
		}
	}
}

void HwDeviceSystemTray::slotUnmountDevice(int parameter)
{
	TDEGenericDevice *hwdevice;
	TQString uuid = m_unmountMenuIndexMap[parameter];
	if (uuid != "") {
		TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
		TDEGenericHardwareList diskDeviceList = hwdevices->listByDeviceClass(TDEGenericDeviceType::Disk);
		for (hwdevice = diskDeviceList.first(); hwdevice; hwdevice = diskDeviceList.next()) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			if ((sdevice->diskUUID() == uuid) || (sdevice->systemPath() == uuid)) {
				if (sdevice->mountPath() != TQString::null) {
					int retcode;
					TQString errstr;
					if (!sdevice->unmountDevice(&errstr, &retcode)) {
						KMessageBox::error(0, i18n("<qt><b>Unable to eject device</b><p>Detailed error information:<br>%1 (code %2)</qt>").arg(errstr).arg(retcode), i18n("Eject Failed"));
					}
					return;
				}
			}
		}
	}
}

void HwDeviceSystemTray::slotHardwareConfig() {
	KCMultiDialog *kcm = new KCMultiDialog(KDialogBase::Plain, i18n("Configure"), this);

	kcm->addModule("hwmanager");
	kcm->setPlainCaption(i18n("Configure Hardware"));
	kcm->exec();
}

void HwDeviceSystemTray::slotSettingsChanged(int category) {
	if (category == (int) TDEApplication::SETTINGS_SHORTCUTS) {
		globalKeys->readSettings();
		globalKeys->updateConnections();
	}
}

void HwDeviceSystemTray::slotEditShortcutKeys() {
	ConfigDialog *dlg = new ConfigDialog(globalKeys, true);

	if (dlg->exec() == TQDialog::Accepted) {
		dlg->commitShortcuts();
		globalKeys->writeSettings(0, true);
		globalKeys->updateConnections();
	}

	delete dlg;
}

void HwDeviceSystemTray::deviceAdded(TDEGenericDevice* device) {
#ifdef __TDE_HAVE_TDEHWLIB
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		if (isMonitoredDevice(sdevice)) {
			TQString uuid = sdevice->diskUUID();
			if (uuid == "") {
				uuid = sdevice->systemPath();
			}
			m_hardwareNotifierContainer->displayMessage(
				i18n("A disk device has been added!"),
				i18n("%1 (%2)").arg(sdevice->friendlyName(), sdevice->deviceNode()), SmallIcon("drive-harddisk"),
				0, 0, "ADD: " + uuid);
		}
	}
#endif
}

void HwDeviceSystemTray::deviceRemoved(TDEGenericDevice* device) {
#ifdef __TDE_HAVE_TDEHWLIB
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		if (isMonitoredDevice(sdevice)) {
			TQString uuid = sdevice->diskUUID();
			if (uuid == "") {
				uuid = sdevice->systemPath();
			}
			m_hardwareNotifierContainer->displayMessage(
				i18n("A disk device has been removed!"),
				i18n("%1 (%2)").arg(sdevice->friendlyName(), sdevice->deviceNode()), SmallIcon("drive-harddisk"),
				0, 0, "REMOVE: " + uuid);
		}
	}
#endif
}

void HwDeviceSystemTray::deviceChanged(TDEGenericDevice* device) {
#ifdef __TDE_HAVE_TDEHWLIB
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		if (isMonitoredDevice(sdevice)) {
			TQString uuid = sdevice->diskUUID();
			if (uuid == "") {
				uuid = sdevice->systemPath();
			}
			m_hardwareNotifierContainer->displayMessage(
				i18n("A disk device has been changed!"),
				i18n("%1 (%2)").arg(sdevice->friendlyName(), sdevice->deviceNode()), SmallIcon("drive-harddisk"),
				0, 0, "CHANGE: " + uuid);
		}
	}
#endif
}

void HwDeviceSystemTray::devicePopupClicked(KPassivePopup* popup, TQPoint point, TQString uuid) {
	TDEGenericDevice *hwdevice;
	if (uuid.startsWith("ADD: ")) {
		uuid = uuid.right(uuid.length() - strlen("ADD: "));
		if (uuid != "") {
			TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
			TDEGenericHardwareList diskDeviceList = hwdevices->listByDeviceClass(TDEGenericDeviceType::Disk);
			for (hwdevice = diskDeviceList.first(); hwdevice; hwdevice = diskDeviceList.next()) {
				TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
				if ((sdevice->diskUUID() == uuid) || (sdevice->systemPath() == uuid)) {
					// Pop up full media notification dialog
					DCOPClient* dcopClient = TDEApplication::dcopClient();
					TQByteArray data;
					TQDataStream arg(data, IO_WriteOnly);
					bool allowNotification = true;
					if (sdevice->isDiskOfType(TDEDiskDeviceType::Camera)) {
						arg << sdevice->friendlyName();
					}
					else {
						arg << TQFileInfo(sdevice->deviceNode()).baseName(true);
					}
					arg << allowNotification;
					dcopClient->send("kded", "medianotifier", "onMediumChange(TQString, bool)", data);
					return;
				}
			}
		}
	}
}

void HwDeviceSystemTray::slotHelpContents() {
	kapp->invokeHelp(TQString::null, "hwdevicetray");
}

#include "hwdevicetray.moc"