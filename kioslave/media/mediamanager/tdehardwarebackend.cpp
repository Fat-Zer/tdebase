/* This file is part of the TDE Project
   Copyright (c) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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

#include "tdehardwarebackend.h"

#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqeventloop.h>
#include <tqstylesheet.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kio/job.h>
#include <kprocess.h>
#include <kmimetype.h>
#include <kmountpoint.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kprotocolinfo.h>

#include "dialog.h"

#define MOUNT_SUFFIX (																\
	(medium->isMounted() ? TQString("_mounted") : TQString("_unmounted")) +									\
	(medium->isEncrypted() ? (sdevice->isDiskOfType(TDEDiskDeviceType::UnlockedCrypt) ? "_decrypted" : "_encrypted") : "" )			\
	)
#define MOUNT_ICON_SUFFIX (															\
	(medium->isMounted() ? TQString("_mount") : TQString("_unmount")) +									\
	(medium->isEncrypted() ? (sdevice->isDiskOfType(TDEDiskDeviceType::UnlockedCrypt) ? "_decrypt" : "_encrypt") : "" )			\
	)

#define CHECK_FOR_AND_EXECUTE_AUTOMOUNT(udi, medium, allowNotification) {									\
		TQMap<TQString,TQString> options = MediaManagerUtils::splitOptions(mountoptions(udi));						\
		kdDebug(1219) << "automount " << options["automount"] << endl;									\
		if (options["automount"] == "true" && allowNotification ) {									\
			TQString error = mount(medium);												\
			if (!error.isEmpty())													\
			kdDebug(1219) << "error " << error << endl;										\
		}																\
	}

/* Constructor */
TDEBackend::TDEBackend(MediaList &list, TQObject* parent)
    : TQObject()
    , BackendBase(list)
    , m_decryptDialog(0)
    , m_parent(parent)
{
	// Initialize the TDE device manager
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	// Connect device monitoring signals/slots
	connect(hwdevices, TQT_SIGNAL(hardwareAdded(TDEGenericDevice*)), this, TQT_SLOT(AddDeviceHandler(TDEGenericDevice*)));
	connect(hwdevices, TQT_SIGNAL(hardwareRemoved(TDEGenericDevice*)), this, TQT_SLOT(RemoveDeviceHandler(TDEGenericDevice*)));
	connect(hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(ModifyDeviceHandler(TDEGenericDevice*)));

	// List devices at startup
	ListDevices();
}

/* Destructor */
TDEBackend::~TDEBackend()
{
	// Remove all media from the media list
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
	TDEGenericHardwareList hwlist = hwdevices->listAllPhysicalDevices();
	TDEGenericDevice *hwdevice;
	for ( hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next() ) {
		if (hwdevice->type() == TDEGenericDeviceType::Disk) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			RemoveDevice(sdevice);
		}
	}
}

void TDEBackend::AddDeviceHandler(TDEGenericDevice *device) {
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		AddDevice(sdevice);
	}
}

void TDEBackend::RemoveDeviceHandler(TDEGenericDevice *device) {
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		RemoveDevice(sdevice);
	}
}

void TDEBackend::ModifyDeviceHandler(TDEGenericDevice *device) {
	if (device->type() == TDEGenericDeviceType::Disk) {
		TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(device);
		ModifyDevice(sdevice);
	}
}

// List devices (at startup)
bool TDEBackend::ListDevices()
{
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
	TDEGenericHardwareList hwlist = hwdevices->listAllPhysicalDevices();
	TDEGenericDevice *hwdevice;
	for ( hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next() ) {
		if (hwdevice->type() == TDEGenericDeviceType::Disk) {
			TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
			AddDevice(sdevice, false);
		}
	}

	return true;
}

