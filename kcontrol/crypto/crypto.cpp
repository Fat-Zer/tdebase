/**
 * crypto.cpp
 *
 * Copyright (c) 2000-2005 George Staikos <staikos@kde.org>
 *               2000 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//
//   Attention.  The suck factor on this code is increasing.  It's a bit of a
//   hack. </understatement>  It might be time to rewrite it soon.
//

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqframe.h>
#include <tqhbuttongroup.h>
#include <tqhgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqregexp.h>
#include <tqvbox.h>
#include <tqvbuttongroup.h>
#include <tqvgroupbox.h>
#include <tqwhatsthis.h>

#include <tdeaboutdata.h>
#include <kcombobox.h>
#include <tdeconfig.h>
#include <kdatepicker.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdefiledialog.h>
#include <kgenericfactory.h>
#include <tdeglobal.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <kmdcodec.h>
#include <tdemessagebox.h>
#include <kpassdlg.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kresolver.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kurlrequester.h>

#include <config.h>
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/stack.h>
#include <openssl/safestack.h>
#undef crypt
#endif

#include <ksslall.h>
#include <kopenssl.h>

#include "crypto.h"
#include "certexport.h"
#include "kdatetimedlg.h"

using namespace KNetwork;

typedef KGenericFactory<KCryptoConfig, TQWidget> KryptoFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_crypto, KryptoFactory("kcmcrypto") )

CipherItem::CipherItem( TQListView *view, const TQString& cipher, int bits,
			int maxBits, KCryptoConfig *module )
    : TQCheckListItem( view, TQString(), CheckBox )
{
    m_cipher = cipher;
    m_bits = bits;
    m_module = module;

    TQString tmp( i18n("%1 (%2 of %3 bits)") );
    setText( 0, tmp.arg( cipher ).arg( bits ).arg( maxBits ));
}

void CipherItem::stateChange( bool )
{
    m_module->configChanged();
}

TQString CipherItem::configName() const
{
    TQString cipherName("cipher_%1");
    return cipherName.arg( m_cipher );
}



OtherCertItem::OtherCertItem( TQListView *view, const TQString& sub, const TQString& md5, bool perm, int policy, TQDateTime exp, KCryptoConfig *module )
    : TQListViewItem( view, TQString() ), _sub(sub), _md5(md5), _exp(exp), _perm(perm), _policy(policy)

{
    m_module = module;
KSSLX509Map cert(sub);
    setText(0, cert.getValue("O"));
    setText(1, cert.getValue("CN").replace("\n", ", "));

    if (_exp.date().year() > 3000 || _exp.date().year() < 1900)
       _exp.setDate(TQDate(3000,1,1));
}

void OtherCertItem::stateChange( bool )
{
    m_module->configChanged();
}

TQString OtherCertItem::configName() const
{
    return _sub;
}


YourCertItem::YourCertItem( TQListView *view, TQString pkcs, TQString pass, TQString name, KCryptoConfig *module )
    : TQListViewItem( view, TQString() )

{
    m_module = module;
KSSLX509Map cert(name);
    TQString tmp = cert.getValue("CN").replace("\n", ", ");
    setText(0, tmp);
    setText(1, cert.getValue("Email"));
    _pkcs = pkcs;
    _name = name;
    _pass = pass;
}

void YourCertItem::stateChange( bool )
{
    m_module->configChanged();
}

TQString YourCertItem::configName() const
{
    return _name;
}



CAItem::CAItem( TQListView *view, TQString name, TQString cert, bool site, bool email, bool code, KCryptoConfig *module )
    : TQListViewItem( view, TQString() )

{
    m_module = module;
KSSLX509Map mcert(name);
TQString tmp;
    setText(0, mcert.getValue("O"));
    tmp = mcert.getValue("OU");
    tmp.replace("\n", ", ");
    setText(1, tmp);
    tmp = mcert.getValue("CN");
    tmp.replace("\n", ", ");
    setText(2, tmp);
    _name = name;
    _cert = cert;
    _site = site;
    _email = email;
    _code = code;
    isNew = false;
    modified = false;
}

void CAItem::stateChange( bool )
{
    m_module->configChanged();
}

TQString CAItem::configName() const
{
    return _name;
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////




KCryptoConfig::KCryptoConfig(TQWidget *parent, const char *name, const TQStringList &)
  : TDECModule(KryptoFactory::instance(), parent, name)
{
TQGridLayout *grid;
TQBoxLayout *top = new TQVBoxLayout(this);
TQString whatstr;

  setQuickHelp( i18n("<h1>Crypto</h1> This module allows you to configure SSL for"
     " use with most TDE applications, as well as manage your personal"
     " certificates and the known certificate authorities."));

  ___lehack = false;
  otherCertDelList.setAutoDelete(true);
  yourCertDelList.setAutoDelete(true);
  authDelList.setAutoDelete(true);
  caDelList.setAutoDelete(true);

  _signers = new KSSLSigners;

     TDEAboutData *about =
     new TDEAboutData(I18N_NOOP("kcmcrypto"), I18N_NOOP("TDE Crypto Control Module"),
                    0, 0, TDEAboutData::License_GPL,
                    I18N_NOOP("(c) 2000 - 2001 George Staikos"));
     about->addAuthor("George Staikos", 0, "staikos@kde.org");
     about->addAuthor("Carsten Pfeiffer", 0, "pfeiffer@kde.org");
     setAboutData( about );

  ///////////////////////////////////////////////////////////////////////////
  // Create the GUI here - there are currently a total of 6 tabs.
  // The first is SSL and cipher related
  // The second is OpenSSL settings
  // The third is user's SSL certificate related
  // The fourth is certificate authentication related
  // The fifth is other SSL certificate related
  // The sixth is CA related
  // The seventh is misc. settings related   (unimplemented)
  // The eighth is peer [email] certificate related (unimplemented)
  ///////////////////////////////////////////////////////////////////////////

  tabs = new TQTabWidget(this);
  top->addWidget(tabs);

  ///////////////////////////////////////////////////////////////////////////
  // FIRST TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSL = new TQFrame(this);
  grid = new TQGridLayout(tabSSL, 7, 2, KDialog::marginHint(),
                                       KDialog::spacingHint() );
  mUseTLS = new TQCheckBox(i18n("Enable &TLS support if supported by the server"), tabSSL);
  connect(mUseTLS, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mUseTLS, 0, 0);
  whatstr = i18n("TLS is the newest revision of the SSL protocol."
                 " It integrates better with other protocols and has"
                 " replaced SSL in protocols such as POP3 and SMTP.");
  TQWhatsThis::add(mUseTLS, whatstr);

  mUseSSLv2 = new TQCheckBox(i18n("Enable SSLv&2"), tabSSL);
  connect(mUseSSLv2, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mUseSSLv2, 1, 0);
  whatstr = i18n("SSL v2 is the second revision of the SSL protocol."
                " It is most common to enable v2 and v3.");
  TQWhatsThis::add(mUseSSLv2, whatstr);

  mUseSSLv3 = new TQCheckBox(i18n("Enable SSLv&3"), tabSSL);
  connect(mUseSSLv3, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mUseSSLv3, 1, 1);
  whatstr = i18n("SSL v3 is the third revision of the SSL protocol."
                " It is most common to enable v2 and v3.");
  TQWhatsThis::add(mUseSSLv3, whatstr);

#ifdef HAVE_SSL
  SSLv2Box = new TQListView(tabSSL, "v2ciphers");
  (void) SSLv2Box->addColumn(i18n("SSLv2 Ciphers to Use"));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL v2 protocol. The actual protocol used will be"
                " negotiated with the server at connection time.");
  TQWhatsThis::add(SSLv2Box, whatstr);
  SSLv2Box->setSelectionMode(TQListView::NoSelection);

  grid->addWidget( SSLv2Box, 2, 0 );
  connect( mUseSSLv2, TQT_SIGNAL( toggled( bool ) ),
	   SSLv2Box, TQT_SLOT( setEnabled( bool )));
#else
  TQLabel *nossllabel = new TQLabel(i18n("SSL ciphers cannot be configured"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSL);
  grid->addMultiCellWidget(nossllabel, 2, 2, 0, 1);
  grid->addRowSpacing( 3, 100 ); // give minimum height to look better
#endif

  // no need to parse kdeglobals.
  config = new TDEConfig("cryptodefaults", false, false);
  policies = new KSimpleConfig("ksslpolicies", false);
  pcerts = new KSimpleConfig("ksslcertificates", false);
  authcfg = new KSimpleConfig("ksslauthmap", false);

#ifdef HAVE_SSL
  SSLv3Box = new TQListView(tabSSL, "v3ciphers");
  (void) SSLv3Box->addColumn(i18n("SSLv3 Ciphers to Use"));
  whatstr = i18n("Select the ciphers you wish to enable when using the"
                " SSL v3 protocol. The actual protocol used will be"
                " negotiated with the server at connection time.");
  TQWhatsThis::add(SSLv3Box, whatstr);
  SSLv3Box->setSelectionMode(TQListView::NoSelection);
  grid->addWidget(SSLv3Box, 2, 1);
  connect( mUseSSLv3, TQT_SIGNAL( toggled( bool ) ),
	   SSLv3Box, TQT_SLOT( setEnabled( bool )));

  loadCiphers();

  //
  //  CipherWizards
  //
  TQHGroupBox *cwbg = new TQHGroupBox(i18n("Cipher Wizard"), tabSSL);
  TQComboBox *cwcb = new TQComboBox(cwbg);
  grid->addMultiCellWidget(cwbg, 3, 3, 0, 1);
  TQString whatStr = i18n("<qt>Use these preconfigurations to more easily configure the SSL encryption settings. You can choose among the following modes: <ul>");

  cwcb->insertItem(TQString());
  cwcb->insertItem(i18n("Most Compatible"));
  whatStr += i18n("<li><b>Most Compatible:</b> Select the settings found to be most compatible.</li>");
  cwcb->insertItem(i18n("US Ciphers Only"));
  whatStr += i18n("<li><b>US Ciphers Only:</b> Select only the US strong (&gt;= 128 bit) encryption ciphers.</li>");
  cwcb->insertItem(i18n("Export Ciphers Only"));
  whatStr += i18n("<li><b>Export Ciphers Only:</b> Select only the weak ciphers (&lt;= 56 bit).</li>");
  cwcb->insertItem(i18n("Enable All"));
  whatStr += i18n("<li><b>Enable All:</b> Select all SSL ciphers and methods.</li></ul>");

  TQWhatsThis::add(cwcb, whatStr);

  connect(cwcb, TQT_SIGNAL(activated(int)), TQT_SLOT(slotSelectCipher(int)));




#endif

  mWarnOnEnter = new TQCheckBox(i18n("Warn on &entering SSL mode"), tabSSL);
  connect(mWarnOnEnter, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mWarnOnEnter, 5, 0);
  whatstr = i18n("If selected, you will be notified when entering an SSL"
                " enabled site");
  TQWhatsThis::add(mWarnOnEnter, whatstr);

  mWarnOnLeave = new TQCheckBox(i18n("Warn on &leaving SSL mode"), tabSSL);
  connect(mWarnOnLeave, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mWarnOnLeave, 5, 1);
  whatstr = i18n("If selected, you will be notified when leaving an SSL"
                " based site.");
  TQWhatsThis::add(mWarnOnLeave, whatstr);

  mWarnOnUnencrypted = new TQCheckBox(i18n("Warn on sending &unencrypted data"), tabSSL);
  connect(mWarnOnUnencrypted, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mWarnOnUnencrypted, 6, 0);
  whatstr = i18n("If selected, you will be notified before sending"
                " unencrypted data via a web browser.");
  TQWhatsThis::add(mWarnOnUnencrypted, whatstr);

#if 0  // NOT IMPLEMENTED IN KDE 3.0
  mWarnOnMixed = new TQCheckBox(i18n("Warn on &mixed SSL/non-SSL pages"), tabSSL);
  connect(mWarnOnMixed, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addWidget(mWarnOnMixed, 6, 1);
  whatstr = i18n("If selected, you will be notified if you view a page"
                " that has both encrypted and non-encrypted parts.");
  TQWhatsThis::add(mWarnOnMixed, whatstr);
#endif

  ///////////////////////////////////////////////////////////////////////////
  // SECOND TAB
  ///////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SSL
  tabOSSL = new TQFrame(this);
  TQBoxLayout *vbox = new TQVBoxLayout(tabOSSL, KDialog::marginHint(), KDialog::spacingHint());

  oInfo = new TQVGroupBox(i18n("Path to OpenSSL Shared Libraries"), tabOSSL);
  vbox->addWidget(oInfo);
  oPath = new KURLRequester(oInfo);
  oPath->setMode(KFile::Directory);
  oTest = new TQPushButton(i18n("&Test"), oInfo);
  connect(oTest, TQT_SIGNAL(clicked()), TQT_SLOT(slotTestOSSL()));

  connect(oPath, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(configChanged()));

  //
  //  Settings for the EGD
  //
  TQFrame *eFrame = new TQFrame(tabOSSL);
  TQVBoxLayout *egrid = new TQVBoxLayout(eFrame);
  mUseEGD = new TQCheckBox(i18n("Use EGD"), eFrame);
  connect(mUseEGD, TQT_SIGNAL(clicked()), TQT_SLOT(slotUseEGD()));
  mUseEFile = new TQCheckBox(i18n("Use entropy file"), eFrame);
  connect(mUseEFile, TQT_SIGNAL(clicked()), TQT_SLOT(slotUseEFile()));
  vbox->addWidget(eFrame);
  egrid->addWidget(mUseEGD);
  egrid->addWidget(mUseEFile);
  TQFrame *egdframe = new TQFrame(tabOSSL);
  TQGridLayout *grid2 = new TQGridLayout(egdframe, 2, 2, KDialog::marginHint(),
                                                       KDialog::spacingHint());
  mEGDLabel = new TQLabel(i18n("Path to EGD:"), egdframe);
  grid2->addWidget(mEGDLabel, 0, 0);
  mEGDPath = new KURLRequester(egdframe);
  grid2->addWidget(mEGDPath, 0, 1);
  connect(mEGDPath, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(configChanged()));
  vbox->addWidget(egdframe);
  whatstr = i18n("If selected, OpenSSL will be asked to use the entropy gathering"
          " daemon (EGD) for initializing the pseudo-random number generator.");
  TQWhatsThis::add(mUseEGD, whatstr);
  whatstr = i18n("If selected, OpenSSL will be asked to use the given file"
          " as entropy for initializing the pseudo-random number generator.");
  TQWhatsThis::add(mUseEFile, whatstr);
  whatstr = i18n("Enter the path to the socket created by the entropy gathering"
                " daemon (or the entropy file) here.");
  TQWhatsThis::add(mEGDPath, whatstr);
  whatstr = i18n("Click here to browse for the EGD socket file.");
  TQWhatsThis::add(mEGDPath, whatstr);

  vbox->addStretch();
#endif



  ///////////////////////////////////////////////////////////////////////////
  // THIRD TAB
  ///////////////////////////////////////////////////////////////////////////
  tabYourSSLCert = new TQFrame(this);

#ifdef HAVE_SSL
  grid = new TQGridLayout(tabYourSSLCert, 16, 6, KDialog::marginHint(), KDialog::spacingHint() );

  yourSSLBox = new TQListView(tabYourSSLCert);
  yourSSLBox->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificates of yours TDE"
                " knows about. You can easily manage them from here.");
  TQWhatsThis::add(yourSSLBox, whatstr);
  grid->addMultiCellWidget(yourSSLBox, 0, 5, 0, 4);
  yourSSLBox->addColumn(i18n("Common Name"));
  yourSSLBox->addColumn(i18n("Email Address"));
  connect(yourSSLBox, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotYourCertSelect()));

  yourSSLImport = new TQPushButton(i18n("I&mport..."), tabYourSSLCert);
  connect(yourSSLImport, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourImport()));
  grid->addWidget(yourSSLImport, 0, 5);

  yourSSLExport = new TQPushButton(i18n("&Export..."), tabYourSSLCert);
  yourSSLExport->setEnabled(false);
  connect(yourSSLExport, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourExport()));
  grid->addWidget(yourSSLExport, 1, 5);

  yourSSLRemove = new TQPushButton(i18n("Remo&ve"), tabYourSSLCert);
  yourSSLRemove->setEnabled(false);
  connect(yourSSLRemove, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourRemove()));
  grid->addWidget(yourSSLRemove, 2, 5);

  yourSSLUnlock = new TQPushButton(i18n("&Unlock"), tabYourSSLCert);
  yourSSLUnlock->setEnabled(false);
  connect(yourSSLUnlock, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourUnlock()));
  grid->addWidget(yourSSLUnlock, 3, 5);

  yourSSLVerify = new TQPushButton(i18n("Verif&y"), tabYourSSLCert);
  yourSSLVerify->setEnabled(false);
  connect(yourSSLVerify, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourVerify()));
  grid->addWidget(yourSSLVerify, 4, 5);

  yourSSLPass = new TQPushButton(i18n("Chan&ge Password..."), tabYourSSLCert);
  yourSSLPass->setEnabled(false);
  connect(yourSSLPass, TQT_SIGNAL(clicked()), TQT_SLOT(slotYourPass()));
  grid->addWidget(yourSSLPass, 5, 5);

  grid->addMultiCellWidget(new KSeparator(KSeparator::HLine, tabYourSSLCert), 6, 6, 0, 5);
  ySubject = KSSLInfoDlg::certInfoWidget(tabYourSSLCert, TQString(TQString()));
  yIssuer = KSSLInfoDlg::certInfoWidget(tabYourSSLCert, TQString(TQString()));
  grid->addMultiCellWidget(ySubject, 7, 11, 0, 2);
  grid->addMultiCellWidget(yIssuer, 7, 11, 3, 5);
  whatstr = i18n("This is the information known about the owner of the certificate.");
  TQWhatsThis::add(ySubject, whatstr);
  whatstr = i18n("This is the information known about the issuer of the certificate.");
  TQWhatsThis::add(yIssuer, whatstr);

  grid->addWidget(new TQLabel(i18n("Valid from:"), tabYourSSLCert), 12, 0);
  grid->addWidget(new TQLabel(i18n("Valid until:"), tabYourSSLCert), 13, 0);
  yValidFrom = new TQLabel(tabYourSSLCert);
  grid->addWidget(yValidFrom, 12, 1);
  yValidUntil = new TQLabel(tabYourSSLCert);
  grid->addWidget(yValidUntil, 13, 1);
  whatstr = i18n("The certificate is valid starting at this date.");
  TQWhatsThis::add(yValidFrom, whatstr);
  whatstr = i18n("The certificate is valid until this date.");
  TQWhatsThis::add(yValidUntil, whatstr);
  grid->addWidget(new TQLabel(i18n("MD5 digest:"), tabYourSSLCert), 14, 0);
  yHash = new TQLabel(tabYourSSLCert);
  grid->addWidget(yHash, 14, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  TQWhatsThis::add(yHash, whatstr);

#if 0
  TQHButtonGroup *ocbg = new TQHButtonGroup(i18n("On SSL Connection..."), tabYourSSLCert);
  yourSSLUseDefault = new TQRadioButton(i18n("&Use default certificate"), ocbg);
  yourSSLList = new TQRadioButton(i18n("&List upon connection"), ocbg);
  yourSSLDont = new TQRadioButton(i18n("&Do not use certificates"), ocbg);
  grid->addMultiCellWidget(ocbg, 14, 14, 0, 5);
#endif
#else
  nossllabel = new TQLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabYourSSLCert);
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 5);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // FOURTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabAuth = new TQFrame(this);

#ifdef HAVE_SSL
  grid = new TQGridLayout(tabAuth, 20, 6, KDialog::marginHint(), KDialog::spacingHint());

  grid->addMultiCellWidget(new TQLabel(i18n("Default Authentication Certificate"), tabAuth), 0, 0, 0, 2);
  defCertBG = new TQVButtonGroup(i18n("Default Action"), tabAuth);
  defSend = new TQRadioButton(i18n("&Send"), defCertBG);
  defPrompt = new TQRadioButton(i18n("&Prompt"), defCertBG);
  defDont = new TQRadioButton(i18n("Do &not send"), defCertBG);
  grid->addMultiCellWidget(defCertBG, 1, 3, 0, 2);
  grid->addMultiCellWidget(new TQLabel(i18n("Default certificate:"), tabAuth), 1, 1, 3, 5);
  defCertBox = new KComboBox(false, tabAuth);
  grid->addMultiCellWidget(defCertBox, 2, 2, 3, 5);

  grid->addMultiCellWidget(new KSeparator(KSeparator::HLine, tabAuth), 4, 4, 0, 5);


  grid->addMultiCellWidget(new TQLabel(i18n("Host authentication:"), tabAuth), 5, 5, 0, 1);
  hostAuthList = new TQListView(tabAuth);
  hostAuthList->setAllColumnsShowFocus(true);
  grid->addMultiCellWidget(hostAuthList, 6, 13, 0, 5);
  hostAuthList->addColumn(i18n("Host"));
  hostAuthList->addColumn(i18n("Certificate"));
  hostAuthList->addColumn(i18n("Policy"));

  grid->addWidget(new TQLabel(i18n("Host:"), tabAuth), 14, 0);
  grid->addWidget(new TQLabel(i18n("Certificate:"), tabAuth), 15, 0);

  authHost = new TQLineEdit(tabAuth);
  grid->addMultiCellWidget(authHost, 14, 14, 1, 4);
  hostCertBox = new KComboBox(false, tabAuth);
  grid->addMultiCellWidget(hostCertBox, 15, 15, 1, 4);

  hostCertBG = new TQHButtonGroup(i18n("Action"), tabAuth);
  hostSend = new TQRadioButton(i18n("Send"), hostCertBG);
  hostPrompt = new TQRadioButton(i18n("Prompt"), hostCertBG);
  hostDont = new TQRadioButton(i18n("Do not send"), hostCertBG);
  grid->addMultiCellWidget(hostCertBG, 16, 16, 0, 5);

  authAdd = new TQPushButton(i18n("Ne&w"), tabAuth);
  authRemove = new TQPushButton(i18n("Remo&ve"), tabAuth);
  grid->addWidget(authAdd, 17, 4);
  grid->addWidget(authRemove, 17, 5);

  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertBG->setEnabled(false);
  authRemove->setEnabled(false);

  connect(defCertBox, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
  connect(defCertBG, TQT_SIGNAL(clicked(int)), this, TQT_SLOT(configChanged()));
  connect(hostAuthList, TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(slotAuthItemChanged()));
  connect(authAdd, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotNewHostAuth()));
  connect(authRemove, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotRemoveHostAuth()));
  connect(authHost, TQT_SIGNAL(textChanged(const TQString &)), this, TQT_SLOT(slotAuthText(const TQString &)));
  connect(hostCertBG, TQT_SIGNAL(clicked(int)), this, TQT_SLOT(slotAuthButtons()));
  connect(hostCertBox, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotAuthCombo()));

#else
  nossllabel = new TQLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabAuth);
  grid->addMultiCellWidget(nossllabel, 3, 3, 0, 5);
#endif



  ///////////////////////////////////////////////////////////////////////////
  // FIFTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabOtherSSLCert = new TQFrame(this);

#ifdef HAVE_SSL
  oGrid = grid = new TQGridLayout(tabOtherSSLCert, 21, 6, KDialog::marginHint(), KDialog::spacingHint());

  otherSSLBox = new TQListView(tabOtherSSLCert);
  otherSSLBox->setAllColumnsShowFocus(true);
  connect(otherSSLBox, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotOtherCertSelect()));
  whatstr = i18n("This list box shows which site and person certificates TDE"
                " knows about. You can easily manage them from here.");
  TQWhatsThis::add(otherSSLBox, whatstr);
  otherSSLBox->addColumn(i18n("Organization"));
  otherSSLBox->addColumn(i18n("Common Name"));
  grid->addMultiCellWidget(otherSSLBox, 0, 7, 0, 4);

  otherSSLExport = new TQPushButton(i18n("&Export..."), tabOtherSSLCert);
  connect(otherSSLExport, TQT_SIGNAL(clicked()), TQT_SLOT(slotExportCert()));
  grid->addWidget(otherSSLExport, 0, 5);
  whatstr = i18n("This button allows you to export the selected certificate"
                " to a file of various formats.");
  TQWhatsThis::add(otherSSLExport, whatstr);

  otherSSLRemove = new TQPushButton(i18n("&Remove"), tabOtherSSLCert);
  connect(otherSSLRemove, TQT_SIGNAL(clicked()), TQT_SLOT(slotRemoveCert()));
  grid->addWidget(otherSSLRemove, 1, 5);
  whatstr = i18n("This button removes the selected certificate"
                " from the certificate cache.");
  TQWhatsThis::add(otherSSLRemove, whatstr);

  otherSSLVerify = new TQPushButton(i18n("&Verify"), tabOtherSSLCert);
  connect(otherSSLVerify, TQT_SIGNAL(clicked()), TQT_SLOT(slotVerifyCert()));
  grid->addWidget(otherSSLVerify, 2, 5);
  whatstr = i18n("This button tests the selected certificate"
                " for validity.");
  TQWhatsThis::add(otherSSLVerify, whatstr);

      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);

  grid->addMultiCellWidget(new KSeparator(KSeparator::HLine, tabOtherSSLCert), 8, 8, 0, 5);
  oSubject = KSSLInfoDlg::certInfoWidget(tabOtherSSLCert, TQString(TQString()));
  oIssuer = KSSLInfoDlg::certInfoWidget(tabOtherSSLCert, TQString(TQString()));
  grid->addMultiCellWidget(oSubject, 9, 13, 0, 2);
  grid->addMultiCellWidget(oIssuer, 9, 13, 3, 5);
  whatstr = i18n("This is the information known about the owner of the certificate.");
  TQWhatsThis::add(oSubject, whatstr);
  whatstr = i18n("This is the information known about the issuer of the certificate.");
  TQWhatsThis::add(oIssuer, whatstr);

  fromLabel = new TQLabel(i18n("Valid from:"), tabOtherSSLCert);
  untilLabel = new TQLabel(i18n("Valid until:"), tabOtherSSLCert);
  grid->addWidget(fromLabel, 14, 0);
  grid->addWidget(untilLabel, 15, 0);
  fromLabel->setEnabled(false);
  untilLabel->setEnabled(false);
  validFrom = new TQLabel(tabOtherSSLCert);
  grid->addWidget(validFrom, 14, 1);
  validUntil = new TQLabel(tabOtherSSLCert);
  grid->addWidget(validUntil, 15, 1);
  whatstr = i18n("The certificate is valid starting at this date.");
  TQWhatsThis::add(validFrom, whatstr);
  whatstr = i18n("The certificate is valid until this date.");
  TQWhatsThis::add(validUntil, whatstr);

  cacheGroup = new TQVButtonGroup(i18n("Cache"), tabOtherSSLCert);
  cachePerm = new TQRadioButton(i18n("Permanentl&y"), cacheGroup);
  cacheUntil = new TQRadioButton(i18n("&Until"), cacheGroup);
  untilDate = new KURLLabel(TQString(), TQString(), cacheGroup);
  cacheGroup->setEnabled(false);
  grid->addMultiCellWidget(cacheGroup, 16, 19, 0, 2);

  cachePerm->setEnabled(false);
  cacheUntil->setEnabled(false);
  untilDate->setEnabled(false);
  connect(cachePerm, TQT_SIGNAL(clicked()), TQT_SLOT(slotPermanent()));
  connect(cacheUntil, TQT_SIGNAL(clicked()), TQT_SLOT(slotUntil()));
  connect(untilDate, TQT_SIGNAL(leftClickedURL()), TQT_SLOT(slotDatePick()));
  whatstr = i18n("Select here to make the cache entry permanent.");
  TQWhatsThis::add(cachePerm, whatstr);
  whatstr = i18n("Select here to make the cache entry temporary.");
  TQWhatsThis::add(cacheUntil, whatstr);
  whatstr = i18n("The date and time until the certificate cache entry should expire.");
  TQWhatsThis::add(untilDate, whatstr);

  policyGroup = new TQVButtonGroup(i18n("Policy"), tabOtherSSLCert);
  policyAccept = new TQRadioButton(i18n("Accep&t"), policyGroup);
  policyReject = new TQRadioButton(i18n("Re&ject"), policyGroup);
  policyPrompt = new TQRadioButton(i18n("&Prompt"), policyGroup);
  policyGroup->setEnabled(false);
  grid->addMultiCellWidget(policyGroup, 16, 19, 3, 5);
  connect(policyGroup, TQT_SIGNAL(clicked(int)), TQT_SLOT(slotPolicyChanged(int)));
  whatstr = i18n("Select this to always accept this certificate.");
  TQWhatsThis::add(policyAccept, whatstr);
  whatstr = i18n("Select this to always reject this certificate.");
  TQWhatsThis::add(policyReject, whatstr);
  whatstr = i18n("Select this if you wish to be prompted for action when receiving this certificate.");
  TQWhatsThis::add(policyPrompt, whatstr);
  grid->addWidget(new TQLabel(i18n("MD5 digest:"), tabOtherSSLCert), 20, 0);
  pHash = new TQLabel(tabOtherSSLCert);
  grid->addWidget(pHash, 20, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  TQWhatsThis::add(pHash, whatstr);

#else
  nossllabel = new TQLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabOtherSSLCert);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


  ///////////////////////////////////////////////////////////////////////////
  // SIXTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCA = new TQFrame(this);

#ifdef HAVE_SSL
  grid = new TQGridLayout(tabSSLCA, 11, 8, KDialog::marginHint(), KDialog::spacingHint());

  caList = new TQListView(tabSSLCA);
  caList->setAllColumnsShowFocus(true);
  whatstr = i18n("This list box shows which certificate authorities TDE"
                 " knows about. You can easily manage them from here.");
  TQWhatsThis::add(caList, whatstr);
  grid->addMultiCellWidget(caList, 0, 3, 0, 6);
  caList->addColumn(i18n("Organization"));
  caList->addColumn(i18n("Organizational Unit"));
  caList->addColumn(i18n("Common Name"));
  connect(caList, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotCAItemChanged()));

  caSSLImport = new TQPushButton(i18n("I&mport..."), tabSSLCA);
  connect(caSSLImport, TQT_SIGNAL(clicked()), TQT_SLOT(slotCAImport()));
  grid->addWidget(caSSLImport, 0, 7);

  caSSLRemove = new TQPushButton(i18n("&Remove"), tabSSLCA);
  connect(caSSLRemove, TQT_SIGNAL(clicked()), TQT_SLOT(slotCARemove()));
  grid->addWidget(caSSLRemove, 1, 7);
  caSSLRemove->setEnabled(false);

  caSSLRestore = new TQPushButton(i18n("Res&tore"), tabSSLCA);
  connect(caSSLRestore, TQT_SIGNAL(clicked()), TQT_SLOT(slotCARestore()));
  grid->addWidget(caSSLRestore, 2, 7);

  caSubject = KSSLInfoDlg::certInfoWidget(tabSSLCA, TQString(TQString()));
  caIssuer = KSSLInfoDlg::certInfoWidget(tabSSLCA, TQString(TQString()));
  grid->addMultiCellWidget(caSubject, 4, 6, 0, 3);
  grid->addMultiCellWidget(caIssuer, 4, 6, 4, 7);

  // Accept for Web Site Signing, Email Signing, Code Signing
  caSite = new TQCheckBox(i18n("Accept for site signing"), tabSSLCA);
  caEmail = new TQCheckBox(i18n("Accept for email signing"), tabSSLCA);
  caCode = new TQCheckBox(i18n("Accept for code signing"), tabSSLCA);
  grid->addMultiCellWidget(caSite, 7, 7, 0, 3);
  connect(caSite, TQT_SIGNAL(clicked()), TQT_SLOT(slotCAChecked()));
  grid->addMultiCellWidget(caEmail, 8, 8, 0, 3);
  connect(caEmail, TQT_SIGNAL(clicked()), TQT_SLOT(slotCAChecked()));
  grid->addMultiCellWidget(caCode, 9, 9, 0, 3);
  connect(caCode, TQT_SIGNAL(clicked()), TQT_SLOT(slotCAChecked()));
  caSite->setEnabled(false);
  caEmail->setEnabled(false);
  caCode->setEnabled(false);
  grid->addWidget(new TQLabel(i18n("MD5 digest:"), tabSSLCA), 10, 0);
  cHash = new TQLabel(tabSSLCA);
  grid->addWidget(cHash, 10, 1);
  whatstr = i18n("A hash of the certificate used to identify it quickly.");
  TQWhatsThis::add(cHash, whatstr);

#else
  nossllabel = new TQLabel(i18n("SSL certificates cannot be managed"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSLCA);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif


#if 0
  ///////////////////////////////////////////////////////////////////////////
  // SEVENTH TAB
  ///////////////////////////////////////////////////////////////////////////
  tabSSLCOpts = new TQFrame(this);

#ifdef HAVE_SSL
  grid = new TQGridLayout(tabSSLCOpts, 9, 4, KDialog::marginHint(), KDialog::spacingHint());
  mWarnSelfSigned = new TQCheckBox(i18n("Warn on &self-signed certificates or unknown CA's"), tabSSLCOpts);
  connect(mWarnSelfSigned, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  mWarnExpired = new TQCheckBox(i18n("Warn on &expired certificates"), tabSSLCOpts);
  connect(mWarnExpired, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  mWarnRevoked = new TQCheckBox(i18n("Warn on re&voked certificates"), tabSSLCOpts);
  connect(mWarnRevoked, TQT_SIGNAL(clicked()), TQT_SLOT(configChanged()));
  grid->addMultiCellWidget(mWarnSelfSigned, 0, 0, 0, 3);
  grid->addMultiCellWidget(mWarnExpired, 1, 1, 0, 3);
  grid->addMultiCellWidget(mWarnRevoked, 2, 2, 0, 3);

  macCert = new TQLineEdit(tabSSLCOpts);
  grid->addMultiCellWidget(macCert, 4, 4, 0, 2);

  macBox = new TQListBox(tabSSLCOpts);
  whatstr = i18n("This list box shows which sites you have decided to accept"
                " a certificate from even though the certificate might fail"
                " the validation procedure.");
  TQWhatsThis::add(macBox, whatstr);
  caSSLBox->setSelectionMode(TQListBox::Single);
  caSSLBox->setColumnMode(TQListBox::FixedNumber);
  grid->addMultiCellWidget(macBox, 5, 8, 0, 2);

  macAdd = new TQPushButton(i18n("&Add"), tabSSLCOpts);
  //connect(macAdd, TQT_SIGNAL(), TQT_SLOT());
  grid->addWidget(macAdd, 4, 3);

  macRemove = new TQPushButton(i18n("&Remove"), tabSSLCOpts);
  //connect(macRemove, TQT_SIGNAL(), TQT_SLOT());
  grid->addWidget(macRemove, 5, 3);

  macClear = new KPushButton(KGuiItem::clear(), tabSSLCOpts);
  //connect(macAdd, TQT_SIGNAL(), TQT_SLOT());
  grid->addWidget(macClear, 6, 3);

#else
  nossllabel = new TQLabel(i18n("These options are not configurable"
                               " because this module was not linked"
                               " with OpenSSL."), tabSSLCOpts);
  grid->addMultiCellWidget(nossllabel, 1, 1, 0, 1);
#endif
#endif

  ///////////////////////////////////////////////////////////////////////////
  // Add the tabs and startup
  ///////////////////////////////////////////////////////////////////////////
  tabs->addTab(tabSSL, i18n("SSL"));
#ifdef HAVE_SSL
  tabs->addTab(tabOSSL, i18n("OpenSSL"));
#endif
  tabs->addTab(tabYourSSLCert, i18n("Your Certificates"));
  tabs->addTab(tabAuth, i18n("Authentication"));
  tabs->addTab(tabOtherSSLCert, i18n("Peer SSL Certificates"));
  tabs->addTab(tabSSLCA, i18n("SSL Signers"));

#if 0
  tabs->addTab(tabSSLCOpts, i18n("Validation Options"));
#endif

  tabs->resize(tabs->sizeHint());
  load();
}

KCryptoConfig::~KCryptoConfig()
{
    delete config;
    delete policies;
    delete pcerts;
    delete authcfg;
    delete _signers;
}

void KCryptoConfig::configChanged()
{
    emit changed(true);
}

void KCryptoConfig::load()
{
   load( false );
}

void KCryptoConfig::load( bool useDefaults )
{
  config->setReadDefaults( useDefaults );
#ifdef HAVE_SSL
  otherCertDelList.clear();
  yourCertDelList.clear();
  authDelList.clear();
  caDelList.clear();
  config->setGroup("TLS");
  mUseTLS->setChecked(config->readBoolEntry("Enabled", true));

  config->setGroup("SSLv2");
  mUseSSLv2->setChecked(config->readBoolEntry("Enabled", true));
#ifdef OPENSSL_NO_SSL2
  mUseSSLv2->setChecked(false);
  mUseSSLv2->setEnabled(false);
#endif

  config->setGroup("SSLv3");
  mUseSSLv3->setChecked(config->readBoolEntry("Enabled", true));

  config->setGroup("Warnings");
  mWarnOnEnter->setChecked(config->readBoolEntry("OnEnter", false));
  mWarnOnLeave->setChecked(config->readBoolEntry("OnLeave", true));
  mWarnOnUnencrypted->setChecked(config->readBoolEntry("OnUnencrypted", true));

#if 0 // NOT IMPLEMENTED IN KDE 2.0
  mWarnOnMixed->setChecked(config->readBoolEntry("OnMixed", true));

  config->setGroup("Validation");
  mWarnSelfSigned->setChecked(config->readBoolEntry("WarnSelfSigned", true));
  mWarnExpired->setChecked(config->readBoolEntry("WarnExpired", true));
  mWarnRevoked->setChecked(config->readBoolEntry("WarnRevoked", true));
#endif

  config->setGroup("EGD");
  slotUseEGD();  // set the defaults
  if (config->readBoolEntry("UseEGD", false)) {
    mUseEGD->setChecked(true);
    slotUseEGD();
  } else if (config->readBoolEntry("UseEFile", false)) {
    mUseEFile->setChecked(true);
    slotUseEFile();
  }
  mEGDPath->setURL(config->readPathEntry("EGDPath"));


#ifdef HAVE_SSL
  config->setGroup("OpenSSL");
  oPath->setURL(config->readPathEntry("Path"));
#endif

  config->setGroup("SSLv2");
  CipherItem *item = static_cast<CipherItem *>(SSLv2Box->firstChild());
  while ( item ) {
      item->setOn(config->readBoolEntry(item->configName(),
					item->bits() >= 56));
      item = static_cast<CipherItem *>(item->nextSibling());
  }

  config->setGroup("SSLv3");
  item = static_cast<CipherItem *>(SSLv3Box->firstChild());
  while ( item ) {
      item->setOn(config->readBoolEntry(item->configName(),
					item->bits() >= 56));
      item = static_cast<CipherItem *>(item->nextSibling());
  }

#ifdef OPENSSL_NO_SSL2
  SSLv2Box->setEnabled( false );
#else
  SSLv2Box->setEnabled( mUseSSLv2->isChecked() );
#endif
  SSLv3Box->setEnabled( mUseSSLv3->isChecked() );

  TQStringList groups = policies->groupList();

  otherSSLBox->clear();
  for (TQStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
    if ((*i).isEmpty() || *i == "<default>" || *i == "General") continue;
    policies->setGroup(*i);
    KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", TQString()).local8Bit());
    if (cert) {
      new OtherCertItem(otherSSLBox, cert->getSubject(), *i,
                        policies->readBoolEntry("Permanent", true),
                        policies->readNumEntry("Policy", 3),
                        policies->readDateTimeEntry("Expires"), this );
      delete cert;
    }
  }

  groups = pcerts->groupList();

  yourSSLBox->clear();
  for (TQStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    pcerts->setGroup(*i);
    YourCertItem *j = new YourCertItem(yourSSLBox,
                     pcerts->readEntry("PKCS12Base64"),
                     pcerts->readEntry("Password"),
                     *i, this );
    j->setPassCache(TQString());
  }

  setAuthCertLists();

  config->setGroup("Auth");
  TQString whichAuth = config->readEntry("AuthMethod", "none");
  if (whichAuth == "send")
    defCertBG->setButton(defCertBG->id(defSend));
  else if (whichAuth == "prompt")
    defCertBG->setButton(defCertBG->id(defPrompt));
  else
    defCertBG->setButton(defCertBG->id(defDont));

  TQString whichCert = config->readEntry("DefaultCert");
  defCertBox->setCurrentItem(0);
  for (int i = 0; i < defCertBox->count(); i++) {
     if (defCertBox->text(i) == whichCert) {
       defCertBox->setCurrentItem(i);
       break;
     }
  }
  hostAuthList->clear();
  groups = authcfg->groupList();
  for (TQStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    authcfg->setGroup(*i);
    KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;
    if (authcfg->readBoolEntry("send", false) == true)
       aa = KSSLCertificateHome::AuthSend;
    else if (authcfg->readBoolEntry("prompt", false) == true)
       aa = KSSLCertificateHome::AuthPrompt;
    HostAuthItem *j = new HostAuthItem(hostAuthList,
                                       KResolver::domainToAscii(*i),
                                       authcfg->readEntry("certificate"),
                                       this );
    j->setAction(aa);
    j->setOriginalName(*i);
  }

  groups = _signers->list();
  TDEConfig sigcfg("ksslcalist", true, false);
  caList->clear();
  for (TQStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    if (!sigcfg.hasGroup(*i)) continue;
    sigcfg.setGroup(*i);
    if (!sigcfg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     sigcfg.readEntry("x509"),
                     sigcfg.readBoolEntry("site", false),
                     sigcfg.readBoolEntry("email", false),
                     sigcfg.readBoolEntry("code", false),
                     this );
  }

  slotCAItemChanged();
  slotOtherCertSelect();
  slotYourCertSelect();
#endif

  emit changed( useDefaults );
}

void KCryptoConfig::save()
{
#ifdef HAVE_SSL
  if (!mUseSSLv2->isChecked() &&
      !mUseSSLv3->isChecked())
    KMessageBox::information(this, i18n("If you do not select at least one"
                                       " SSL algorithm, either SSL will not"
                                       " work or the application may be"
                                       " forced to choose a suitable default."),
                                   i18n("SSL"));

  config->setGroup("TLS");
  config->writeEntry("Enabled", mUseTLS->isChecked());

  config->setGroup("SSLv2");
#ifdef OPENSSL_NO_SSL2
  config->writeEntry("Enabled", false);
#else
  config->writeEntry("Enabled", mUseSSLv2->isChecked());
#endif

  config->setGroup("SSLv3");
  config->writeEntry("Enabled", mUseSSLv3->isChecked());

  config->setGroup("Warnings");
  config->writeEntry("OnEnter", mWarnOnEnter->isChecked());
  config->writeEntry("OnLeave", mWarnOnLeave->isChecked());
  config->writeEntry("OnUnencrypted", mWarnOnUnencrypted->isChecked());

  config->setGroup("EGD");
  config->writeEntry("UseEGD", mUseEGD->isChecked());
  config->writeEntry("UseEFile", mUseEFile->isChecked());
  config->writePathEntry("EGDPath", mEGDPath->url());

#if 0  // NOT IMPLEMENTED IN KDE 2.0
  config->writeEntry("OnMixed", mWarnOnMixed->isChecked());

  config->setGroup("Validation");
  config->writeEntry("WarnSelfSigned", mWarnSelfSigned->isChecked());
  config->writeEntry("WarnExpired", mWarnExpired->isChecked());
  config->writeEntry("WarnRevoked", mWarnRevoked->isChecked());
#endif

#ifdef HAVE_SSL
  config->setGroup("OpenSSL");
  config->writePathEntry("Path", oPath->url());
#endif

  int ciphercount = 0;
  config->setGroup("SSLv2");
  CipherItem *item = static_cast<CipherItem *>(SSLv2Box->firstChild());
  while ( item ) {
    if (item->isOn()) {
      config->writeEntry(item->configName(), true);
      ciphercount++;
    } else config->writeEntry(item->configName(), false);

    item = static_cast<CipherItem *>(item->nextSibling());
  }

  if (mUseSSLv2->isChecked() && ciphercount == 0)
    KMessageBox::information(this, i18n("If you do not select at least one"
                                       " cipher, SSLv2 will not work."),
                                   i18n("SSLv2 Ciphers"));

  ciphercount = 0;
  config->setGroup("SSLv3");
  item = static_cast<CipherItem *>(SSLv3Box->firstChild());
  while ( item ) {
    if (item->isOn()) {
      config->writeEntry(item->configName(), true);
      ciphercount++;
    } else config->writeEntry(item->configName(), false);

    item = static_cast<CipherItem *>(item->nextSibling());
  }

  KSSLCertificateCache _cc;

  if (mUseSSLv3->isChecked() && ciphercount == 0)
    KMessageBox::information(this, i18n("If you do not select at least one"
                                       " cipher, SSLv3 will not work."),
                                   i18n("SSLv3 Ciphers"));
  // SSL Policies code
  for (OtherCertItem *x = otherCertDelList.first(); x != 0; x = otherCertDelList.next()) {
     KSSLX509Map cert(x->configName());
     TQString thisCN = cert.getValue("CN");
     _cc.removeByCN(thisCN);
     otherCertDelList.remove(x);
  }
  // Go through the non-deleted ones and save them
  for (OtherCertItem *x =
        static_cast<OtherCertItem *>(otherSSLBox->firstChild());
                                                              x;
             x = static_cast<OtherCertItem *>(x->nextSibling())) {
     KSSLX509Map cert(x->configName());
     TQString thisCN = cert.getValue("CN");
     TQDateTime expires = x->getExpires();
     _cc.modifyByCN(thisCN, (KSSLCertificateCache::KSSLCertificatePolicy)x->getPolicy(), x->isPermanent(), expires);
  }

  // SSL Personal certificates code
  for (YourCertItem *x = yourCertDelList.first(); x != 0; x = yourCertDelList.next()) {
     pcerts->deleteGroup(x->configName());
     yourCertDelList.remove(x);
  }
  // Go through the non-deleted ones and save them
  for (YourCertItem *x =
        static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                            x;
             x = static_cast<YourCertItem *>(x->nextSibling())) {
     pcerts->setGroup(x->configName());
     pcerts->writeEntry("PKCS12Base64", x->getPKCS());
     pcerts->writeEntry("Password", x->getPass());
  }

  bool doGen = false;

  // CA certificates code
  for (CAItem *x = caDelList.first(); x != 0; x = caDelList.next()) {
     _signers->remove(x->configName());
     caDelList.remove(x);
     doGen = true;
  }

  // Go through the non-deleted ones and save them
  for (CAItem *x = static_cast<CAItem *>(caList->firstChild()); x;
               x = static_cast<CAItem *>(x->nextSibling())) {
     if (!x->modified && !x->isNew) continue;
     if (x->isNew) {
        x->isNew = false;
        _signers->addCA(x->getCert(),
                        x->getSite(),
                        x->getEmail(),
                        x->getCode());
     } else {
        _signers->setUse(x->configName(),
                         x->getSite(),
                         x->getEmail(),
                         x->getCode());
     }
     x->modified = false;
     doGen = true;
  }

  if (doGen) genCAList();


  config->setGroup("Auth");
  TQString whichAuth = config->readEntry("AuthMethod", "none");
  if (defCertBG->selected() == defSend)
    config->writeEntry("AuthMethod", "send");
  else if (defCertBG->selected() == defPrompt)
    config->writeEntry("AuthMethod", "prompt");
  else
    config->writeEntry("AuthMethod", "none");

  if (defCertBox->currentItem() == 0)
     config->writeEntry("DefaultCert", TQString());
  else config->writeEntry("DefaultCert", defCertBox->currentText());

  for (HostAuthItem *x = authDelList.first(); x != 0; x = authDelList.next()) {
     authcfg->deleteGroup(x->configName());
     authDelList.remove(x);
  }

  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     if (!x->originalName().isNull())
        authcfg->deleteGroup(x->originalName());
  }

  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     authcfg->setGroup(KResolver::domainToAscii(x->configName()));
     authcfg->writeEntry("certificate", x->getCertName());
     authcfg->writeEntry("prompt", (x->getAction() == KSSLCertificateHome::AuthPrompt));
     authcfg->writeEntry("send", (x->getAction() == KSSLCertificateHome::AuthSend));
  }

#endif

  config->sync();
  policies->sync();
  pcerts->sync();
  authcfg->sync();

  // insure proper permissions -- contains sensitive data
  TQString cfgName(TDEGlobal::dirs()->findResource("config", "cryptodefaults"));
  if (!cfgName.isEmpty())
    ::chmod(TQFile::encodeName(cfgName), 0600);

  emit changed(false);
}

TQString KCryptoConfig::handbookSection() const
{
 	int index = tabs->currentPageIndex();
 	if (index == 0)
		return "ssl_tab";
	else if (index == 1)
		return "openssl";
	else if (index == 2)
		return "your-certificates";
	else if (index == 3)
		return "authentication";
	else if (index == 4)
		return "peer-ssl-certificates";
	else if (index == 5)
		return "ssl-signers";
 	else
 		return TQString::null;
}

void KCryptoConfig::defaults()
{
   load( true );
}

void KCryptoConfig::genCAList() {
   _signers->regenerate();

}

void KCryptoConfig::slotSelectCipher(int id) {
    switch(id) {
	    case 1:
		    cwCompatible();
		    break;
	    case 2:
		    cwUS();
		    break;
	    case 3:
		    cwExp();
		    break;
	    case 4:
		    cwAll();
    }
}

void KCryptoConfig::cwCompatible() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv2Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 56 && item->bits() <= 128 );
  }

  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 56 && item->bits() <= 128 );
  }

  mUseTLS->setChecked(true);
#ifdef OPENSSL_NO_SSL2
  mUseSSLv2->setChecked(false);
#else
  mUseSSLv2->setChecked(true);
#endif
  mUseSSLv3->setChecked(true);
  configChanged();
  #endif
}


void KCryptoConfig::cwUS() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv2Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 128 );
  }

  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() >= 128 );
  }

  configChanged();
  #endif
}


void KCryptoConfig::cwExp() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv2Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() <= 56 && item->bits() > 0);
  }

  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( item->bits() <= 56 && item->bits() > 0);
  }

  configChanged();
  #endif
}


void KCryptoConfig::cwAll() {
  #ifdef HAVE_SSL
  CipherItem *item;
  for ( item = static_cast<CipherItem *>(SSLv2Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( true );
  }

  for ( item = static_cast<CipherItem *>(SSLv3Box->firstChild()); item;
	item = static_cast<CipherItem *>(item->nextSibling()) ) {
    item->setOn( true );
  }

  mUseTLS->setChecked(true);
#ifdef OPENSSL_NO_SSL2
  mUseSSLv2->setChecked(false);
#else
  mUseSSLv2->setChecked(true);
#endif
  mUseSSLv3->setChecked(true);
  configChanged();
  #endif
}



void KCryptoConfig::slotExportCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
   if (x) {
     policies->setGroup(x->getMD5());
     KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", TQString()).local8Bit());
     if (cert) {
        KCertExport kce;
        kce.setCertificate(cert);
        kce.exec();
        delete cert;
     } else KMessageBox::sorry(this, i18n("Could not open the certificate."),
                                     i18n("SSL"));
   }
}



void KCryptoConfig::slotRemoveCert() {
TQListViewItem *act = otherSSLBox->selectedItem();
OtherCertItem *x = static_cast<OtherCertItem *>(act);
   if (x) {
      TQListViewItem *next = act->itemBelow();
      if (!next) next = act->itemAbove();
      otherSSLBox->takeItem(x);
      otherCertDelList.append(x);
      configChanged();
      if (next) otherSSLBox->setSelected(next, true);
   }
}



void KCryptoConfig::slotVerifyCert() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
  if (!x) return;

  policies->setGroup(x->getMD5());
  KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", TQString()).local8Bit());

  if (!cert) {
    KMessageBox::error(this, i18n("Error obtaining the certificate."), i18n("SSL"));
    return;
  }

  cert->chain().setCertChain(policies->readListEntry("Chain"));

  KSSLCertificate::KSSLValidation v = cert->revalidate(KSSLCertificate::SSLServer);

  /*
   *  Don't enable this until we keep this info in the cache
  if (v != KSSLCerticiate::Ok)
	  v = cert->revalidate(KSSLCertificate::SMIMESign);
  if (v != KSSLCerticiate::Ok)
	  v = cert->revalidate(KSSLCertificate::SMIMEEncrypt);
  */

  if (v == KSSLCertificate::Ok) {
     KMessageBox::information(this, i18n("This certificate passed the verification tests successfully."), i18n("SSL"));
  } else {
     KMessageBox::detailedError(this, i18n("This certificate has failed the tests and should be considered invalid."), KSSLCertificate::verifyText(v), i18n("SSL"));
  }

  delete cert;
}



