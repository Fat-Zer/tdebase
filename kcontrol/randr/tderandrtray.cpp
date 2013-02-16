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

#include <tqtimer.h>
#include <tqimage.h>
#include <tqtooltip.h>

#include <tdeaction.h>
#include <tdeapplication.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <tdeglobal.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>

#include <cstdlib>
#include <unistd.h>

#include "configdialog.h"

#include "tderandrtray.h"
#include "tderandrpassivepopup.h"
#include "tderandrtray.moc"

#define OUTPUT_CONNECTED		(1 << 0)
#define OUTPUT_UNKNOWN			(1 << 1)
#define OUTPUT_DISCONNECTED		(1 << 2)
#define OUTPUT_ON			(1 << 3)
#define OUTPUT_ALL			(0xf)

KRandRSystemTray::KRandRSystemTray(TQWidget* parent, const char *name)
	: KSystemTray(parent, name)
	, m_popupUp(false)
	, m_help(new KHelpMenu(this, TDEGlobal::instance()->aboutData(), false, actionCollection()))
{
	setPixmap(KSystemTray::loadSizedIcon("randr", width()));
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	connect(this, TQT_SIGNAL(quitSelected()), this, TQT_SLOT(_quit()));
	TQToolTip::add(this, i18n("Screen resize & rotate"));
	my_parent = parent;

	//printf("Reading configuration...\n\r");
	globalKeys = new TDEGlobalAccel(TQT_TQOBJECT(this));
	TDEGlobalAccel* keys = globalKeys;
#include "tderandrbindings.cpp"
	// the keys need to be read from kdeglobals, not kickerrc
	globalKeys->readSettings();
	globalKeys->setEnabled(true);
	globalKeys->updateConnections();

	connect(kapp, TQT_SIGNAL(settingsChanged(int)), TQT_SLOT(slotSettingsChanged(int)));

#if (TQT_VERSION-0 >= 0x030200) // XRANDR support
//	connect(this, TQT_SIGNAL(screenSizeChanged(int, int)), kapp->desktop(), TQT_SLOT( desktopResized()));
#endif

	randr_display = XOpenDisplay(NULL);

	if (isValid() == true) {
		last_known_x = currentScreen()->currentPixelWidth();
		last_known_y = currentScreen()->currentPixelHeight();
	}

	t_config = new KSimpleConfig("kiccconfigrc");

	TQString cur_profile;
	cur_profile = getCurrentProfile();
	if (cur_profile != "") {
		applyIccConfiguration(cur_profile, NULL);
	}

	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	connect(hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(deviceChanged(TDEGenericDevice*)));
}

/*!
 * \b TQT_SLOT which called if tderandrtray is exited by the user. In this case the user
 * is asked through a yes/no box if "KRandRTray should start automatically on log in" and the
 * result is written to the KDE configfile.
 */
void KRandRSystemTray::_quit (){
	r_config = new KSimpleConfig("tderandrtrayrc");

	TQString tmp1 = i18n ("Start KRandRTray automatically when you log in?");
	int tmp2 = KMessageBox::questionYesNo ( 0, tmp1, i18n("Question"), i18n("Start Automatically"), i18n("Do Not Start"));
	r_config->setGroup("General");
	r_config->writeEntry ("Autostart", tmp2 == KMessageBox::Yes);
	r_config->sync ();

	exit(0);
}

void KRandRSystemTray::resizeEvent ( TQResizeEvent * )
{
	// Honor Free Desktop specifications that allow for arbitrary system tray icon sizes
	TQPixmap origpixmap;
	TQPixmap scaledpixmap;
	TQImage newIcon;
	origpixmap = KSystemTray::loadSizedIcon( "randr", width() );
	newIcon = origpixmap;
	newIcon = newIcon.smoothScale(width(), height());
	scaledpixmap = newIcon;
	setPixmap(scaledpixmap);
}

void KRandRSystemTray::mousePressEvent(TQMouseEvent* e)
{
	// Popup the context menu with left-click
	if (e->button() == Qt::LeftButton) {
		contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
		e->accept();
		return;
	}

	KSystemTray::mousePressEvent(e);
}