// Create a media instance for a new storage device
void TDEBackend::AddDevice(TDEStorageDevice * sdevice, bool allowNotification)
{
	kdDebug(1219) << "TDEBackend::AddDevice for " << sdevice->uniqueID() << endl;

	// If the device is already listed, do not process it
	// This should not happen, but who knows...
	/** @todo : refresh properties instead ? */
	if (m_mediaList.findById(sdevice->uniqueID())) {
		kdDebug(1219) << "TDEBackend::AddDevice for " << sdevice->uniqueID() << " found existing entry in media list" << endl;
		return;
	}

	// Add volume block devices
	if (sdevice->isDiskOfType(TDEDiskDeviceType::HDD)) {
		/* We only list volumes that...
		*  - are encrypted with LUKS or
		*  - have a filesystem or
		*  - have an audio track
		*/
		if (!(sdevice->isDiskOfType(TDEDiskDeviceType::LUKS))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem))
			&& !(sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
			) {
			// 
		}
		else {
			// Create medium
			Medium* medium = new Medium(sdevice->uniqueID(), "");
			setVolumeProperties(medium);

			// Do not list the LUKS backend device if it has been unlocked elsewhere
			if (sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) {
				if (sdevice->holdingDevices().count() > 0) {
					medium->setHidden(true);
				}
				else {
					medium->setHidden(false);
				}
			}
			
			// Insert medium into list
			m_mediaList.addMedium(medium, allowNotification);

			kdDebug(1219) << "TDEBackend::AddDevice inserted hard medium for " << sdevice->uniqueID() << endl;

			// Automount if enabled
			CHECK_FOR_AND_EXECUTE_AUTOMOUNT(sdevice->uniqueID(), medium, allowNotification)
		}
	}

	// Add CD drives
	if ((sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
		) {

		// Create medium
		Medium* medium = new Medium(sdevice->uniqueID(), "");
		setVolumeProperties(medium);
		
		// Insert medium into list
		m_mediaList.addMedium(medium, allowNotification);

		kdDebug(1219) << "TDEBackend::AddDevice inserted optical medium for " << sdevice->uniqueID() << endl;

		// Automount if enabled
		CHECK_FOR_AND_EXECUTE_AUTOMOUNT(sdevice->uniqueID(), medium, allowNotification)
	}

	// Floppy & zip drives
	if ((sdevice->isDiskOfType(TDEDiskDeviceType::Floppy)) ||
		(sdevice->isDiskOfType(TDEDiskDeviceType::Zip)) ||
		(sdevice->isDiskOfType(TDEDiskDeviceType::Jaz))
		) {
		if ((sdevice->checkDiskStatus(TDEDiskDeviceStatus::Removable)) && (!(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Inserted)))) {
			allowNotification = false;
		}

		/* We only list volumes that...
		*  - are encrypted with LUKS or
		*  - have a filesystem or
		*  - are a floppy disk
		*/
		if (!(sdevice->isDiskOfType(TDEDiskDeviceType::LUKS))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem))
			&& !(sdevice->isDiskOfType(TDEDiskDeviceType::Floppy))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
			) {
			// 
		}
		else {
			// Create medium
			Medium* medium = new Medium(sdevice->uniqueID(), "");

			setFloppyProperties(medium);

			// Do not list the LUKS backend device if it has been unlocked elsewhere
			if (sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) {
				if (sdevice->holdingDevices().count() > 0) {
					medium->setHidden(true);
				}
				else {
					medium->setHidden(false);
				}
			}

			m_mediaList.addMedium(medium, allowNotification);

			kdDebug(1219) << "TDEBackend::AddDevice inserted floppy medium for " << sdevice->uniqueID() << endl;

			return;
		}
	}

	// PTP camera
	if (sdevice->isDiskOfType(TDEDiskDeviceType::Camera)) {
		// PTP cameras are handled by the "camera" kioslave
		if (KProtocolInfo::isKnownProtocol( TQString("camera") ) )
		{
			// Create medium
			Medium* medium = new Medium(sdevice->uniqueID(), "");
			setCameraProperties(medium);
			m_mediaList.addMedium(medium, allowNotification);

			kdDebug(1219) << "TDEBackend::AddDevice inserted camera medium for " << sdevice->uniqueID() << endl;

			return;
		}
	}
}

void TDEBackend::RemoveDevice(TDEStorageDevice * sdevice)
{
	kdDebug(1219) << "TDEBackend::RemoveDevice for " << sdevice->uniqueID() << endl;

	if (!m_mediaList.findById(sdevice->uniqueID())) {
		kdDebug(1219) << "TDEBackend::RemoveDevice for " << sdevice->uniqueID() << " existing entry in media list was not found" << endl;
		return;
	}

	m_mediaList.removeMedium(sdevice->uniqueID(), true);
}

void TDEBackend::ModifyDevice(TDEStorageDevice * sdevice)
{
	kdDebug(1219) << "TDEBackend::ModifyDevice for " << sdevice->uniqueID() << endl;

	bool allowNotification = false;
	ResetProperties(sdevice, allowNotification);
}

void TDEBackend::ResetProperties(TDEStorageDevice * sdevice, bool allowNotification, bool overrideIgnoreList)
{
	kdDebug(1219) << "TDEBackend::ResetProperties for " << sdevice->uniqueID() << " allowNotification: " << allowNotification << " overrideIgnoreList: " << overrideIgnoreList << endl;

	if (!m_mediaList.findById(sdevice->uniqueID())) {
		// This device is not currently in the device list, so add it and exit
		kdDebug(1219) << "TDEBackend::ResetProperties for " << sdevice->uniqueID() << " existing entry in media list was not found" << endl;
		AddDevice(sdevice);
		return;
	}

	// If we should ignore device change events for this device, do so
	if (overrideIgnoreList == false) {
		if (m_ignoreDeviceChangeEvents.contains(sdevice->uniqueID())) {
			return;
		}
	}

	Medium* m = new Medium(sdevice->uniqueID(), "");

	// Keep these conditions in sync with ::AddDevice above, OR ELSE!!!
	// BEGIN

	if (!(sdevice->isDiskOfType(TDEDiskDeviceType::LUKS))
		&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem))
		&& !(sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
		) {
	}
	else {
		// Do not list the LUKS backend device if it has been unlocked elsewhere
		if (sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) {
			if (sdevice->holdingDevices().count() > 0) {
				m->setHidden(true);
			}
			else {
				m->setHidden(false);
			}
		}
		setVolumeProperties(m);
	}

	if ((sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
		) {
		setVolumeProperties(m);
	}

	// Floppy & zip drives
	if ((sdevice->isDiskOfType(TDEDiskDeviceType::Floppy)) ||
		(sdevice->isDiskOfType(TDEDiskDeviceType::Zip)) ||
		(sdevice->isDiskOfType(TDEDiskDeviceType::Jaz))
		) {

		if (!(sdevice->isDiskOfType(TDEDiskDeviceType::LUKS))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::ContainsFilesystem))
			&& !(sdevice->isDiskOfType(TDEDiskDeviceType::Floppy))
			&& !(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
			) {
			// 
		}
		else {
			// Do not list the LUKS backend device if it has been unlocked elsewhere
			if (sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) {
				if (sdevice->holdingDevices().count() > 0) {
					m->setHidden(true);
				}
				else {
					m->setHidden(false);
				}
			}

			setFloppyProperties(m);
		}
	}

	if (sdevice->isDiskOfType(TDEDiskDeviceType::Camera)) {
		setCameraProperties(m);
	}

	// END

	if ((sdevice->checkDiskStatus(TDEDiskDeviceStatus::Removable)) && (!(sdevice->checkDiskStatus(TDEDiskDeviceStatus::Inserted)))) {
		kdDebug(1219) << "TDEBackend::ResetProperties for " << sdevice->uniqueID() << " device was removed from system" << endl;
		RemoveDevice(sdevice);
		return;
	}

	m_mediaList.changeMediumState(*m, allowNotification);

	delete m;
}

