/* This file is part of the KDE project
   Copyright (C) 2003, George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "web_module.h"
#include <tqfileinfo.h>
#include <tqhbox.h>
#include <tqspinbox.h>
#include <tqtimer.h>

#include <dom/html_inline.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <konq_pixmapprovider.h>
#include <kparts/browserextension.h>
#include <kstandarddirs.h>


KonqSideBarWebModule::KonqSideBarWebModule(KInstance *instance, TQObject *parent, TQWidget *widgetParent, TQString &desktopName, const char* name)
	: KonqSidebarPlugin(instance, parent, widgetParent, desktopName, name)
{
	_htmlPart = new KHTMLSideBar(universalMode());
	connect(_htmlPart, TQT_SIGNAL(reload()), this, TQT_SLOT(reload()));
	connect(_htmlPart, TQT_SIGNAL(completed()), this, TQT_SLOT(pageLoaded()));
	connect(_htmlPart,
		TQT_SIGNAL(setWindowCaption(const TQString&)),
		this,
		TQT_SLOT(setTitle(const TQString&)));
	connect(_htmlPart,
		TQT_SIGNAL(openURLRequest(const TQString&, KParts::URLArgs)),
		this,
		TQT_SLOT(urlClicked(const TQString&, KParts::URLArgs)));
	connect(_htmlPart->browserExtension(),
		TQT_SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
		this,
		TQT_SLOT(formClicked(const KURL&, const KParts::URLArgs&)));
	connect(_htmlPart,
		TQT_SIGNAL(setAutoReload()), this, TQT_SLOT( setAutoReload() ));
	connect(_htmlPart,
		TQT_SIGNAL(openURLNewWindow(const TQString&, KParts::URLArgs)),
		this,
		TQT_SLOT(urlNewWindow(const TQString&, KParts::URLArgs)));
	connect(_htmlPart,
		TQT_SIGNAL(submitFormRequest(const char*,const TQString&,const TQByteArray&,const TQString&,const TQString&,const TQString&)),
		this,
		TQT_SIGNAL(submitFormRequest(const char*,const TQString&,const TQByteArray&,const TQString&,const TQString&,const TQString&)));

	_desktopName = desktopName;

	KSimpleConfig ksc(_desktopName);
	ksc.setGroup("Desktop Entry");
        reloadTimeout = ksc.readNumEntry("Reload", 0);
	_url = ksc.readPathEntry("URL");
	_htmlPart->openURL(_url );
	// Must load this delayed
	TQTimer::singleShot(0, this, TQT_SLOT(loadFavicon()));
}


KonqSideBarWebModule::~KonqSideBarWebModule() {
	delete _htmlPart;
	_htmlPart = 0L;
}


TQWidget *KonqSideBarWebModule::getWidget() {
	return _htmlPart->widget();
}

void KonqSideBarWebModule::setAutoReload(){
	KDialogBase dlg(0, "", true, i18n("Set Refresh Timeout (0 disables)"),
			KDialogBase::Ok|KDialogBase::Cancel);
	TQHBox *hbox = dlg.makeHBoxMainWidget();
	
	TQSpinBox *mins = new TQSpinBox( 0, 120, 1, hbox );
	mins->setSuffix( i18n(" min") );
	TQSpinBox *secs = new TQSpinBox( 0, 59, 1, hbox );
	secs->setSuffix( i18n(" sec") );

	if( reloadTimeout > 0 )	{
		int seconds = reloadTimeout / 1000;
		secs->setValue( seconds % 60 );
		mins->setValue( ( seconds - secs->value() ) / 60 );
	}
	
	if( dlg.exec() == TQDialog::Accepted ) {
		int msec = ( mins->value() * 60 + secs->value() ) * 1000;
		reloadTimeout = msec;
		KSimpleConfig ksc(_desktopName);
		ksc.setGroup("Desktop Entry");
		ksc.writeEntry("Reload", reloadTimeout);	
		reload();
	}
}

void *KonqSideBarWebModule::provides(const TQString &) {
	return 0L;
}


void KonqSideBarWebModule::handleURL(const KURL &) {
}


void KonqSideBarWebModule::urlNewWindow(const TQString& url, KParts::URLArgs args)
{
	emit createNewWindow(KURL(url), args);
}


void KonqSideBarWebModule::urlClicked(const TQString& url, KParts::URLArgs args) 
{
	emit openURLRequest(KURL(url), args);
}


void KonqSideBarWebModule::formClicked(const KURL& url, const KParts::URLArgs& args) 
{
	_htmlPart->browserExtension()->setURLArgs(args);
	_htmlPart->openURL(url);
}


void KonqSideBarWebModule::loadFavicon() {
	TQString icon = KonqPixmapProvider::iconForURL(_url.url());
	if (icon.isEmpty()) {
		KonqFavIconMgr::downloadHostIcon(_url);
		icon = KonqPixmapProvider::iconForURL(_url.url());
	}

	if (!icon.isEmpty()) {
		emit setIcon(icon);

		KSimpleConfig ksc(_desktopName);
		ksc.setGroup("Desktop Entry");
		if (icon != ksc.readPathEntry("Icon")) {
			ksc.writePathEntry("Icon", icon);
		}
	}
}


void KonqSideBarWebModule::reload() {
	_htmlPart->openURL(_url);
}


void KonqSideBarWebModule::setTitle(const TQString& title) {
	if (!title.isEmpty()) {
		emit setCaption(title);

		KSimpleConfig ksc(_desktopName);
		ksc.setGroup("Desktop Entry");
		if (title != ksc.readPathEntry("Name")) {
			ksc.writePathEntry("Name", title);
		}
	}
}


void KonqSideBarWebModule::pageLoaded() {
	if( reloadTimeout > 0 ) {
		TQTimer::singleShot( reloadTimeout, this, TQT_SLOT( reload() ) );
	}
}


extern "C" {
	KDE_EXPORT KonqSidebarPlugin* create_konqsidebar_web(KInstance *instance, TQObject *parent, TQWidget *widget, TQString &desktopName, const char *name) {
		return new KonqSideBarWebModule(instance, parent, widget, desktopName, name);
	}
}


extern "C" {
	KDE_EXPORT bool add_konqsidebar_web(TQString* fn, TQString* param, TQMap<TQString,TQString> *map) {
		Q_UNUSED(param);
		KGlobal::dirs()->addResourceType("websidebardata", KStandardDirs::kde_default("data") + "konqsidebartng/websidebar");
		KURL url;
		url.setProtocol("file");
		TQStringList paths = KGlobal::dirs()->resourceDirs("websidebardata");
		for (TQStringList::Iterator i = paths.begin(); i != paths.end(); ++i) {
			if (TQFileInfo(*i + "websidebar.html").exists()) {
				url.setPath(*i + "websidebar.html");
				break;
			}
		}

		if (url.path().isEmpty())
			return false;
		map->insert("Type", "Link");
		map->insert("URL", url.url());
		map->insert("Icon", "netscape");
		map->insert("Name", i18n("Web SideBar Plugin"));
		map->insert("Open", "true");
		map->insert("X-TDE-KonqSidebarModule","konqsidebar_web");
		fn->setLatin1("websidebarplugin%1.desktop");
		return true;
	}
}


#include "web_module.moc"

