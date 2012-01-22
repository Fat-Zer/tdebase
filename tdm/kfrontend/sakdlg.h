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

#include <kprocess.h>

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
    SAKDlg(TQWidget *parent);
    ~SAKDlg();
    virtual void show();

    void updateLabel( TQString &txt );
    void closeDialogForced();

private slots:
    void slotSAKProcessExited();
    void handleInputPipe();

protected slots:
    virtual void reject();

private:
    TQFrame      *frame;
    TQGridLayout *frameLayout;
    TQLabel      *mStatusLabel;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    TQStringList layoutsList;
    TQStringList::iterator currLayout;
    int         sPid, sFd;
    KProcess*   mSAKProcess;
    int         mPipe_fd;
    TQString mPipeFilename;

protected:
    bool closingDown;
};

#endif

