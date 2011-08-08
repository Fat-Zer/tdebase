/**
 * displayconfig.cpp
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
#include <tqtabwidget.h>

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
#include <kcmoduleloader.h>
#include <kgenericfactory.h>

#include <unistd.h>
#include <ksimpleconfig.h>
#include <string>
#include <stdio.h>
#include <tqstring.h>

#include <math.h>
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

#include "displayconfig.h"

using namespace std;

/**** DLL Interface ****/
typedef KGenericFactory<KDisplayConfig, TQWidget> KDisplayCFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_displayconfig, KDisplayCFactory("kcmdisplayconfig") )

KSimpleConfig *systemconfig;


// This routine is courtsey of an answer on "Stack Overflow"
// It takes an LSB-first int and makes it an MSB-first int (or vice versa)
unsigned int reverse_bits(register unsigned int x)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

TQString capitalizeString(TQString in) {
	return in.left(1).upper() + in.right(in.length()-1);
}

TQPoint moveTQRectOutsideTQRect(TQRect base, TQRect movable, int fallback_level = 0) {
	TQPoint final_result;

	double base_center_x = base.x() + (base.width()/2);
	double base_center_y = base.y() + (base.height()/2);
	double movable_center_x = movable.x() + (movable.width()/2);
	double movable_center_y = movable.y() + (movable.height()/2);

	double max_x_movement = (base.width()/2) + (movable.width()/2);
	double max_y_movement = (base.height()/2) + (movable.height()/2);

	double x_diff = abs(base_center_x-movable_center_x);
	double y_diff = abs(base_center_y-movable_center_y);

	int invert_movement;

	// Calculate the angles of the four corners of the base rectangle
	double angle_1 = atan2((base.height()/2),	(base.width()/2));
	double angle_2 = atan2((base.height()/2),	(base.width()/2)*(-1));
	double angle_3 = atan2((base.height()/2)*(-1),	(base.width()/2)*(-1));
	double angle_4 = atan2((base.height()/2)*(-1),	(base.width()/2));

	// Calculate the angle that the movable rectangle center is on
	double movable_angle = atan2(base_center_y-movable_center_y, movable_center_x-base_center_x);

	// Fix up coordinates
	if (angle_1 < 0) angle_1 = angle_1 + (2*PI);
	if (angle_2 < 0) angle_2 = angle_2 + (2*PI);
	if (angle_3 < 0) angle_3 = angle_3 + (2*PI);
	if (angle_4 < 0) angle_4 = angle_4 + (2*PI);
	if (movable_angle < 0) movable_angle = movable_angle + (2*PI);

	// Now calculate quadrant
	int quadrant;
	if ((movable_angle < angle_2) && (movable_angle >= angle_1)) {
		quadrant = 2;
	}
	else if ((movable_angle < angle_3) && (movable_angle >= angle_2)) {
		quadrant = 3;
	}
	else if ((movable_angle < angle_4) && (movable_angle >= angle_3)) {
		quadrant = 4;
	}
	else {
		quadrant = 1;
	}

	if (fallback_level == 0) {
		if ((quadrant == 2) || (quadrant == 4)) {
			// Move it in the Y direction
			if (movable_center_y < base_center_y)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint(0, (max_y_movement-y_diff)*invert_movement);
		}
		else {
			// Move it in the X direction
			if (movable_center_x < base_center_x)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint((max_x_movement-x_diff)*invert_movement, 0);
		}
	}
	if (fallback_level == 1) {
		if ((quadrant == 1) || (quadrant == 3)) {
			// Move it in the Y direction
			if (movable_center_y < base_center_y)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint(0, (max_y_movement-y_diff)*invert_movement);
		}
		else {
			// Move it in the X direction
			if (movable_center_x < base_center_x)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint((max_x_movement-x_diff)*invert_movement, 0);
		}
	}
	if (fallback_level == 2) {
		// Ooh, nasty, I need to move the rectangle the other (suboptimal) direction
		if ((quadrant == 2) || (quadrant == 4)) {
			// Move it in the Y direction
			if (movable_center_y >= base_center_y)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint(0, (max_y_movement+y_diff)*invert_movement);
		}
		else {
			// Move it in the X direction
			if (movable_center_x >= base_center_x)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint((max_x_movement+x_diff)*invert_movement, 0);
		}
	}
	if (fallback_level == 3) {
		// Ooh, nasty, I need to move the rectangle the other (suboptimal) direction
		if ((quadrant == 1) || (quadrant == 3)) {
			// Move it in the Y direction
			if (movable_center_y >= base_center_y)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint(0, (max_y_movement+y_diff)*invert_movement);
		}
		else {
			// Move it in the X direction
			if (movable_center_x >= base_center_x)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint((max_x_movement+x_diff)*invert_movement, 0);
		}
	}

	// Check for intersection
	TQRect test_rect = movable;
	test_rect.moveBy(final_result.x(), final_result.y());
	if (test_rect.intersects(base)) {
		if (final_result.x() > 0)
			final_result.setX(final_result.x()+1);
		if (final_result.x() < 0)
			final_result.setX(final_result.x()-1);
		if (final_result.y() > 0)
			final_result.setY(final_result.y()+1);
		if (final_result.y() < 0)
			final_result.setY(final_result.y()-1);
	}

	return final_result;
}

