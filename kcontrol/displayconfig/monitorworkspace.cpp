/**
 * monitorworkspace.cpp
 *
 * Copyright (c) 2011 Timothy Pearson <kb9vqf@pearsoncomputing.net>
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

#include <unistd.h>
#include <string>
#include <stdio.h>
#include <tqstring.h>

#include "monitorworkspace.h"

using namespace std;

/**** Monitor Region ****/

MonitorRegion::MonitorRegion()
{
}

MonitorRegion::MonitorRegion(TQRect rect)
{
	rectangles.resize(1);
	rectangles[0] = rect;
}

MonitorRegion::MonitorRegion(TQMemArray<TQRect> newrects)
{
	rectangles = newrects;
}

MonitorRegion::~MonitorRegion()
{
}

TQMemArray<TQRect> MonitorRegion::rects()
{
	return rectangles;
}

MonitorRegion MonitorRegion::unite(MonitorRegion rect)
{
	int i;
	int j;
	TQMemArray<TQRect> newrectangles = rectangles.copy();	// This MUST be a copy, otherwise Bad Things will happen VERY quickly!
	newrectangles.resize(rectangles.count() + rect.rects().count());
	j=0;
	for (i=rectangles.count();i<newrectangles.count();i++) {
		newrectangles[i] = rect.rects()[j];
		j++;
	}
	MonitorRegion newregion(newrectangles);
	return newregion;
}

bool MonitorRegion::contains(TQRect rect)
{
	int i;
	for (i=0;i<rectangles.count();i++) {
		if (rectangles[i].intersects(rect))
			return true;
	}
	return false;
}

/**** Draggable Monitor Widget ****/
DraggableMonitor::DraggableMonitor( TQWidget* parent, const char* name, int wflags )
	: TQLabel( parent, name, wflags )
{
	setAlignment(AlignHCenter | AlignVCenter);
	setFrameShape(Box);
	setFrameShadow(Plain);
	setLineWidth(4);
	setMidLineWidth(4);
}

DraggableMonitor::~DraggableMonitor()
{

}

void DraggableMonitor::mousePressEvent(TQMouseEvent *event)
{
	lastMousePosition = event->pos();
	emit(monitorSelected(screen_id));
}

void DraggableMonitor::mouseMoveEvent(TQMouseEvent *event)
{
	TQPoint mousePos = event->pos();
	TQPoint mouseMove = TQPoint(mousePos.x() - lastMousePosition.x(), mousePos.y() - lastMousePosition.y());

	int moveToX = x()+mouseMove.x();
	int moveToY = y()+mouseMove.y();

	int maxX = parentWidget()->width() - width() - 1;
	int maxY = parentWidget()->height() - height() - 1;

	if (moveToX < 1) moveToX = 1;
	if (moveToY < 1) moveToY = 1;
	if (moveToX > maxX) moveToX = maxX;
	if (moveToY > maxY) moveToY = maxY;

	if (!is_primary)
		move(moveToX, moveToY);
}

void DraggableMonitor::mouseReleaseEvent(TQMouseEvent *event)
{
	emit(monitorDragComplete(screen_id));
}

/**** Draggable Monitor Container ****/
MonitorWorkspace::MonitorWorkspace( TQWidget* parent, const char* name )
	: TQWorkspace( parent, name )
{

}

MonitorWorkspace::~MonitorWorkspace()
{

}

void MonitorWorkspace::resizeEvent( TQResizeEvent* re )
{
	TQWorkspace::resizeEvent(re);
	emit(workspaceRelayoutNeeded());
}

#include "monitorworkspace.moc"
