//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
//

#ifndef __QUERYDLG_H__
#define __QUERYDLG_H__

#include <qdialog.h>
#include <qstringlist.h>

#include <kpassdlg.h>

class QFrame;
class QGridLayout;
class QLabel;
class KPushButton;
class QListView;

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

    void updateLabel( QString &txt );
    void setUnlockIcon();
    void setWarningIcon();
    const char * getEntry();

private slots:
    void slotOK();

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
    KPushButton *ok;
    KPasswordEdit *pin_box;
};

#endif