TQPoint moveTQRectSoThatItTouchesTQRect(TQRect base, TQRect movable, int fallback_level = 0) {
	TQPoint final_result;

	double base_center_x = base.x() + (base.width()/2);
	double base_center_y = base.y() + (base.height()/2);
	double movable_center_x = movable.x() + (movable.width()/2);
	double movable_center_y = movable.y() + (movable.height()/2);

	double max_x_movement = (base.width()/2) + (movable.width()/2);
	double max_y_movement = (base.height()/2) + (movable.height()/2);

	double x_diff = abs(base_center_x-movable_center_x);
	double y_diff = abs(base_center_y-movable_center_y);

	int invert_movement;

	// Calculate the angles of the four corners of the base rectangle
	double angle_1 = atan2((base.height()/2),	(base.width()/2));
	double angle_2 = atan2((base.height()/2),	(base.width()/2)*(-1));
	double angle_3 = atan2((base.height()/2)*(-1),	(base.width()/2)*(-1));
	double angle_4 = atan2((base.height()/2)*(-1),	(base.width()/2));

	// Calculate the angle that the movable rectangle center is on
	double movable_angle = atan2(base_center_y-movable_center_y, movable_center_x-base_center_x);

	// Fix up coordinates
	if (angle_1 < 0) angle_1 = angle_1 + (2*PI);
	if (angle_2 < 0) angle_2 = angle_2 + (2*PI);
	if (angle_3 < 0) angle_3 = angle_3 + (2*PI);
	if (angle_4 < 0) angle_4 = angle_4 + (2*PI);
	if (movable_angle < 0) movable_angle = movable_angle + (2*PI);

	// Now calculate quadrant
	int quadrant;
	if ((movable_angle < angle_2) && (movable_angle >= angle_1)) {
		quadrant = 2;
	}
	else if ((movable_angle < angle_3) && (movable_angle >= angle_2)) {
		quadrant = 3;
	}
	else if ((movable_angle < angle_4) && (movable_angle >= angle_3)) {
		quadrant = 4;
	}
	else {
		quadrant = 1;
	}

	if (fallback_level == 0) {
		if ((quadrant == 2) || (quadrant == 4)) {
			// Move it in the Y direction
			if (movable_center_y < base_center_y)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint(0, (max_y_movement-y_diff)*invert_movement);
		}
		else {
			// Move it in the X direction
			if (movable_center_x < base_center_x)
				invert_movement = -1;
			else
				invert_movement = 1;
			final_result = TQPoint((max_x_movement-x_diff)*invert_movement, 0);
		}
	}

	// Check for intersection
	TQRect test_rect = movable;
	test_rect.moveBy(final_result.x(), final_result.y());
	if (test_rect.intersects(base)) {
		if (final_result.x() > 0)
			final_result.setX(final_result.x()-1);
		if (final_result.x() < 0)
			final_result.setX(final_result.x()+1);
		if (final_result.y() > 0)
			final_result.setY(final_result.y()-1);
		if (final_result.y() < 0)
			final_result.setY(final_result.y()+1);
	}

	return final_result;
}

TQPoint moveTQRectOutsideMonitorRegion(TQRect rect, MonitorRegion region) {
	// This is a fun little class (not!)
	// It needs to move the TQRect so that it does not overlap any rectangles in the region
	long rect_center_x = rect.x() + (rect.width()/2);
	long rect_center_y = rect.y() + (rect.height()/2);

	// First, see if the rectangle actually overlaps the region
	if (!region.contains(rect))
		return TQPoint(0,0);

	// Then, break the region into the series of source rectangles
	TQMemArray<TQRect> rectangles = region.rects();

	// Next, find which rectangle is closest to the center of the TQRect
	int closest = 0;
	long distance = 16384*16384;
	int fallback_mode;
	long test_distance;
	long test_center_x;
	long test_center_y;
	for ( int i = 0; i < rectangles.size(); i++ ) {
		test_center_x = rectangles[i].x() + (rectangles[i].width()/2);
		test_center_y = rectangles[i].y() + (rectangles[i].height()/2);
		test_distance = pow(test_center_x-rect_center_x,2) + pow(test_center_y-rect_center_y,2);
		if (test_distance < distance) {
			// Make sure this is an outer rectangle; i,e. there is empty space where
			// we would want to move the TQRect...
			// To do that we will move the TQRect in all four possible directions,
			// and see if any put the TQRect in an empty location
			// If they do, then this current rectangle is considered usable
			// and it is added to the distance checking routine.
			TQPoint test_moveby = moveTQRectOutsideTQRect(rectangles[i], rect, 0);
			TQRect test_rect = rect;
			test_rect.moveBy(test_moveby.x(), test_moveby.y());
			if (!region.contains(test_rect)) {
				closest = i;
				distance = test_distance;
				fallback_mode = 0;
			}
			else {
				test_moveby = moveTQRectOutsideTQRect(rectangles[i], rect, 1);
				test_rect = rect;
				test_rect.moveBy(test_moveby.x(), test_moveby.y());
				if (!region.contains(test_rect)) {
					closest = i;
					distance = test_distance;
					fallback_mode = 1;
				}
				else {
					test_moveby = moveTQRectOutsideTQRect(rectangles[i], rect, 2);
					test_rect = rect;
					test_rect.moveBy(test_moveby.x(), test_moveby.y());
					if (!region.contains(test_rect)) {
						closest = i;
						distance = test_distance;
						fallback_mode = 2;
					}
					else {
						test_moveby = moveTQRectOutsideTQRect(rectangles[i], rect, 3);
						test_rect = rect;
						test_rect.moveBy(test_moveby.x(), test_moveby.y());
						if (!region.contains(test_rect)) {
							closest = i;
							distance = test_distance;
							fallback_mode = 3;
						}
					}
				}
			}
		}
	}

	// Finally, calculate the required translation to move the TQRect outside the MonitorRegion
	// so that it touches the closest line found above
	return moveTQRectOutsideTQRect(rectangles[closest], rect, fallback_mode);
}