void KCryptoConfig::slotUntil() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

   cachePerm->setChecked(false);
   untilDate->setEnabled(true);

   if (!x) return;
   x->setPermanent(false);

   configChanged();
}



void KCryptoConfig::slotPermanent() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

   cacheUntil->setChecked(false);
   untilDate->setEnabled(false);

   if (!x) return;
   x->setPermanent(true);

   configChanged();
}



void KCryptoConfig::slotPolicyChanged(int id) {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
if (!x) return;

   if (id == policyGroup->id(policyAccept)) {
     x->setPolicy(KSSLCertificateCache::Accept);
   } else if (id == policyGroup->id(policyReject)) {
     x->setPolicy(KSSLCertificateCache::Reject);
   } else if (id == policyGroup->id(policyPrompt)) {
     x->setPolicy(KSSLCertificateCache::Prompt);
   }

   configChanged();
}



void KCryptoConfig::slotDatePick() {
KDateTimeDlg kdtd;
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());

   if (!x || !untilDate->isEnabled()) return;

TQDateTime qdt = x->getExpires();

   kdtd.setDateTime(qdt);
   int rc = kdtd.exec();
   if (rc == KDialog::Accepted) {
      x->setExpires(kdtd.getDateTime());
      untilDate->setText(TDEGlobal::locale()->formatDateTime(x->getExpires()));
      configChanged();
   }
}



