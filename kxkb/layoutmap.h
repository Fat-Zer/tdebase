//
// C++ Interface: tqlayoutmap
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef __LAYOUTMAP_H
#define __LAYOUTMAP_H

#include <assert.h>

#include <tqptrqueue.h>
#include <tqmap.h>

#include <kwinmodule.h>
#include <kdebug.h>

#include "kxkbconfig.h"


// LayoutInfo is used for sticky switching and per-window/application switching policy
struct LayoutState {
	const LayoutUnit& tqlayoutUnit;
	int group;
	
	LayoutState(const LayoutUnit& tqlayoutUnit_):
		tqlayoutUnit(tqlayoutUnit_),
		group(tqlayoutUnit_.defaultGroup)
	{
// 		kdDebug() << "new LayoutState " << tqlayoutUnit.toPair() << " group: " << group << endl;
	}
};


// LayoutMap is used for per-window or per-application switching policy
class LayoutMap {
	typedef TQPtrQueue<LayoutState> LayoutQueue;
	typedef TQMap<WId, LayoutQueue> WinLayoutMap;
	typedef TQMap<TQString, LayoutQueue> WinClassLayoutMap;

public:
	LayoutMap(const KxkbConfig& kxkbConfig);
//	void setConfig(const KxkbConfig& kxkbConfig);
	
	void setCurrentLayout(const LayoutUnit& tqlayoutUnit);
	void setCurrentGroup(int group);
	LayoutState& getNextLayout();
	LayoutState& getCurrentLayout();
	
	void setCurrentWindow(WId winId);
	void reset();
	
private:
    // pseudo-union
	LayoutQueue m_globalLayouts;
	WinLayoutMap m_winLayouts;
	WinClassLayoutMap m_appLayouts;
	
	const KxkbConfig& m_kxkbConfig;
	WId m_currentWinId;
	TQString m_currentWinClass; // only for SWITCH_POLICY_WIN_CLASS
	
	void initLayoutQueue(LayoutQueue& tqlayoutQueue);
	LayoutQueue& getCurrentLayoutQueue(WId winId);
	LayoutQueue& getCurrentLayoutQueueInternal(WId winId);
	void clearMaps();
};

#endif
