//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
//

#ifndef __INFODLG_H__
#define __INFODLG_H__

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
class InfoDlg : public QDialog
{
    Q_OBJECT

public:
    InfoDlg(LockProcess *parent);
    ~InfoDlg();
    virtual void show();

    void updateLabel( TQString &txt );
    void setUnlockIcon();
    void setKDEIcon();
    void setInfoIcon();
    void setWarningIcon();
    void setErrorIcon();

private:
    TQFrame      *frame;
    TQGridLayout *frameLayout;
    TQLabel      *mtqStatusLabel;
    TQLabel      *mpixLabel;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    TQStringList tqlayoutsList;
    TQStringList::iterator currLayout;
    int         sPid, sFd;
};

#endif

