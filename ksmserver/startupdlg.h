/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef STARTUPDLG_H
#define STARTUPDLG_H

#include <tqpixmap.h>
#include <tqimage.h>
#include <tqdatetime.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <tqpushbutton.h>
#include <tqframe.h>
#include <kguiitem.h>
#include <tqtoolbutton.h>
#include <ksharedpixmap.h>

class TQPushButton;
class TQVButtonGroup;
class TQPopupMenu;
class TQTimer;
class TQPainter;
class TQString;
class KAction;

#include "timed.h"
#include <kapplication.h>
#include <kpixmapio.h>

#include <config.h>

#ifndef NO_QT3_DBUS_SUPPORT
/* We acknowledge the the dbus API is unstable */
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/connection.h>
#endif // NO_QT3_DBUS_SUPPORT

#ifdef COMPILE_HALBACKEND
#include <hal/libhal.h>
#endif

// The startup-in-progress dialog
class KSMStartupIPDlg : public KSMModalDialog
{
    Q_OBJECT

public:
    static TQWidget* showStartupIP();

protected:
    ~KSMStartupIPDlg();

private:
    KSMStartupIPDlg( TQWidget* parent );
};

#endif