void KRandRSystemTray::reloadDisplayConfiguration()
{
	// Reload the randr configuration...
	int i;
	int activeOutputs = 0;
	int screenDeactivated = 0;

	if (isValid() == true) {
		randr_screen_info = read_screen_info(randr_display);

		// Count outputs in the active state
		activeOutputs = 0;
		for (i = 0; i < randr_screen_info->n_output; i++) {
			// Look for ON outputs
			if (!randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}
			// Look for CONNECTED outputs
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				continue;
			}

			activeOutputs++;
		}

		if (activeOutputs < 1) {
			// Houston, we have a problem!
			// There are no active displays!
			// Activate the first connected display we come across...
			for (i = 0; i < randr_screen_info->n_output; i++) {
				// Look for OFF outputs
				if (randr_screen_info->outputs[i]->cur_crtc) {
					continue;
				}
				// Look for CONNECTED outputs
				if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
					continue;
				}
	
				// Activate this output
				randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
				randr_screen_info->cur_output = randr_screen_info->outputs[i];
				randr_screen_info->cur_output->auto_set = 1;
				randr_screen_info->cur_output->off_set = 0;
				output_auto (randr_screen_info, randr_screen_info->cur_output);
				i=main_low_apply(randr_screen_info);
		
				if (randr_screen_info->outputs[i]->cur_crtc) {
					// Output successfully activated!
					set_primary_output(randr_screen_info, randr_screen_info->cur_output->id);
					break;
				}
			}
		}

		for (i = 0; i < randr_screen_info->n_output; i++) {
			// Look for ON outputs
			if (!randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}
			// Look for DISCONNECTED outputs
			if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
				continue;
			}

			// Deactivate this display to avoid a crash!
			randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[i];
			randr_screen_info->cur_output->auto_set = 0;
			randr_screen_info->cur_output->off_set = 1;
			output_off(randr_screen_info, randr_screen_info->cur_output);
			main_low_apply(randr_screen_info);

			screenDeactivated = 1;
		}

		if (screenDeactivated == 1) {
			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
	}
}

void KRandRSystemTray::contextMenuAboutToShow(TDEPopupMenu* menu)
{
	int lastIndex = 0;

	reloadDisplayConfiguration();

	menu->clear();
	menu->setCheckable(true);

	bool valid = isValid();

	if (!valid) {
		lastIndex = menu->insertItem(i18n("Required X Extension Not Available"));
		menu->setItemEnabled(lastIndex, false);

	}
	else {
		m_screenPopups.clear();
		for (int s = 0; s < numScreens() /*&& numScreens() > 1 */; s++) {
			setCurrentScreen(s);
			if (s == screenIndexOfWidget(this)) {
				/*lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);*/
			} else {
				TDEPopupMenu* subMenu = new TDEPopupMenu(menu, TQString("screen%1").arg(s+1).latin1());
				m_screenPopups.append(subMenu);
				populateMenu(subMenu);
				lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1), subMenu);
				connect(subMenu, TQT_SIGNAL(activated(int)), TQT_SLOT(slotScreenActivated()));
			}
		}

		setCurrentScreen(screenIndexOfWidget(this));
		populateMenu(menu);
	}

	addOutputMenu(menu);

	// Find any user ICC profiles
	TQStringList cfgProfiles;
	cfgProfiles = t_config->groupList();
	if (cfgProfiles.isEmpty() == false) {
		menu->insertTitle(SmallIcon("kcoloredit"), i18n("Color Profile"));
	}
	for (TQStringList::Iterator t(cfgProfiles.begin()); t != cfgProfiles.end(); ++t) {
		lastIndex = menu->insertItem(*t);
		if (t_config->readEntry("CurrentProfile") == (*t)) {
			menu->setItemChecked(lastIndex, true);
		}
		menu->setItemEnabled(lastIndex, t_config->readBoolEntry("EnableICC", false));
		menu->connectItem(lastIndex, this, TQT_SLOT(slotColorProfileChanged(int)));
	}

	if (valid) {
		// Find any display profiles
		TQStringList displayProfiles;
		displayProfiles = getDisplayConfigurationProfiles(locateLocal("config", "/", true));
		if (displayProfiles.isEmpty() == false) {
			menu->insertTitle(SmallIcon("background"), i18n("Display Profiles"));
		}
		lastIndex = menu->insertItem(SmallIcon("bookmark"), "<default>");
		menu->connectItem(lastIndex, this, TQT_SLOT(slotDisplayProfileChanged(int)));
		for (TQStringList::Iterator t(displayProfiles.begin()); t != displayProfiles.end(); ++t) {
			lastIndex = menu->insertItem(SmallIcon("bookmark"), *t);
			menu->connectItem(lastIndex, this, TQT_SLOT(slotDisplayProfileChanged(int)));
		}
	}

	menu->insertTitle(SmallIcon("randr"), i18n("Global Configuation"));

	TDEAction *actColors = new TDEAction( i18n( "Configure Displays..." ),
		SmallIconSet( "configure" ), TDEShortcut(), TQT_TQOBJECT(this), TQT_SLOT( slotDisplayConfig() ),
		actionCollection() );
	actColors->plug( menu );

