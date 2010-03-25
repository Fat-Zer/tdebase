/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include "shutdowndlg.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qimage.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qregexp.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kuser.h>
#include <kpixmap.h>
#include <kimageeffect.h>
#include <kdialog.h>
#include <kseparator.h>
#include <kconfig.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <dmctl.h>
#include <kaction.h>


#include <X11/Xlib.h>

#include "shutdowndlg.moc"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, "feedbackwidget", WType_Popup ),
   m_currentY( 0 ), 
   m_grayOpacity( 0.0f ),
   m_compensation( 0.0f ),
   m_fadeBackwards( FALSE ),
   m_unfadedImage(),
   m_grayImage(),
   m_fadeTime(),
   m_pmio()

{
    m_grayImage = QImage::QImage();
    m_unfadedImage = QImage::QImage();
    resize(0, 0);
    setShown(true);
    QTimer::singleShot( 500, this, SLOT( slotPaintEffect() ) );
}

// called after stopping shutdown-feedback -> smooth fade-back to color-mode
void KSMShutdownFeedback::fadeBack( void )
{
    m_fadeTime.restart();
    m_fadeBackwards = TRUE;
    // its possible that we have to fade back, before all is completely gray, so we cannot start
    // with completely gray when fading back...
    m_compensation = 1.0f - m_grayOpacity;
    // wait until we're completely back in color-mode...
    while ( m_grayOpacity > 0.0f )
    	    slotPaintEffect();
}

