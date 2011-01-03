//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
//

#ifndef __QUERYDLG_H__
#define __QUERYDLG_H__

#include <tqdialog.h>
#include <tqstringlist.h>

#include <kpassdlg.h>

#include "lockprocess.h"

class TQFrame;
class TQGridLayout;
class TQLabel;
class KPushButton;
class TQListView;

//===========================================================================
//
// Simple dialog for displaying an query dialog.
// It does not handle password validation.
//
class QueryDlg : public QDialog
{
    Q_OBJECT

public:
    QueryDlg(LockProcess *parent);
    ~QueryDlg();
    virtual void show();

    void updateLabel( TQString &txt );
    void setUnlockIcon();
    void setWarningIcon();
    const char * getEntry();

private slots:
    void slotOK();

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
    KPushButton *ok;
    KPasswordEdit *pin_box;
};

#endif