void TDEBackend::setVolumeProperties(Medium* medium)
{
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return;
	}

	medium->setName(generateName(sdevice->deviceNode()));
	if ((sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) || (sdevice->isDiskOfType(TDEDiskDeviceType::UnlockedCrypt))) {
		medium->setEncrypted(true);
	}
	else {
		medium->setEncrypted(false);
	}

	// USAGE: mountableState(Device node, Mount point, Filesystem type, Mounted ?)
	medium->mountableState(sdevice->deviceNode(), sdevice->mountPath(), sdevice->fileSystemName(), !sdevice->mountPath().isNull());

	TQString diskLabel = sdevice->diskLabel();
	if (diskLabel.isNull()) {
		diskLabel = i18n("%1 Removable Device").arg(sdevice->deviceFriendlySize());
	}
	medium->setLabel(diskLabel);

	TQString mimeType;

	if ((sdevice->isDiskOfType(TDEDiskDeviceType::CDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
		) {
		// This device is a CD drive of some sort

		// Default
		mimeType = "media/cdrom" + MOUNT_SUFFIX;

		if (sdevice->isDiskOfType(TDEDiskDeviceType::CDROM)) {
			mimeType = "media/cdrom" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankcd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
		if (sdevice->isDiskOfType(TDEDiskDeviceType::CDRW)) {
			mimeType = "media/cdwriter" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankcd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
	
		if (sdevice->isDiskOfType(TDEDiskDeviceType::DVDROM)) {
			mimeType = "media/dvd" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankdvd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
	
		if (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
			mimeType = "media/dvd" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankdvd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
	
		if (sdevice->isDiskOfType(TDEDiskDeviceType::DVDRW)) {
			mimeType = "media/dvd" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankdvd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}

		if (sdevice->isDiskOfType(TDEDiskDeviceType::BDROM)) {
			mimeType = "media/bluray" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankbd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
	
		if (sdevice->isDiskOfType(TDEDiskDeviceType::BDRW)) {
			mimeType = "media/bluray" + MOUNT_SUFFIX;
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank)) {
				mimeType = "media/blankbd" + MOUNT_SUFFIX;
				medium->unmountableState("");
			}
		}
	
		if (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio)) {
			mimeType = "media/audiocd" + MOUNT_SUFFIX;
		}

		if (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo)) {
			mimeType = "media/vcd";
		}
		if (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo)) {
			mimeType = "media/dvdvideo";
		}
		if (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo)) {
			mimeType = "media/bdvideo";
		}
	
		medium->setIconName(TQString::null);
	}
	else {
		// This device is a hard or flash disk of some kind

		// Default
		mimeType = "media/hdd" + MOUNT_SUFFIX;

		if (sdevice->isDiskOfType(TDEDiskDeviceType::USB)) {
			mimeType = "media/removable" + MOUNT_SUFFIX;
			medium->needMounting();

			if (sdevice->isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
				medium->setIconName("compact_flash" + MOUNT_ICON_SUFFIX);
			}
			if (sdevice->isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
				medium->setIconName("memory_stick" + MOUNT_ICON_SUFFIX);
			}
			if (sdevice->isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
				medium->setIconName("smart_media" + MOUNT_ICON_SUFFIX);
			}
			if (sdevice->isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
				medium->setIconName("sd_mmc" + MOUNT_ICON_SUFFIX);
			}
			if (sdevice->isDiskOfType(TDEDiskDeviceType::MediaDevice)) {
				medium->setIconName("ipod" + MOUNT_ICON_SUFFIX);

				if (sdevice->vendorModel().upper().contains("IPOD") && KProtocolInfo::isKnownProtocol( TQString("ipod") ) )
				{
					medium->unmountableState( "ipod:/" );
					medium->mountableState(!sdevice->mountPath().isNull());
				}
			}
			if (sdevice->isDiskOfType(TDEDiskDeviceType::Tape)) {
				medium->setIconName("magnetic_tape" + MOUNT_ICON_SUFFIX);
			}
			if (medium->isMounted() && TQFile::exists(medium->mountPoint() + "/dcim"))
			{
				mimeType = "media/camera" + MOUNT_SUFFIX;
			}
		}
	}

	if (!medium->needMounting()) {
		if (sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) {
			if (sdevice->checkDiskStatus(TDEDiskDeviceStatus::UsedByDevice)) {
				// Encrypted base devices must be set to this mimetype or they won't open when the base device node is passed to the kioslave
				mimeType = "media/removable_mounted";
			}
		}
	}

	medium->setMimeType(mimeType);
}