void KCryptoConfig::slotOtherCertSelect() {
OtherCertItem *x = static_cast<OtherCertItem *>(otherSSLBox->selectedItem());
TQString iss = TQString();
   if (x) {
      otherSSLExport->setEnabled(true);
      otherSSLVerify->setEnabled(true);
      otherSSLRemove->setEnabled(true);

      fromLabel->setEnabled(true);
      untilLabel->setEnabled(true);
      policyGroup->setEnabled(true);
      cacheGroup->setEnabled(true);
      cachePerm->setEnabled(true);
      cacheUntil->setEnabled(true);
      policies->setGroup(x->getMD5());

      KSSLCertificate *cert = KSSLCertificate::fromString(policies->readEntry("Certificate", TQString()).local8Bit());

      if (cert) {
         TQPalette cspl;
         iss = cert->getIssuer();
         cspl = validFrom->palette();
         if (TQDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
            cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
         } else {
            cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
         }
         validFrom->setPalette(cspl);

         cspl = validUntil->palette();
         if (TQDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
            cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
         } else {
            cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
         }
         validUntil->setPalette(cspl);

         validFrom->setText(cert->getNotBefore());
         validUntil->setText(cert->getNotAfter());
         untilDate->setText(x ? TDEGlobal::locale()->formatDateTime(x->getExpires())
                              : TDEGlobal::locale()->formatDateTime(TQDateTime::currentDateTime(Qt::UTC)));
         untilDate->setEnabled(x && !x->isPermanent());
         pHash->setText(cert->getMD5DigestText());
         delete cert;
      } else {
         validFrom->setText(TQString());
         validUntil->setText(TQString());
         pHash->clear();
      }

      switch(x->getPolicy()) {
      case KSSLCertificateCache::Accept:
        policyGroup->setButton(policyGroup->id(policyAccept));
      break;
      case KSSLCertificateCache::Reject:
        policyGroup->setButton(policyGroup->id(policyReject));
      break;
      case KSSLCertificateCache::Prompt:
        policyGroup->setButton(policyGroup->id(policyPrompt));
      break;
      }

      cachePerm->setChecked(x->isPermanent());
      cacheUntil->setChecked(!x->isPermanent());

   } else {
      otherSSLExport->setEnabled(false);
      otherSSLVerify->setEnabled(false);
      otherSSLRemove->setEnabled(false);
      fromLabel->setEnabled(false);
      untilLabel->setEnabled(false);
      policyGroup->setEnabled(false);
      cacheGroup->setEnabled(false);
      cachePerm->setChecked(false);
      cacheUntil->setChecked(false);
      policyAccept->setChecked(false);
      policyReject->setChecked(false);
      policyPrompt->setChecked(false);
      cachePerm->setEnabled(false);
      cacheUntil->setEnabled(false);
      validFrom->setText(TQString());
      validUntil->setText(TQString());
      untilDate->setText(TQString());
      untilDate->setEnabled(false);
      pHash->clear();
   }


   oSubject->setValues(x ? x->getSub() : TQString(TQString()));
   oIssuer->setValues(iss);

}


