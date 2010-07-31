/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 1997 to 2002 Cristian Tibirna <tibirna@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_PLACEMENT_H
#define KWIN_PLACEMENT_H

#include <tqpoint.h>
#include <tqvaluelist.h>

namespace KWinInternal
{

class Workspace;
class Client;

class Placement
    {
    public:

        Placement(Workspace* w);

        /**
         * Placement policies. How workspace decides the way windows get positioned
         * on the screen. The better the policy, the heavier the resource use.
         * Normally you don't have to worry. What the WM adds to the startup time
         * is nil compared to the creation of the window itself in the memory
         */
        enum Policy
            {
            NoPlacement, // not really a placement
            Default, // special, means to use the global default
            Unknown, // special, means the function should use its default
            Random,
            Smart,
            Cascade,
            Centered,
            ZeroCornered,
            UnderMouse, // special
            OnMainWindow, // special
            Maximizing
            };

        void place(Client* c, TQRect& area );
        
        void placeAtRandom            (Client* c, const TQRect& area, Policy next = Unknown );
        void placeCascaded            (Client* c, TQRect& area, Policy next = Unknown );
        void placeSmart               (Client* c, const TQRect& area, Policy next = Unknown );
        void placeMaximizing          (Client* c, TQRect& area, Policy next = Unknown );
        void placeCentered    (Client* c, const TQRect& area, Policy next = Unknown );
        void placeZeroCornered(Client* c, const TQRect& area, Policy next = Unknown );
        void placeDialog      (Client* c, TQRect& area, Policy next = Unknown );
        void placeUtility     (Client* c, TQRect& area, Policy next = Unknown );

        void reinitCascading( int desktop );

        static Policy policyFromString( const TQString& policy, bool no_special );
        static const char* policyToString( Policy policy );

    private:

        void place(Client* c, TQRect& area, Policy policy, Policy nextPlacement = Unknown );
        void placeUnderMouse(Client* c, TQRect& area, Policy next = Unknown );
        void placeOnMainWindow(Client* c, TQRect& area, Policy next = Unknown );
        TQRect checkArea( const Client*c, const TQRect& area );

        Placement();

    //CT needed for cascading+
        struct DesktopCascadingInfo 
            {
            TQPoint pos;
            int col;
            int row;
            };

        TQValueList<DesktopCascadingInfo> cci;

        Workspace* m_WorkspacePtr;
    };

} // namespace

#endif
