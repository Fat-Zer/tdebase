/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tqlayout.h>
#include <tqtabwidget.h>

#include <kaboutdata.h>
#include <kcmodule.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include "installer.h"

class KSplashThemeMgr : public TDECModule
{
public:
  KSplashThemeMgr( TQWidget *parent, const char *name, const TQStringList &/*unused*/);
  ~KSplashThemeMgr();

  TQString quickHelp() const;

  virtual void init();
  virtual void save();
  virtual void load();
  virtual void defaults();

private:
  SplashInstaller *mInstaller;
};

typedef KGenericFactory< KSplashThemeMgr, TQWidget > KSplashThemeMgrFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ksplashthemes, KSplashThemeMgrFactory("ksplashthemes") )

// -----------------------------------------------------------------------------------------

KSplashThemeMgr::KSplashThemeMgr( TQWidget *parent, const char *name, const TQStringList &)
  : TDECModule( KSplashThemeMgrFactory::instance(), parent, name ), mInstaller(new SplashInstaller(this))
{
  init();

#if 0
  TQHBoxLayout *box = new TQHBoxLayout(this);
  TQTabWidget *tab = new TQTabWidget(this); // There will be more tabs in the future.
  box->addWidget(tab);
  tab->addTab( mInstaller, i18n("&Theme Installer") );
#else
  TQHBoxLayout *box = new TQHBoxLayout(this, 0, 0);
  box->addWidget(mInstaller);
#endif
  connect( mInstaller, TQT_SIGNAL(changed(bool)), TQT_SIGNAL(changed(bool)) );
  TDEAboutData *about = new TDEAboutData( "kcmksplash"
                                      ,I18N_NOOP("TDE splash screen theme manager")
                                      ,"0.1"
                                      ,0
                                      ,TDEAboutData::License_GPL
                                      ,I18N_NOOP("(c) 2003 KDE developers") );
  about->addAuthor("Ravikiran Rajagopal", 0, "ravi@ee.eng.ohio-state.edu");
  about->addCredit("Brian Ledbetter", I18N_NOOP("Original KSplash/ML author"), "brian@shadowcom.net");
  about->addCredit(I18N_NOOP("TDE Theme Manager authors" ), I18N_NOOP("Original installer code") );
  // Once string freeze is over, replace second argument with "Icon"
  about->addCredit("Hans Karlsson", 0, "karlsson.h@home.se" );
  setAboutData(about);
}

KSplashThemeMgr::~KSplashThemeMgr()
{
  // Do not delete the installer as it is now owned by the tab widget.
}

TQString KSplashThemeMgr::quickHelp() const
{
  return i18n("<h1>Splash Screen Theme Manager </h1> Install and view splash screen themes.");
}

void KSplashThemeMgr::init()
{
  TDEGlobal::dirs()->addResourceType("ksplashthemes", KStandardDirs::kde_default("data") + "ksplash/Themes");
}

void KSplashThemeMgr::save()
{
  mInstaller->save();
}

void KSplashThemeMgr::load()
{
  mInstaller->load();
}

void KSplashThemeMgr::defaults()
{
  mInstaller->defaults();
}