// Handle floppies and zip drives
bool TDEBackend::setFloppyProperties(Medium* medium)
{
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return false;
	}

	medium->setName(generateName(sdevice->deviceNode()));
	medium->setLabel(i18n("Unknown Drive"));

	// Certain disks have a lot in common with hard drives
	// FIXME
	// Any more?
	if ((sdevice->isDiskOfType(TDEDiskDeviceType::Zip)) || (sdevice->isDiskOfType(TDEDiskDeviceType::Jaz))) {
		medium->setName(generateName(sdevice->deviceNode()));
		if ((sdevice->isDiskOfType(TDEDiskDeviceType::LUKS)) || (sdevice->isDiskOfType(TDEDiskDeviceType::UnlockedCrypt))) {
			medium->setEncrypted(true);
		}
		else {
			medium->setEncrypted(false);
		}
	
		// USAGE: mountableState(Device node, Mount point, Filesystem type, Mounted ?)
		medium->mountableState(sdevice->deviceNode(), sdevice->mountPath(), sdevice->fileSystemName(), !sdevice->mountPath().isNull());
	}

	if (sdevice->isDiskOfType(TDEDiskDeviceType::Floppy)) {
		setFloppyMountState(medium);

		// We don't use the routine above as floppy disks are extremely slow (we don't want them accessed at all during media listing)
		medium->mountableState(sdevice->deviceNode(), sdevice->mountPath(), sdevice->fileSystemName(), !sdevice->mountPath().isNull());

		if (sdevice->mountPath().isNull()) {
			medium->setMimeType("media/floppy_unmounted");
		}
		else {
			medium->setMimeType("media/floppy_mounted" );
		}
		medium->setLabel(i18n("Floppy Drive"));
	}

	if (sdevice->isDiskOfType(TDEDiskDeviceType::Zip)) {
		if (sdevice->mountPath().isNull()) {
			medium->setMimeType("media/zip_unmounted");
		}
		else {
			medium->setMimeType("media/zip_mounted" );
		}

		// Set label
		TQString diskLabel = sdevice->diskLabel();
		if (diskLabel.isNull()) {
			diskLabel = i18n("%1 Zip Disk").arg(sdevice->deviceFriendlySize());
		}
		medium->setLabel(diskLabel);
	}
	
	/** @todo Mimetype for JAZ drives ? */
	
	medium->setIconName(TQString::null);
	
	return true;
}

void TDEBackend::setCameraProperties(Medium* medium)
{
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return;
	}

	TQString cameraName = sdevice->friendlyName();
	cameraName.replace("/", "_");
	medium->setName(cameraName);

	TQString device = "camera:/";

	TQStringList devNodeList = TQStringList::split("/", sdevice->deviceNode(), TRUE);
	TQString devNode0 = devNodeList[devNodeList.count()-2];
	TQString devNode1 = devNodeList[devNodeList.count()-1];

	if ((devNode0 != "") && (devNode1 != "")) {
		device.sprintf("camera://@[usb:%s,%s]/", devNode0.ascii(), devNode1.ascii());
	}

	medium->unmountableState(device);
	medium->setMimeType("media/gphoto2camera");
	medium->setIconName(TQString::null);

	if (sdevice->friendlyName() != "") {
		medium->setLabel(sdevice->friendlyName());
	}
	else {
		medium->setLabel(i18n("Camera"));
	}
}

void TDEBackend::setFloppyMountState( Medium *medium )
{
	KMountPoint::List mtab = KMountPoint::currentMountPoints();
	KMountPoint::List::iterator it = mtab.begin();
	KMountPoint::List::iterator end = mtab.end();
	
	TQString fstype;
	TQString mountpoint;
	for (; it!=end; ++it) {
		if ((*it)->mountedFrom() == medium->deviceNode() ) {
			fstype = (*it)->mountType().isNull() ? (*it)->mountType() : "auto";
			mountpoint = (*it)->mountPoint();
			medium->mountableState( medium->deviceNode(), mountpoint, fstype, true );
			return;
		}
	}
}