void KSMShutdownFeedback::slotPaintEffect()
{
    // determine which fade to use
    if (KConfigGroup(KGlobal::config(), "Logout").readBoolEntry("doFancyLogout", true))
    {

	float doFancyLogoutAdditionalDarkness  = (float)KConfigGroup(KGlobal::config(), "Logout").readDoubleNumEntry("doFancyLogoutAdditionalDarkness", 0.6);

	float doFancyLogoutFadeTime = (float)KConfigGroup(KGlobal::config(), "Logout").readDoubleNumEntry("doFancyLogoutFadeTime", 4000);

	float doFancyLogoutFadeBackTime = (float)KConfigGroup(KGlobal::config(), "Logout").readDoubleNumEntry("doFancyLogoutFadeBackTime", 1000);
	
	// if slotPaintEffect() is called first time, we have to initialize the gray image
	// we also could do that in the constructor, but then the displaying of the
	// logout-UI would be too much delayed...
	if ( m_grayImage.isNull() )
	{
		setBackgroundMode( QWidget::NoBackground );
		setGeometry( QApplication::desktop()->geometry() );
		m_root.resize( width(), height() ); // for the default logout

		m_grayImage = QPixmap::grabWindow(qt_xrootwin(), 0, 0, QApplication::desktop()->width(), QApplication::desktop()->height()).convertToImage();
		m_unfadedImage = m_grayImage.copy();
		register uchar * r = m_grayImage.bits();
		register uchar * g = m_grayImage.bits() + 1;
		register uchar * b = m_grayImage.bits() + 2;
		uchar * end = m_grayImage.bits() + m_grayImage.numBytes();
	
		while ( r != end ) {
		*r = *g = *b = (uchar) ( ( (*r)*11 + ((*g)<<4) + (*b)*5 ) * doFancyLogoutAdditionalDarkness / 32.0f );
		r += 4;
		g += 4;
		b += 4;
		}
		// start timer which is used for cpu-speed-independent fading
		m_fadeTime.start();
		m_rowsDone = 0;
	}
	
	// return if fading is completely done...
	if ( ( m_grayOpacity >= 1.0f && m_fadeBackwards == FALSE ) || ( m_grayOpacity <= 0.0f && m_fadeBackwards == TRUE ) )
		return;
	
	
	if ( m_fadeBackwards == FALSE )
	{
		m_grayOpacity = m_fadeTime.elapsed() / doFancyLogoutFadeTime;
		if ( m_grayOpacity > 1.0f )
		m_grayOpacity = 1.0f;
	}
	else
	{
		m_grayOpacity = 1.0f - m_fadeTime.elapsed() / doFancyLogoutFadeBackTime - m_compensation;
		if ( m_grayOpacity < 0.0f )
		m_grayOpacity = 0.0f;
	}
	
	const int imgWidth = m_unfadedImage.width();
	int imgHeight = m_unfadedImage.height();
	int heightUnit = imgHeight / 3;
	if( heightUnit < 1 )
		heightUnit = 1;

	int y1 = static_cast<int>( imgHeight*m_grayOpacity - heightUnit + m_grayOpacity*heightUnit*2.0f );
	if( y1 > imgHeight )
		y1 = imgHeight;
	
	int y2 = y1+heightUnit;
	if( y2 > imgHeight )
		y2 = imgHeight;
	
	if( m_fadeBackwards == FALSE )
	{
		if( y1 > 0 && y1 < imgHeight && y1-m_rowsDone > 0 && m_rowsDone < imgHeight )
		{
		QImage img( imgWidth, y1-m_rowsDone, 32 );
		memcpy( img.bits(), m_grayImage.scanLine( m_rowsDone ), imgWidth*(y1-m_rowsDone)*4 );
		// conversion is slow as hell if desktop-depth != 24bpp...
		//Pixmap pm = m_pmio.convertToPixmap( img );
		//bitBlt( this, 0, m_rowsDone, &pm );
//		QImage pm = m_pmio.convertToImage( img );
		bitBlt( this, 0, m_rowsDone, &img );		
		m_rowsDone = y1;
		}
	}
	else
	{
		// when fading back we have to blit area which isnt gray anymore to unfaded image
		if( y2 > 0 && y2 < imgHeight && m_rowsDone > y2 )
		{
		QImage img( imgWidth, m_rowsDone-y2, 32 );
		memcpy( img.bits(), m_unfadedImage.scanLine( y2 ), imgWidth*(m_rowsDone-y2)*4 );
		// conversion is slow as hell if desktop-depth != 24bpp...
		//QPixmap pm = m_pmio.convertToPixmap( img );
		//bitBlt( this, 0, y2, &pm );
		bitBlt( this, 0, y2, &img );
		m_rowsDone = y2;
		}
	}
	
	int start_y1 = y1;
	if( start_y1 < 0 )
		start_y1 = 0;
	if( y2 > start_y1 )
	{
		QImage img( imgWidth, y2-start_y1, 32 );
		memcpy( img.bits(), m_grayImage.scanLine( start_y1 ), ( y2-start_y1 ) * imgWidth * 4 );
		register uchar * rs = m_unfadedImage.scanLine( start_y1 );
		register uchar * gs = rs + 1;
		register uchar * bs = gs + 1;
		register uchar * rd = img.bits();
		register uchar * gd = rd + 1;
		register uchar * bd = gd + 1;
		for( int y = start_y1; y < y2; ++y )
		{
		// linear gradients look bad, so use cos-function
		short int opac = static_cast<short int>( 128 - cosf( M_PI*(y-y1)/heightUnit )*128.0f );
		for( short int x = 0; x < imgWidth; ++x )
		{
			*rd += ( ( ( *rs - *rd ) * opac ) >> 8 );
			rs += 4; rd += 4;
			*gd += ( ( ( *gs - *gd ) * opac ) >> 8 );
			gs += 4; gd += 4;
			*bd += ( ( ( *bs - *bd ) * opac ) >> 8 );
			bs += 4; bd += 4;
		}
		}
		// conversion is slow as hell if desktop-depth != 24bpp...
		//QPixmap pm = m_pmio.convertToPixmap( img );
		//bitBlt( this, 0, start_y1, &pm );
		bitBlt( this, 0, start_y1, &img );
	}
	
	QTimer::singleShot( 5, this, SLOT( slotPaintEffect() ) );

    }
    // standard logout fade
    else
    {
	    if ( m_currentY >= height() ) {
	        if ( backgroundMode() == QWidget::NoBackground ) {
	            setBackgroundMode( QWidget::NoBackground );
	            setBackgroundPixmap( m_root );
	        }
	        return;
	    }

	    KPixmap pixmap;
	    pixmap = QPixmap::grabWindow( qt_xrootwin(), 0, m_currentY, width(), 10 );
	    QImage image = pixmap.convertToImage();
	    KImageEffect::blend( Qt::black, image, 0.4 );
	    KImageEffect::toGray( image, true );
	    pixmap.convertFromImage( image );
	    bitBlt( this, 0, m_currentY, &pixmap );
	    bitBlt( &m_root, 0, m_currentY, &pixmap );
	    m_currentY += 10;
	    QTimer::singleShot( 1, this, SLOT( slotPaintEffect() ) );
    }

}

