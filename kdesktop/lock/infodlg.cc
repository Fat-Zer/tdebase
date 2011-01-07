//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>

#include <config.h>

#include "infodlg.h"

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
// Simple dialog for displaying an unlock status or recurring error message
//
InfoDlg::InfoDlg(LockProcess *parent)
    : TQDialog(parent, "information dialog", true, WX11BypassWM),
      mUnlockingFailed(false)
{
    frame = new TQFrame( this );
    frame->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
    frame->setLineWidth( 2 );

    mpixLabel = new TQLabel( frame, "pixlabel" );
    mpixLabel->setPixmap(DesktopIcon("unlock"));

    KUser user;

    mStatusLabel = new TQLabel( "<b> </b>", frame );
    mStatusLabel->setAlignment( TQLabel::AlignCenter );

    TQVBoxLayout *unlockDialogLayout = new TQVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    TQHBoxLayout *layStatus = new TQHBoxLayout( 0, 0, KDialog::spacingHint());
    layStatus->addWidget( mStatusLabel );

    frameLayout = new TQGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
    frameLayout->addMultiCellWidget( mpixLabel, 0, 2, 0, 0, AlignTop );
    frameLayout->addLayout( layStatus, 1, 1 );

    installEventFilter(this);
}

InfoDlg::~InfoDlg()
{
    hide();
}

void InfoDlg::updateLabel(TQString &txt)
{
    mStatusLabel->setPaletteForegroundColor(Qt::black);
    mStatusLabel->setText("<b>" + txt + "</b>");
}

void InfoDlg::setUnlockIcon()
{
    mpixLabel->setPixmap(DesktopIcon("unlock"));
}

void InfoDlg::setKDEIcon()
{
    mpixLabel->setPixmap(DesktopIcon("about_kde"));
}

void InfoDlg::setInfoIcon()
{
    mpixLabel->setPixmap(DesktopIcon("messagebox_info"));
}

void InfoDlg::setWarningIcon()
{
    mpixLabel->setPixmap(DesktopIcon("messagebox_warning"));
}

void InfoDlg::setErrorIcon()
{
    mpixLabel->setPixmap(DesktopIcon("messagebox_critical"));
}

void InfoDlg::show()
{
    TQDialog::show();
    TQApplication::flushX();
}

#include "infodlg.moc"