// 	TDEAction *actPrefs = new TDEAction( i18n( "Configure Display..." ),
// 		SmallIconSet( "configure" ), TDEShortcut(), this, TQT_SLOT( slotPrefs() ),
// 		actionCollection() );
// 	actPrefs->plug( menu );

	TDEAction *actSKeys = new TDEAction( i18n( "Configure Shortcut Keys..." ),
		SmallIconSet( "configure" ), TDEShortcut(), TQT_TQOBJECT(this), TQT_SLOT( slotSKeys() ),
		actionCollection() );
	actSKeys->plug( menu );

	menu->insertItem(SmallIcon("help"),KStdGuiItem::help().text(), m_help->menu());
	TDEAction *quitAction = actionCollection()->action(KStdAction::name(KStdAction::Quit));
	quitAction->plug(menu);

	m_menu = menu;
}

void KRandRSystemTray::slotScreenActivated()
{
	setCurrentScreen(m_screenPopups.find(static_cast<const TDEPopupMenu*>(sender())));
}

void KRandRSystemTray::configChanged()
{
	refresh();

	static bool first = true;

	if ((last_known_x == currentScreen()->currentPixelWidth()) && \
		(last_known_y == currentScreen()->currentPixelHeight())) {
		first = true;
	}

	last_known_x = currentScreen()->currentPixelWidth();
	last_known_y = currentScreen()->currentPixelHeight();

	if (!first) {
		emit (screenSizeChanged(currentScreen()->currentPixelWidth(), currentScreen()->currentPixelHeight()));

		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		currentScreen()->changedMessage(), SmallIcon("window_fullscreen"),
		this, "ScreenChangeNotification");
	}

	first = false;

	TQString cur_profile;
	cur_profile = getCurrentProfile();
	if (cur_profile != "") {
		applyIccConfiguration(cur_profile, NULL);
	}
}

int KRandRSystemTray::GetDefaultResolutionParameter()
{
	int returnIndex = 0;

	int numSizes = currentScreen()->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = currentScreen()->pixelCount(i);
	}

	int highest = -1, highestIndex = -1;

	for (int i = 0; i < numSizes; i++) {
		if (sizeSort[i] && sizeSort[i] > highest) {
			highest = sizeSort[i];
			highestIndex = i;
		}
	}
	sizeSort[highestIndex] = -1;
	Q_ASSERT(highestIndex != -1);

	returnIndex = highestIndex;

	delete [] sizeSort;
	sizeSort = 0L;

	return returnIndex;
}

int KRandRSystemTray::GetHackResolutionParameter() {
	int resparm;

	resparm = GetDefaultResolutionParameter();
	resparm++;

	return resparm;
}