void KCryptoConfig::slotYourImport() {

   TQString certFile = KFileDialog::getOpenFileName(TQString(), "application/x-pkcs12");
   if (certFile.isEmpty())
      return;

#ifdef HAVE_SSL
KSSLPKCS12 *cert = NULL;
TQCString pass;

TryImportPassAgain:
   int rc = KPasswordDialog::getPassword(pass, i18n("Certificate password"));
   if (rc != KPasswordDialog::Accepted) return;

   cert = KSSLPKCS12::loadCertFile(certFile, TQString(pass));

   if (!cert) {
      rc = KMessageBox::warningYesNo(this, i18n("The certificate file could not be loaded. Try a different password?"), i18n("SSL"),i18n("Try"),i18n("Do Not Try"));
      if (rc == KMessageBox::Yes) goto TryImportPassAgain;
      return;
   }

   // At this point, we know that we can read the certificate in.
   // The procedure will be to convert it to Base64 in it's raw form
   // and add it to the ListView - eventually going into the SimpleConfig.

   // FIXME: prompt if the user wants the password stored along with the
   //        certificate

   TQString name = cert->getCertificate()->getSubject();
   for (YourCertItem *i =
        static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                            i;
             i = static_cast<YourCertItem *>(i->nextSibling())) {
      if (i->configName() == name) {
         rc = KMessageBox::warningContinueCancel(this, i18n("A certificate with that name already exists. Are you sure that you wish to replace it?"), i18n("SSL"), i18n("Replace"));
         if (rc == KMessageBox::Cancel) {
            delete cert;
            return;
         }
         yourSSLBox->takeItem(i);
         yourCertDelList.append(i);
      }
   }

   new YourCertItem(yourSSLBox,
                    cert->toString(),
                    TQString(),  // the password - don't store it yet!
                    name,
                    this );

   setAuthCertLists();
   configChanged();
   delete cert;
#endif

   offerImportToKMail( certFile );
}


