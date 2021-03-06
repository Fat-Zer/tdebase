/*
  Copyright (c) 2002 Leo Savernik <l.savernik@aon.at>
  Derived from jsopts.cpp and javaopts.cpp, code copied from there is
  copyrighted to its respective owners.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <tdeconfig.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#include "domainlistview.h"
#include "policies.h"
#include "policydlg.h"

DomainListView::DomainListView(TDEConfig *config,const TQString &title,
		TQWidget *parent,const char *name) :
	TQGroupBox(title, parent, name), config(config) {
  setColumnLayout(0, Qt::Vertical);
  layout()->setSpacing(0);
  layout()->setMargin(0);
  TQGridLayout* thisLayout = new TQGridLayout(layout());
  thisLayout->setAlignment(Qt::AlignTop);
  thisLayout->setSpacing(KDialog::spacingHint());
  thisLayout->setMargin(KDialog::marginHint());

  domainSpecificLV = new TDEListView(this);
  domainSpecificLV->addColumn(i18n("Host/Domain"));
  domainSpecificLV->addColumn(i18n("Policy"), 100);
  connect(domainSpecificLV,TQT_SIGNAL(doubleClicked(TQListViewItem *)), TQT_SLOT(changePressed()));
  connect(domainSpecificLV,TQT_SIGNAL(returnPressed(TQListViewItem *)), TQT_SLOT(changePressed()));
  connect(domainSpecificLV, TQT_SIGNAL( executed( TQListViewItem *)), TQT_SLOT( updateButton()));
  connect(domainSpecificLV, TQT_SIGNAL(selectionChanged()), TQT_SLOT(updateButton()));
  thisLayout->addMultiCellWidget(domainSpecificLV, 0, 5, 0, 0);

  addDomainPB = new TQPushButton(i18n("&New..."), this);
  thisLayout->addWidget(addDomainPB, 0, 1);
  connect(addDomainPB, TQT_SIGNAL(clicked()), TQT_SLOT(addPressed()));

  changeDomainPB = new TQPushButton( i18n("Chan&ge..."), this);
  thisLayout->addWidget(changeDomainPB, 1, 1);
  connect(changeDomainPB, TQT_SIGNAL(clicked()), this, TQT_SLOT(changePressed()));

  deleteDomainPB = new TQPushButton(i18n("De&lete"), this);
  thisLayout->addWidget(deleteDomainPB, 2, 1);
  connect(deleteDomainPB, TQT_SIGNAL(clicked()), this, TQT_SLOT(deletePressed()));

  importDomainPB = new TQPushButton(i18n("&Import..."), this);
  thisLayout->addWidget(importDomainPB, 3, 1);
  connect(importDomainPB, TQT_SIGNAL(clicked()), this, TQT_SLOT(importPressed()));
  importDomainPB->setEnabled(false);
  importDomainPB->hide();

  exportDomainPB = new TQPushButton(i18n("&Export..."), this);
  thisLayout->addWidget(exportDomainPB, 4, 1);
  connect(exportDomainPB, TQT_SIGNAL(clicked()), this, TQT_SLOT(exportPressed()));
  exportDomainPB->setEnabled(false);
  exportDomainPB->hide();

  TQSpacerItem* spacer = new TQSpacerItem(20, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  thisLayout->addItem(spacer, 5, 1);

  TQWhatsThis::add( addDomainPB, i18n("Click on this button to manually add a host or domain "
                                       "specific policy.") );
  TQWhatsThis::add( changeDomainPB, i18n("Click on this button to change the policy for the "
                                          "host or domain selected in the list box.") );
  TQWhatsThis::add( deleteDomainPB, i18n("Click on this button to delete the policy for the "
                                          "host or domain selected in the list box.") );
  updateButton();
}

DomainListView::~DomainListView() {
  // free all policies
  DomainPolicyMap::Iterator it = domainPolicies.begin();
  for (; it != domainPolicies.end(); ++it) {
    delete it.data();
  }/*next it*/
}