TQStringList TDEBackend::mountoptions(const TQString &name)
{
	const Medium* medium = m_mediaList.findById(name);
	if (!medium) {
		return TQStringList(); // we know nothing about that device
	}
	if (!isInFstab(medium).isNull()) {
		return TQStringList(); // device is listed in fstab, therefore is not handled by us
	}

	if (medium->isEncrypted()) {
		// if not decrypted yet then there are no mountoptions
		return TQStringList();
	}

	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return TQStringList(); // we can't get the information needed in order to process mount options
	}

	TQStringList result;

	// Allow only those options which are valid for the given device
	// pmount only allows a subset of options, and those are given in the map below
	// FIXME
	TQMap<TQString,bool> valids;
	valids["ro"] = true;
	//valids["quiet"] = false;
	//valids["flush"] = false;
	//valids["uid"] = false;
	valids["utf8"] = true;
	//valids["shortname"] = false;
	valids["locale"] = true;
	valids["sync"] = true;
	valids["noatime"] = true;
	//valids["data"] = false;

	TQString drive_udi = driveUDIFromDeviceUID(medium->id());

	KConfig config("mediamanagerrc");

	bool use_defaults = true;
	if (config.hasGroup(drive_udi)) {
		config.setGroup(drive_udi);
		use_defaults = config.readBoolEntry("use_defaults", false);
	}
	if (use_defaults) {
		config.setGroup("DefaultOptions");
	}
	result << TQString("use_defaults=%1").arg(use_defaults ? "true" : "false");

	bool removable = false;
	if (!drive_udi.isNull()) {
		removable = ((sdevice->checkDiskStatus(TDEDiskDeviceStatus::Removable)) || (sdevice->checkDiskStatus(TDEDiskDeviceStatus::Hotpluggable)));
	}

	TQString tmp;
	bool value;
	if (use_defaults) {
		value = config.readBoolEntry("automount", false);
	}
	else {
		QString current_group = config.group();
		config.setGroup(drive_udi);
		value = config.readBoolEntry("automount", false);
		config.setGroup(current_group);
	}

	if ((sdevice->checkDiskStatus(TDEDiskDeviceStatus::Blank))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDAudio))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::CDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::DVDVideo))
		|| (sdevice->isDiskOfType(TDEDiskDeviceType::BDVideo))
		) {
		value = false;
	}

	result << TQString("automount=%1").arg(value ? "true" : "false");

	if (valids.contains("ro")) {
		value = config.readBoolEntry("ro", false);
		tmp = TQString("ro=%1").arg(value ? "true" : "false");
		if (sdevice->fileSystemName() != "iso9660") {
			result << tmp;
		}
	}

	if (valids.contains("quiet")) {
		value = config.readBoolEntry("quiet", false);
		tmp = TQString("quiet=%1").arg(value ? "true" : "false");
		if (sdevice->fileSystemName() != "iso9660") {
			result << tmp;
		}
	}

	if (valids.contains("flush")) {
		value = config.readBoolEntry("flush", sdevice->fileSystemName().endsWith("fat"));
		tmp = TQString("flush=%1").arg(value ? "true" : "false");
		result << tmp;
	}

	if (valids.contains("uid")) {
		value = config.readBoolEntry("uid", true);
		tmp = TQString("uid=%1").arg(value ? "true" : "false");
		result << tmp;
	}

	if (valids.contains("utf8")) {
		value = config.readBoolEntry("utf8", true);
		tmp = TQString("utf8=%1").arg(value ? "true" : "false");
		result << tmp;
	}

	if (valids.contains("shortname")) {
		TQString svalue = config.readEntry("shortname", "lower").lower();
		if (svalue == "winnt") {
			result << "shortname=winnt";
		}
		else if (svalue == "win95") {
			result << "shortname=win95";
		}
		else if (svalue == "mixed") {
			result << "shortname=mixed";
		}
		else {
			result << "shortname=lower";
		}
	}

	// pass our locale to the ntfs-3g driver so it can translate local characters
	if (valids.contains("locale") && (sdevice->fileSystemName() == "ntfs-3g")) {
		// have to obtain LC_CTYPE as returned by the `locale` command
		// check in the same order as `locale` does
		char *cType;
		if ( (cType = getenv("LC_ALL")) || (cType = getenv("LC_CTYPE")) || (cType = getenv("LANG")) ) {
			result << TQString("locale=%1").arg(cType);
		}
	}

	if (valids.contains("sync")) {
		value = config.readBoolEntry("sync", ( valids.contains("flush") && !sdevice->fileSystemName().endsWith("fat") ) && removable);
		tmp = TQString("sync=%1").arg(value ? "true" : "false");
		if (sdevice->fileSystemName() != "iso9660") {
			result << tmp;
		}
	}

	if (valids.contains("noatime")) {
		value = config.readBoolEntry("atime", !sdevice->fileSystemName().endsWith("fat"));
		tmp = TQString("atime=%1").arg(value ? "true" : "false");
		if (sdevice->fileSystemName() != "iso9660") {
			result << tmp;
		}
	}

	TQString mount_point;
	mount_point = config.readEntry("mountpoint", TQString::null);

	if (!mount_point.startsWith("/")) {
		mount_point = "/media/" + mount_point;
	}
	if (mount_point != "") {
		result << TQString("mountpoint=%1").arg(mount_point);
	}

	TQString file_system_name;
	file_system_name = sdevice->fileSystemName();
	if (file_system_name != "") {
		result << TQString("filesystem=%1").arg(file_system_name);
	}

	if (valids.contains("data")) {
		TQString svalue = config.readEntry("journaling").lower();
		if (svalue == "ordered") {
			result << "journaling=ordered";
		}
		else if (svalue == "writeback") {
			result << "journaling=writeback";
		}
		else if (svalue == "data") {
			result << "journaling=data";
		}
		else {
			result << "journaling=ordered";
		}
	}

	return result;
}