void KCryptoConfig::slotYourExport() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      TQString pprompt = i18n("Enter the certificate password:");
      TQCString oldpass;
      do {
         int i = KPasswordDialog::getPassword(oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
      slotYourUnlock();
   }

  // For now, we will only export to PKCS#12
   TQString certFile = KFileDialog::getSaveFileName(TQString(),
		                                   "application/x-pkcs12");
   if (certFile.isEmpty())
      return;

   if (!pkcs->toFile(certFile))
      KMessageBox::sorry(this, i18n("Export failed."), i18n("SSL"));
}


void KCryptoConfig::slotYourVerify() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
TQString iss;
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      TQString pprompt = i18n("Enter the certificate password:");
      TQCString oldpass;
      do {
         int i = KPasswordDialog::getPassword(oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
      slotYourUnlock();
   }

  KSSLCertificate::KSSLValidation v = pkcs->revalidate(KSSLCertificate::SSLClient);
  if (v != KSSLCertificate::Ok)
	  v = pkcs->revalidate(KSSLCertificate::SMIMEEncrypt);
  if (v != KSSLCertificate::Ok)
	  v = pkcs->revalidate(KSSLCertificate::SMIMESign);

  if (v == KSSLCertificate::Ok) {
     KMessageBox::information(this, i18n("This certificate passed the verification tests successfully."), i18n("SSL"));
  } else {
     KMessageBox::detailedError(this, i18n("This certificate has failed the tests and should be considered invalid."), KSSLCertificate::verifyText(v), i18n("SSL"));
  }

  delete pkcs;
}