TQPoint compressTQRectTouchingMonitorRegion(TQRect rect, MonitorRegion region, TQSize workspace) {
	// This is another fun little class (not!)
	// It needs to move the TQRect so that it touches the closest outside line of the MonitorRegion
	bool should_move;
	long rect_center_x = rect.x() + (rect.width()/2);
	long rect_center_y = rect.y() + (rect.height()/2);

	// First, break the region into the series of source rectangles
	TQMemArray<TQRect> rectangles = region.rects();

	// Next, find which rectangle is closest to the center of the TQRect
	should_move = false;
	int closest = 0;
	long distance = 16384*16384;
	int fallback_mode;
	long test_distance;
	long test_center_x;
	long test_center_y;
	for ( int i = 0; i < rectangles.size(); i++ ) {
		test_center_x = rectangles[i].x() + (rectangles[i].width()/2);
		test_center_y = rectangles[i].y() + (rectangles[i].height()/2);
		test_distance = pow(test_center_x-rect_center_x,2) + pow(test_center_y-rect_center_y,2);
		if ( (abs(test_center_x-(workspace.width()/2))<2) && (abs(test_center_y-(workspace.height()/2))<2) ) {
			test_distance=0;	// Give the primary monitor "gravity" so it can attract all other monitors to itself
		}
		if (test_distance < distance) {
			// Make sure this is an outer rectangle; i,e. there is empty space where
			// we would want to move the TQRect...
			// To do that we will move the TQRect in all four possible directions,
			// and see if any put the TQRect in an empty location
			// If they do, then this current rectangle is considered usable
			// and it is added to the distance checking routine.
			TQPoint test_moveby = moveTQRectSoThatItTouchesTQRect(rectangles[i], rect, 0);
			TQRect test_rect = rect;
			test_rect.moveBy(test_moveby.x(), test_moveby.y());
			if (!region.contains(test_rect)) {
				closest = i;
				distance = test_distance;
				fallback_mode = 0;
				should_move = true;
			}
		}
	}

	// Finally, calculate the required translation to move the TQRect outside the MonitorRegion
	// so that it touches the closest line found above
	if (should_move)
		return moveTQRectSoThatItTouchesTQRect(rectangles[closest], rect, fallback_mode);
	else
		return TQPoint(0, 0);
}

void KDisplayConfig::updateDraggableMonitorInformation (int monitor_id) {
	updateDraggableMonitorInformationInternal(monitor_id, true);
}

void KDisplayConfig::updateDraggableMonitorInformationInternal (int monitor_id, bool recurse) {
	int i;
	int j;
	DraggableMonitor *primary_monitor;
	DraggableMonitor *moved_monitor;
	SingleScreenData *screendata;
	TQObjectList monitors;

	// Find the moved draggable monitor object
	monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
				if (monitor->screen_id == monitor_id) {
					moved_monitor = monitor;
					screendata = m_screenInfoArray.at(moved_monitor->screen_id);
				}
			}
		}
	}

	if (screendata->is_extended) {
		moved_monitor->show();
	}
	else {
		moved_monitor->hide();
	}

	// Handle resizing
	moved_monitor->setFixedSize(screendata->current_x_pixel_count*base->monitorPhyArrange->resize_factor, screendata->current_y_pixel_count*base->monitorPhyArrange->resize_factor);

	// Find the primary monitor
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (screendata->is_primary)
			j=i;
	}
	monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
				if (monitor->screen_id == j)
					primary_monitor = monitor;
			}
		}
	}

	if (moved_monitor != primary_monitor) {
		// Run layout rules
		applyMonitorLayoutRules(moved_monitor);

		int toffset_x = moved_monitor->x() - ((base->monitorPhyArrange->width()/2)-(primary_monitor->width()/2));
		int toffset_y = moved_monitor->y() - ((base->monitorPhyArrange->height()/2)-(primary_monitor->height()/2));
	
		int offset_x = toffset_x / base->monitorPhyArrange->resize_factor;
		int offset_y = toffset_y / base->monitorPhyArrange->resize_factor;
	
		screendata = m_screenInfoArray.at(monitor_id);
		screendata->absolute_x_position = offset_x;
		screendata->absolute_y_position = offset_y;
	}
	else {
		// Reset the position of the primary monitor
		moveMonitor(primary_monitor, 0, 0);
	}

	layoutDragDropDisplay();

// 	// FIXME Yes, this should work.  For some reason it creates big problems instead
// 	// Run layout rules on all monitors
// 	if (recurse == true) {
// 		applyMonitorLayoutRules();
// 	}
}

bool KDisplayConfig::applyMonitorLayoutRules() {
	int i;
	for (i=0;i<numberOfScreens;i++) {
		updateDraggableMonitorInformationInternal(i, false);
	}

	return false;
}

bool KDisplayConfig::applyMonitorLayoutRules(DraggableMonitor* monitor_to_move) {
	int i;
	int j;
	bool monitor_was_moved;
	SingleScreenData *screendata;
	TQObjectList monitors;

	// Ensure that the monitors:
	// 1) Do NOT overlap
	// 2) Touch on at least one side
	monitor_was_moved = false;

	// Handle 1)

	// First, create a region from the monitor rectangles
	// The moved monitor cannot exist within that region
	MonitorRegion other_monitors;
	monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
				if (monitor != monitor_to_move) {
					other_monitors = other_monitors.unite(MonitorRegion(monitor->geometry()));
				}
			}
		}
	}

	// Now get the required move X/Y direction
	TQPoint req_move = moveTQRectOutsideMonitorRegion(monitor_to_move->geometry(), other_monitors);

	// And move the monitor
	if (!monitor_to_move->isHidden())
		monitor_to_move->move(monitor_to_move->x()+req_move.x(), monitor_to_move->y()+req_move.y());
	else {
		req_move.setX(0);
		req_move.setY(0);
		monitor_to_move->move(base->monitorPhyArrange->width(), base->monitorPhyArrange->height());
	}

	if ((req_move.x() != 0) || (req_move.y() != 0))
		monitor_was_moved = true;

	// Handle 2)
	// Now we need to shrink the monitors so that no gaps appear between then
	// All shrinking must take place towards the nearest extant monitor edge

	// First, determine which rectangles touch the primary monitor, or touch rectangles that then
	// in turn touch the primary monitor
	// FIXME

	// Only run this routine if we don't touch the primary monitor, or touch any rectangles that
	// actually do touch the primary monitor (possible through other rectangles, etc.)...
	// This would be for efficiency
	// FIXME
