//
// C++ Implementation: tqlayoutmap
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "layoutmap.h"

#include "x11helper.h"


LayoutMap::LayoutMap(const KxkbConfig& kxkbConfig_):
	m_kxkbConfig(kxkbConfig_),
	m_currentWinId( X11Helper::UNKNOWN_WINDOW_ID )
{
}

// private
void LayoutMap::clearMaps()
{
	m_appLayouts.clear();
	m_winLayouts.clear();
	m_globalLayouts.clear();
	//setCurrentWindow( -1 );
}

void LayoutMap::reset()
{
	clearMaps();
	setCurrentWindow( X11Helper::UNKNOWN_WINDOW_ID );
}



void LayoutMap::setCurrentWindow(WId winId)
{
	m_currentWinId = winId;
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_WIN_CLASS )
		m_currentWinClass = X11Helper::getWindowClass(winId, qt_xdisplay());
}

// private
//LayoutQueue& 
TQPtrQueue<LayoutState>& LayoutMap::getCurrentLayoutQueueInternal(WId winId)
{
	if( winId == X11Helper::UNKNOWN_WINDOW_ID )
		return m_globalLayouts;
	
	switch( m_kxkbConfig.m_switchingPolicy ) {
		case SWITCH_POLICY_WIN_CLASS: {
//			TQString winClass = X11Helper::getWindowClass(winId, qt_xdisplay());
			return m_appLayouts[ m_currentWinClass ];
		}
		case SWITCH_POLICY_WINDOW:
			return m_winLayouts[ winId ];

		default:
			return m_globalLayouts;
	}
}

// private
//LayoutQueue& 
TQPtrQueue<LayoutState>& LayoutMap::getCurrentLayoutQueue(WId winId)
{
	TQPtrQueue<LayoutState>& tqlayoutQueue = getCurrentLayoutQueueInternal(winId);
	if( tqlayoutQueue.count() == 0 ) {
		initLayoutQueue(tqlayoutQueue);
		kdDebug() << "map: Created queue for " << winId << " size: " << tqlayoutQueue.count() << endl;
	}
	
	return tqlayoutQueue;
}

LayoutState& LayoutMap::getCurrentLayout() {
	return *getCurrentLayoutQueue(m_currentWinId).head();
}

LayoutState& LayoutMap::getNextLayout() {
	LayoutQueue& tqlayoutQueue = getCurrentLayoutQueue(m_currentWinId);
	LayoutState* tqlayoutState = tqlayoutQueue.dequeue();
	tqlayoutQueue.enqueue(tqlayoutState);
	
	kdDebug() << "map: Next tqlayout: " << tqlayoutQueue.head()->tqlayoutUnit.toPair() 
			<< " group: " << tqlayoutQueue.head()->tqlayoutUnit.defaultGroup << " for " << m_currentWinId << endl;
	
	return *tqlayoutQueue.head();
}

void LayoutMap::setCurrentGroup(int group) {
	getCurrentLayout().group = group;
}

void LayoutMap::setCurrentLayout(const LayoutUnit& tqlayoutUnit) {
	LayoutQueue& tqlayoutQueue = getCurrentLayoutQueue(m_currentWinId);
	kdDebug() << "map: Storing tqlayout: " << tqlayoutUnit.toPair() 
			<< " group: " << tqlayoutUnit.defaultGroup << " for " << m_currentWinId << endl;
	
	int queueSize = (int)tqlayoutQueue.count();
	for(int ii=0; ii<queueSize; ii++) {
		if( tqlayoutQueue.head()->tqlayoutUnit == tqlayoutUnit )
			return;	// if present return when it's in head
		
		LayoutState* tqlayoutState = tqlayoutQueue.dequeue();
		if( ii < queueSize - 1 ) {
			tqlayoutQueue.enqueue(tqlayoutState);
		}
		else {
			delete tqlayoutState;
			tqlayoutQueue.enqueue(new LayoutState(tqlayoutUnit));
		}
	}
	for(int ii=0; ii<queueSize - 1; ii++) {
		LayoutState* tqlayoutState = tqlayoutQueue.dequeue();
		tqlayoutQueue.enqueue(tqlayoutState);
	}
}

// private
void LayoutMap::initLayoutQueue(LayoutQueue& tqlayoutQueue) {
	int queueSize = ( m_kxkbConfig.m_stickySwitching ) 
			? m_kxkbConfig.m_stickySwitchingDepth : m_kxkbConfig.m_tqlayouts.count();
	for(int ii=0; ii<queueSize; ii++) {
		tqlayoutQueue.enqueue( new LayoutState(m_kxkbConfig.m_tqlayouts[ii]) );
	}
}
