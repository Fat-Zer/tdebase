/**
 * smartcard.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
 * Copyright (c) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
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

#include "iccconfig.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kurlrequester.h>
#include <kgenericfactory.h>

#include <unistd.h>
#include <ksimpleconfig.h>
#include <string>
#include <stdio.h>
#include <qstring.h>

using namespace std;

/**** DLL Interface ****/
typedef KGenericFactory<KICCConfig, QWidget> KICCCFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_iccconfig, KICCCFactory("kcmiccconfig") )

KSimpleConfig *config;

/**** KICCConfig ****/

KICCConfig::KICCConfig(QWidget *parent, const char *name, const QStringList &)
  : KCModule(KICCCFactory::instance(), parent, name)
{

  QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
  config = new KSimpleConfig( QString::fromLatin1( KDE_CONFDIR "/kicc/kiccconfigrc" ));

  KAboutData *about =
  new KAboutData(I18N_NOOP("kcmiccconfig"), I18N_NOOP("KDE ICC Profile Control Module"),
                0, 0, KAboutData::License_GPL,
                I18N_NOOP("(c) 2009 Timothy Pearson"));

  about->addAuthor("Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net");
  setAboutData( about );

  base = new ICCConfigBase(this);
  layout->add(base);

  setRootOnlyMsg(i18n("<b>The ICC color profile is a system wide setting, and requires administrator access</b><br>To alter the system's ICC profile, click on the \"Administrator Mode\" button below."));
  setUseRootOnlyMsg(true);

  connect(base->enableSupport, SIGNAL(clicked()), SLOT(changed()));
  connect(base->enableSupport, SIGNAL(toggled(bool)), base->iccFile, SLOT(setEnabled(bool)));

  connect(base->iccFile, SIGNAL(textChanged(const QString&)), SLOT(changed()));

  load();

  if (getuid() != 0 || !config->checkConfigFilesWritable( true )) {
    base->enableSupport->setEnabled(false);
    base->iccFile->setEnabled(false);
  }
}

KICCConfig::~KICCConfig()
{
    delete config;
}

void KICCConfig::load()
{
	load( false );
}

void KICCConfig::load(bool useDefaults )
{
  //Update the toggle buttons with the current configuration

  config->setReadDefaults( useDefaults );

  base->enableSupport->setChecked(config->readBoolEntry("EnableICC", false));
  base->iccFile->setEnabled(config->readBoolEntry("EnableICC", false));
  base->iccFile->setURL(config->readEntry("ICCFile"));

  emit changed(useDefaults);
}

void KICCConfig::save()
{
	config->writeEntry("EnableICC", base->enableSupport->isChecked());
	config->writeEntry("ICCFile", base->iccFile->url());

	if (base->enableSupport->isChecked()) {
		// Apply ICC settings with XCalib
		string icc_command="/usr/bin/xcalib ";
		FILE *pipe_xcalib;
		char xcalib_result[2048];
		int i;
		xcalib_result[0]=0;
	
		icc_command.append(base->iccFile->url().ascii());
		if ((pipe_xcalib = popen(icc_command.c_str(), "r")) == NULL)
		{
			printf("Xcalib pipe error\n\r");
		}
		else {
			fgets(xcalib_result, 2048, pipe_xcalib);
			pclose(pipe_xcalib);
			for (i=1;i<2048;i++) {
				if (xcalib_result[i] == 0) {
					xcalib_result[i-1]=0;
					i=2048;
				}
			}
			if (strlen(xcalib_result) > 2) {
				KMessageBox::error(this, QString("Unable to apply ICC configuration:\n\r%1").arg(xcalib_result));
			}
		}
	}

	emit changed(false);
}

void KICCConfig::defaults()
{
	load( true );
}

QString KICCConfig::quickHelp() const
{
  return i18n("<h1>ICC Profile Configuration</h1> This module allows you to configure KDE support"
     " for ICC profiles. This allows you to easily color correct your monitor"
     " for a more lifelike and vibrant image.");
}

#include "iccconfig.moc"