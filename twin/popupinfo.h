/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_POPUPINFO_H
#define KWIN_POPUPINFO_H
#include <tqwidget.h>
#include <tqtimer.h>
#include <tqvaluelist.h>

namespace KWinInternal
{

class Workspace;

class PopupInfo : public TQWidget
    {
    Q_OBJECT
    public:
        PopupInfo( Workspace* ws, const char *name=0 );
        ~PopupInfo();

        void reset();
        void hide();
        void showInfo(TQString infoString);

        void reconfigure();

    protected:
        void paintEvent( TQPaintEvent* );
        void paintContents();

    private:
        TQTimer m_delayedHideTimer;
        int m_delayTime;
        bool m_show;
        bool m_shown;
        TQString m_infoString;
        Workspace* workspace;
    };

} // namespace

#endif
