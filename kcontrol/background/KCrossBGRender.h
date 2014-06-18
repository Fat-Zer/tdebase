/*
 * Copyright (C) 2008 Danilo Cesar Lemes de Paula <danilo@mandriva.com>
 * Copyright (C) 2008 Gustavo Boiko <boiko@mandriva.com>
 * Mandriva Conectiva
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#ifndef __KCROSSBGRENDER_H__
#define __KCROSSBGRENDER_H__


#include <tqvaluelist.h>
#include <tqpixmap.h>
#include <tqvaluelist.h>
#include <tqdatetime.h>

#include "bgrender.h"

class TQDomElement;

typedef struct crossEvent{
	bool transition;
	TQString pix1;
	TQString pix2;
	TQTime stime; //start time
	TQTime etime; //end time
} KBGCrossEvent;


class KCrossBGRender: public KBackgroundRenderer{
	
TQ_OBJECT

public:
	KCrossBGRender(int desk, int screen, bool drawBackgroundPerScreen, TDEConfig *config=0);
	~KCrossBGRender();

	bool needWallpaperChange();
	void changeWallpaper(bool init=false);
	TQPixmap pixmap();
	bool usingCrossXml(){return useCrossEfect;};


private:
	TQPixmap pix;
	int secs;
	TQString xmlFileName;
	bool useCrossEfect;
	
	int actualPhase;
	 
	void createStartTime(TQDomElement e);
	void createTransition(TQDomElement e);
	void createStatic(TQDomElement e);
	bool setCurrentEvent(bool init = false);
	void initCrossFade(TQString xml);
	void fixEnabled();
	TQPixmap getCurrentPixmap();
	KBGCrossEvent current;
	TQValueList<KBGCrossEvent> timeList;
};

#endif // __KCROSSBGRENDER_H__ 