void KCryptoConfig::slotYourRemove() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
   if (x) {
      yourSSLBox->takeItem(x);
      yourCertDelList.append(x);
      setAuthCertLists();
      configChanged();
   }
}


void KCryptoConfig::slotYourUnlock() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
TQString iss;
   if (!x || !yourSSLUnlock->isEnabled()) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      TQString pprompt = i18n("Enter the certificate password:");
      TQCString oldpass;
      do {
         int i = KPasswordDialog::getPassword(oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) return;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
      x->setPassCache(oldpass);
   }

   // update the info
   iss = pkcs->getCertificate()->getIssuer();
   ySubject->setValues(x ? x->getName() : TQString(TQString()));
   yIssuer->setValues(iss);
   TQPalette cspl;
   KSSLCertificate *cert = pkcs->getCertificate();
   cspl = yValidFrom->palette();
   if (TQDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
	   cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
   } else {
	   cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
   }
   yValidFrom->setPalette(cspl);

   cspl = yValidUntil->palette();
   if (TQDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
	   cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
   } else {
	   cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
   }
   yValidUntil->setPalette(cspl);

   yValidFrom->setText(cert->getNotBefore());
   yValidUntil->setText(cert->getNotAfter());
   yHash->setText(cert->getMD5DigestText());
   yourSSLUnlock->setEnabled(false);
   delete pkcs;
}