//////

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, KApplication::ShutdownType sdtype )
  : QDialog( parent, 0, TRUE, WType_Popup ), targets(0)
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.

{
    QVBoxLayout* vbox = new QVBoxLayout( this );
	

    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    frame->setLineWidth( style().pixelMetric( QStyle::PM_DefaultFrameWidth, frame ) );
	// we need to set the minimum size for the logout box, since it
	// gets too small if there isn't all options available
	frame->setMinimumWidth(400);
    vbox->addWidget( frame );
    vbox = new QVBoxLayout( frame, 2 * KDialog::marginHint(),
                            2 * KDialog::spacingHint() );

	// default factor
	bool doUbuntuLogout = KConfigGroup(KGlobal::config(), "Logout").readBoolEntry("doUbuntuLogout", false);	

	// slighty more space for the new logout    
	int factor = 2;

	if(doUbuntuLogout) 
	{ 	
		factor = 8;
	}
	else {
		QLabel* label = new QLabel( i18n("End Session for \"%1\"").arg(KUser().loginName()), frame );
		QFont fnt = label->font();
		fnt.setBold( true );
		fnt.setPointSize( fnt.pointSize() * 3 / 2 );
		label->setFont( fnt );
		vbox->addWidget( label, 0, AlignHCenter );
	}

	// for the basic layout, within this box either the ubuntu dialog or
	// standard konqy+buttons will be placed.
	QHBoxLayout* hbox = new QHBoxLayout( vbox, factor * KDialog::spacingHint() );

	// from here on we have to adapt to the two different dialogs
	QFrame* lfrm;
	QVBoxLayout* buttonlay;
	QHBoxLayout* hbuttonbox;
	QFont btnFont;

	if(doUbuntuLogout)
	{
		// first line of buttons
		hbuttonbox = new QHBoxLayout( hbox, factor * KDialog::spacingHint() );
		hbuttonbox->setAlignment( Qt::AlignHCenter );
		// End session
		FlatButton* btnLogout = new FlatButton( frame );
		btnLogout->setTextLabel( QString("&") + i18n("Log out"), false );
		btnLogout->setPixmap( DesktopIcon( "back") );
		int i = btnLogout->textLabel().find( QRegExp("\\&"), 0 );    // i == 1
		btnLogout->setAccel( "ALT+" + btnLogout->textLabel().lower()[i+1] ) ;
		hbuttonbox->addWidget ( btnLogout );
		connect(btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));

	}
	else
	{

		// konqy
		lfrm = new QFrame( frame );
		lfrm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
		hbox->addWidget( lfrm, AlignCenter );

		buttonlay = new QVBoxLayout( hbox, factor * KDialog::spacingHint() );
		buttonlay->setAlignment( Qt::AlignHCenter );

		QLabel* icon = new QLabel( lfrm );
		icon->setPixmap( UserIcon( "shutdownkonq" ) );
		lfrm->setFixedSize( icon->sizeHint());
		icon->setFixedSize( icon->sizeHint());

		buttonlay->addStretch( 1 );
		// End session
		KPushButton* btnLogout = new KPushButton( KGuiItem( i18n("&End Current Session"), "undo"), frame );
		btnFont = btnLogout->font();
		buttonlay->addWidget( btnLogout );
		connect(btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));
	}

	

	m_halCtx = NULL;

	if (maysd) 	{

		// respect lock on resume & disable suspend/hibernate settings 
		// from power-manager
		KConfig config("power-managerrc");
		bool disableSuspend = config.readBoolEntry("disableSuspend", false);
		bool disableHibernate = config.readBoolEntry("disableHibernate", false);
		m_lockOnResume = config.readBoolEntry("lockOnResume", true);

		bool canSuspend = false;
		bool canHibernate = false;

		// Query HAL for suspend/resume support
		m_halCtx = libhal_ctx_new();

		DBusError error;
		dbus_error_init(&error);
		m_dbusConn = dbus_connection_open_private(DBUS_SYSTEM_BUS, &error);
		if (!m_dbusConn)
		{
			dbus_error_free(&error);
			libhal_ctx_free(m_halCtx);
			m_halCtx = NULL;
		}
		else
		{
			dbus_bus_register(m_dbusConn, &error);
			if (dbus_error_is_set(&error))
			{
				dbus_error_free(&error);
				libhal_ctx_free(m_halCtx);
				m_dbusConn = NULL;
				m_halCtx = NULL;
			}
			else
			{
				libhal_ctx_set_dbus_connection(m_halCtx, m_dbusConn);
				if (!libhal_ctx_init(m_halCtx, &error))
				{
					if (dbus_error_is_set(&error))
						dbus_error_free(&error);
					libhal_ctx_free(m_halCtx);
					m_dbusConn = NULL;
					m_halCtx = NULL;
				}
			}
		}

		if (m_halCtx)
		{
			if (libhal_device_get_property_bool(m_halCtx,
												"/org/freedesktop/Hal/devices/computer",
												"power_management.can_suspend", 
												NULL))
			{
				canSuspend = true;
			}

			if (libhal_device_get_property_bool(m_halCtx, 
												"/org/freedesktop/Hal/devices/computer",
												"power_management.can_hibernate",
												NULL))
			{
				canHibernate = true;
			}
		}


		if(doUbuntuLogout) {

			if (canSuspend && !disableSuspend)
			{
				// Suspend
				FlatButton* btnSuspend = new FlatButton( frame );
				btnSuspend->setTextLabel( i18n("&Suspend"), false );
				btnSuspend->setPixmap( DesktopIcon( "suspend") );
			    int i = btnSuspend->textLabel().find( QRegExp("\\&"), 0 );    // i == 1
				btnSuspend->setAccel( "ALT+" + btnSuspend->textLabel().lower()[i+1] ) ;
				hbuttonbox->addWidget ( btnSuspend);
				connect(btnSuspend, SIGNAL(clicked()), SLOT(slotSuspend()));
			}
		
			if (canHibernate && !disableHibernate)
			{
				// Hibernate
				FlatButton* btnHibernate = new FlatButton( frame );
		    	btnHibernate->setTextLabel( i18n("&Hibernate"), false );
				btnHibernate->setPixmap( DesktopIcon( "hibernate") );
				int i = btnHibernate->textLabel().find( QRegExp("\\&"), 0 );    // i == 1
				btnHibernate->setAccel( "ALT+" + btnHibernate->textLabel().lower()[i+1] ) ;		
				hbuttonbox->addWidget ( btnHibernate);	
				connect(btnHibernate, SIGNAL(clicked()), SLOT(slotHibernate()));
			}

			// Separator (within buttonlay)
			vbox->addWidget( new KSeparator( frame ) );

			// bottom buttons
			QHBoxLayout* hbuttonbox2 = new QHBoxLayout( vbox, factor * KDialog::spacingHint() );
			hbuttonbox2->setAlignment( Qt::AlignHCenter );
			
			// Reboot
			FlatButton* btnReboot = new FlatButton( frame );
			btnReboot->setTextLabel( i18n("&Restart"), false );
			btnReboot->setPixmap( DesktopIcon( "reload") );
    		int i = btnReboot->textLabel().find( QRegExp("\\&"), 0 );    // i == 1
			btnReboot->setAccel( "ALT+" + btnReboot->textLabel().lower()[i+1] ) ;
			hbuttonbox2->addWidget ( btnReboot);
			connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
			if ( sdtype == KApplication::ShutdownTypeReboot )
				btnReboot->setFocus();

			// BAD CARMA .. this code is copied line by line from standard konqy dialog
			int def, cur;
			if ( DM().bootOptions( rebootOptions, def, cur ) ) {
			btnReboot->setPopupDelay(300); // visually add dropdown
			targets = new QPopupMenu( frame );
			if ( cur == -1 )
				cur = def;
		
			int index = 0;
			for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index)
				{
					QString label = (*it);
					label=label.replace('&',"&&");
				if (index == cur)
				targets->insertItem( label + i18n("current option in boot loader", " (current)"), index);
				else
				targets->insertItem( label, index );
				}
		
			btnReboot->setPopup(targets);
			connect( targets, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
			}
			// BAD CARMA .. this code is copied line by line from standard konqy dialog [EOF]
 		
			// Shutdown
			FlatButton* btnHalt = new FlatButton( frame );
			btnHalt->setTextLabel( i18n("&Turn Off"), false );
			btnHalt->setPixmap( DesktopIcon( "exit") );
			i = btnHalt->textLabel().find( QRegExp("\\&"), 0 );    // i == 1
			btnHalt->setAccel( "ALT+" + btnHalt->textLabel().lower()[i+1] ) ;
			hbuttonbox2->addWidget ( btnHalt );
			connect(btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
				if ( sdtype == KApplication::ShutdownTypeHalt )
					btnHalt->setFocus();

			// cancel buttonbox
			QHBoxLayout* hbuttonbox3 = new QHBoxLayout( vbox, factor * KDialog::spacingHint() );
			hbuttonbox3->setAlignment( Qt::AlignRight );

			// Back to Desktop
			KSMPushButton* btnBack = new KSMPushButton( KStdGuiItem::cancel(), frame );
			hbuttonbox3->addWidget( btnBack );
			connect(btnBack, SIGNAL(clicked()), SLOT(reject()));
	
		}
		else 
		{
			// Shutdown
			KPushButton* btnHalt = new KPushButton( KGuiItem( i18n("&Turn Off Computer"), "exit"), frame );
			btnHalt->setFont( btnFont );
			buttonlay->addWidget( btnHalt );
			connect(btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
			if ( sdtype == KApplication::ShutdownTypeHalt )
				btnHalt->setFocus();
	
			// Reboot
			KSMDelayedPushButton* btnReboot = new KSMDelayedPushButton( KGuiItem( i18n("&Restart Computer"), "reload"), frame );
			btnReboot->setFont( btnFont );
			buttonlay->addWidget( btnReboot );
	
			connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
			if ( sdtype == KApplication::ShutdownTypeReboot )
				btnReboot->setFocus();
	
			// this section is copied as-is into ubuntulogout as well
			int def, cur;
			if ( DM().bootOptions( rebootOptions, def, cur ) ) {
			targets = new QPopupMenu( frame );
			if ( cur == -1 )
				cur = def;
		
			int index = 0;
			for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index)
				{
					QString label = (*it);
					label=label.replace('&',"&&");
				if (index == cur)
				targets->insertItem( label + i18n("current option in boot loader", " (current)"), index);
				else
				targets->insertItem( label, index );
				}
		
			btnReboot->setPopup(targets);
			connect( targets, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
			}
	
	
			if (canSuspend && !disableSuspend)
			{
				KPushButton* btnSuspend = new KPushButton( KGuiItem( i18n("&Suspend Computer"), "suspend"), frame );
				btnSuspend->setFont( btnFont );
				buttonlay->addWidget( btnSuspend );
				connect(btnSuspend, SIGNAL(clicked()), SLOT(slotSuspend()));
			}
	
			if (canHibernate && !disableHibernate)
			{
				KPushButton* btnHibernate = new KPushButton( KGuiItem( i18n("&Hibernate Computer"), "hibernate"), frame );
				btnHibernate->setFont( btnFont );
				buttonlay->addWidget( btnHibernate );
				connect(btnHibernate, SIGNAL(clicked()), SLOT(slotHibernate()));
			}

			buttonlay->addStretch( 1 );
		
			// Separator
			buttonlay->addWidget( new KSeparator( frame ) );

			// Back to Desktop
			KPushButton* btnBack = new KPushButton( KStdGuiItem::cancel(), frame );
			buttonlay->addWidget( btnBack );
			connect(btnBack, SIGNAL(clicked()), SLOT(reject()));

		}

	}
	else {
		// finish the dialog correctly
		if(doUbuntuLogout)
		{
			// cancel buttonbox
			QHBoxLayout* hbuttonbox3 = new QHBoxLayout( vbox, factor * KDialog::spacingHint() );
			hbuttonbox3->setAlignment( Qt::AlignRight );

			// Back to Desktop
			KSMPushButton* btnBack = new KSMPushButton( KStdGuiItem::cancel(), frame );
			hbuttonbox3->addWidget( btnBack );

			connect(btnBack, SIGNAL(clicked()), SLOT(reject()));
		}
		else 
		{
			// Separator
			buttonlay->addWidget( new KSeparator( frame ) );

			// Back to Desktop
			KPushButton* btnBack = new KPushButton( KStdGuiItem::cancel(), frame );
			buttonlay->addWidget( btnBack );

			connect(btnBack, SIGNAL(clicked()), SLOT(reject()));
		}


	}


}


KSMShutdownDlg::~KSMShutdownDlg()
{
    if (m_halCtx)
    {
        DBusError error;
        dbus_error_init(&error);
        libhal_ctx_shutdown(m_halCtx, &error);
        libhal_ctx_free(m_halCtx);
    }
}


void KSMShutdownDlg::slotLogout()
{
    m_shutdownType = KApplication::ShutdownTypeNone;
    accept();
}


void KSMShutdownDlg::slotReboot()
{
    // no boot option selected -> current
    m_bootOption = QString::null;
    m_shutdownType = KApplication::ShutdownTypeReboot;
    accept();
}

void KSMShutdownDlg::slotReboot(int opt)
{
    if (int(rebootOptions.size()) > opt)
        m_bootOption = rebootOptions[opt];
    m_shutdownType = KApplication::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotHalt()
{
    m_bootOption = QString::null;
    m_shutdownType = KApplication::ShutdownTypeHalt;
    accept();
}

void KSMShutdownDlg::slotSuspend()
{
    if (m_lockOnResume) {
        DCOPRef("kdesktop", "KScreensaverIface").send("lock");
    }

    if (m_dbusConn) 
    {
        DBusMessage *msg = dbus_message_new_method_call(
                              "org.freedesktop.Hal",
                              "/org/freedesktop/Hal/devices/computer", 
                              "org.freedesktop.Hal.Device.SystemPowerManagement", 
                              "Suspend");

        int wakeup=0;
	    dbus_message_append_args(msg, DBUS_TYPE_INT32, &wakeup, DBUS_TYPE_INVALID);

        dbus_connection_send(m_dbusConn, msg, NULL);

        dbus_message_unref(msg);
    }

    reject(); // continue on resume
}

void KSMShutdownDlg::slotHibernate()
{
    if (m_lockOnResume) {
        DCOPRef("kdesktop", "KScreensaverIface").send("lock");
    }

    if (m_dbusConn) 
    {
        DBusMessage *msg = dbus_message_new_method_call(
                              "org.freedesktop.Hal",
                              "/org/freedesktop/Hal/devices/computer", 
                              "org.freedesktop.Hal.Device.SystemPowerManagement", 
                              "Hibernate");

        dbus_connection_send(m_dbusConn, msg, NULL);

        dbus_message_unref(msg);
    }

    reject(); // continue on resume
}

bool KSMShutdownDlg::confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype, QString& bootOption )
{
    kapp->enableStyles();
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, sdtype );

    // Show dialog (will save the background in showEvent)
    QSize sh = l->sizeHint();
    QRect rect = KGlobalSettings::desktopGeometry(QCursor::pos());

    l->move(rect.x() + (rect.width() - sh.width())/2,
            rect.y() + (rect.height() - sh.height())/2);
    bool result = l->exec();
    sdtype = l->m_shutdownType;
    bootOption = l->m_bootOption;

    delete l;

    kapp->disableStyles();
    return result;
}

