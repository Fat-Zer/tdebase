#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqvgroupbox.h>

#include <tdeio/ioslave_defaults.h>
#include <knuminput.h>
#include <tdelocale.h>
#include <kdialog.h>
#include <tdeconfig.h>

#include "ksaveioconfig.h"
#include "netpref.h"

#define MAX_TIMEOUT_VALUE  3600

KIOPreferences::KIOPreferences( TQWidget* parent )
               :TDECModule( parent, "kcmtdeio" )
{
    TQVBoxLayout* mainLayout = new TQVBoxLayout( this, 0,
                                               KDialog::spacingHint() );
    gb_Timeout = new TQVGroupBox( i18n("Timeout Values"), this, "gb_Timeout" );
    TQWhatsThis::add( gb_Timeout, i18n("Here you can set timeout values. "
                    "You might want to tweak them if your "
                    "connection is very slow. The maximum "
                    "allowed value is %1 seconds.").arg(MAX_TIMEOUT_VALUE));
    mainLayout->addWidget( gb_Timeout );

    sb_socketRead = new KIntNumInput( gb_Timeout, "sb_socketRead" );
    sb_socketRead->setSuffix( i18n( " sec" ) );
    sb_socketRead->setLabel( i18n( "Soc&ket read:" ), AlignVCenter);
    connect(sb_socketRead, TQT_SIGNAL(valueChanged ( int )),
            this, TQT_SLOT(configChanged()));

    sb_proxyConnect = new KIntNumInput( sb_socketRead, 0, gb_Timeout,
            10, "sb_proxyConnect" );
    sb_proxyConnect->setSuffix( i18n( " sec" ) );
    sb_proxyConnect->setLabel( i18n( "Pro&xy connect:" ), AlignVCenter);
    connect(sb_proxyConnect, TQT_SIGNAL(valueChanged ( int )),
            this, TQT_SLOT(configChanged()));

    sb_serverConnect = new KIntNumInput( sb_proxyConnect, 0, gb_Timeout,
            10, "sb_serverConnect" );
    sb_serverConnect->setSuffix( i18n( " sec" ) );
    sb_serverConnect->setLabel( i18n("Server co&nnect:"), AlignVCenter);
    connect(sb_serverConnect, TQT_SIGNAL(valueChanged ( int )),
            this, TQT_SLOT(configChanged()));

    sb_serverResponse = new KIntNumInput( sb_serverConnect, 0, gb_Timeout,
            10, "sb_serverResponse" );
    sb_serverResponse->setSuffix( i18n( " sec" ) );
    sb_serverResponse->setLabel( i18n("&Server response:"), AlignVCenter);
    connect(sb_serverResponse, TQT_SIGNAL(valueChanged ( int )),
            this, TQT_SLOT(configChanged()));

    gb_Ftp = new TQVGroupBox( i18n( "FTP Options" ), this, "gb_Ftp" );
    cb_ftpEnablePasv = new TQCheckBox( i18n( "Enable passive &mode (PASV)" ), gb_Ftp );
    TQWhatsThis::add(cb_ftpEnablePasv, i18n( "Enables FTP's \"passive\" mode. This is required to allow FTP to work from behind firewalls." ));
    cb_ftpMarkPartial = new TQCheckBox( i18n( "Mark &partially uploaded files" ), gb_Ftp );
    TQWhatsThis::add(cb_ftpMarkPartial, i18n( "<p>Marks partially uploaded FTP files.</p>"
                                             "<p>When this option is enabled, partially uploaded files "
                                             "will have a \".part\" extension. This extension will be removed "
                                             "once the transfer is complete.</p>"));

    mainLayout->addWidget( gb_Ftp );

    connect(cb_ftpEnablePasv, TQT_SIGNAL(toggled(bool)), TQT_SLOT(configChanged()));
    connect(cb_ftpMarkPartial, TQT_SIGNAL(toggled(bool)), TQT_SLOT(configChanged()));

    mainLayout->addStretch();

    load();
}

KIOPreferences::~KIOPreferences()
{
}

void KIOPreferences::load()
{
  KProtocolManager proto;

  sb_socketRead->setRange( MIN_TIMEOUT_VALUE, MAX_TIMEOUT_VALUE );
  sb_serverResponse->setRange( MIN_TIMEOUT_VALUE, MAX_TIMEOUT_VALUE );
  sb_serverConnect->setRange( MIN_TIMEOUT_VALUE, MAX_TIMEOUT_VALUE );
  sb_proxyConnect->setRange( MIN_TIMEOUT_VALUE, MAX_TIMEOUT_VALUE );

  sb_socketRead->setValue( proto.readTimeout() );
  sb_serverResponse->setValue( proto.responseTimeout() );
  sb_serverConnect->setValue( proto.connectTimeout() );
  sb_proxyConnect->setValue( proto.proxyConnectTimeout() );

  TDEConfig config( "tdeio_ftprc", true, false );
  cb_ftpEnablePasv->setChecked( !config.readBoolEntry( "DisablePassiveMode", false ) );
  cb_ftpMarkPartial->setChecked( config.readBoolEntry( "MarkPartial", true ) );
  emit changed( false );
}

void KIOPreferences::save()
{
  KSaveIOConfig::setReadTimeout( sb_socketRead->value() );
  KSaveIOConfig::setResponseTimeout( sb_serverResponse->value() );
  KSaveIOConfig::setConnectTimeout( sb_serverConnect->value() );
  KSaveIOConfig::setProxyConnectTimeout( sb_proxyConnect->value() );

  TDEConfig config( "tdeio_ftprc", false, false );
  config.writeEntry( "DisablePassiveMode", !cb_ftpEnablePasv->isChecked() );
  config.writeEntry( "MarkPartial", cb_ftpMarkPartial->isChecked() );
  config.sync();

  KSaveIOConfig::updateRunningIOSlaves (this);

  emit changed( false );
}

void KIOPreferences::defaults()
{
  sb_socketRead->setValue( DEFAULT_READ_TIMEOUT );
  sb_serverResponse->setValue( DEFAULT_RESPONSE_TIMEOUT );
  sb_serverConnect->setValue( DEFAULT_CONNECT_TIMEOUT );
  sb_proxyConnect->setValue( DEFAULT_PROXY_CONNECT_TIMEOUT );

  cb_ftpEnablePasv->setChecked( true );
  cb_ftpMarkPartial->setChecked( true );

  emit changed(true);
}

TQString KIOPreferences::quickHelp() const
{
  return i18n("<h1>Network Preferences</h1>Here you can define"
              " the behavior of TDE programs when using Internet"
              " and network connections. If you experience timeouts"
              " or use a modem to connect to the Internet, you might"
              " want to adjust these settings." );
}

#include "netpref.moc"
