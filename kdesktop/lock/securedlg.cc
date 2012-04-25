//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010-2011 Timothy Pearson <kb9vqf@pearsoncomputing.net>

#include <config.h>

#include "securedlg.h"

#include <dmctl.h>

#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <tdesu/defaults.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include <kuser.h>
#include <dcopref.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqmessagebox.h>
#include <tqsimplerichtext.h>
#include <tqlabel.h>
#include <tqstringlist.h>
#include <tqfontmetrics.h>
#include <tqstyle.h>
#include <tqapplication.h>
#include <tqlistview.h>
#include <tqheader.h>
#include <tqcheckbox.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <fixx11h.h>

#ifndef AF_LOCAL
# define AF_LOCAL	AF_UNIX
#endif

extern bool trinity_desktop_lock_use_system_modal_dialogs;
extern bool trinity_desktop_lock_use_sak;

//===========================================================================
//
// Simple dialog for displaying an unlock status or recurring error message
//
SecureDlg::SecureDlg(LockProcess *parent)
    : TQDialog(parent, "information dialog", true, (trinity_desktop_lock_use_system_modal_dialogs?((WFlags)WStyle_StaysOnTop):((WFlags)WX11BypassWM))),
      mUnlockingFailed(false), retInt(NULL)
{
    if (trinity_desktop_lock_use_system_modal_dialogs) {
        // Signal that we do not want any window controls to be shown at all
        Atom kde_wm_system_modal_notification;
        kde_wm_system_modal_notification = XInternAtom(tqt_xdisplay(), "_KDE_WM_MODAL_SYS_NOTIFICATION", False);
        XChangeProperty(tqt_xdisplay(), winId(), kde_wm_system_modal_notification, XA_INTEGER, 32, PropModeReplace, (unsigned char *) "TRUE", 1L);
    }
    setCaption(i18n("Secure Desktop Area"));

    frame = new TQFrame( this );
    if (trinity_desktop_lock_use_system_modal_dialogs)
        frame->setFrameStyle( TQFrame::NoFrame );
    else
        frame->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
    frame->setLineWidth( 2 );

    KSMModalDialogHeader* theader = new KSMModalDialogHeader( frame );

    KUser user;

    mLogonStatus = new TQLabel( frame );
    mLogonStatus->setText(i18n("'%1' is currently logged on").arg( user.fullName() ));

    KSeparator *sep = new KSeparator( KSeparator::HLine, frame );

    mLockButton = new TQPushButton( frame );
    mLockButton->setText(i18n("Lock Session"));

    mTaskButton = new TQPushButton( frame );
    mTaskButton->setText(i18n("Task Manager"));

    mCancelButton = new TQPushButton( frame );
    mCancelButton->setText(i18n("Cancel"));

    mShutdownButton = new TQPushButton( frame );
    mShutdownButton->setText(i18n("Logoff Menu"));

    mSwitchButton = new TQPushButton( frame );
    mSwitchButton->setText(i18n("Switch User"));
    mSwitchButton->setEnabled(false); // FIXME

    TQVBoxLayout *unlockDialogLayout = new TQVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    TQHBoxLayout *layStatus = new TQHBoxLayout( 0, 0, KDialog::spacingHint());
    layStatus->addWidget( mLogonStatus );

    TQGridLayout *layPBGrid = new TQGridLayout( 0, 0, KDialog::spacingHint());
    layPBGrid->addWidget( mLockButton, 0, 0 );
    layPBGrid->addWidget( mTaskButton, 0, 1 );
    layPBGrid->addWidget( mShutdownButton, 0, 2 );
    layPBGrid->addWidget( mCancelButton, 0, 3 );
    layPBGrid->addWidget( mSwitchButton, 1, 0 );

    frameLayout = new TQGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
    frameLayout->addMultiCellWidget( theader, 0, 0, 0, 1, AlignTop | AlignLeft );
    frameLayout->addMultiCellLayout( layStatus, 1, 1, 0, 1, AlignLeft | AlignVCenter);
    frameLayout->addMultiCellWidget( sep, 2, 2, 0, 1 );
    frameLayout->addMultiCellLayout( layPBGrid, 3, 3, 0, 1, AlignLeft | AlignVCenter);

    connect(mCancelButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotBtnCancel()));
    connect(mLockButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotBtnLock()));
    connect(mTaskButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotBtnTask()));
    connect(mShutdownButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotBtnShutdown()));
    connect(mSwitchButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotBtnSwitchUser()));

    TQSize dlgSz = sizeHint();
    int btnSize = dlgSz.width();
    btnSize = btnSize / 4;
    btnSize = btnSize - (KDialog::spacingHint() / 2);
    mLockButton->setFixedWidth(btnSize);
    mTaskButton->setFixedWidth(btnSize);
    mCancelButton->setFixedWidth(btnSize);
    mShutdownButton->setFixedWidth(btnSize);
    mSwitchButton->setFixedWidth(btnSize);

    installEventFilter(this);
    setFixedSize( sizeHint() );
}

SecureDlg::~SecureDlg()
{
    hide();
}

void SecureDlg::slotBtnCancel()
{
    if (retInt) *retInt = 0;
    hide();
}

void SecureDlg::slotBtnLock()
{
    if (retInt) *retInt = 1;
    hide();
}

void SecureDlg::slotBtnTask()
{
    if (retInt) *retInt = 2;
    hide();
}

void SecureDlg::slotBtnShutdown()
{
    if (retInt) *retInt = 3;
    hide();
}

void SecureDlg::slotBtnSwitchUser()
{
    if (retInt) *retInt = 4;
    hide();
}

void SecureDlg::setRetInt(int *i)
{
    retInt = i;
}

void SecureDlg::closeDialogForced()
{
    if (retInt) *retInt = 0;
    TQDialog::reject();
}

void SecureDlg::reject()
{
    closeDialogForced();
}

void SecureDlg::show()
{
    TQDialog::show();
    TQApplication::flushX();
}

#include "securedlg.moc"