void KCryptoConfig::slotYourCertSelect() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
TQString iss;

   yourSSLExport->setEnabled(x != NULL);
   yourSSLPass->setEnabled(x != NULL);
   yourSSLUnlock->setEnabled(false);
   yourSSLVerify->setEnabled(x != NULL);
   yourSSLRemove->setEnabled(x != NULL);

   if (x) {
      KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
      if (pkcs) {
         TQPalette cspl;
         KSSLCertificate *cert = pkcs->getCertificate();
         iss = cert->getIssuer();
         cspl = yValidFrom->palette();
         if (TQDateTime::currentDateTime(Qt::UTC) < cert->getQDTNotBefore()) {
            cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
         } else {
            cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
         }
         yValidFrom->setPalette(cspl);

         cspl = yValidUntil->palette();
         if (TQDateTime::currentDateTime(Qt::UTC) > cert->getQDTNotAfter()) {
            cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
         } else {
            cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
         }
         yValidUntil->setPalette(cspl);

         yValidFrom->setText(cert->getNotBefore());
         yValidUntil->setText(cert->getNotAfter());
         yHash->setText(cert->getMD5DigestText());
         delete pkcs;
      } else {
         yourSSLUnlock->setEnabled(x != NULL);
         yHash->clear();
      }
   } else {
      yHash->clear();
   }

   ySubject->setValues(x ? x->getName() : TQString(TQString()));
   yIssuer->setValues(iss);
}


void KCryptoConfig::slotYourPass() {
YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->selectedItem());
TQCString oldpass = "";
   if (!x) return;

   KSSLPKCS12 *pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPass());
   if (!pkcs)
      pkcs = KSSLPKCS12::fromString(x->getPKCS(), x->getPassCache());
   if (!pkcs) {
      TQString pprompt = i18n("Enter the OLD password for the certificate:");
      do {
         int i = KPasswordDialog::getPassword(oldpass, pprompt);
         if (i != KPasswordDialog::Accepted) break;
         pkcs = KSSLPKCS12::fromString(x->getPKCS(), oldpass);
         pprompt = i18n("Decoding failed. Please try again:");
      } while (!pkcs);
   }

   if (pkcs) {
      x->setPassCache(oldpass);
      slotYourUnlock();
      KPasswordDialog *kpd = new KPasswordDialog(KPasswordDialog::NewPassword, false, 0, this);
      kpd->setPrompt(i18n("Enter the new certificate password"));
      kpd->setAllowEmptyPasswords(true);

      int i = kpd->exec();
      if (i == KPasswordDialog::Accepted) {
         TQCString pass = kpd->password();
         pkcs->changePassword(TQString(oldpass), TQString(pass));
         x->setPKCS(pkcs->toString());
         x->setPassCache(pass);
         configChanged();
      }
      delete kpd;
      delete pkcs;
   }
}


void KCryptoConfig::slotCAImport() {
    TQString certFile = KFileDialog::getOpenFileName(TQString(), "application/x-x509-ca-cert");

    if (certFile.isEmpty())
        return;

#ifdef HAVE_SSL
#define sk_free KOSSL::self()->sk_free
#define sk_num KOSSL::self()->sk_num
#define sk_value KOSSL::self()->sk_value

	// First try to load using the OpenSSL method
	X509_STORE *certStore = KOSSL::self()->X509_STORE_new();
	X509_LOOKUP *certLookup = KOSSL::self()->X509_STORE_add_lookup(certStore, KOSSL::self()->X509_LOOKUP_file());

	if (certLookup &&
	    KOSSL::self()->X509_LOOKUP_load_file(certLookup,
		                                 certFile.local8Bit(),
						 X509_FILETYPE_PEM)) {
		for (int i = 0; i < sk_X509_OBJECT_num(certStore->objs); i++) {
			X509_OBJECT* x5o = sk_X509_OBJECT_value(certStore->objs, i);
			if (!x5o) continue;

			if (x5o->type != X509_LU_X509) continue;

			X509 *x5 = x5o->data.x509;
			if (!x5) continue;

			// Easier to use in this form
			KSSLCertificate *x = KSSLCertificate::fromX509(x5);

			// Only import CA's
			if (!x || !x->x509V3Extensions().certTypeCA()) {
				if (x) {
					TQString emsg = x->getSubject() + ":\n" +
					i18n("This is not a signer certificate.");
				 	KMessageBox::error(this,
							   emsg,
						   	   i18n("SSL"));
					delete x;
				}
				continue;
			}

			TQString name = x->getSubject();

			// search for dups
			for (CAItem *m = static_cast<CAItem *>(caList->firstChild());
                                                                   m;
        	                 m = static_cast<CAItem *>(m->nextSibling())) {
			         if (m->configName() == name) {
				    KSSLCertificate *y = KSSLCertificate::fromString(m->getCert().local8Bit());
				    if (!y) continue;
				    if (*x == *y) {
					TQString emsg = name + ":\n" +
						i18n("You already have this signer certificate installed.");
				 	KMessageBox::error(this,
							   emsg,
						   	   i18n("SSL"));
					 delete x;
					 delete y;
					 x = NULL;
					 break;
			            }
				    delete y;
				 }
			}

			if (!x) continue;

			// Ok, add it to the list
			(new CAItem(caList,
				    name,
				    x->toString(),
				    true, true, true, this)
			 )->isNew = true;
			delete x;
		}

		  // Can the PEM code be wiped out now?
	} else {   // try to load it manually as a single X.509 DER encoded
		// ASSUMPTION: we only read one certificate in this code
		TQFile qf(certFile);
		TQString name;
		TQString certtext;
		KSSLCertificate *x;
		qf.open(IO_ReadOnly);
		qf.readLine(certtext, qf.size());

		if (certStore) { KOSSL::self()->X509_STORE_free(certStore);
				certStore = NULL; }

		if (certtext.contains("-----BEGIN CERTIFICATE-----")) {
			qf.reset();
			certtext = TQString();
			while (!qf.atEnd()) {
				TQString xx;
				qf.readLine(xx, qf.size());
				certtext += xx;
			}
			certtext = certtext.replace("-----BEGIN CERTIFICATE-----", TQString());
			certtext = certtext.replace("-----END CERTIFICATE-----", TQString());
			certtext = certtext.stripWhiteSpace();
			certtext = certtext.replace("\n", TQString());
		} else {
			// Must [could?] be DER
			qf.close();
			qf.open(IO_ReadOnly);
			char *cr;
			cr = new char[qf.size()+1];
			qf.readBlock(cr, qf.size());
			TQByteArray qba;
			qba.duplicate(cr, qf.size());
			certtext = KCodecs::base64Encode(qba);
			delete [] cr;
		}

		qf.close();

		x = KSSLCertificate::fromString(certtext.latin1());

		if (!x) {
			KMessageBox::sorry(this,
				i18n("The certificate file could not be loaded."),
				i18n("SSL"));
			return;
		}

		if (!x->x509V3Extensions().certTypeCA()) {
			KMessageBox::sorry(this,
					i18n("This is not a signer certificate."),
					i18n("SSL"));
			return;
		}

		name = x->getSubject();

		for (CAItem *i = static_cast<CAItem *>(caList->firstChild());
                                                                   i;
                         i = static_cast<CAItem *>(i->nextSibling())) {
		         if (i->configName() == name) {
			    KSSLCertificate *y = KSSLCertificate::fromString(i->getCert().local8Bit());
			    if (!y) continue;
			    if (*x == *y) {
				 KMessageBox::error(this,
				    i18n("You already have this signer certificate installed."),
				    i18n("SSL"));
				 delete x;
				 delete y;
				 return;
			    }
			    delete y;
			 }
		}

		(new CAItem(caList, name, x->toString(), true, true, true, this))->isNew = true;

		delete x;
	}


	if (certStore) KOSSL::self()->X509_STORE_free(certStore);

	configChanged();
#undef sk_free
#undef sk_num
#undef sk_value
#endif

        offerImportToKMail( certFile );
}

void KCryptoConfig::offerImportToKMail( const TQString& certFile )
{
    if ( KMessageBox::questionYesNo( this, i18n( "Do you want to make this certificate available to KMail as well?" ), TQString(), i18n("Make Available"), i18n("Do Not Make Available") ) == KMessageBox::Yes ) {
       TDEProcess proc;
       proc << "kleopatra";
       proc << "--import-certificate";
       proc << certFile;
       if ( !proc.start( TDEProcess::DontCare ) )
           KMessageBox::error( this, i18n( "Could not execute Kleopatra. You might have to install or update the tdepim package." ) );
   }
}


void KCryptoConfig::slotCARemove() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
 if (x) {
    caList->takeItem(x);
    caDelList.append(x);
    configChanged();
    slotCAItemChanged();
 }
}


void KCryptoConfig::slotCARestore() {

 int rc = KMessageBox::warningContinueCancel(this, i18n("This will revert your certificate signers database to the TDE default.\nThis operation cannot be undone.\nAre you sure you wish to continue?"), i18n("SSL"),i18n("Revert"));
      if (rc == KMessageBox::Cancel) {
          return;
      }

// For now, we just rm the existing file and rebuild

   TQString path = TDEGlobal::dirs()->saveLocation("config");

   path += "/ksslcalist";

   TQFile::remove(path);

   // Remove all our old work and rebuild the GUI/List
   caDelList.clear();
   caList->clear();


  TQStringList groups = _signers->list();
  TDEConfig sigcfg("ksslcalist", true, false);

  for (TQStringList::Iterator i = groups.begin();
                             i != groups.end();
                             ++i) {
    if ((*i).isEmpty() || *i == "<default>") continue;
    if (!sigcfg.hasGroup(*i)) continue;
    sigcfg.setGroup(*i);
    if (!sigcfg.hasKey("x509")) continue;
                new CAItem(caList,
                     (*i),
                     sigcfg.readEntry("x509", TQString()),
                     sigcfg.readBoolEntry("site", false),
                     sigcfg.readBoolEntry("email", false),
                     sigcfg.readBoolEntry("code", false),
                     this );
  }

   genCAList();
   slotCAItemChanged();
}


void KCryptoConfig::slotCAItemChanged() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
 if (x) {
    caSSLRemove->setEnabled(true);
    caSubject->setValues(x ? x->getName() : TQString(TQString()));
    KSSLCertificate *cert = KSSLCertificate::fromString(x->getCert().local8Bit());
    if (!cert) {
       caIssuer->setValues(TQString(TQString()));
       caSite->setEnabled(false);
       caEmail->setEnabled(false);
       caCode->setEnabled(false);
       caSite->setChecked(false);
       caEmail->setChecked(false);
       caCode->setChecked(false);
       cHash->clear();
    } else {
       caSite->setEnabled(cert->x509V3Extensions().certTypeSSLCA());
       caEmail->setEnabled(cert->x509V3Extensions().certTypeEmailCA());
       caCode->setEnabled(cert->x509V3Extensions().certTypeCodeCA());
       caSite->setChecked(x->getSite());
       caEmail->setChecked(x->getEmail());
       caCode->setChecked(x->getCode());
       caIssuer->setValues(cert->getIssuer());
       cHash->setText(cert->getMD5DigestText());
       delete cert;
    }
 } else {
    caSSLRemove->setEnabled(false);
    caSite->setEnabled(false);
    caEmail->setEnabled(false);
    caCode->setEnabled(false);
    caSubject->setValues(TQString(TQString()));
    caIssuer->setValues(TQString(TQString()));
    cHash->clear();
 }
}


