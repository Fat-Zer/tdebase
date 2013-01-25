/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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

#include <tqapplication.h>
#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqdesktopwidget.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqvbox.h>
#include <tqvbuttongroup.h>
#include <tqwhatsthis.h>

#include <tdecmodule.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>

#include "krandrmodule.h"
#include "krandrmodule.moc"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

// DLL Interface for kcontrol
typedef KGenericFactory<KRandRModule, TQWidget > KSSFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_randr, KSSFactory("krandr") )
extern "C"

{
	KDE_EXPORT void init_randr()
	{
		KRandRModule::performApplyOnStartup();
	}

	KDE_EXPORT bool test_randr()
	{
	        int eventBase, errorBase;
		if( XRRQueryExtension(tqt_xdisplay(), &eventBase, &errorBase ) )
			return true;
		return false;
	}
}

void KRandRModule::performApplyOnStartup()
{
	TDEConfig config("kcmrandrrc", true);
	if (RandRDisplay::applyOnStartup(config))
	{
		// Load settings and apply appropriate config
		RandRDisplay display;
		if (display.isValid() && display.loadDisplay(config))
			display.applyProposed(false);
	}
}

KRandRModule::KRandRModule(TQWidget *parent, const char *name, const TQStringList&)
    : TDECModule(parent, name)
	, m_changed(false)
{
	if (!isValid()) {
		TQVBoxLayout *topLayout = new TQVBoxLayout(this);
		topLayout->addWidget(new TQLabel(i18n("<qt>Your X server does not support resizing and rotating the display. Please update to version 4.3 or greater. You need the X Resize And Rotate extension (RANDR) version 1.1 or greater to use this feature.</qt>"), this));
		kdWarning() << "Error: " << errorCode() << endl;
		return;
	}

	TQVBoxLayout* topLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

	TQHBox* screenBox = new TQHBox(this);
	topLayout->addWidget(screenBox);
	TQLabel *screenLabel = new TQLabel(i18n("Settings for screen:"), screenBox);
	m_screenSelector = new KComboBox(screenBox);

	for (int s = 0; s < numScreens(); s++) {
		m_screenSelector->insertItem(i18n("Screen %1").arg(s+1));
	}

	m_screenSelector->setCurrentItem(currentScreenIndex());
        screenLabel->setBuddy( m_screenSelector );
	TQWhatsThis::add(m_screenSelector, i18n("The screen whose settings you would like to change can be selected using this drop-down list."));

	connect(m_screenSelector, TQT_SIGNAL(activated(int)), TQT_SLOT(slotScreenChanged(int)));

	if (numScreens() <= 1)
		m_screenSelector->setEnabled(false);

	TQHBox* sizeBox = new TQHBox(this);
	topLayout->addWidget(sizeBox);
	TQLabel *sizeLabel = new TQLabel(i18n("Screen size:"), sizeBox);
	m_sizeCombo = new KComboBox(sizeBox);
	TQWhatsThis::add(m_sizeCombo, i18n("The size, otherwise known as the resolution, of your screen can be selected from this drop-down list."));
	connect(m_sizeCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(slotSizeChanged(int)));
        sizeLabel->setBuddy( m_sizeCombo );

	TQHBox* refreshBox = new TQHBox(this);
	topLayout->addWidget(refreshBox);
	TQLabel *rateLabel = new TQLabel(i18n("Refresh rate:"), refreshBox);
	m_refreshRates = new KComboBox(refreshBox);
	TQWhatsThis::add(m_refreshRates, i18n("The refresh rate of your screen can be selected from this drop-down list."));
	connect(m_refreshRates, TQT_SIGNAL(activated(int)), TQT_SLOT(slotRefreshChanged(int)));
        rateLabel->setBuddy( m_refreshRates );

	m_rotationGroup = new TQButtonGroup(2, Qt::Horizontal, i18n("Orientation (degrees counterclockwise)"), this);
	topLayout->addWidget(m_rotationGroup);
	m_rotationGroup->setRadioButtonExclusive(true);
	TQWhatsThis::add(m_rotationGroup, i18n("The options in this section allow you to change the rotation of your screen."));

	m_applyOnStartup = new TQCheckBox(i18n("Apply settings on TDE startup"), this);
	topLayout->addWidget(m_applyOnStartup);
	TQWhatsThis::add(m_applyOnStartup, i18n("If this option is enabled the size and orientation settings will be used when TDE starts."));
	connect(m_applyOnStartup, TQT_SIGNAL(clicked()), TQT_SLOT(setChanged()));

	TQHBox* syncBox = new TQHBox(this);
	syncBox->layout()->addItem(new TQSpacerItem(20, 1, TQSizePolicy::Maximum));
	m_syncTrayApp = new TQCheckBox(i18n("Allow tray application to change startup settings"), syncBox);
	topLayout->addWidget(syncBox);
	TQWhatsThis::add(m_syncTrayApp, i18n("If this option is enabled, options set by the system tray applet will be saved and loaded when TDE starts instead of being temporary."));
	connect(m_syncTrayApp, TQT_SIGNAL(clicked()), TQT_SLOT(setChanged()));

	topLayout->addStretch(1);

	// just set the "apply settings on startup" box
	load();
	m_syncTrayApp->setEnabled(m_applyOnStartup->isChecked());

	slotScreenChanged(TQApplication::desktop()->primaryScreen());

	setButtons(TDECModule::Apply);
}

void KRandRModule::addRotationButton(int thisRotation, bool checkbox)
{
	Q_ASSERT(m_rotationGroup);
	if (!checkbox) {
		TQRadioButton* thisButton = new TQRadioButton(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotRotationChanged()));
	} else {
		TQCheckBox* thisButton = new TQCheckBox(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotRotationChanged()));
	}
}

