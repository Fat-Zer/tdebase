/*************************************************************************
 
    windowdrag.h  - The windowDrag object, used to drag windows across
                     desktops
    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
    Send comments and bug fixes to larrosa@kde.org
 
*************************************************************************/
#ifndef WINDOWDRAG_H
#define WINDOWDRAG_H

#include <tqdragobject.h>
#include <X11/Xlib.h>

class PagerWindowDrag : public TQStoredDrag
{
public:
    PagerWindowDrag(WId w,int deltax,int deltay,int origdesk,TQWidget *parent);
    virtual ~PagerWindowDrag();

    static bool canDecode( TQDragMoveEvent *e);
    static bool decode ( TQDropEvent *e, WId &w,int &deltax,int &deltay,int &origdesk);
    
};

#endif