// 	if () {
		TQPoint req_move2(-1,-1);
		while ((req_move2.x() != 0) || (req_move2.y() != 0)) {
			// First, create a region from the monitor rectangles
			MonitorRegion other_monitors2;
			monitors = base->monitorPhyArrange->childrenListObject();
			if ( monitors.count() ) {
				for ( i = 0; i < int(monitors.count()); ++i ) {
					if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
						DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
						if (monitor != monitor_to_move) {
							other_monitors2 = other_monitors2.unite(MonitorRegion(monitor->geometry()));
						}
					}
				}
			}
	
			// Now get the required move X/Y direction
			req_move2 = compressTQRectTouchingMonitorRegion(monitor_to_move->geometry(), other_monitors, base->monitorPhyArrange->size());
		
			// And move the monitor
			if (!monitor_to_move->isHidden())
				monitor_to_move->move(monitor_to_move->x()+req_move2.x(), monitor_to_move->y()+req_move2.y());
			else {
				req_move2.setX(0);
				req_move2.setY(0);
				monitor_to_move->move(base->monitorPhyArrange->width(), base->monitorPhyArrange->height());
			}
	
			if ((req_move2.x() != 0) || (req_move2.y() != 0))
				monitor_was_moved = true;
		}
// 	}

	return monitor_was_moved;
}

void KDisplayConfig::moveMonitor(DraggableMonitor* monitor, int realx, int realy) {
	int i;
	int j;
	DraggableMonitor *primary_monitor;
	SingleScreenData *screendata;

	// Find the primary monitor
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (screendata->is_primary)
			j=i;
	}
	TQObjectList monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
				if (monitor->screen_id == j)
					primary_monitor = monitor;
			}
		}
	}

	int tx = realx * base->monitorPhyArrange->resize_factor;
	int ty = realy * base->monitorPhyArrange->resize_factor;

	if (!monitor->isHidden())
		monitor->move((base->monitorPhyArrange->width()/2)-(primary_monitor->width()/2)+tx,(base->monitorPhyArrange->height()/2)-(primary_monitor->height()/2)+ty);
	else
		monitor->move(base->monitorPhyArrange->width(), base->monitorPhyArrange->height());
}

int KDisplayConfig::realResolutionSliderValue() {
	return base->resolutionSlider->maxValue() - base->resolutionSlider->value();
}

void KDisplayConfig::setRealResolutionSliderValue(int index) {
	base->resolutionSlider->setValue(base->resolutionSlider->maxValue() - index);
}

/**** KDisplayConfig ****/

