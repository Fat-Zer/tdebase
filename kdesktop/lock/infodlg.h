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

class QFrame;
class QGridLayout;
class QLabel;
class KPushButton;
class QListView;

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
    TQLabel      *mStatusLabel;
    TQLabel      *mpixLabel;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    TQStringList layoutsList;
    TQStringList::iterator currLayout;
    int         sPid, sFd;
};

#endif