void DomainListView::updateButton()
{
    TQListViewItem *index = domainSpecificLV->currentItem();
    bool enable = ( index != 0 );
    changeDomainPB->setEnabled( enable );
    deleteDomainPB->setEnabled( enable );

}

void DomainListView::addPressed()
{
//    JavaPolicies pol_copy(m_pConfig,m_groupname,false);
    Policies *pol = createPolicies();
    pol->defaults();
    PolicyDialog pDlg(pol, this);
    setupPolicyDlg(AddButton,pDlg,pol);
    if( pDlg.exec() ) {
        TQListViewItem* index = new TQListViewItem( domainSpecificLV, pDlg.domain(),
                                                  pDlg.featureEnabledPolicyText() );
	pol->setDomain(pDlg.domain());
        domainPolicies.insert(index, pol);
        domainSpecificLV->setCurrentItem( index );
        emit changed(true);
    } else {
        delete pol;
    }
    updateButton();
}

void DomainListView::changePressed()
{
    TQListViewItem *index = domainSpecificLV->currentItem();
    if ( index == 0 )
    {
        KMessageBox::information( 0, i18n("You must first select a policy to be changed." ) );
        return;
    }

    Policies *pol = domainPolicies[index];
    // This must be copied because the policy dialog is allowed to change
    // the data even if the changes are rejected in the end.
    Policies *pol_copy = copyPolicies(pol);

    PolicyDialog pDlg( pol_copy, this );
    pDlg.setDisableEdit( true, index->text(0) );
    setupPolicyDlg(ChangeButton,pDlg,pol_copy);
    if( pDlg.exec() )
    {
        pol_copy->setDomain(pDlg.domain());
        domainPolicies[index] = pol_copy;
	pol_copy = pol;
        index->setText(0, pDlg.domain() );
        index->setText(1, pDlg.featureEnabledPolicyText());
        emit changed(true);
    }
    delete pol_copy;
}

void DomainListView::deletePressed()
{
    TQListViewItem *index = domainSpecificLV->currentItem();
    if ( index == 0 )
    {
        KMessageBox::information( 0, i18n("You must first select a policy to delete." ) );
        return;
    }

    DomainPolicyMap::Iterator it = domainPolicies.find(index);
    if (it != domainPolicies.end()) {
      delete it.data();
      domainPolicies.remove(it);
      delete index;
      emit changed(true);
    }
    updateButton();
}

void DomainListView::importPressed()
{
  // PENDING(kalle) Implement this.
}

void DomainListView::exportPressed()
{
  // PENDING(kalle) Implement this.
}

void DomainListView::initialize(const TQStringList &domainList)
{
    domainSpecificLV->clear();
    domainPolicies.clear();
//    JavaPolicies pol(m_pConfig,m_groupname,false);
    for (TQStringList::ConstIterator it = domainList.begin();
         it != domainList.end(); ++it) {
      TQString domain = *it;
      Policies *pol = createPolicies();
      pol->setDomain(domain);
      pol->load();

      TQString policy;
      if (pol->isFeatureEnabledPolicyInherited())
        policy = i18n("Use Global");
      else if (pol->isFeatureEnabled())
        policy = i18n("Accept");
      else
        policy = i18n("Reject");
      TQListViewItem *index =
        new TQListViewItem( domainSpecificLV, domain, policy );

      domainPolicies[index] = pol;
    }
}

void DomainListView::save(const TQString &group, const TQString &domainListKey) {
    TQStringList domainList;
    DomainPolicyMap::Iterator it = domainPolicies.begin();
    for (; it != domainPolicies.end(); ++it) {
    	TQListViewItem *current = it.key();
	Policies *pol = it.data();
	pol->save();
	domainList.append(current->text(0));
    }
    config->setGroup(group);
    config->writeEntry(domainListKey, domainList);
}

void DomainListView::setupPolicyDlg(PushButton /*trigger*/,
		PolicyDialog &/*pDlg*/,Policies */*copy*/) {
  // do nothing
}

#include "domainlistview.moc"
