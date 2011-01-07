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
#include <fixx11h.h>

#ifndef AF_LOCAL
# define AF_LOCAL	AF_UNIX
#endif

//===========================================================================
//
// Simple dialog for displaying a password/PIN entry dialog
//
QueryDlg::QueryDlg(LockProcess *parent)
    : TQDialog(parent, "query dialog", true, WX11BypassWM),
      mUnlockingFailed(false)
{
    frame = new TQFrame( this );
    frame->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
    frame->setLineWidth( 2 );

    mpixLabel = new TQLabel( frame, "pixlabel" );
    mpixLabel->setPixmap(DesktopIcon("unlock"));

    KUser user;

    mStatusLabel = new TQLabel( "<b> </b>", frame );
    //mStatusLabel->setAlignment( TQLabel::AlignCenter );
    mStatusLabel->setAlignment( TQLabel::AlignLeft );

    KSeparator *sep = new KSeparator( KSeparator::HLine, frame );

    ok = new KPushButton( i18n("Unl&ock"), frame );

    TQVBoxLayout *unlockDialogLayout = new TQVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    TQHBoxLayout *layStatus = new TQHBoxLayout( 0, 0, KDialog::spacingHint());
    layStatus->addWidget( mStatusLabel );

    TQHBoxLayout *layPin = new TQHBoxLayout( 0, 0, KDialog::spacingHint());
    pin_box = new KPasswordEdit( this, "pin_box" );
    layPin->addWidget( pin_box );
    pin_box->setFocus();

    TQHBoxLayout *layButtons = new TQHBoxLayout( 0, 0, KDialog::spacingHint());
    layButtons->addStretch();
    layButtons->addWidget( ok );

    frameLayout = new TQGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
    frameLayout->addMultiCellWidget( mpixLabel, 0, 2, 0, 0, AlignTop );
    frameLayout->addLayout( layStatus, 0, 1 );
    frameLayout->addLayout( layPin, 2, 1 );
    frameLayout->addMultiCellWidget( sep, 3, 3, 0, 1 );
    frameLayout->addMultiCellLayout( layButtons, 4, 4, 0, 1 );

    connect(ok, TQT_SIGNAL(clicked()), TQT_SLOT(slotOK()));

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

void QueryDlg::updateLabel(TQString &txt)
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
    TQDialog::show();
    TQApplication::flushX();
}

#include "querydlg.moc"