KDisplayConfig::KDisplayConfig(TQWidget *parent, const char *name, const TQStringList &)
  : KCModule(KDisplayCFactory::instance(), parent, name), m_randrsimple(0)
{

	m_randrsimple = new KRandrSimpleAPI();
	
	TQVBoxLayout *layout = new TQVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
	systemconfig = new KSimpleConfig( TQString::tqfromLatin1( KDE_CONFDIR "/kdisplay/kdisplayconfigrc" ));
	
	KAboutData *about =
		new KAboutData(I18N_NOOP("kcmdisplayconfig"), I18N_NOOP("TDE Display Profile Control Module"),
			0, 0, KAboutData::License_GPL,
			I18N_NOOP("(c) 2011 Timothy Pearson"));

	about->addAuthor("Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net");
	setAboutData( about );
	
	base = new DisplayConfigBase(this);
	layout->add(base);
	
	setRootOnlyMsg(i18n("<b>The global display configuration is a system wide setting, and requires administrator access</b><br>To alter the system's global display configuration, click on the \"Administrator Mode\" button below."));
	setUseRootOnlyMsg(true);

	connect(base->systemEnableSupport, TQT_SIGNAL(clicked()), TQT_SLOT(changed()));
	connect(base->systemEnableSupport, TQT_SIGNAL(clicked()), TQT_SLOT(processLockoutControls()));
	connect(base->monitorDisplaySelectDD, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
	connect(base->resolutionSlider, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(resolutionSliderChanged(int)));
	connect(base->monitorDisplaySelectDD, TQT_SIGNAL(activated(int)), TQT_SLOT(selectScreen(int)));
	connect(base->monitorPhyArrange, TQT_SIGNAL(workspaceRelayoutNeeded()), this, TQT_SLOT(layoutDragDropDisplay()));

	connect(base->isPrimaryMonitorCB, TQT_SIGNAL(clicked()), TQT_SLOT(changed()));
	connect(base->isPrimaryMonitorCB, TQT_SIGNAL(clicked()), TQT_SLOT(ensurePrimaryMonitorIsAvailable()));
	connect(base->isExtendedMonitorCB, TQT_SIGNAL(clicked()), TQT_SLOT(changed()));
	connect(base->isExtendedMonitorCB, TQT_SIGNAL(clicked()), TQT_SLOT(updateExtendedMonitorInformation()));

	connect(base->systemEnableSupport, TQT_SIGNAL(toggled(bool)), base->monitorDisplaySelectDD, TQT_SLOT(setEnabled(bool)));

	load();

	addTab( "iccconfig", i18n( "Color Profiles" ) );	// [FIXME[ No way to save settings here yet

	processLockoutControls();
}

KDisplayConfig::~KDisplayConfig()
{
	delete systemconfig;
	if (m_randrsimple) {
		delete m_randrsimple;
		m_randrsimple = 0;
	}
}

void KDisplayConfig::updateExtendedMonitorInformation () {
	SingleScreenData *screendata;

	screendata = m_screenInfoArray.at(base->monitorDisplaySelectDD->currentItem());
	screendata->is_extended = base->isExtendedMonitorCB->isChecked();

	updateDisplayedInformation();
}

void KDisplayConfig::deleteProfile () {

}

void KDisplayConfig::renameProfile () {

}

void KDisplayConfig::addProfile () {

}

void KDisplayConfig::load()
{
	load( false );
}

void KDisplayConfig::selectProfile (int slotNumber) {

}

void KDisplayConfig::selectScreen (int slotNumber) {
	base->monitorDisplaySelectDD->setCurrentItem(slotNumber);
	updateDisplayedInformation();
}

void KDisplayConfig::updateArray (void) {
	// Discover display information
	int i;
	int j;

	XRROutputInfo *output_info;

	// Clear existing info
	SingleScreenData *screendata;
	for ( screendata = m_screenInfoArray.first(); screendata; screendata = m_screenInfoArray.next() ) {
		m_screenInfoArray.remove(screendata);
		delete screendata;
	}
	
	numberOfScreens = 0;
	if (m_randrsimple->isValid() == true) {
		randr_display = XOpenDisplay(NULL);
		randr_screen_info = m_randrsimple->read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;

			// Create new data object
			screendata = new SingleScreenData;
			m_screenInfoArray.append(screendata);
			screendata->screenFriendlyName = TQString(i18n("%1. %2 output on %3")).arg(i+1).arg(capitalizeString(output_info->name)).arg(":0");	// [FIXME] How can I get the name of the Xorg graphics driver currently in use?
			screendata->generic_screen_detected = false;

			// Attempt to use KMS to find screen EDID and name
			TQString edid = getEDIDMonitorName(0, output_info->name);	// [FIXME] Don't hardwire to card 0!
			if (!edid.isNull()) {
				screendata->screenFriendlyName = TQString(i18n("%1. %2 on %3 on card %4")).arg(i+1).arg(edid).arg(capitalizeString(output_info->name)).arg("0");	// [FIXME] How can I get the name of the Xorg graphics driver currently in use?
			}

			// Get resolutions
			RandRScreen *cur_screen = m_randrsimple->screen(i);
			if (cur_screen) {
				screendata->screen_connected = true;
				for (int j = 0; j < cur_screen->numSizes(); j++) {
					screendata->resolutions.append(i18n("%1 x %2").tqarg(cur_screen->pixelSize(j).width()).tqarg(cur_screen->pixelSize(j).height()));
				}
				screendata->current_resolution_index = cur_screen->proposedSize();
	
				// Get refresh rates
				TQStringList rr = cur_screen->refreshRates(screendata->current_resolution_index);
				for (TQStringList::Iterator it = rr.begin(); it != rr.end(); ++it) {
					screendata->refresh_rates.append(*it);
				}
				screendata->current_refresh_rate_index = cur_screen->proposedRefreshRate();
	
				// Get color depths
				// [FIXME]
				screendata->color_depths.append(i18n("Default"));
				screendata->current_color_depth_index = 0;
	
				// Get orientation flags
				// RandRScreen::Rotate0
				// RandRScreen::Rotate90
				// RandRScreen::Rotate180
				// RandRScreen::Rotate270
				// RandRScreen::ReflectX
				// RandRScreen::ReflectY
	
				screendata->rotations.append(i18n("Normal"));
				screendata->rotations.append(i18n("Rotate 90 degrees"));
				screendata->rotations.append(i18n("Rotate 180 degrees"));
				screendata->rotations.append(i18n("Rotate 270 degrees"));
				screendata->current_orientation_mask = cur_screen->proposedRotation();
				switch (screendata->current_orientation_mask & RandRScreen::RotateMask) {
					case RandRScreen::Rotate0:
						screendata->current_rotation_index = 0;
						break;
					case RandRScreen::Rotate90:
						screendata->current_rotation_index = 1;
						break;
					case RandRScreen::Rotate180:
						screendata->current_rotation_index = 2;
						break;
					case RandRScreen::Rotate270:
						screendata->current_rotation_index = 3;
						break;
					default:
						// Shouldn't hit this one
						Q_ASSERT(screendata->current_orientation_mask & RandRScreen::RotateMask);
						break;
				}
				screendata->has_x_flip = (screendata->current_orientation_mask & RandRScreen::ReflectX);
				screendata->has_y_flip = (screendata->current_orientation_mask & RandRScreen::ReflectY);
				screendata->supports_transformations = (cur_screen->rotations() != RandRScreen::Rotate0);
	
				// Determine if this display is primary and/or extended
				// [FIXME]
				screendata->is_primary = false;
				screendata->is_extended = false;
	
				// Get this screen's absolute position
				// [FIXME]
				screendata->absolute_x_position = 0;
				screendata->absolute_y_position = 0;
	
				// Get this screen's current resolution
				screendata->current_x_pixel_count = cur_screen->pixelSize(screendata->current_resolution_index).width();
				screendata->current_y_pixel_count = cur_screen->pixelSize(screendata->current_resolution_index).height();
			}
			else {
				// Fill in generic data for this disconnected output
				screendata->screenFriendlyName = screendata->screenFriendlyName + TQString(" (") + i18n("disconnected") + TQString(")");
				screendata->screen_connected = false;

				screendata->resolutions = i18n("Default");
				screendata->refresh_rates = i18n("Default");
				screendata->color_depths = i18n("Default");
				screendata->rotations = i18n("N/A");
				
				screendata->current_resolution_index = 0;
				screendata->current_refresh_rate_index = 0;
				screendata->current_color_depth_index = 0;
				
				screendata->current_rotation_index = 0;
				screendata->current_orientation_mask = 0;
				screendata->has_x_flip = false;
				screendata->has_y_flip = false;
				screendata->supports_transformations = false;
				
				screendata->is_primary = false;
				screendata->is_extended = false;
				screendata->absolute_x_position = 0;
				screendata->absolute_y_position = 0;
				screendata->current_x_pixel_count = 640;
				screendata->current_y_pixel_count = 480;
			}

			// Check for more screens...
			numberOfScreens++;
		}
	}
	else {
		screendata = new SingleScreenData;
		m_screenInfoArray.append(screendata);

		// Fill in a bunch of generic data
		screendata->screenFriendlyName = i18n("Default output on generic video card");
		screendata->generic_screen_detected = true;
		screendata->screen_connected = true;
		
		screendata->resolutions = i18n("Default");
		screendata->refresh_rates = i18n("Default");
		screendata->color_depths = i18n("Default");
		screendata->rotations = i18n("N/A");
		
		screendata->current_resolution_index = 0;
		screendata->current_refresh_rate_index = 0;
		screendata->current_color_depth_index = 0;
		
		screendata->current_rotation_index = 0;
		screendata->current_orientation_mask = 0;
		screendata->has_x_flip = false;
		screendata->has_y_flip = false;
		screendata->supports_transformations = false;
		
		screendata->is_primary = true;
		screendata->is_extended = true;
		screendata->absolute_x_position = 0;
		screendata->absolute_y_position = 0;
		screendata->current_x_pixel_count = 640;
		screendata->current_y_pixel_count = 480;

		numberOfScreens++;
	}

	// [FIXME]
	// Set this on the real primary monitor only!
	screendata = m_screenInfoArray.at(0);
	screendata->is_primary = true;
}