bool TDEBackend::setMountoptions(const TQString &name, const TQStringList &options )
{
	const Medium* medium = m_mediaList.findById(name);
	if (!medium) {
		return false; // we know nothing about that device
	}
	if (!isInFstab(medium).isNull()) {
		return false; // device is listed in fstab, therefore is not handled by us
	}

	TQString drive_udi = driveUDIFromDeviceUID(medium->id());

	kdDebug(1219) << "setMountoptions " << name << " " << options << endl;
	
	KConfig config("mediamanagerrc");
	config.setGroup(drive_udi);
	
	TQMap<TQString,TQString> valids = MediaManagerUtils::splitOptions(options);
	
	const char *names[] = { "use_defaults", "ro", "quiet", "atime", "uid", "utf8", "flush", "sync", 0 };
	for (int index = 0; names[index]; ++index) {
		if (valids.contains(names[index])) {
			config.writeEntry(names[index], valids[names[index]] == "true");
		}
	}
	
	if (valids.contains("shortname")) {
		config.writeEntry("shortname", valids["shortname"]);
	}
	
	if (valids.contains("journaling")) {
		config.writeEntry("journaling", valids["journaling"]);
	}
	
	if (!mountoptions(name).contains(TQString("mountpoint=%1").arg(valids["mountpoint"]))) {
		config.writeEntry("mountpoint", valids["mountpoint"]);
	}
	
	if (valids.contains("automount")) {
		config.setGroup(drive_udi);
		config.writeEntry("automount", valids["automount"]);
	}
	
	return true;
}

void TDEBackend::slotPasswordReady() {
	m_decryptionPassword = m_decryptDialog->getPassword();
	m_decryptPasswordValid = true;
}

void TDEBackend::slotPasswordCancel() {
	m_decryptionPassword = TQString::null;
	m_decryptPasswordValid = true;
}

TQString TDEBackend::mount(const Medium *medium)
{
	if (medium->isMounted()) {
		return TQString(); // that was easy
	}
	
	TQString mountPoint = isInFstab(medium);
	if (!mountPoint.isNull())
	{
		struct mount_job_data data;
		data.completed = false;
		data.medium = medium;
		
		KIO::Job *job = KIO::mount( false, 0, medium->deviceNode(), mountPoint );
		connect(job, TQT_SIGNAL( result (KIO::Job *)), TQT_SLOT( slotResult( KIO::Job *)));
		mount_jobs[job] = &data;
		// The caller expects the device to be mounted when the function
		// completes. Thus block until the job completes.
		while (!data.completed) {
			kapp->eventLoop()->enterLoop();
		}
		// Return the error message (if any) to the caller
		return (data.error) ? data.errorMessage : TQString::null;
		
	}

	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return i18n("Internal error");
	}

	TQString optionString;
	TQString diskLabel;
	
	TQMap<TQString,TQString> valids = MediaManagerUtils::splitOptions(mountoptions(medium->id()));
	if (valids["ro"] == "true") {
		optionString.append(" -r");
	}
	
	if (valids["atime"] != "true") {
		optionString.append(" -A");
	}
	
	if (valids["utf8"] == "true") {
		optionString.append(" -c utf8");
	}
	
	if (valids["sync"] == "true") {
		optionString.append(" -s");
	}
	
	TQString mount_point = valids["mountpoint"];
	if (mount_point.startsWith("/media/")) {
		diskLabel = mount_point.mid(7);
	}

	if (valids.contains("filesystem")) {
		optionString.append(TQString(" -t %1").arg(valids["filesystem"]));
	}

	if (valids.contains("locale")) {
		optionString.append(TQString(" -c %1").arg(valids["locale"]));
	}
	
	TQString qerror = i18n("Cannot mount encrypted drives!");

	if (!medium->isEncrypted()) {
		// normal volume
		TQString mountMessages;
		TQString mountedPath = sdevice->mountDevice(diskLabel, optionString, &mountMessages);
		if (mountedPath.isNull()) {
			qerror = i18n("<qt>Unable to mount this device.<p>Potential reasons include:<br>Improper device and/or user privilege level<br>Corrupt data on storage device");
			if (!mountMessages.isNull()) {
				qerror.append(i18n("<p>Technical details:<br>").append(mountMessages));
			}
			qerror.append("</qt>");
		}
		else {
			qerror = "";
		}
	}
	else {
		TQString iconName = medium->iconName();
		if (iconName.isEmpty())
		{
			TQString mime =  medium->mimeType();
			iconName = KMimeType::mimeType(mime)->icon(mime, false);
		}

		bool continue_trying_to_decrypt = true;
		while (continue_trying_to_decrypt == true) {
			m_decryptPasswordValid = false;

			m_decryptDialog = new Dialog(sdevice->deviceNode(), iconName);
			m_decryptDialog->show();
	
			connect(m_decryptDialog, TQT_SIGNAL (user1Clicked()), this, TQT_SLOT (slotPasswordReady()));
			connect(m_decryptDialog, TQT_SIGNAL (cancelClicked()), this, TQT_SLOT (slotPasswordCancel()));
			connect(this, TQT_SIGNAL (signalDecryptionPasswordError(TQString)), m_decryptDialog, TQT_SLOT (slotDialogError(TQString)));

			while (m_decryptPasswordValid == false) {
				tqApp->processEvents();
			}

			m_decryptDialog->setEnabled(false);
			tqApp->processEvents();

			if (m_decryptionPassword.isNull()) {
				delete m_decryptDialog;
				return TQString("Decryption aborted");
			}
			else {
				// Just for some added fun, if udev emits a medium change event, which I then forward, with mounted==0, it stops the MediaProtocol::listDir method dead in its tracks,
				// and therefore the media:/ kioslave won't refresh after the encrypted device mount
				// Therefore, I need to ignore all change events on this device during the mount process and hope nothing bad happens as a result!
				if (!m_ignoreDeviceChangeEvents.contains(sdevice->uniqueID())) {
					m_ignoreDeviceChangeEvents.append(sdevice->uniqueID());
				}

				// mount encrypted volume with password
				int mountRetcode;
				TQString mountMessages;
				TQString mountedPath = sdevice->mountEncryptedDevice(m_decryptionPassword, diskLabel, optionString, &mountMessages, &mountRetcode);
				if (mountedPath.isNull()) {
					if (mountRetcode == 0) {
						// Mounting was successful
						// Because the TDE hardware backend is event driven it might take a little while for the new unencrypted mapped device to show up
						// Wait up to 30 seconds for it to appear...
						for (int i=0;i<300;i++) {
							mountedPath = sdevice->mountPath();
							if (!mountedPath.isNull()) {
								break;
							}
							tqApp->processEvents(50);
							usleep(50000);
						}
					}
				}
				if (mountedPath.isNull()) {
					if (mountRetcode == 25600) {
						// Probable LUKS failure
						// Retry
						m_decryptDialog->setEnabled(true);
						continue_trying_to_decrypt = true;
					}
					else {
						qerror = i18n("<qt>Unable to mount this device.<p>Potential reasons include:<br>Improper device and/or user privilege level<br>Corrupt data on storage device<br>Incorrect encryption password");
						if (!mountMessages.isNull()) {
							qerror.append(i18n("<p>Technical details:<br>").append(mountMessages));
						}
						qerror.append("</qt>");
						continue_trying_to_decrypt = false;
					}
				}
				else {
					qerror = "";
					continue_trying_to_decrypt = false;
				}

				delete m_decryptDialog;
			}
		}
	}

	if (!qerror.isEmpty()) {
		return qerror;
	}

	ResetProperties(sdevice, false, true);

	if (m_ignoreDeviceChangeEvents.contains(sdevice->uniqueID())) {
		m_ignoreDeviceChangeEvents.remove(sdevice->uniqueID());
	}

	return TQString();
}