KSMDelayedPushButton::KSMDelayedPushButton( const KGuiItem &item,
					    QWidget *parent,
					    const char *name)
  : KPushButton( item, parent, name), pop(0), popt(0)
{
  connect(this, SIGNAL(pressed()), SLOT(slotPressed()));
  connect(this, SIGNAL(released()), SLOT(slotReleased()));
  popt = new QTimer(this);
  connect(popt, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void KSMDelayedPushButton::setPopup(QPopupMenu *p)
{
  pop = p;
  setIsMenuButton(p != 0);
}

void KSMDelayedPushButton::slotPressed()
{
  if (pop)
    popt->start(QApplication::startDragTime());
}

void KSMDelayedPushButton::slotReleased()
{
  popt->stop();
}

void KSMDelayedPushButton::slotTimeout()
{
  QPoint bl = mapToGlobal(rect().bottomLeft());
  QWidget *par = (QWidget*)parent();
  QPoint br = par->mapToGlobal(par->rect().bottomRight());
  pop->popup( bl );
  popt->stop();
  setDown(false);
}

KSMPushButton::KSMPushButton( const KGuiItem &item,
					    QWidget *parent,
					    const char *name)
  : KPushButton( item, parent, name),
    m_pressed(false)
{
	setDefault( false );
	setAutoDefault ( false );	
}


void KSMPushButton::keyPressEvent( QKeyEvent* e )
{
switch ( e->key() ) 
  {
    case Key_Enter:
    case Key_Return:
    case Key_Space:
      m_pressed = TRUE;
	  setDown(true);
      emit pressed();
      break;
	case Key_Escape:
		e->ignore();
	break;
    default:
      e->ignore();
    }

	QPushButton::keyPressEvent(e);
}


void KSMPushButton::keyReleaseEvent( QKeyEvent* e )
{
	switch ( e->key() ) 
	{
		case Key_Space:
		case Key_Enter:
		case Key_Return:
			if ( m_pressed ) 
			{
			setDown(false);
			m_pressed = FALSE;
			emit released();
			emit clicked();
			}
		break;
		case Key_Escape:
			e->ignore();
		break;
		default:
			e->ignore();
	}

}




FlatButton::FlatButton( QWidget *parent, const char *name )
  : QToolButton( parent, name/*, WNoAutoErase*/ ),
    m_pressed(false)
{
  init();
}


FlatButton::~FlatButton() {}


void FlatButton::init()
{
	setUsesTextLabel(true);
	setUsesBigPixmap(true);
	setAutoRaise(true);
	setTextPosition( QToolButton::Under );
	setFocusPolicy(QWidget::StrongFocus);	
 }


void FlatButton::keyPressEvent( QKeyEvent* e )
{
	switch ( e->key() ) 
	{
		case Key_Enter:
		case Key_Return:
		case Key_Space:
		m_pressed = TRUE;
		setDown(true);
		emit pressed();
		break;
		case Key_Escape:
			e->ignore();
		break;
		default:
			e->ignore();
	}

	QToolButton::keyPressEvent(e);
}


void FlatButton::keyReleaseEvent( QKeyEvent* e )
{
	switch ( e->key() ) 
	{
		case Key_Space:
		case Key_Enter:
		case Key_Return:
			if ( m_pressed ) 
			{
			setDown(false);
			m_pressed = FALSE;
			emit released();
			emit clicked();
			}
		break;
		case Key_Escape:
			e->ignore();
		break;
		default:
			e->ignore();
	}

}