TQString KDisplayConfig::getEDIDMonitorName(int card, TQString displayname) {
	TQString edid;
	TQByteArray binaryedid = getEDID(card, displayname); 
	if (binaryedid.isNull())
		return TQString();

	// Get the manufacturer ID
	unsigned char letter_1 = ((binaryedid[8]>>2) & 0x1F) + 0x40;
	unsigned char letter_2 = (((binaryedid[8] & 0x03) << 3) | ((binaryedid[9]>>5) & 0x07)) + 0x40;
	unsigned char letter_3 = (binaryedid[9] & 0x1F) + 0x40;
	TQChar qletter_1 = TQChar(letter_1);
	TQChar qletter_2 = TQChar(letter_2);
	TQChar qletter_3 = TQChar(letter_3);
	TQString manufacturer_id = TQString("%1%2%3").arg(qletter_1).arg(qletter_2).arg(qletter_3);

	// Get the model ID
	unsigned int raw_model_id = (((binaryedid[10] << 8) | binaryedid[11]) << 16) & 0xFFFF0000;
	// Reverse the bit order
	unsigned int model_id = reverse_bits(raw_model_id);

	// Try to get the model name
	bool has_friendly_name = false;
	unsigned char descriptor_block[18];
	int i;
	for (i=72;i<90;i++) {
		descriptor_block[i-72] = binaryedid[i] & 0xFF;
	}
	if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
		for (i=90;i<108;i++) {
			descriptor_block[i-90] = binaryedid[i] & 0xFF;
		}
		if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
			for (i=108;i<126;i++) {
				descriptor_block[i-108] = binaryedid[i] & 0xFF;
			}
		}
	}

	TQString monitor_name;
	if ((descriptor_block[0] == 0) && (descriptor_block[1] == 0) && (descriptor_block[3] == 0xFC)) {
		char* pos = strchr((char *)(descriptor_block+5), '\n');
		if (pos) {
			*pos = 0;
			has_friendly_name = true;
			monitor_name = TQString((char *)(descriptor_block+5));
		}
		else {
			has_friendly_name = false;
		}
	}

	// [FIXME]
	// Look up manudacturer names if possible!

	if (has_friendly_name)
		edid = TQString("%1 %2").arg(manufacturer_id).arg(monitor_name);
	else
		edid = TQString("%1 0x%2").arg(manufacturer_id).arg(model_id, 0, 16);

	return edid;
}

TQByteArray KDisplayConfig::getEDID(int card, TQString displayname) {
	TQFile file(TQString("/sys/class/drm/card%1-%2/edid").arg(card).arg(displayname));
	if (!file.open (IO_ReadOnly))
		return TQByteArray();
	TQByteArray binaryedid = file.readAll();
	file.close();
	return binaryedid;
}

