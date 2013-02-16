/*
  Copyright (c) 2002 Leo Savernik <l.savernik@aon.at>
  Derived from jsopt.cpp, code copied from there is copyrighted to its
  respective owners.

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

#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqwhatsthis.h>

#include <tdeconfig.h>
#include <kdebug.h>
#include <tdelocale.h>

#include "jspolicies.h"

// == class JSPolicies ==

JSPolicies::JSPolicies(TDEConfig* config,const TQString &group,
		bool global,const TQString &domain) :
	Policies(config,group,global,domain,"javascript.","EnableJavaScript") {
}

JSPolicies::JSPolicies() : Policies(0,TQString::null,false,
	TQString::null,TQString::null,TQString::null) {
}

JSPolicies::~JSPolicies() {
}

void JSPolicies::load() {
  Policies::load();

  TQString key;

//  enableJavaScriptDebugCB->setChecked( m_pConfig->readBoolEntry("EnableJavaScriptDebug",false));
//  enableDebugOutputCB->setChecked( m_pConfig->readBoolEntry("EnableJSDebugOutput") );
  key = prefix + "WindowOpenPolicy";
  window_open = config->readUnsignedNumEntry(key,
  	is_global ? TDEHTMLSettings::KJSWindowOpenSmart : INHERIT_POLICY);

  key = prefix + "WindowResizePolicy";
  window_resize = config->readUnsignedNumEntry(key,
  	is_global ? TDEHTMLSettings::KJSWindowResizeAllow : INHERIT_POLICY);

  key = prefix + "WindowMovePolicy";
  window_move = config->readUnsignedNumEntry(key,
  	is_global ? TDEHTMLSettings::KJSWindowMoveAllow : INHERIT_POLICY);

  key = prefix + "WindowFocusPolicy";
  window_focus = config->readUnsignedNumEntry(key,
  	is_global ? TDEHTMLSettings::KJSWindowFocusAllow : INHERIT_POLICY);

  key = prefix + "WindowStatusPolicy";
  window_status = config->readUnsignedNumEntry(key,
  	is_global ? TDEHTMLSettings::KJSWindowStatusAllow : INHERIT_POLICY);
}

void JSPolicies::defaults() {
  Policies::defaults();
//  enableJavaScriptGloballyCB->setChecked( true );
//  enableJavaScriptDebugCB->setChecked( false );
//  js_popup->setButton(0);
 // enableDebugOutputCB->setChecked( false );
  window_open = is_global ? TDEHTMLSettings::KJSWindowOpenSmart : INHERIT_POLICY;
  window_resize = is_global ? TDEHTMLSettings::KJSWindowResizeAllow : INHERIT_POLICY;
  window_move = is_global ? TDEHTMLSettings::KJSWindowMoveAllow : INHERIT_POLICY;
  window_focus = is_global ? TDEHTMLSettings::KJSWindowFocusAllow : INHERIT_POLICY;
  window_status = is_global ? TDEHTMLSettings::KJSWindowStatusAllow : INHERIT_POLICY;
}

void JSPolicies::save() {
  Policies::save();

  TQString key;
  key = prefix + "WindowOpenPolicy";
  if (window_open != INHERIT_POLICY)
    config->writeEntry(key, window_open);
  else
    config->deleteEntry(key);

  key = prefix + "WindowResizePolicy";
  if (window_resize != INHERIT_POLICY)
    config->writeEntry(key, window_resize);
  else
    config->deleteEntry(key);

  key = prefix + "WindowMovePolicy";
  if (window_move != INHERIT_POLICY)
    config->writeEntry(key, window_move);
  else
    config->deleteEntry(key);

  key = prefix + "WindowFocusPolicy";
  if (window_focus != INHERIT_POLICY)
    config->writeEntry(key, window_focus);
  else
    config->deleteEntry(key);

  key = prefix + "WindowStatusPolicy";
  if (window_status != INHERIT_POLICY)
    config->writeEntry(key, window_status);
  else
    config->deleteEntry(key);

  // don't do a config->sync() here for sake of efficiency
}

// == class JSPoliciesFrame ==

JSPoliciesFrame::JSPoliciesFrame(JSPolicies *policies, const TQString &title,
		TQWidget* parent) :
	TQGroupBox(title, parent, "jspoliciesframe"),
	policies(policies) {

  bool is_per_domain = !policies->isGlobal();

  setColumnLayout(0, Qt::Vertical);
  layout()->setSpacing(0);
  layout()->setMargin(0);
  TQGridLayout *this_layout = new TQGridLayout(layout(),5,10+is_per_domain*2);
  this_layout->setAlignment(Qt::AlignTop);
  this_layout->setSpacing(3);
  this_layout->setMargin(11);

  TQString wtstr;	// what's this description
  int colIdx;		// column index

  // === window.open ================================
  colIdx = 0;
  TQLabel *label = new TQLabel(i18n("Open new windows:"),this);
  this_layout->addWidget(label,0,colIdx++);

  js_popup = new TQButtonGroup(this);
  js_popup->setExclusive(true);
  js_popup->setHidden(true);

  TQRadioButton* policy_btn;
  if (is_per_domain) {
    policy_btn = new TQRadioButton(i18n("Use global"), this);
    TQWhatsThis::add(policy_btn,i18n("Use setting from global policy."));
    js_popup->insert(policy_btn,INHERIT_POLICY);
    this_layout->addWidget(policy_btn,0,colIdx++);
    this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);
  }/*end if*/

  policy_btn = new TQRadioButton(i18n("Allow"), this);
  TQWhatsThis::add(policy_btn,i18n("Accept all popup window requests."));
  js_popup->insert(policy_btn,TDEHTMLSettings::KJSWindowOpenAllow);
  this_layout->addWidget(policy_btn,0,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Ask"), this);
  TQWhatsThis::add(policy_btn,i18n("Prompt every time a popup window is requested."));
  js_popup->insert(policy_btn,TDEHTMLSettings::KJSWindowOpenAsk);
  this_layout->addWidget(policy_btn,0,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Deny"), this);
  TQWhatsThis::add(policy_btn,i18n("Reject all popup window requests."));
  js_popup->insert(policy_btn,TDEHTMLSettings::KJSWindowOpenDeny);
  this_layout->addWidget(policy_btn,0,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Smart"), this);
  TQWhatsThis::add(policy_btn, i18n("Accept popup window requests only when "
                                   "links are activated through an explicit "
                                   "mouse click or keyboard operation."));
  js_popup->insert(policy_btn,TDEHTMLSettings::KJSWindowOpenSmart);
  this_layout->addWidget(policy_btn,0,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  wtstr = i18n("If you disable this, Konqueror will stop "
               "interpreting the <i>window.open()</i> "
               "JavaScript command. This is useful if you "
               "regularly visit sites that make extensive use "
               "of this command to pop up ad banners.<br>"
               "<br><b>Note:</b> Disabling this option might "
               "also break certain sites that require <i>"
               "window.open()</i> for proper operation. Use "
               "this feature carefully.");
  TQWhatsThis::add(label, wtstr);
  connect(js_popup, TQT_SIGNAL(clicked(int)), TQT_SLOT(setWindowOpenPolicy(int)));

  // === window.resizeBy/resizeTo ================================
  colIdx = 0;
  label = new TQLabel(i18n("Resize window:"),this);
  this_layout->addWidget(label,1,colIdx++);

  js_resize = new TQButtonGroup(this);
  js_resize->setExclusive(true);
  js_resize->setHidden(true);

  if (is_per_domain) {
    policy_btn = new TQRadioButton(i18n("Use global"), this);
    TQWhatsThis::add(policy_btn,i18n("Use setting from global policy."));
    js_resize->insert(policy_btn,INHERIT_POLICY);
    this_layout->addWidget(policy_btn,1,colIdx++);
    this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);
  }/*end if*/

  policy_btn = new TQRadioButton(i18n("Allow"), this);
  TQWhatsThis::add(policy_btn,i18n("Allow scripts to change the window size."));
  js_resize->insert(policy_btn,TDEHTMLSettings::KJSWindowResizeAllow);
  this_layout->addWidget(policy_btn,1,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Ignore"), this);
  TQWhatsThis::add( policy_btn,i18n("Ignore attempts of scripts to change the window size. "
  				"The web page will <i>think</i> it changed the "
				"size but the actual window is not affected."));
  js_resize->insert(policy_btn,TDEHTMLSettings::KJSWindowResizeIgnore);
  this_layout->addWidget(policy_btn,1,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  wtstr = i18n("Some websites change the window size on their own by using "
  		"<i>window.resizeBy()</i> or <i>window.resizeTo()</i>. "
		"This option specifies the treatment of such "
		"attempts.");
  TQWhatsThis::add(label, wtstr);
  connect(js_resize, TQT_SIGNAL(clicked(int)), TQT_SLOT(setWindowResizePolicy(int)));

  // === window.moveBy/moveTo ================================
  colIdx = 0;
  label = new TQLabel(i18n("Move window:"),this);
  this_layout->addWidget(label,2,colIdx++);

  js_move = new TQButtonGroup(this);
  js_move->setExclusive(true);
  js_move->setHidden(true);

  if (is_per_domain) {
    policy_btn = new TQRadioButton(i18n("Use global"), this);
    TQWhatsThis::add(policy_btn,i18n("Use setting from global policy."));
    js_move->insert(policy_btn,INHERIT_POLICY);
    this_layout->addWidget(policy_btn,2,colIdx++);
    this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);
  }/*end if*/

  policy_btn = new TQRadioButton(i18n("Allow"), this);
  TQWhatsThis::add(policy_btn,i18n("Allow scripts to change the window position."));
  js_move->insert(policy_btn,TDEHTMLSettings::KJSWindowMoveAllow);
  this_layout->addWidget(policy_btn,2,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Ignore"), this);
  TQWhatsThis::add(policy_btn,i18n("Ignore attempts of scripts to change the window position. "
  				"The web page will <i>think</i> it moved the "
				"window but the actual position is not affected."));
  js_move->insert(policy_btn,TDEHTMLSettings::KJSWindowMoveIgnore);
  this_layout->addWidget(policy_btn,2,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  wtstr = i18n("Some websites change the window position on their own by using "
  		"<i>window.moveBy()</i> or <i>window.moveTo()</i>. "
		"This option specifies the treatment of such "
		"attempts.");
  TQWhatsThis::add(label, wtstr);
  connect(js_move, TQT_SIGNAL(clicked(int)), TQT_SLOT(setWindowMovePolicy(int)));

  // === window.focus ================================
  colIdx = 0;
  label = new TQLabel(i18n("Focus window:"),this);
  this_layout->addWidget(label,3,colIdx++);

  js_focus = new TQButtonGroup(this);
  js_focus->setExclusive(true);
  js_focus->setHidden(true);

  if (is_per_domain) {
    policy_btn = new TQRadioButton(i18n("Use global"), this);
    TQWhatsThis::add(policy_btn,i18n("Use setting from global policy."));
    js_focus->insert(policy_btn,INHERIT_POLICY);
    this_layout->addWidget(policy_btn,3,colIdx++);
    this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);
  }/*end if*/

  policy_btn = new TQRadioButton(i18n("Allow"), this);
  TQWhatsThis::add( policy_btn,i18n("Allow scripts to focus the window.") );
  js_focus->insert(policy_btn,TDEHTMLSettings::KJSWindowFocusAllow);
  this_layout->addWidget(policy_btn,3,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Ignore"), this);
  TQWhatsThis::add( policy_btn,i18n("Ignore attempts of scripts to focus the window. "
  				"The web page will <i>think</i> it brought "
				"the focus to the window but the actual "
				"focus will remain unchanged.") );
  js_focus->insert(policy_btn,TDEHTMLSettings::KJSWindowFocusIgnore);
  this_layout->addWidget(policy_btn,3,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  wtstr = i18n("Some websites set the focus to their browser window on their "
  		"own by using <i>window.focus()</i>. This usually leads to "
		"the window being moved to the front interrupting whatever "
		"action the user was dedicated to at that time. "
		"This option specifies the treatment of such "
		"attempts.");
  TQWhatsThis::add(label, wtstr);
  connect(js_focus, TQT_SIGNAL(clicked(int)), TQT_SLOT(setWindowFocusPolicy(int)));

  // === window.status ================================
  colIdx = 0;
  label = new TQLabel(i18n("Modify status bar text:"),this);
  this_layout->addWidget(label,4,colIdx++);

  js_statusbar = new TQButtonGroup(this);
  js_statusbar->setExclusive(true);
  js_statusbar->setHidden(true);

  if (is_per_domain) {
    policy_btn = new TQRadioButton(i18n("Use global"), this);
    TQWhatsThis::add(policy_btn,i18n("Use setting from global policy."));
    js_statusbar->insert(policy_btn,INHERIT_POLICY);
    this_layout->addWidget(policy_btn,4,colIdx++);
    this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);
  }/*end if*/

  policy_btn = new TQRadioButton(i18n("Allow"), this);
  TQWhatsThis::add(policy_btn,i18n("Allow scripts to change the text of the status bar."));
  js_statusbar->insert(policy_btn,TDEHTMLSettings::KJSWindowStatusAllow);
  this_layout->addWidget(policy_btn,4,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  policy_btn = new TQRadioButton(i18n("Ignore"), this);
  TQWhatsThis::add( policy_btn,i18n("Ignore attempts of scripts to change the status bar text. "
  				"The web page will <i>think</i> it changed "
				"the text but the actual text will remain "
				"unchanged.") );
  js_statusbar->insert(policy_btn,TDEHTMLSettings::KJSWindowStatusIgnore);
  this_layout->addWidget(policy_btn,4,colIdx++);
  this_layout->addItem(new TQSpacerItem(10,0),0,colIdx++);

  wtstr = i18n("Some websites change the status bar text by setting "
  		"<i>window.status</i> or <i>window.defaultStatus</i>, "
		"thus sometimes preventing displaying the real URLs of hyperlinks. "
		"This option specifies the treatment of such "
		"attempts.");
  TQWhatsThis::add(label, wtstr);
  connect(js_statusbar, TQT_SIGNAL(clicked(int)), TQT_SLOT(setWindowStatusPolicy(int)));
}

JSPoliciesFrame::~JSPoliciesFrame() {
}

void JSPoliciesFrame::refresh() {
  TQRadioButton *button;
  button = static_cast<TQRadioButton *>(js_popup->find(
  		policies->window_open));
  if (button != 0) button->setChecked(true);
  button = static_cast<TQRadioButton *>(js_resize->find(
  		policies->window_resize));
  if (button != 0) button->setChecked(true);
  button = static_cast<TQRadioButton *>(js_move->find(
  		policies->window_move));
  if (button != 0) button->setChecked(true);
  button = static_cast<TQRadioButton *>(js_focus->find(
  		policies->window_focus));
  if (button != 0) button->setChecked(true);
  button = static_cast<TQRadioButton *>(js_statusbar->find(
  		policies->window_status));
  if (button != 0) button->setChecked(true);
}

void JSPoliciesFrame::setWindowOpenPolicy(int id) {
  policies->window_open = id;
  emit changed();
}

void JSPoliciesFrame::setWindowResizePolicy(int id) {
  policies->window_resize = id;
  emit changed();
}

void JSPoliciesFrame::setWindowMovePolicy(int id) {
  policies->window_move = id;
  emit changed();
}

void JSPoliciesFrame::setWindowFocusPolicy(int id) {
  policies->window_focus = id;
  emit changed();
}

void JSPoliciesFrame::setWindowStatusPolicy(int id) {
  policies->window_status = id;
  emit changed();
}

#include "jspolicies.moc"