void KRandRSystemTray::populateMenu(TDEPopupMenu* menu)
{
	int lastIndex = 0;

	menu->insertTitle(SmallIcon("window_fullscreen"), i18n("Screen Size"));

	int numSizes = currentScreen()->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = currentScreen()->pixelCount(i);
	}

	for (int j = 0; j < numSizes; j++) {
		int highest = -1, highestIndex = -1;

		for (int i = 0; i < numSizes; i++) {
			if (sizeSort[i] && sizeSort[i] > highest) {
				highest = sizeSort[i];
				highestIndex = i;
			}
		}
		sizeSort[highestIndex] = -1;
		Q_ASSERT(highestIndex != -1);

		lastIndex = menu->insertItem(i18n("%1 x %2").arg(currentScreen()->pixelSize(highestIndex).width()).arg(currentScreen()->pixelSize(highestIndex).height()));

		if (currentScreen()->proposedSize() == highestIndex)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, highestIndex);
		menu->connectItem(lastIndex, this, TQT_SLOT(slotResolutionChanged(int)));
	}
	delete [] sizeSort;
	sizeSort = 0L;

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (currentScreen()->rotations() != RandRScreen::Rotate0) {
		menu->insertTitle(SmallIcon("reload"), i18n("Orientation"));

		for (int i = 0; i < 6; i++) {
			if ((1 << i) & currentScreen()->rotations()) {
				lastIndex = menu->insertItem(currentScreen()->rotationIcon(1 << i), RandRScreen::rotationName(1 << i));

				if (currentScreen()->proposedRotation() & (1 << i))
					menu->setItemChecked(lastIndex, true);

				menu->setItemParameter(lastIndex, 1 << i);
				menu->connectItem(lastIndex, this, TQT_SLOT(slotOrientationChanged(int)));
			}
		}
	}

	TQStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	if (rr.count())
		menu->insertTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	for (TQStringList::Iterator it = rr.begin(); it != rr.end(); ++it, i++) {
		lastIndex = menu->insertItem(*it);

		if (currentScreen()->proposedRefreshRate() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, TQT_SLOT(slotRefreshRateChanged(int)));
	}
}

