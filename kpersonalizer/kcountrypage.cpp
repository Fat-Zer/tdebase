/***************************************************************************
                          kcountrypage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <tqstringlist.h>
#include <tqlabel.h>
#include <tqmap.h>

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kprocess.h>
#include <klanguagebutton.h>

#include "kfindlanguage.h"

#include "kcountrypage.h"

KCountryPage::KCountryPage(TQWidget *parent, const char *name ) : KCountryPageDlg(parent,name) {

	px_introSidebar->setPixmap(UserIcon("step1.png"));

	connect(cb_country, TQT_SIGNAL(activated(const TQString &)), TQT_SLOT(setLangForCountry(const TQString &)));
	connect(cb_language, TQT_SIGNAL(activated(const TQString &)), TQT_SLOT(setLanguageChanged()));

	// naturally, the language is not changed on startup
	b_savedLanguageChanged = false;
	b_startedLanguageChanged = false;

	// set appropriate Trinity version (kapplication.h)
	txt_welcome->setText(i18n("<h3>Welcome to Trinity %1</h3>").arg(TDE_VERSION_STRING));

	flang = new KFindLanguage();

	// need this ones for decision over restarts of kp, kicker, etc
	s_oldlocale = TDEGlobal::locale()->language();

	// load the Menus and guess the selection
	loadCountryList(cb_country);
	fillLanguageMenu(cb_language);
	cb_language->setCurrentItem(flang->getBestLang());
	cb_country->setCurrentItem("C");

	// Highlight the users's country
	for(int i = 0; i < cb_country->count(); i++) {
		if(cb_country->id(i) == flang->getCountry()) {
			cb_country->setCurrentItem(cb_country->id(i));
			break;
		}
	}

	setLanguageChanged();
}

KCountryPage::~KCountryPage(){
	delete flang;
}


void KCountryPage::loadCountryList(KLanguageButton *combo) {

	TQString sub = TQString::fromLatin1("l10n/");

	// clear the list
	combo->clear();

	TQStringList regionfiles = TDEGlobal::dirs()->findAllResources("locale", sub + "*.desktop");
	TQMap<TQString,TQString> regionnames;

	for ( TQStringList::ConstIterator it = regionfiles.begin(); it != regionfiles.end(); ++it ) {
		KSimpleConfig entry(*it);
		entry.setGroup(TQString::fromLatin1("KCM Locale"));
		TQString name = entry.readEntry(TQString::fromLatin1("Name"), i18n("without name"));

		TQString tag = *it;
		int index;

		index = tag.findRev('/');
		if (index != -1)
			tag = tag.mid(index + 1);

		index = tag.findRev('.');
		if (index != -1)
			tag.truncate(index);

		regionnames.insert(name, tag);
	}

	for ( TQMap<TQString,TQString>::ConstIterator mit = regionnames.begin(); mit != regionnames.end(); ++mit ) {
		combo->insertSubmenu( mit.key(), '-' + mit.data(), sub );
	}

	// add all languages to the list
	TQStringList countrylist = TDEGlobal::dirs()->findAllResources("locale", sub + "*/entry.desktop");
	countrylist.sort();

	for ( TQStringList::ConstIterator it = countrylist.begin(); it != countrylist.end(); ++it ) {
		KSimpleConfig entry(*it);
		entry.setGroup(TQString::fromLatin1("KCM Locale"));
		TQString name = entry.readEntry(TQString::fromLatin1("Name"), i18n("without name"));
		TQString submenu = '-' + entry.readEntry("Region");

		TQString tag = *it;
		int index = tag.findRev('/');
		tag.truncate(index);
		index = tag.findRev('/');
		tag = tag.mid(index+1);

		TQPixmap flag( locate( "locale", TQString::fromLatin1("l10n/%1/flag.png").arg(tag) ) );
		TQIconSet icon( flag );
		combo->insertItem( icon, name, tag, submenu );
	}
}

void KCountryPage::fillLanguageMenu(KLanguageButton *combo) {
	combo->clear();
	TQString submenu; // we are working on this menu
	TQStringList langlist = flang->getLangList();
	TQMap<TQString,TQString> langmap = flang->getLangMap();
	TQStringList::ConstIterator it;
	for ( it = langlist.begin(); it != langlist.end(); ++it ) {
		if ((*it).isNull()) {
			combo->insertSeparator();
			submenu = TQString::fromLatin1("all");
			combo->insertSubmenu(i18n("All"), submenu, TQString::null);
			continue;
		}
		combo->insertItem(langmap[(*it)], (*it), submenu );
	}
}

/** No descriptions */
bool KCountryPage::save(KLanguageButton *comboCountry, KLanguageButton *comboLang) {
	kdDebug() << "KCountryPage::save()" << endl;
	TDEConfigBase *config = TDEGlobal::config();

	config->setGroup(TQString::fromLatin1("Locale"));
	config->writeEntry(TQString::fromLatin1("Country"), comboCountry->current(), true, true);
	config->writeEntry(TQString::fromLatin1("Language"), comboLang->current(), true, true);
	config->sync();

	// only make the system reload the language, if the selected one deferes from the old saved one.
	if (b_savedLanguageChanged) {
		// Tell kdesktop about the new language
		TQCString replyType; TQByteArray replyData;
		TQByteArray data, da;
		TQDataStream stream( data, IO_WriteOnly );
		stream << comboLang->current();
		if ( !kapp->dcopClient()->isAttached() )
			kapp->dcopClient()->attach();
		// tdesycoca needs to be rebuilt
		TDEProcess proc;
		proc << TQString::fromLatin1("kbuildsycoca");
		proc.start(TDEProcess::DontCare);
		kdDebug() << "KLocaleConfig::save : sending signal to kdesktop" << endl;
		// inform kicker and kdeskop about the new language
		kapp->dcopClient()->send( "kicker", "Panel", "restart()", TQString::null);
		// call, not send, so that we know it's done before coming back
		// (we both access kdeglobals...)
		kapp->dcopClient()->call( "kdesktop", "KDesktopIface", "languageChanged(TQString)", data, replyType, replyData );
	}
	// KPersonalizer::next() probably waits for a return-value
	return true;
}

void KCountryPage::setLangForCountry(const TQString &country) {
	KSimpleConfig ent(locate("locale", "l10n/" + country + "/entry.desktop"), true);
	ent.setGroup(TQString::fromLatin1("KCM Locale"));
	langs = ent.readListEntry(TQString::fromLatin1("Languages"));

	TQString lang = TQString::fromLatin1("en_US");
	// use the first INSTALLED langauge in the list, or default to C
	for ( TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
		if (cb_language->contains(*it)) {
			lang = *it;
			break;
		}
    }

	cb_language->setCurrentItem(lang);
	setLanguageChanged();
}

void KCountryPage::setLanguageChanged() {
	// is the selcted language the same like the one in kdeglobals from before the start?
	b_savedLanguageChanged = (flang->getOldLang() != cb_language->current().lower());
	// is the selected language the same like the one we started kp with from main.cpp?
	b_startedLanguageChanged = (s_oldlocale != cb_language->current());
}


#include "kcountrypage.moc"
