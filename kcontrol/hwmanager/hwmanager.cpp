/**
 * hwmanager.cpp
 *
 * Copyright (c) 2009-2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kinputdialog.h>
#include <kurlrequester.h>
#include <kgenericfactory.h>

#include <unistd.h>
#include <ksimpleconfig.h>
#include <string>
#include <stdio.h>
#include <tqstring.h>

#include "hwmanager.h"

using namespace std;

/**** DLL Interface ****/
typedef KGenericFactory<TDEHWManager, TQWidget> TDEHWManagerFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_hwmanager, TDEHWManagerFactory("kcmhwmanager") )

KSimpleConfig *config;
KSimpleConfig *systemconfig;

/**** TDEHWManager ****/

TDEHWManager::TDEHWManager(TQWidget *parent, const char *name, const TQStringList &)
  : KCModule(TDEHWManagerFactory::instance(), parent, name)
{
	TQVBoxLayout *layout = new TQVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
	config = new KSimpleConfig( TQString::fromLatin1( "hwmanagerrc" ));
	systemconfig = new KSimpleConfig( TQString::fromLatin1( KDE_CONFDIR "/tdehw/hwmanagerrc" ));

	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmhwmanager"), I18N_NOOP("TDE Hardware Device Manager"),
		0, 0, KAboutData::License_GPL,
		I18N_NOOP("(c) 2012 Timothy Pearson"));
	
	about->addAuthor("Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net");
	setAboutData( about );

	base = new TDEHWManagerBase(this);
	layout->add(base);

	base->deviceFilter->setListView(base->deviceTree);

	setRootOnlyMsg(i18n("<b>Hardware settings are system wide, and requires administrator access</b><br>To alter the system's hardware settings, click on the \"Administrator Mode\" button below."));
	setUseRootOnlyMsg(true);

	TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();

	connect(base->showByConnection, TQT_SIGNAL(clicked()), TQT_SLOT(changed()));
	connect(base->showByConnection, TQT_SIGNAL(clicked()), TQT_SLOT(populateTreeView()));

	connect(hwdevices, TQT_SIGNAL(hardwareAdded(TDEGenericDevice*)), this, TQT_SLOT(populateTreeView()));
	connect(hwdevices, TQT_SIGNAL(hardwareRemoved(TDEGenericDevice*)), this, TQT_SLOT(populateTreeView()));
// 	connect(hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(populateTreeView()));

	load();

	populateTreeView();
}

TDEHWManager::~TDEHWManager()
{
	delete config;
	delete systemconfig;
}

void TDEHWManager::load()
{
	load( false );
}

void TDEHWManager::load(bool useDefaults )
{
	emit changed(useDefaults);
}

void TDEHWManager::save()
{
	emit changed(false);
}

void TDEHWManager::defaults()
{
	load( true );
}

void TDEHWManager::populateTreeView()
{
	bool show_by_connection = base->showByConnection->isChecked();

	// Figure out which device, if any, was selected
	TQString selected_syspath;
	DeviceIconItem* selItem = dynamic_cast<DeviceIconItem*>(base->deviceTree->selectedItem());
	if (selItem) {
		if (selItem->device()) {
			selected_syspath = selItem->device()->systemPath();
		}
	}

	base->deviceTree->clear();

	if (show_by_connection) {
		DeviceIconItem* rootitem = new DeviceIconItem(base->deviceTree, "Linux System", DesktopIcon("misc", base->deviceTree->iconSize()), 0);

		TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
		TDEGenericHardwareList hwlist = hwdevices->listByDeviceClass(TDEGenericDeviceType::Root);
		TDEGenericDevice *hwdevice;
		for ( hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next() ) {
			DeviceIconItem* item = new DeviceIconItem(rootitem, hwdevice->friendlyName(), hwdevice->icon(base->deviceTree->iconSize()), hwdevice);
			if ((!selected_syspath.isNull()) && (hwdevice->systemPath() == selected_syspath)) {
				base->deviceTree->ensureItemVisible(item);
				base->deviceTree->setSelected(item, true);
			}
			populateTreeViewLeaf(item, show_by_connection, selected_syspath);
		}
	}
	else {
		TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
		for (int i=0;i<=TDEGenericDeviceType::Last;i++) {
			if (i != TDEGenericDeviceType::Root) {
				DeviceIconItem* rootitem = new DeviceIconItem(base->deviceTree, hwdevices->getFriendlyDeviceTypeStringFromType((TDEGenericDeviceType::TDEGenericDeviceType)i), hwdevices->getDeviceTypeIconFromType((TDEGenericDeviceType::TDEGenericDeviceType)i, base->deviceTree->iconSize()), 0);
				TDEGenericDevice *hwdevice;
				TDEGenericHardwareList hwlist = hwdevices->listByDeviceClass((TDEGenericDeviceType::TDEGenericDeviceType)i);
				for ( hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next() ) {
					DeviceIconItem* item = new DeviceIconItem(rootitem, hwdevice->friendlyName(), hwdevice->icon(base->deviceTree->iconSize()), hwdevice);
					if ((!selected_syspath.isNull()) && (hwdevice->systemPath() == selected_syspath)) {
						base->deviceTree->ensureItemVisible(item);
						base->deviceTree->setSelected(item, true);
					}
				}
			}
		}
	}

	if (!selected_syspath.isNull()) {
		
	}
}

void TDEHWManager::populateTreeViewLeaf(DeviceIconItem *parent, bool show_by_connection, TQString selected_syspath) {
	if (show_by_connection) {
		TDEHardwareDevices *hwdevices = KGlobal::hardwareDevices();
		TDEGenericHardwareList hwlist = hwdevices->listAllPhysicalDevices();
		TDEGenericDevice *hwdevice;
		for ( hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next() ) {
			if (hwdevice->parentDevice() == parent->device()) {
				DeviceIconItem* item = new DeviceIconItem(parent, hwdevice->friendlyName(), hwdevices->getDeviceTypeIconFromType(hwdevice->type(), base->deviceTree->iconSize()), hwdevice);
				if ((!selected_syspath.isNull()) && (hwdevice->systemPath() == selected_syspath)) {
					base->deviceTree->ensureItemVisible(item);
					base->deviceTree->setSelected(item, true);
				}
				populateTreeViewLeaf(item, show_by_connection, selected_syspath);
			}
		}
	}
}

TQString TDEHWManager::quickHelp() const
{
  return i18n("<h1>TDE Hardware Device Manager</h1> This module allows you to configure hardware devices on your system");
}

#include "hwmanager.moc"