void KCryptoConfig::slotCAChecked() {
CAItem *x = static_cast<CAItem *>(caList->selectedItem());
 if (x) {
   x->setSite(caSite->isChecked());
   x->setEmail(caEmail->isChecked());
   x->setCode(caCode->isChecked());
   x->modified = true;
   configChanged();
 }
}



void KCryptoConfig::slotNewHostAuth() {
    HostAuthItem *j = new HostAuthItem(hostAuthList,
                                       TQString(),
                                       TQString(),
                                       this );
    j->setAction(KSSLCertificateHome::AuthSend);
    hostAuthList->setSelected(j, true);
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertBG->setEnabled(true);
  authRemove->setEnabled(true);

  hostCertBG->setButton(0);

  authHost->setFocus();
}



void KCryptoConfig::slotRemoveHostAuth() {
TQListViewItem *act = hostAuthList->selectedItem();
HostAuthItem *x = static_cast<HostAuthItem *>(act);

  if (x) {
      TQListViewItem *next = act->itemBelow();
      if (!next) next = act->itemAbove();
      hostAuthList->takeItem(x);
      authDelList.append(x);
      configChanged();
      if (next) hostAuthList->setSelected(next, true);
  }
}


void KCryptoConfig::slotAuthItemChanged() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
  // Make sure the fields are enabled
  authHost->setEnabled(true);
  hostCertBox->setEnabled(true);
  hostCertBG->setEnabled(true);
  authRemove->setEnabled(true);

  // Populate the fields
  switch(x->getAction()) {
  case KSSLCertificateHome::AuthSend:
    hostCertBG->setButton(hostCertBG->id(hostSend));
   break;
  case KSSLCertificateHome::AuthPrompt:
    hostCertBG->setButton(hostCertBG->id(hostPrompt));
   break;
  case KSSLCertificateHome::AuthDont:
    hostCertBG->setButton(hostCertBG->id(hostDont));
   break;
  default:
    hostSend->setChecked(false);
    hostPrompt->setChecked(false);
    hostDont->setChecked(false);
   break;
  }

  ___lehack = true;
  authHost->setText(x->configName());
  ___lehack = false;
  hostCertBox->setCurrentItem(0);

  TQString theCert = x->getCertName();
  for (int i = 0; i < hostCertBox->count(); i++) {
    if (hostCertBox->text(i) == theCert) {
       hostCertBox->setCurrentItem(i);
       break;
    }
  }
} else {
  authHost->clear();
  authHost->setEnabled(false);
  hostCertBox->setEnabled(false);
  hostCertBG->setEnabled(false);
  authRemove->setEnabled(false);
}
}


void KCryptoConfig::slotAuthText(const TQString &t) {
if (___lehack) return;
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
   x->setHost(t);
configChanged();
}
}



void KCryptoConfig::slotAuthButtons() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

if (x) {
   KSSLCertificateHome::KSSLAuthAction aa = KSSLCertificateHome::AuthDont;
   int sel = hostCertBG->id(hostCertBG->selected());

   if (sel ==  hostCertBG->id(hostSend))
    aa = KSSLCertificateHome::AuthSend;
   else if (sel == hostCertBG->id(hostPrompt))
    aa = KSSLCertificateHome::AuthPrompt;
   else
    aa =  KSSLCertificateHome::AuthDont;

   x->setAction(aa);
configChanged();
}
}


void KCryptoConfig::slotAuthCombo() {
HostAuthItem *x = static_cast<HostAuthItem *>(hostAuthList->selectedItem());

  if (x) {
    if (hostCertBox->currentItem() == 0)
      x->setCertName(TQString());
    else x->setCertName(hostCertBox->currentText());
    configChanged();
  }
}


void KCryptoConfig::slotTestOSSL() {
KOSSL::self()->destroy();

if (!KOSSL::self()->hasLibSSL()) {
   KMessageBox::detailedSorry(this,
                              i18n("Failed to load OpenSSL."),
                              i18n("libssl was not found or successfully loaded."),
                              i18n("OpenSSL"));
   return;
}

if (!KOSSL::self()->hasLibCrypto()) {
   KMessageBox::detailedSorry(this,
                              i18n("Failed to load OpenSSL."),
                              i18n("libcrypto was not found or successfully loaded."),
                              i18n("OpenSSL"));
   return;
}

   KMessageBox::information(this, i18n("OpenSSL was successfully loaded."), i18n("OpenSSL"));
}


void KCryptoConfig::slotUseEGD() {
  if (mUseEGD->isChecked()) {
    mUseEFile->setChecked(false);
  }
  mEGDLabel->setText(i18n("Path to EGD:"));
  mEGDPath->setEnabled(mUseEGD->isChecked());
  mEGDLabel->setEnabled(mUseEGD->isChecked());
  configChanged();
}


void KCryptoConfig::slotUseEFile() {
  if (mUseEFile->isChecked()) {
    mUseEGD->setChecked(false);
  }
  mEGDLabel->setText(i18n("Path to entropy file:"));
  mEGDPath->setEnabled(mUseEFile->isChecked());
  mEGDLabel->setEnabled(mUseEFile->isChecked());
  configChanged();
}


//  Lets make this a separate module.  it's a whole lot of work and can really be
// encompassed in a separate module quite nicely.
void KCryptoConfig::slotGeneratePersonal() {
#if 0
  TQStringList qslCertTypes;

  qslCertTypes << i18n("Personal SSL")
               << i18n("Server SSL")
               << i18n("S/MIME")
               << i18n("PGP")
               << i18n("GPG")
               << i18n("SSL Personal Request")
               << i18n("SSL Server Request")
               << i18n("Netscape SSL")
               << i18n("Server certificate authority", "Server CA")
               << i18n("Personal certificate authority", "Personal CA")
               << i18n("Secure MIME certificate authority", "S/MIME CA");
#endif

}


#ifdef HAVE_SSL

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define SSL_CONST const
#else
#define SSL_CONST
#endif

// This gets all the available ciphers from OpenSSL
bool KCryptoConfig::loadCiphers() {
unsigned int i;
SSL_CTX *ctx;
SSL *ssl;
SSL_CONST SSL_METHOD *meth;

  SSLv2Box->clear();
  SSLv3Box->clear();
  CipherItem *item;

#ifndef OPENSSL_NO_SSL2
  meth = SSLv2_client_method();
  SSLeay_add_ssl_algorithms();
  ctx = SSL_CTX_new(meth);
  if (ctx == NULL) return false;

  ssl = SSL_new(ctx);
  if (!ssl) return false;

  for (i=0; ; i++) {
    int j, k;
    SSL_CONST SSL_CIPHER *sc;
    sc = (meth->get_cipher)(i);
    if (!sc)
      break;
    // Leak of sc*?
    TQString scn(sc->name);
    if (scn.contains("ADH-") || scn.contains("NULL-") || scn.contains("DES-CBC3-SHA") || scn.contains("FZA-")) {
      continue;
    }
    k = SSL_CIPHER_get_bits(sc, &j);

    item = new CipherItem( SSLv2Box, sc->name, k, j, this );
  }

  if (ctx) SSL_CTX_free(ctx);
  if (ssl) SSL_free(ssl);
#endif

# ifndef OPENSSL_NO_SSL3_METHOD
  // We repeat for SSLv3
  meth = SSLv3_client_method();
  SSLeay_add_ssl_algorithms();
  ctx = SSL_CTX_new(meth);
  if (ctx == NULL) return false;

  ssl = SSL_new(ctx);
  if (!ssl) return false;

  for (i=0; ; i++) {
    int j, k;
    SSL_CONST SSL_CIPHER *sc;
    sc = (meth->get_cipher)(i);
    if (!sc)
      break;
    // Leak of sc*?
    TQString scn(sc->name);
    if (scn.contains("ADH-") || scn.contains("NULL-") || scn.contains("DES-CBC3-SHA") || scn.contains("FZA-")) {
      continue;
    }
    k = SSL_CIPHER_get_bits(sc, &j);

    item = new CipherItem( SSLv3Box, sc->name, k, j, this );
  }

  if (ctx) SSL_CTX_free(ctx);
  if (ssl) SSL_free(ssl);
#endif

return true;
}
#endif


void KCryptoConfig::setAuthCertLists() {
TQString oldDef, oldHost;
bool noneDef, noneHost;

// get the old setting
  oldDef = defCertBox->currentText();
  oldHost = hostCertBox->currentText();
  noneDef = (0 == defCertBox->currentItem());
  noneHost = (0 == hostCertBox->currentItem());

// repopulate
  defCertBox->clear();
  hostCertBox->clear();
  TQStringList defCertStrList; // = KSSLCertificateHome::getCertificateList();
  defCertStrList.append(i18n("None"));
  for (YourCertItem *x = static_cast<YourCertItem *>(yourSSLBox->firstChild());
                                                                             x;
                           x = static_cast<YourCertItem *>(x->nextSibling())) {
     defCertStrList.append(x->configName());
  }
  defCertBox->insertStringList(defCertStrList);
  hostCertBox->insertStringList(defCertStrList);

// try to restore it

  defCertBox->setCurrentItem(0);
  if (!noneDef) {
    for (int i = 0; i < defCertBox->count(); i++) {
      if (defCertBox->text(i) == oldDef) {
         defCertBox->setCurrentItem(i);
         break;
      }
    }
  }

  if (!noneDef && defCertBox->currentItem() == 0) configChanged();

  hostCertBox->setCurrentItem(0);
  if (!noneHost) {
    for (int i = 0; i < hostCertBox->count(); i++) {
      if (hostCertBox->text(i) == oldHost) {
         hostCertBox->setCurrentItem(i);
         break;
      }
    }
  }

  if (!noneHost && hostCertBox->currentItem() == 0) configChanged();

  // Update the host entries too
  for (HostAuthItem *x =
        static_cast<HostAuthItem *>(hostAuthList->firstChild());
                                                              x;
             x = static_cast<HostAuthItem *>(x->nextSibling())) {
     TQString newValue = TQString();
     for (int i = 1; i < hostCertBox->count(); i++) {
        if (hostCertBox->text(i) == x->getCertName()) {
           newValue = x->getCertName();
           break;
        }
     }
     if (newValue != x->getCertName()) configChanged();
     x->setCertName(newValue);
  }


}

#include "crypto.moc"
