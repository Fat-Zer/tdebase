//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
//

#ifndef __SAKDLG_H__
#define __SAKDLG_H__

#include <tqdialog.h>
#include <tqstringlist.h>

#include "lockprocess.h"

class TQFrame;
class TQGridLayout;
class TQLabel;
class KPushButton;
class TQListView;

//===========================================================================
//
// Simple dialog for displaying an info message.
// It does not handle password validation.
//
class SAKDlg : public TQDialog
{
    Q_OBJECT

public:
    SAKDlg(LockProcess *parent);
    ~SAKDlg();
    virtual void show();

    void updateLabel( TQString &txt );
    void setUnlockIcon();
    void setKDEIcon();
    void setInfoIcon();
    void setWarningIcon();
    void setErrorIcon();

private slots:
    void slotSAKProcessExited();

protected slots:
    virtual void reject();

private:
    TQFrame      *frame;
    TQGridLayout *frameLayout;
    TQLabel      *mStatusLabel;
    TQLabel      *mpixLabel;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    TQStringList layoutsList;
    TQStringList::iterator currLayout;
    int         sPid, sFd;
    KProcess*   mSAKProcess;
};

#endif

