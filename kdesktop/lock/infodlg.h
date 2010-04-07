//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
//

#ifndef __INFODLG_H__
#define __INFODLG_H__

#include <qdialog.h>
#include <qstringlist.h>

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

    void updateLabel( QString &txt );
    void setUnlockIcon();
    void setWarningIcon();

private:
    QFrame      *frame;
    QGridLayout *frameLayout;
    QLabel      *mStatusLabel;
    QLabel      *mpixLabel;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    QStringList layoutsList;
    QStringList::iterator currLayout;
    int         sPid, sFd;
};

#endif