void KDisplayConfig::updateDisplayedInformation () {
	// Insert data into the GUI
	int i;
	SingleScreenData *screendata;

	ensureMonitorDataConsistency();

	screendata = m_screenInfoArray.at(base->monitorDisplaySelectDD->currentItem());

	if (screendata->screen_connected) {
		base->resolutionSlider->setEnabled(true);
		base->refreshRateDD->setEnabled(true);
		base->rotationSelectDD->setEnabled(true);
		base->orientationHFlip->setEnabled(true);
		base->orientationVFlip->setEnabled(true);
		base->isPrimaryMonitorCB->setEnabled(true);
		base->isExtendedMonitorCB->setEnabled(true);
	}

	// Update the resolutions for the selected screen
	base->resolutionSlider->blockSignals(true);
	base->resolutionSlider->setMaxValue(screendata->refresh_rates.count());
	setRealResolutionSliderValue(screendata->current_resolution_index);
	resolutionSliderTextUpdate(realResolutionSliderValue());
	base->resolutionSlider->blockSignals(false);

	// Now the refresh rates for the selected screen
	base->refreshRateDD->blockSignals(true);
	base->refreshRateDD->clear();
	for (i=0;i<screendata->refresh_rates.count();i++) {
		base->refreshRateDD->insertItem(screendata->refresh_rates[i], i);
	}
	base->refreshRateDD->setCurrentItem(screendata->current_refresh_rate_index);
	base->refreshRateDD->blockSignals(false);

	// Now the rotations and transformations for the selected screen
	base->rotationSelectDD->blockSignals(true);
	base->orientationHFlip->blockSignals(true);
	base->orientationVFlip->blockSignals(true);
	base->rotationSelectDD->clear();
	if (screendata->supports_transformations) {
		for (i=0;i<screendata->rotations.count();i++) {
			base->rotationSelectDD->insertItem(screendata->rotations[i], i);
		}
		base->rotationSelectDD->setCurrentItem(screendata->current_rotation_index);
		base->orientationHFlip->show();
		base->orientationVFlip->show();
		base->orientationHFlip->setChecked(screendata->has_x_flip);
		base->orientationVFlip->setChecked(screendata->has_y_flip);
	}
	else {
		base->rotationSelectDD->insertItem("Normal", 0);
		base->rotationSelectDD->setCurrentItem(0);
		base->orientationHFlip->hide();
		base->orientationVFlip->hide();

	}
	base->rotationSelectDD->blockSignals(false);
	base->orientationHFlip->blockSignals(false);
	base->orientationVFlip->blockSignals(false);

	base->isPrimaryMonitorCB->blockSignals(true);
	base->isExtendedMonitorCB->blockSignals(true);
	if (screendata->generic_screen_detected) {
		base->isPrimaryMonitorCB->setEnabled(false);
		base->isPrimaryMonitorCB->setChecked(true);
		base->isExtendedMonitorCB->setEnabled(false);
		base->isExtendedMonitorCB->setChecked(true);
	}
	else {
		base->isPrimaryMonitorCB->setEnabled(true);
		base->isPrimaryMonitorCB->setChecked(screendata->is_primary);
		if (screendata->is_primary) {
			base->isExtendedMonitorCB->setEnabled(false);
			base->isExtendedMonitorCB->setChecked(true);
		}
		else {
			base->isExtendedMonitorCB->setEnabled(true);
			base->isExtendedMonitorCB->setChecked(screendata->is_extended);
		}
	}
	base->isPrimaryMonitorCB->blockSignals(false);
	base->isExtendedMonitorCB->blockSignals(false);

	if (!screendata->screen_connected) {
		base->resolutionSlider->setEnabled(false);
		base->refreshRateDD->setEnabled(false);
		base->rotationSelectDD->setEnabled(false);
		base->orientationHFlip->setEnabled(false);
		base->orientationVFlip->setEnabled(false);
		base->isPrimaryMonitorCB->setEnabled(false);
		base->isExtendedMonitorCB->setEnabled(false);
	}
}

void KDisplayConfig::refreshDisplayedInformation () {
	// Insert data into the GUI
	int i;
	SingleScreenData *screendata;

	// First, the screens
	int currentScreenIndex = base->monitorDisplaySelectDD->currentItem();
	base->monitorDisplaySelectDD->clear();
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		base->monitorDisplaySelectDD->insertItem(screendata->screenFriendlyName, i);
	}
	base->monitorDisplaySelectDD->setCurrentItem(currentScreenIndex);

	updateDisplayedInformation();

	updateDragDropDisplay();
}

void KDisplayConfig::updateDragDropDisplay() {
	// Insert data into the GUI
	int i;
	int largest_x_pixels;
	int largest_y_pixels;
	TQObjectList monitors;
	SingleScreenData *screendata;

	// Clear any screens from the workspace
	monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				TQWidget *monitor = TQT_TQWIDGET(monitors.at( i ));
				if ( !monitor->close(TRUE) ) {
					Q_ASSERT("zombie monitor will not go away!");
				}
			}
		}
	}

	int currentScreenIndex = base->monitorDisplaySelectDD->currentItem();

	// Add the screens to the workspace
	// Set the scaling small to start with
	base->monitorPhyArrange->resize_factor = 0.0625;	// This always needs to divide by a multiple of 2
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		DraggableMonitor *m = new DraggableMonitor( base->monitorPhyArrange, 0, WStyle_Customize | WDestructiveClose | WStyle_NoBorder | WX11BypassWM );
		connect(m, TQT_SIGNAL(workspaceRelayoutNeeded()), this, TQT_SLOT(layoutDragDropDisplay()));
		connect(m, TQT_SIGNAL(monitorSelected(int)), this, TQT_SLOT(selectScreen(int)));
		connect(m, TQT_SIGNAL(monitorDragComplete(int)), this, TQT_SLOT(updateDraggableMonitorInformation(int)));
		m->screen_id = i;
		m->setFixedSize(screendata->current_x_pixel_count*base->monitorPhyArrange->resize_factor, screendata->current_y_pixel_count*base->monitorPhyArrange->resize_factor);
		m->setText(TQString("%1").arg(i+1));
		m->show();
		updateDraggableMonitorInformation(i);	// Make sure the new monitors don't overlap
	}

	layoutDragDropDisplay();
}

void KDisplayConfig::layoutDragDropDisplay() {
	int i;
	int largest_x_pixels;
	int largest_y_pixels;
	TQObjectList monitors;
	SingleScreenData *screendata;

	// Ensure data is consistent
	ensureMonitorDataConsistency();

	// Arrange the screens
	// First, center the primary monitor
	monitors = base->monitorPhyArrange->childrenListObject();
	if ( monitors.count() ) {
		for ( i = 0; i < int(monitors.count()); ++i ) {
			if (::tqqt_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )))) {
				DraggableMonitor *monitor = static_cast<DraggableMonitor*>(TQT_TQWIDGET(monitors.at( i )));
				screendata = m_screenInfoArray.at(monitor->screen_id);
				moveMonitor(monitor, screendata->absolute_x_position, screendata->absolute_y_position);
			}
		}
	}
}

