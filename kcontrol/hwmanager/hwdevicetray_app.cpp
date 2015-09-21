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

#include <kdebug.h>

#include "hwdevicetray_app.h"

#include "hwdevicetray.h"

HwDeviceApp::HwDeviceApp()
	: m_tray(new HwDeviceSystemTray(0L, "HwDeviceTray"))
{
	m_tray->show();
}

#include "hwdevicetray_app.moc"