void KRandRSystemTray::slotResolutionChanged(int parameter)
{
	if (currentScreen()->currentSize() == parameter) {
		//printf("This resolution is already in use; applying again...\n\r");
		currentScreen()->proposeSize(parameter);
		currentScreen()->applyProposed();
		return;
	}

	currentScreen()->proposeSize(parameter);

	currentScreen()->proposeRefreshRate(-1);

	if (currentScreen()->applyProposedAndConfirm()) {
		TDEConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotOrientationChanged(int parameter)
{
	int propose = currentScreen()->currentRotation();

	if (parameter & RandRScreen::RotateMask)
		propose &= RandRScreen::ReflectMask;

	propose ^= parameter;

	if (currentScreen()->currentRotation() == propose)
		return;

	currentScreen()->proposeRotation(propose);

	if (currentScreen()->applyProposedAndConfirm()) {
		TDEConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotRefreshRateChanged(int parameter)
{
	if (currentScreen()->currentRefreshRate() == parameter)
		return;

	currentScreen()->proposeRefreshRate(parameter);

	if (currentScreen()->applyProposedAndConfirm()) {
		TDEConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotPrefs()
{
	KCMultiDialog *kcm = new KCMultiDialog( KDialogBase::Plain, i18n( "Configure" ), this );

	kcm->addModule( "displayconfig" );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->exec();
}

void KRandRSystemTray::slotDisplayConfig()
{
	KCMultiDialog *kcm = new KCMultiDialog( KDialogBase::Plain, i18n( "Configure" ), this );

	kcm->addModule( "displayconfig" );
	kcm->setPlainCaption( i18n( "Configure Displays" ) );
	kcm->exec();
}

void KRandRSystemTray::slotSettingsChanged(int category)
{
	if ( category == (int) TDEApplication::SETTINGS_SHORTCUTS ) {
		globalKeys->readSettings();
		globalKeys->updateConnections();
	}
}

void KRandRSystemTray::slotSKeys()
{
	ConfigDialog *dlg = new ConfigDialog(globalKeys, true);

	if ( dlg->exec() == TQDialog::Accepted ) {
		dlg->commitShortcuts();
		globalKeys->writeSettings(0, true);
		globalKeys->updateConnections();
	}

	delete dlg;
}

void KRandRSystemTray::slotCycleDisplays()
{
	XRROutputInfo *output_info;
	char *output_name;
	int i;
	int current_on_index = -1;
	int max_index = -1;
	int prev_on_index;

	randr_screen_info = read_screen_info(randr_display);

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ON outputs...
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}
		// ...that are connected
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}

		output_name = output_info->name;
		current_on_index = i;
		if (i > max_index) {
			max_index = i;
		}
	}

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for CONNECTED outputs....
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}
		// ...that are not ON
		if (randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		output_name = output_info->name;
		if (i > max_index) {
			max_index = i;
		}
	}

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ALL outputs that are not connected....
		if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
			continue;
		}
		// ...or ON
		if (randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		output_name = output_info->name;
		if (i > max_index) {
			max_index = i;
		}
	}

	//printf("Active: %d\n\r", current_on_index);
	//printf("Max: %d\n\r", max_index);

	if ((current_on_index == -1) && (max_index == -1)) {
		// There is no connected display available!  ABORT
		return;
	}

	prev_on_index = current_on_index;
	current_on_index = current_on_index + 1;
	if (current_on_index > max_index) {
		current_on_index = 0;
	}
	while (RR_Disconnected == randr_screen_info->outputs[current_on_index]->info->connection) {
		current_on_index = current_on_index + 1;
		if (current_on_index > max_index) {
			current_on_index = 0;
		}
	}
	if (prev_on_index != current_on_index) {
		randr_screen_info->cur_crtc = randr_screen_info->outputs[current_on_index]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[current_on_index];
		randr_screen_info->cur_output->auto_set = 1;
 		randr_screen_info->cur_output->off_set = 0;
		output_auto (randr_screen_info, randr_screen_info->cur_output);
		i=main_low_apply(randr_screen_info);

		if (randr_screen_info->outputs[current_on_index]->cur_crtc) {
			// Output successfully activated!
			set_primary_output(randr_screen_info, randr_screen_info->cur_output->id);

			if (prev_on_index != -1) {
				if (randr_screen_info->outputs[prev_on_index]->cur_crtc != NULL) {
					if (RR_Disconnected != randr_screen_info->outputs[prev_on_index]->info->connection) {
						randr_screen_info->cur_crtc = randr_screen_info->outputs[prev_on_index]->cur_crtc;
						randr_screen_info->cur_output = randr_screen_info->outputs[prev_on_index];
						randr_screen_info->cur_output->auto_set = 0;
						randr_screen_info->cur_output->off_set = 1;
						output_off(randr_screen_info, randr_screen_info->cur_output);
						i=main_low_apply(randr_screen_info);
					}
				}
			}

			// Do something about the disconnected outputs
			for (i = 0; i < randr_screen_info->n_output; i++) {
				output_info = randr_screen_info->outputs[i]->info;
				// Look for ON outputs
				if (!randr_screen_info->outputs[i]->cur_crtc) {
					continue;
				}
				if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
					continue;
				}

				output_name = output_info->name;

				// Deactivate this display to avoid a crash!
				randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
				randr_screen_info->cur_output = randr_screen_info->outputs[i];
				randr_screen_info->cur_output->auto_set = 0;
				randr_screen_info->cur_output->off_set = 1;
				output_off(randr_screen_info, randr_screen_info->cur_output);
				main_low_apply(randr_screen_info);
			}

			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
		else {
			output_name = randr_screen_info->outputs[current_on_index]->info->name;
 			KMessageBox::sorry(my_parent, i18n("<b>Unable to activate output %1</b><p>Either the output is not connected to a display,<br>or the display configuration is not detectable").arg(output_name), i18n("Output Unavailable"));
		}
	}
}

void KRandRSystemTray::findPrimaryDisplay()
{
	int i;

	for (i = 0; i < randr_screen_info->n_output; i++) {
		// Look for ON outputs...
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		// ...that are connected
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}

		randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[i];
	}
}