TQString TDEBackend::mount(const TQString &_udi)
{
	const Medium* medium = m_mediaList.findById(_udi);
	if (!medium)
		return i18n("No such medium: %1").arg(_udi);
	
	return mount(medium);
}

TQString TDEBackend::unmount(const TQString &_udi)
{
	const Medium* medium = m_mediaList.findById(_udi);
	
	if ( !medium )
		return i18n("No such medium: %1").arg(_udi);
	
	if (!medium->isMounted())
		return TQString(); // that was easy

	TQString mountPoint = isInFstab(medium);
	if (!mountPoint.isNull())
	{
		struct mount_job_data data;
		data.completed = false;
		data.medium = medium;
		
		KIO::Job *job = KIO::unmount( medium->mountPoint(), false );
		connect(job, TQT_SIGNAL( result (KIO::Job *)), TQT_SLOT( slotResult( KIO::Job *)));
		mount_jobs[job] = &data;
		// The caller expects the device to be unmounted when the function
		// completes. Thus block until the job completes.
		while (!data.completed) {
			kapp->eventLoop()->enterLoop();
		}
		// Return the error message (if any) to the caller
		return (data.error) ? data.errorMessage : TQString::null;
	}

	TQString udi = TQString::null;

	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(medium->id());
	if (!sdevice) {
		return i18n("Internal error");
	}

	TQString qerror;
	TQString origqerror;

	// Save these for later
	TQString uid = sdevice->uniqueID();
	TQString node = sdevice->deviceNode();

	TQString unmountMessages;
	int unmountRetcode = 0;
	if (!sdevice->unmountDevice(&unmountMessages, &unmountRetcode)) {
		// Unmount failed!
		qerror = "<qt>" + i18n("Unfortunately, the device <b>%1</b> (%2) named <b>'%3'</b> and currently mounted at <b>%4</b> could not be unmounted. ").arg("system:/media/" + medium->name(), medium->deviceNode(), medium->prettyLabel(), medium->prettyBaseURL().pathOrURL());
		if (!unmountMessages.isNull()) {
			qerror.append(i18n("<p>Technical details:<br>").append(unmountMessages));
		}
		qerror.append("</qt>");
	}
	else {
		qerror = "";
	}

	if (unmountRetcode == 1280) {
		// Failed as BUSY
		TQString processesUsingDev = listUsingProcesses(medium);
		if (!processesUsingDev.isNull()) {
			if (KMessageBox::warningYesNo(0, i18n("<qt>The device <b>%1</b> (%2) named <b>'%3'</b> and currently mounted at <b>%4</b> can not be unmounted at this time.<p>%5<p><b>Would you like to forcibly terminate these processes?</b><br><i>All unsaved data would be lost</i>").arg("system:/media/" + medium->name()).arg(medium->deviceNode()).arg(medium->prettyLabel()).arg(medium->prettyBaseURL().pathOrURL()).arg(processesUsingDev)) == KMessageBox::Yes) {
				killUsingProcesses(medium);
				if (!sdevice->unmountDevice(&unmountMessages, &unmountRetcode)) {
					// Unmount failed!
					qerror = "<qt>" + i18n("Unfortunately, the device <b>%1</b> (%2) named <b>'%3'</b> and currently mounted at <b>%4</b> could not be unmounted. ").arg("system:/media/" + medium->name(), medium->deviceNode(), medium->prettyLabel(), medium->prettyBaseURL().pathOrURL());
					if (!unmountMessages.isNull()) {
						qerror.append(i18n("<p>Technical details:<br>").append(unmountMessages));
					}
					qerror.append("</qt>");
				}
				else {
					qerror = "";
				}
			}
		}
	}

	if (qerror != "") {
		return qerror;
	}

	// There is a possibility that the storage device was unceremoniously removed from the system immediately after it was unmounted
	// There is no reliable way to know if this happened either!
	// For now, see if the device node still exists
	TQFileInfo checkDN(node);
	if (!checkDN.exists()) {
		m_mediaList.removeMedium(uid, true);
	}
	else {
		TQString mountedPath = sdevice->mountPath();
		if (!mountedPath.isNull()) {
			// Because the TDE hardware backend is event driven it might take a little while for the device to show up as unmounted
			// Wait up to 30 seconds for the mount to disappear...
			for (int i=0;i<300;i++) {
				mountedPath = sdevice->mountPath();
				if (mountedPath.isNull()) {
					break;
				}
				tqApp->processEvents(50);
				usleep(50000);
			}
		}

		ResetProperties(sdevice, false);
	}

	return TQString();
}

