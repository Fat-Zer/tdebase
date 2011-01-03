//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>
//

#ifndef __LOCKDLG_H__
#define __LOCKDLG_H__

#include <kgreeterplugin.h>

#include <tqdialog.h>
#include <tqstringlist.h>

#include "lockprocess.h"

struct GreeterPluginHandle;
class LockProcess;
class TQFrame;
class TQGridLayout;
class TQLabel;
class KPushButton;
class TQListView;

//===========================================================================
//
// Simple dialog for entering a password.
// It does not handle password validation.
//
class PasswordDlg : public TQDialog, public KGreeterPluginHandler
{
    Q_OBJECT

public:
    PasswordDlg(LockProcess *parent, GreeterPluginHandle *plugin);
    ~PasswordDlg();
    virtual void show();

    // from KGreetPluginHandler
    virtual void gplugReturnText( const char *text, int tag );
    virtual void gplugReturnBinary( const char *data );
    virtual void gplugSetUser( const TQString & );
    virtual void gplugStart();
    virtual void gplugActivity();
    virtual void gplugMsgBox( TQMessageBox::Icon type, const TQString &text );

protected:
    virtual void timerEvent(TQTimerEvent *);
    virtual bool eventFilter(TQObject *, TQEvent *);

private slots:
    void slotSwitchUser();
    void slotSessionActivated();
    void slotStartNewSession();
    void slotOK();
    void tqlayoutClicked();
    void slotActivity();

private:
    void setLayoutText( const TQString &txt );
    void capsLocked();
    void updateLabel();
    int Reader (void *buf, int count);
    bool GRead (void *buf, int count);
    bool GWrite (const void *buf, int count);
    bool GSendInt (int val);
    bool GSendStr (const char *buf);
    bool GSendArr (int len, const char *buf);
    bool GRecvInt (int *val);
    bool GRecvArr (char **buf);
    void handleVerify();
    void reapVerify();
    void cantCheck();
    GreeterPluginHandle *mPlugin;
    KGreeterPlugin *greet;
    TQFrame      *frame;
    TQGridLayout *frameLayout;
    TQLabel      *mtqStatusLabel;
    KPushButton *mNewSessButton, *ok, *cancel;
    TQPushButton *mLayoutButton;
    int         mFailedTimerId;
    int         mTimeoutTimerId;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    TQStringList tqlayoutsList;
    TQStringList::iterator currLayout;
    int         sPid, sFd;
    TQListView   *lv;
};

#endif

