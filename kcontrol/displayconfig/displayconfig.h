/**
 * displayconfig.h
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

#ifndef _KCM_DisplayCONFIG_H
#define _KCM_DisplayCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqptrlist.h>
#include <tqslider.h>
#include <tqworkspace.h>
#include <tqobjectlist.h>
#include <tqwidgetlist.h>

#include <dcopobject.h>

#include <libkrandr/libkrandr.h>

#include "monitorworkspace.h"
#include "displayconfigbase.h"

class KConfig;
class KPopupMenu;
class KListViewItem;

struct SingleScreenData {
	TQString screenFriendlyName;
	bool generic_screen_detected;
	bool screen_connected;

	TQStringList resolutions;
	TQStringList refresh_rates;
	TQStringList color_depths;
	TQStringList rotations;

	int current_resolution_index;
	int current_refresh_rate_index;
	int current_color_depth_index;

	int current_rotation_index;
	int current_orientation_mask;
	bool has_x_flip;
	bool has_y_flip;
	bool supports_transformations;

	bool is_primary;
	bool is_extended;
	int absolute_x_position;
	int absolute_y_position;
	int current_x_pixel_count;
	int current_y_pixel_count;
};

class KDisplayConfig : public KCModule, public DCOPObject
{
  K_DCOP
  Q_OBJECT


public:
	//KDisplayConfig(TQWidget *parent = 0L, const char *name = 0L);
	KDisplayConfig(TQWidget *parent, const char *name, const TQStringList &);
	virtual ~KDisplayConfig();
	
	DisplayConfigBase *base;
	
	void load();
	void load( bool useDefaults);
	void save();
	void defaults();
	
	int buttons();
	TQString quickHelp() const;

k_dcop:

private:

	KConfig *config;
	bool _ok;
	Display *randr_display;
	ScreenInfo *randr_screen_info;
	int numberOfProfiles;
	int numberOfScreens;
	TQStringList cfgScreenInfo;
	TQStringList cfgProfiles;
	void refreshDisplayedInformation ();
	void updateDisplayedInformation ();
	TQString extractFileName(TQString displayName, TQString profileName);
	TQString *displayFileArray;
	int findProfileIndex(TQString profileName);
	int findScreenIndex(TQString screenName);
	TQString m_defaultProfile;
	KRandrSimpleAPI *m_randrsimple;
	TQPtrList<SingleScreenData> m_screenInfoArray;
	int realResolutionSliderValue();
	void setRealResolutionSliderValue(int index);
	void addTab( const TQString name, const TQString label );
	void moveMonitor(DraggableMonitor* monitor, int realx, int realy);
	bool applyMonitorLayoutRules(void);
	bool applyMonitorLayoutRules(DraggableMonitor* primary_monitor);
	void updateDraggableMonitorInformationInternal (int, bool);
	TQByteArray getEDID(int, TQString);
	TQString getEDIDMonitorName(int, TQString);

private slots:
	void selectProfile (int slotNumber);
	void selectScreen (int slotNumber);
	void resolutionSliderChanged(int index);
	void resolutionSliderTextUpdate(int index);
	void updateArray (void);
	void addProfile (void);
	void renameProfile (void);
	void deleteProfile (void);
	void ensurePrimaryMonitorIsAvailable (void);
	void updateDragDropDisplay (void);
	void layoutDragDropDisplay (void);
	void ensureMonitorDataConsistency (void);
	void updateDraggableMonitorInformation (int);
	void updateExtendedMonitorInformation (void);
	void processLockoutControls (void);
};

#endif