void KRandRModule::slotScreenChanged(int screen)
{
	setCurrentScreen(screen);

	// Clear resolutions
	m_sizeCombo->clear();

	// Add new resolutions
	for (int i = 0; i < currentScreen()->numSizes(); i++) {
		m_sizeCombo->insertItem(i18n("%1 x %2").arg(currentScreen()->pixelSize(i).width()).arg(currentScreen()->pixelSize(i).height()));

		// Aspect ratio
		/* , aspect ratio %5)*/
		/*.arg((double)currentScreen()->size(i).mwidth / (double)currentScreen()->size(i).mheight))*/
	}

	// Clear rotations
	for (int i = m_rotationGroup->count() - 1; i >= 0; i--)
		m_rotationGroup->remove(m_rotationGroup->find(i));

	// Create rotations
	for (int i = 0; i < RandRScreen::OrientationCount; i++)
		addRotationButton(1 << i, i > RandRScreen::RotationCount - 1);

	populateRefreshRates();

	update();

	setChanged();
}

void KRandRModule::slotRotationChanged()
{
	if (m_rotationGroup->find(0)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate0);
	else if (m_rotationGroup->find(1)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate90);
	else if (m_rotationGroup->find(2)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate180);
	else {
		Q_ASSERT(m_rotationGroup->find(3)->isOn());
		currentScreen()->proposeRotation(RandRScreen::Rotate270);
	}

	if (m_rotationGroup->find(4)->isOn())
		currentScreen()->proposeRotation(currentScreen()->proposedRotation() ^ RandRScreen::ReflectX);

	if (m_rotationGroup->find(5)->isOn())
		currentScreen()->proposeRotation(currentScreen()->proposedRotation() ^ RandRScreen::ReflectY);

	setChanged();
}

void KRandRModule::slotSizeChanged(int index)
{
	int oldProposed = currentScreen()->proposedSize();

	currentScreen()->proposeSize(index);

	if (currentScreen()->proposedSize() != oldProposed) {
		currentScreen()->proposeRefreshRate(0);

		populateRefreshRates();

		// Item with index zero is already selected
	}

	setChanged();
}

void KRandRModule::slotRefreshChanged(int index)
{
	currentScreen()->proposeRefreshRate(index);

	setChanged();
}

void KRandRModule::populateRefreshRates()
{
	m_refreshRates->clear();

	TQStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	m_refreshRates->setEnabled(rr.count());

	for (TQStringList::Iterator it = rr.begin(); it != rr.end(); ++it)
		m_refreshRates->insertItem(*it);
}


void KRandRModule::defaults()
{
	load( true );
}

void KRandRModule::load()
{
	load( false );
}

void KRandRModule::load( bool useDefaults )
{
	if (!isValid())
		return;

	// Don't load screen configurations:
	// It will be correct already if they wanted to retain their settings over TDE restarts,
	// and if it isn't correct they have changed a) their X configuration, b) the screen
	// with another program, or c) their hardware.
	TDEConfig config("kcmrandrrc", true);

   config.setReadDefaults( useDefaults );

	m_oldApply = loadDisplay(config, false);
	m_oldSyncTrayApp = syncTrayApp(config);

	m_applyOnStartup->setChecked(m_oldApply);
	m_syncTrayApp->setChecked(m_oldSyncTrayApp);

	emit changed( useDefaults ); 
}

void KRandRModule::save()
{
	if (!isValid())
		return;

	apply();

	m_oldApply = m_applyOnStartup->isChecked();
	m_oldSyncTrayApp = m_syncTrayApp->isChecked();
	TDEConfig config("kcmrandrrc");
	saveDisplay(config, m_oldApply, m_oldSyncTrayApp);

	setChanged();
}

void KRandRModule::setChanged()
{
	bool isChanged = (m_oldApply != m_applyOnStartup->isChecked()) || (m_oldSyncTrayApp != m_syncTrayApp->isChecked());
	m_syncTrayApp->setEnabled(m_applyOnStartup->isChecked());

	if (!isChanged)
		for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
			if (screen(screenIndex)->proposedChanged()) {
				isChanged = true;
				break;
			}
		}

	if (isChanged != m_changed) {
		m_changed = isChanged;
		emit changed(m_changed);
	}
}

void KRandRModule::apply()
{
	if (m_changed) {
		applyProposed();

		update();
	}
}


void KRandRModule::update()
{
	m_sizeCombo->blockSignals(true);
	m_sizeCombo->setCurrentItem(currentScreen()->proposedSize());
	m_sizeCombo->blockSignals(false);

	m_rotationGroup->blockSignals(true);
	switch (currentScreen()->proposedRotation() & RandRScreen::RotateMask) {
		case RandRScreen::Rotate0:
			m_rotationGroup->setButton(0);
			break;
		case RandRScreen::Rotate90:
			m_rotationGroup->setButton(1);
			break;
		case RandRScreen::Rotate180:
			m_rotationGroup->setButton(2);
			break;
		case RandRScreen::Rotate270:
			m_rotationGroup->setButton(3);
			break;
		default:
			// Shouldn't hit this one
			Q_ASSERT(currentScreen()->proposedRotation() & RandRScreen::RotateMask);
			break;
	}
	m_rotationGroup->find(4)->setDown(currentScreen()->proposedRotation() & RandRScreen::ReflectX);
	m_rotationGroup->find(5)->setDown(currentScreen()->proposedRotation() & RandRScreen::ReflectY);
	m_rotationGroup->blockSignals(false);

	m_refreshRates->blockSignals(true);
	m_refreshRates->setCurrentItem(currentScreen()->proposedRefreshRate());
	m_refreshRates->blockSignals(false);
}

