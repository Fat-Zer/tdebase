//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>

#include <config.h>

#include "querydlg.h"

#include <dmctl.h>

#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdesu/defaults.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include <kuser.h>
#include <dcopref.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qsimplerichtext.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qfontmetrics.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qheader.h>
#include <qcheckbox.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <fixx11h.h>

#ifndef AF_LOCAL
# define AF_LOCAL	AF_UNIX
#endif

//===========================================================================
//
// Simple dialog for displaying a password/PIN entry dialog
//
QueryDlg::QueryDlg(LockProcess *parent)
    : QDialog(parent, "query dialog", true, WX11BypassWM),
      mUnlockingFailed(false)
{
    frame = new QFrame( this );
    frame->setFrameStyle( QFrame::Panel | QFrame::Raised );
    frame->setLineWidth( 2 );

    mpixLabel = new QLabel( frame, "pixlabel" );
    mpixLabel->setPixmap(DesktopIcon("unlock"));

    KUser user;

    mStatusLabel = new QLabel( "<b> </b>", frame );
    //mStatusLabel->setAlignment( QLabel::AlignCenter );
    mStatusLabel->setAlignment( QLabel::AlignLeft );

    KSeparator *sep = new KSeparator( KSeparator::HLine, frame );

    ok = new KPushButton( i18n("Unl&ock"), frame );

    QVBoxLayout *unlockDialogLayout = new QVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    QHBoxLayout *layStatus = new QHBoxLayout( 0, 0, KDialog::spacingHint());
    layStatus->addWidget( mStatusLabel );

    QHBoxLayout *layPin = new QHBoxLayout( 0, 0, KDialog::spacingHint());
    pin_box = new KPasswordEdit( this, "pin_box" );
    layPin->addWidget( pin_box );
    pin_box->setFocus();

    QHBoxLayout *layButtons = new QHBoxLayout( 0, 0, KDialog::spacingHint());
    layButtons->addStretch();
    layButtons->addWidget( ok );

    frameLayout = new QGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
    frameLayout->addMultiCellWidget( mpixLabel, 0, 2, 0, 0, AlignTop );
    frameLayout->addLayout( layStatus, 0, 1 );
    frameLayout->addLayout( layPin, 2, 1 );
    frameLayout->addMultiCellWidget( sep, 3, 3, 0, 1 );
    frameLayout->addMultiCellLayout( layButtons, 4, 4, 0, 1 );

    connect(ok, SIGNAL(clicked()), SLOT(slotOK()));

    installEventFilter(this);
}

QueryDlg::~QueryDlg()
{
    hide();
}

void QueryDlg::slotOK()
{
    close();
}

const char * QueryDlg::getEntry()
{
    return pin_box->password();
}

void QueryDlg::updateLabel(QString &txt)
{
    mStatusLabel->setPaletteForegroundColor(Qt::black);
    mStatusLabel->setText("<b>" + txt + "</b>");
}

void QueryDlg::setUnlockIcon()
{
    mpixLabel->setPixmap(DesktopIcon("unlock"));
}

void QueryDlg::setWarningIcon()
{
    mpixLabel->setPixmap(DesktopIcon("messagebox_warning"));
}

void QueryDlg::show()
{
    QDialog::show();
    QApplication::flushX();
}

#include "querydlg.moc"