void KDisplayConfig::ensureMonitorDataConsistency() {
	int i;
	SingleScreenData *screendata;

	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (!screendata->screen_connected) {
			screendata->is_primary = false;
			screendata->is_extended = false;
		}
	}

	bool has_primary_monitor = false;
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (screendata->is_primary)
			has_primary_monitor = true;
	}
	if (!has_primary_monitor) {
		for (i=0;i<numberOfScreens;i++) {
			screendata = m_screenInfoArray.at(i);
			if (!has_primary_monitor) {
				if (screendata->is_extended) {
					screendata->is_primary = true;
					screendata->is_extended = true;
					has_primary_monitor = true;
				}
			}
		}
	}
	if (!has_primary_monitor) {
		for (i=0;i<numberOfScreens;i++) {
			screendata = m_screenInfoArray.at(i);
			if (!has_primary_monitor) {
				if (screendata->screen_connected) {
					screendata->is_primary = true;
					screendata->is_extended = true;
					has_primary_monitor = true;
				}
			}
		}
	}

	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (screendata->is_primary) {
			screendata->absolute_x_position = 0;
			screendata->absolute_y_position = 0;
			screendata->is_extended = true;
		}
	}

	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		TQString resolutionstring = screendata->resolutions[screendata->current_resolution_index];
		int separator_pos = resolutionstring.find(" x ");
		TQString x_res_string = resolutionstring.left(separator_pos);
		TQString y_res_string = resolutionstring.right(resolutionstring.length()-separator_pos-3);
		screendata->current_x_pixel_count = x_res_string.toInt();
		screendata->current_y_pixel_count = y_res_string.toInt();
	}
}

void KDisplayConfig::resolutionSliderTextUpdate(int index) {
	SingleScreenData *screendata;
	screendata = m_screenInfoArray.at(base->monitorDisplaySelectDD->currentItem());

	base->resolutionLabel->setText(screendata->resolutions[realResolutionSliderValue()] + TQString(" ") + i18n("pixels"));
}

void KDisplayConfig::resolutionSliderChanged(int index) {
	SingleScreenData *screendata;
	screendata = m_screenInfoArray.at(base->monitorDisplaySelectDD->currentItem());

	screendata->current_resolution_index = realResolutionSliderValue();
	updateDisplayedInformation();
	updateDraggableMonitorInformation(base->monitorDisplaySelectDD->currentItem());
}

TQString KDisplayConfig::extractFileName(TQString displayName, TQString profileName) {

}

void KDisplayConfig::ensurePrimaryMonitorIsAvailable() {
	// Ensure that only one monitor, and not less than one monitor, is marked as primary
	int i;
	SingleScreenData *screendata;

	// First, the screens
	int currentScreenIndex = base->monitorDisplaySelectDD->currentItem();
	for (i=0;i<numberOfScreens;i++) {
		screendata = m_screenInfoArray.at(i);
		if (i != currentScreenIndex)
			screendata->is_primary = false;
	}
	screendata = m_screenInfoArray.at(currentScreenIndex);
	screendata->is_primary = true;
	refreshDisplayedInformation();
}

int KDisplayConfig::findProfileIndex(TQString profileName) {

}

int KDisplayConfig::findScreenIndex(TQString screenName) {

}

void KDisplayConfig::processLockoutControls() {
	if (getuid() != 0 || !systemconfig->checkConfigFilesWritable( true )) {
		base->globalTab->setEnabled(false);
		base->resolutionTab->setEnabled(false);
	}
	else {
		base->globalTab->setEnabled(true);
		if (base->systemEnableSupport->isChecked()) {
			base->resolutionTab->setEnabled(true);
		}
		else {
			base->resolutionTab->setEnabled(false);
		}
	}
}

void KDisplayConfig::addTab( const TQString name, const TQString label )
{
	// [FIXME] This is incomplete...Apply may not work...
	TQWidget *page = new TQWidget( base->mainTabContainerWidget, name.latin1() );
	TQVBoxLayout *top = new TQVBoxLayout( page, KDialog::marginHint() );
	
	KCModule *kcm = KCModuleLoader::loadModule( name, page );
	
	if ( kcm )
	{
		top->addWidget( kcm );
		base->mainTabContainerWidget->addTab( page, label );
		
		connect( kcm, TQT_SIGNAL( changed(bool) ), this, TQT_SLOT( changed() ) );
		//m_modules.insert(kcm, false);
	}
	else {
		delete page;
	}
}

void KDisplayConfig::load(bool useDefaults )
{
	// Update the toggle buttons with the current configuration
	int i;
	int j;
	
	updateArray();
	
	systemconfig->setGroup(NULL);
	base->systemEnableSupport->setChecked(systemconfig->readBoolEntry("EnableDisplayControl", false));
	
	refreshDisplayedInformation();
	
	emit changed(useDefaults);
}

void KDisplayConfig::save()
{
	int i;
	int j;
	KRandrSimpleAPI *m_randrsimple = new KRandrSimpleAPI();

	// Write system configuration
	systemconfig->setGroup(NULL);
	systemconfig->writeEntry("EnableDisplayControl", base->systemEnableSupport->isChecked());


	systemconfig->sync();

	emit changed(false);
}

void KDisplayConfig::defaults()
{
	load( true );
}

TQString KDisplayConfig::quickHelp() const
{
	return i18n("<h1>Monitor & Display Configuration</h1> This module allows you to configure monitors attached to your"
	" computer via TDE.");
}

#include "displayconfig.moc"
