/**
 * kcookiespolicies.cpp - Cookies configuration
 *
 * Original Authors
 * Copyright (c) Waldo Bastian <bastian@kde.org>
 * Copyright (c) 1999 David Faure <faure@kde.org>
 *
 * Re-written by:
 * Copyright (c) 2000- Dawit Alemayehu <adawit@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <tqheader.h>
#include <tqvbox.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqtoolbutton.h>
#include <tqvbuttongroup.h>

#include <kiconloader.h>
#include <kidna.h>
#include <tdemessagebox.h>
#include <tdelistview.h>
#include <tdelistviewsearchline.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <dcopref.h>

#include "ksaveioconfig.h"

#include "kcookiespolicies.h"
#include "kcookiespoliciesdlg_ui.h"

KCookiesPolicies::KCookiesPolicies(TQWidget *parent)
                 :TDECModule(parent, "kcmtdeio")
{
    TQVBoxLayout *mainLayout = new TQVBoxLayout(this, 0, 0);

    dlg = new KCookiesPolicyDlgUI (this);
    dlg->lvDomainPolicy->header()->setStretchEnabled(true, 0);
    dlg->lvDomainPolicy->setColumnWidthMode(0, TDEListView::Manual);
    dlg->lvDomainPolicy->setColumnWidthMode(1, TDEListView::Maximum);
    dlg->tbClearSearchLine->setIconSet(SmallIconSet(TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase"));
    dlg->kListViewSearchLine->setListView(dlg->lvDomainPolicy);
    TQValueList<int> columns;
    columns.append(0);
    dlg->kListViewSearchLine->setSearchColumns(columns);

    mainLayout->addWidget(dlg);

    load();
}

KCookiesPolicies::~KCookiesPolicies()
{
}

void KCookiesPolicies::configChanged ()
{
  //kdDebug() << "KCookiesPolicies::configChanged..." << endl;
  emit changed((d_configChanged=true));
}

void KCookiesPolicies::cookiesEnabled( bool enable )
{
  dlg->bgDefault->setEnabled( enable );
  dlg->bgPreferences->setEnabled ( enable );
  dlg->gbDomainSpecific->setEnabled( enable );

  if (enable)
  {
    ignoreCookieExpirationDate ( enable );
    autoAcceptSessionCookies ( enable );
  }
}

void KCookiesPolicies::ignoreCookieExpirationDate ( bool enable )
{
  bool isAutoAcceptChecked = dlg->cbAutoAcceptSessionCookies->isChecked();
  enable = (enable && isAutoAcceptChecked);

  dlg->bgDefault->setEnabled( !enable );
  dlg->gbDomainSpecific->setEnabled( !enable );
}

void KCookiesPolicies::autoAcceptSessionCookies ( bool enable )
{
  bool isIgnoreExpirationChecked = dlg->cbIgnoreCookieExpirationDate->isChecked();
  enable = (enable && isIgnoreExpirationChecked);

  dlg->bgDefault->setEnabled( !enable );
  dlg->gbDomainSpecific->setEnabled( !enable );
}

void KCookiesPolicies::addNewPolicy(const TQString& domain)
{
  PolicyDlg pdlg (i18n("New Cookie Policy"), this);
  pdlg.setEnableHostEdit (true, domain);

  if (dlg->rbPolicyAccept->isChecked())
    pdlg.setPolicy(KCookieAdvice::Reject);
  else
    pdlg.setPolicy(KCookieAdvice::Accept);

  if (pdlg.exec() && !pdlg.domain().isEmpty())
  {
    TQString domain = KIDNA::toUnicode(pdlg.domain());
    int advice = pdlg.advice();

    if ( !handleDuplicate(domain, advice) )
    {
      const char* strAdvice = KCookieAdvice::adviceToStr(advice);
      TQListViewItem* index = new TQListViewItem (dlg->lvDomainPolicy,
                                                domain, i18n(strAdvice));
      m_pDomainPolicy.insert (index, strAdvice);
      configChanged();
    }
  }
}


void KCookiesPolicies::addPressed()
{
  addNewPolicy (TQString::null);
}

void KCookiesPolicies::changePressed()
{
  TQListViewItem* index = dlg->lvDomainPolicy->currentItem();

  if (!index)
    return;

  TQString oldDomain = index->text(0);

  PolicyDlg pdlg (i18n("Change Cookie Policy"), this);
  pdlg.setPolicy (KCookieAdvice::strToAdvice(m_pDomainPolicy[index]));
  pdlg.setEnableHostEdit (true, oldDomain);

  if( pdlg.exec() && !pdlg.domain().isEmpty())
  {
    TQString newDomain = KIDNA::toUnicode(pdlg.domain());
    int advice = pdlg.advice();
    if (newDomain == oldDomain || !handleDuplicate(newDomain, advice))
    {
      m_pDomainPolicy[index] = KCookieAdvice::adviceToStr(advice);
      index->setText(0, newDomain);
      index->setText(1, i18n(m_pDomainPolicy[index]) );
      configChanged();
    }
  }
}

bool KCookiesPolicies::handleDuplicate( const TQString& domain, int advice )
{
  TQListViewItem* item = dlg->lvDomainPolicy->firstChild();
  while ( item != 0 )
  {
    if ( item->text(0) == domain )
    {
      TQString msg = i18n("<qt>A policy already exists for"
                         "<center><b>%1</b></center>"
                         "Do you want to replace it?</qt>").arg(domain);
      int res = KMessageBox::warningContinueCancel(this, msg,
                                          i18n("Duplicate Policy"),
                                          i18n("Replace"));
      if ( res == KMessageBox::Continue )
      {
        m_pDomainPolicy[item]= KCookieAdvice::adviceToStr(advice);
        item->setText(0, domain);
        item->setText(1, i18n(m_pDomainPolicy[item]));
        configChanged();
        return true;
      }
      else
        return true;  // User Cancelled!!
    }
    item = item->nextSibling();
  }
  return false;
}

void KCookiesPolicies::deletePressed()
{
  TQListViewItem* nextItem = 0L;
  TQListViewItem* item = dlg->lvDomainPolicy->firstChild ();

  while (item != 0L)
  {
    if (dlg->lvDomainPolicy->isSelected (item))
    {
      nextItem = item->itemBelow();
      if ( !nextItem )
        nextItem = item->itemAbove();

      delete item;
      item = nextItem;
    }
    else
    {
      item = item->itemBelow();
    }
  }

  if (nextItem)
    dlg->lvDomainPolicy->setSelected (nextItem, true);

  updateButtons();
  configChanged();
}

void KCookiesPolicies::deleteAllPressed()
{
  m_pDomainPolicy.clear();
  dlg->lvDomainPolicy->clear();
  updateButtons();
  configChanged();
}

void KCookiesPolicies::updateButtons()
{
  bool hasItems = dlg->lvDomainPolicy->childCount() > 0;

  dlg->pbChange->setEnabled ((hasItems && d_itemsSelected == 1));
  dlg->pbDelete->setEnabled ((hasItems && d_itemsSelected > 0));
  dlg->pbDeleteAll->setEnabled ( hasItems );
}

void KCookiesPolicies::updateDomainList(const TQStringList &domainConfig)
{
  dlg->lvDomainPolicy->clear();

  TQStringList::ConstIterator it = domainConfig.begin();
  for (; it != domainConfig.end(); ++it)
  {
    TQString domain;
    KCookieAdvice::Value advice = KCookieAdvice::Dunno;

    splitDomainAdvice(*it, domain, advice);

    if (!domain.isEmpty())
    {
        TQListViewItem* index = new TQListViewItem( dlg->lvDomainPolicy, KIDNA::toUnicode(domain),
                                                  i18n(KCookieAdvice::adviceToStr(advice)) );
        m_pDomainPolicy[index] = KCookieAdvice::adviceToStr(advice);
    }
  }
}

void KCookiesPolicies::selectionChanged ()
{
  TQListViewItem* item = dlg->lvDomainPolicy->firstChild ();

  d_itemsSelected = 0;

  while (item != 0L)
  {
    if (dlg->lvDomainPolicy->isSelected (item))
      d_itemsSelected++;
    item = item->nextSibling ();
  }

  updateButtons ();
}

void KCookiesPolicies::load()
{
  d_itemsSelected = 0;
  d_configChanged = false;

  TDEConfig cfg ("kcookiejarrc", true);
  cfg.setGroup ("Cookie Policy");

  bool enableCookies = cfg.readBoolEntry("Cookies", true);
  dlg->cbEnableCookies->setChecked (enableCookies);
  cookiesEnabled( enableCookies );

  KCookieAdvice::Value advice = KCookieAdvice::strToAdvice (cfg.readEntry(
                                               "CookieGlobalAdvice", "Ask"));
  switch (advice)
  {
    case KCookieAdvice::Accept:
      dlg->rbPolicyAccept->setChecked (true);
      break;
    case KCookieAdvice::Reject:
      dlg->rbPolicyReject->setChecked (true);
      break;
    case KCookieAdvice::Ask:
    case KCookieAdvice::Dunno:
    default:
      dlg->rbPolicyAsk->setChecked (true);
  }

  bool enable = cfg.readBoolEntry("RejectCrossDomainCookies", true);
  dlg->cbRejectCrossDomainCookies->setChecked (enable);

  bool sessionCookies = cfg.readBoolEntry("AcceptSessionCookies", true);
  dlg->cbAutoAcceptSessionCookies->setChecked (sessionCookies);
  bool cookieExpiration = cfg.readBoolEntry("IgnoreExpirationDate", false);
  dlg->cbIgnoreCookieExpirationDate->setChecked (cookieExpiration);
  updateDomainList(cfg.readListEntry("CookieDomainAdvice"));

  if (enableCookies)
  {
    ignoreCookieExpirationDate( cookieExpiration );
    autoAcceptSessionCookies( sessionCookies );
    updateButtons();
  }

  // Connect the main swicth :) Enable/disable cookie support
  connect( dlg->cbEnableCookies, TQT_SIGNAL( toggled(bool) ),
           TQT_SLOT( cookiesEnabled(bool) ) );
  connect( dlg->cbEnableCookies, TQT_SIGNAL( toggled(bool) ),
           TQT_SLOT( configChanged() ) );

  // Connect the preference check boxes...
  connect ( dlg->cbRejectCrossDomainCookies, TQT_SIGNAL(clicked()),
            TQT_SLOT(configChanged()));
  connect ( dlg->cbAutoAcceptSessionCookies, TQT_SIGNAL(toggled(bool)),
            TQT_SLOT(configChanged()));
  connect ( dlg->cbIgnoreCookieExpirationDate, TQT_SIGNAL(toggled(bool)),
            TQT_SLOT(configChanged()));

  connect ( dlg->cbAutoAcceptSessionCookies, TQT_SIGNAL(toggled(bool)),
            TQT_SLOT(autoAcceptSessionCookies(bool)));
  connect ( dlg->cbIgnoreCookieExpirationDate, TQT_SIGNAL(toggled(bool)),
            TQT_SLOT(ignoreCookieExpirationDate(bool)));

  // Connect the default cookie policy radio buttons...
  connect(dlg->bgDefault, TQT_SIGNAL(clicked(int)), TQT_SLOT(configChanged()));

  // Connect signals from the domain specific policy listview.
  connect( dlg->lvDomainPolicy, TQT_SIGNAL(selectionChanged()),
           TQT_SLOT(selectionChanged()) );
  connect( dlg->lvDomainPolicy, TQT_SIGNAL(doubleClicked (TQListViewItem *)),
           TQT_SLOT(changePressed() ) );
  connect( dlg->lvDomainPolicy, TQT_SIGNAL(returnPressed ( TQListViewItem * )),
           TQT_SLOT(changePressed() ) );

  // Connect the buttons...
  connect( dlg->pbNew, TQT_SIGNAL(clicked()), TQT_SLOT( addPressed() ) );
  connect( dlg->pbChange, TQT_SIGNAL( clicked() ), TQT_SLOT( changePressed() ) );
  connect( dlg->pbDelete, TQT_SIGNAL( clicked() ), TQT_SLOT( deletePressed() ) );
  connect( dlg->pbDeleteAll, TQT_SIGNAL( clicked() ), TQT_SLOT( deleteAllPressed() ) );
}

void KCookiesPolicies::save()
{
  // If nothing changed, ignore the save request.
  if (!d_configChanged)
    return;

  TDEConfig cfg ( "kcookiejarrc" );
  cfg.setGroup( "Cookie Policy" );

  bool state = dlg->cbEnableCookies->isChecked();
  cfg.writeEntry( "Cookies", state );
  state = dlg->cbRejectCrossDomainCookies->isChecked();
  cfg.writeEntry( "RejectCrossDomainCookies", state );
  state = dlg->cbAutoAcceptSessionCookies->isChecked();
  cfg.writeEntry( "AcceptSessionCookies", state );
  state = dlg->cbIgnoreCookieExpirationDate->isChecked();
  cfg.writeEntry( "IgnoreExpirationDate", state );

  TQString advice;
  if (dlg->rbPolicyAccept->isChecked())
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Accept);
  else if (dlg->rbPolicyReject->isChecked())
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Reject);
  else
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Ask);

  cfg.writeEntry("CookieGlobalAdvice", advice);

  TQStringList domainConfig;
  TQListViewItem *at = dlg->lvDomainPolicy->firstChild();

  while( at )
  {
    domainConfig.append(TQString::fromLatin1("%1:%2").arg(KIDNA::toAscii(at->text(0))).arg(m_pDomainPolicy[at]));
    at = at->nextSibling();
  }

  cfg.writeEntry("CookieDomainAdvice", domainConfig);
  cfg.sync();

  // Update the cookiejar...
  if (!dlg->cbEnableCookies->isChecked())
    (void)DCOPRef("kded", "kcookiejar").send("shutdown");
  else
  {
    if (!DCOPRef("kded", "kcookiejar").send("reloadPolicy"))
      KMessageBox::sorry(0, i18n("Unable to communicate with the cookie handler service.\n"
                                 "Any changes you made will not take effect until the service "
                                 "is restarted."));
  }

  // Force running io-slave to reload configurations...
  KSaveIOConfig::updateRunningIOSlaves (this);
  emit changed( false );
}


void KCookiesPolicies::defaults()
{
  dlg->cbEnableCookies->setChecked( true );
  dlg->rbPolicyAsk->setChecked( true );
  dlg->rbPolicyAccept->setChecked( false );
  dlg->rbPolicyReject->setChecked( false );
  dlg->cbRejectCrossDomainCookies->setChecked( true );
  dlg->cbAutoAcceptSessionCookies->setChecked( true );
  dlg->cbIgnoreCookieExpirationDate->setChecked( false );
  dlg->lvDomainPolicy->clear();

  cookiesEnabled( dlg->cbEnableCookies->isChecked() );
  updateButtons();
}

void KCookiesPolicies::splitDomainAdvice (const TQString& cfg, TQString &domain,
                                          KCookieAdvice::Value &advice)
{
  int sepPos = cfg.findRev(':');

  // Ignore any policy that does not contain a domain...
  if ( sepPos <= 0 )
    return;

  domain = cfg.left(sepPos);
  advice = KCookieAdvice::strToAdvice( cfg.mid( sepPos+1 ) );
}

TQString KCookiesPolicies::quickHelp() const
{
  return i18n("<h1>Cookies</h1> Cookies contain information that Konqueror"
              " (or any other TDE application using the HTTP protocol) stores"
              " on your computer from a remote Internet server. This means"
              " that a web server can store information about you and your"
              " browsing activities on your machine for later use. You might"
              " consider this an invasion of privacy.<p>However, cookies are"
              " useful in certain situations. For example, they are often used"
              " by Internet shops, so you can 'put things into a shopping"
              " basket'. Some sites require you have a browser that supports"
              " cookies.<p>Because most people want a compromise between privacy"
              " and the benefits cookies offer, TDE offers you the ability to"
              " customize the way it handles cookies. You might, for example"
              " want to set TDE's default policy to ask you whenever a server"
              " wants to set a cookie or simply reject or accept everything."
              " For example, you might choose to accept all cookies from your"
              " favorite shopping web site. For this all you have to do is"
              " either browse to that particular site and when you are presented"
              " with the cookie dialog box, click on <i> This domain </i> under"
              " the 'apply to' tab and choose accept or simply specify the name"
              " of the site in the <i> Domain Specific Policy </i> tab and set"
              " it to accept. This enables you to receive cookies from trusted"
              " web sites without being asked every time TDE receives a cookie."
             );
}

#include "kcookiespolicies.moc"