void KRandRSystemTray::addOutputMenu(TDEPopupMenu* menu)
{
	XRROutputInfo *output_info;
	char *output_name;
	int i;
	int lastIndex = 0;
	int connected_displays = 0;

	if (isValid() == true) {
		menu->insertTitle(SmallIcon("kcmkwm"), i18n("Output Port"));

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for ON outputs
			if (!randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				continue;
			}

			output_name = output_info->name;
			//printf("ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Active)").arg(output_name));
			menu->setItemChecked(lastIndex, true);
			menu->connectItem(lastIndex, this, TQT_SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);

			connected_displays++;
		}

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for CONNECTED outputs....
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				continue;
			}
			// ...that are not ON
			if (randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}

			output_name = output_info->name;
			//printf("CONNECTED, NOT ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Connected, Inactive)").arg(output_name));
			menu->setItemChecked(lastIndex, false);
			menu->connectItem(lastIndex, this, TQT_SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);

			connected_displays++;
		}

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for ALL outputs that are not connected....
			if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
				continue;
			}
			// ...or ON
			if (randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}

			output_name = output_info->name;
			//printf("DISCONNECTED, NOT ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Disconnected, Inactive)").arg(output_name));
			menu->setItemChecked(lastIndex, false);
			menu->setItemEnabled(lastIndex, false);
			menu->connectItem(lastIndex, this, TQT_SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);
		}

		lastIndex = menu->insertItem(SmallIcon("forward"), i18n("Next available output"));
		if (connected_displays < 2) {
			menu->setItemEnabled(lastIndex, false);
		}
		menu->connectItem(lastIndex, this, TQT_SLOT(slotCycleDisplays()));
	}
}

void KRandRSystemTray::slotColorProfileChanged(int parameter)
{
	t_config->writeEntry("CurrentProfile", m_menu->text(parameter));
	applyIccConfiguration(m_menu->text(parameter), NULL);
}

void KRandRSystemTray::slotDisplayProfileChanged(int parameter)
{
	TQString profileName = m_menu->text(parameter);
	if (profileName == "<default>") {
		profileName = "";
	}
	TQPtrList<SingleScreenData> profileData = loadDisplayConfiguration(profileName, locateLocal("config", "/", true));
	applyDisplayConfiguration(profileData, TRUE, locateLocal("config", "/", true));
	destroyScreenInformationObject(profileData);
}

void KRandRSystemTray::slotOutputChanged(int parameter)
{
	char *output_name;
	int i;
	int num_outputs_on;

	num_outputs_on = 0;
	for (i = 0; i < randr_screen_info->n_output; i++) {
		// Look for ON outputs
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		num_outputs_on++;
	}

	if (!randr_screen_info->outputs[parameter]->cur_crtc) {
		//printf("Screen was off, turning it on...\n\r");

		randr_screen_info->cur_crtc = randr_screen_info->outputs[parameter]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[parameter];
		randr_screen_info->cur_output->auto_set = 1;
 		randr_screen_info->cur_output->off_set = 0;
		output_auto (randr_screen_info, randr_screen_info->cur_output);
		i=main_low_apply(randr_screen_info);

		if (!randr_screen_info->outputs[parameter]->cur_crtc) {
			output_name = randr_screen_info->outputs[parameter]->info->name;
 			KMessageBox::sorry(my_parent, i18n("<b>Unable to activate output %1</b><p>Either the output is not connected to a display,<br>or the display configuration is not detectable").arg(output_name), i18n("Output Unavailable"));
		}
	}
	else {
		if (num_outputs_on > 1) {
			//printf("Screen was on, turning it off...\n\r");
			randr_screen_info->cur_crtc = randr_screen_info->outputs[parameter]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[parameter];
			randr_screen_info->cur_output->auto_set = 0;
			randr_screen_info->cur_output->off_set = 1;
			output_off(randr_screen_info, randr_screen_info->cur_output);
			i=main_low_apply(randr_screen_info);

			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
		else {
			KMessageBox::sorry(my_parent, i18n("<b>You are attempting to deactivate the only active output</b><p>You must keep at least one display output active at all times!"), i18n("Invalid Operation Requested"));
		}
	}
}

void KRandRSystemTray::deviceChanged (TDEGenericDevice* device) {
	if (device->type() == TDEGenericDeviceType::Monitor) {
		KRandrPassivePopup::message(
		i18n("New display output options are available!"),
		i18n("A screen has been added, removed, or changed"), SmallIcon("window_fullscreen"),
		this, "ScreenChangeNotification");

		reloadDisplayConfiguration();
		applyHotplugRules(locateLocal("config", "/", true));
	}
}