TQString TDEBackend::isInFstab(const Medium *medium)
{
	KMountPoint::List fstab = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions|KMountPoint::NeedRealDeviceName);

	KMountPoint::List::iterator it = fstab.begin();
	KMountPoint::List::iterator end = fstab.end();

	for (; it!=end; ++it)
	{
		TQString reald = (*it)->realDeviceName();
		if ( reald.endsWith( "/" ) ) {
			reald = reald.left( reald.length() - 1 );
		}
		if ((*it)->mountedFrom() == medium->deviceNode() || ( !medium->deviceNode().isEmpty() && reald == medium->deviceNode() ) )
		{
			TQStringList opts = (*it)->mountOptions();
			if (opts.contains("user") || opts.contains("users")) {
				return (*it)->mountPoint();
			}
		}
	}

	return TQString::null;
}

TQString TDEBackend::listUsingProcesses(const Medium* medium)
{
	TQString proclist, fullmsg;
	TQString cmdline = TQString("/usr/bin/env fuser -vm %1 2>&1").arg(KProcess::quote(medium->mountPoint()));
	FILE *fuser = popen(cmdline.latin1(), "r");
	
	uint counter = 0;
	if (fuser) {
		proclist += "<pre>";
		TQTextIStream is(fuser);
		TQString tmp;
		while (!is.atEnd()) {
			tmp = is.readLine();
			tmp = TQStyleSheet::escape(tmp) + "\n";
		
			proclist += tmp;
			if (counter++ > 10) {
				proclist += "...";
				break;
			}
		}
		proclist += "</pre>";
		(void)pclose( fuser );
	}
	if (counter) {
		fullmsg = i18n("Programs still using the device "
			"have been detected. They are listed below. You have to "
			"close them or change their working directory before "
			"attempting to unmount the device again.");
		fullmsg += "<br>" + proclist;
		return fullmsg;
	}
	else {
		return TQString::null;
	}
}

TQString TDEBackend::killUsingProcesses(const Medium* medium)
{
	TQString proclist, fullmsg;
	TQString cmdline = TQString("/usr/bin/env fuser -vmk %1 2>&1").arg(KProcess::quote(medium->mountPoint()));
	FILE *fuser = popen(cmdline.latin1(), "r");
	
	uint counter = 0;
	if (fuser) {
		proclist += "<pre>";
		TQTextIStream is(fuser);
		TQString tmp;
		while (!is.atEnd()) {
			tmp = is.readLine();
			tmp = TQStyleSheet::escape(tmp) + "\n";
		
			proclist += tmp;
			if (counter++ > 10) {
				proclist += "...";
				break;
			}
		}
		proclist += "</pre>";
		(void)pclose( fuser );
	}
	if (counter) {
		fullmsg = i18n("Programs that were still using the device "
			"have been forcibly terminated. They are listed below.");
		fullmsg += "<br>" + proclist;
		return fullmsg;
	}
	else {
		return TQString::null;
	}
}

TQString TDEBackend::generateName(const TQString &devNode)
{
    return KURL(devNode).fileName();
}

TQString TDEBackend::driveUDIFromDeviceUID(TQString uuid) {
	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	TDEStorageDevice * sdevice = hwdevices->findDiskByUID(uuid);
	TQString ret;
	if (sdevice) {
		ret = sdevice->diskUUID();
		if (ret == "") {
			ret = sdevice->deviceNode();
		}
		if (ret == "") {
			ret = sdevice->uniqueID();
		}
	}
	if (ret == "") {
		return TQString::null;
	}
	else {
		return ret;
	}
}

#include "tdehardwarebackend.moc"