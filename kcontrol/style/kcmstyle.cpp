/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tqcheckbox.h>
#include <kcombobox.h>
#include <tqlistbox.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqslider.h>
#include <tqstylefactory.h>
#include <tqtabwidget.h>
#include <tqvbox.h>
#include <tqfile.h>
#include <tqsettings.h>
#include <tqobjectlist.h>
#include <tqpixmapcache.h>
#include <tqwhatsthis.h>
#include <tqpushbutton.h>

#include <dcopclient.h>
#include <tdeapplication.h>
#include <tdeglobalsettings.h>
#include <kdebug.h>
#include <kipc.h>
#include <tdeaboutdata.h>
#include <kdialog.h>
#include <klibloader.h>
#include <tdelistview.h>
#include <tdemessagebox.h>
#include <ksimpleconfig.h>
#include <tdestyle.h>
#include <kstandarddirs.h>

#include "../krdb/krdb.h"

#include "kcmstyle.h"
#include "styleconfdialog.h"

#include <X11/Xlib.h>
// X11 namespace cleanup
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/

// Plugin Interface
// Danimo: Why do we use the old interface?!
extern "C"
{
    KDE_EXPORT TDECModule *create_style(TQWidget *parent, const char*)
    {
        TDEGlobal::locale()->insertCatalogue("kcmstyle");
        return new KCMStyle(parent, "kcmstyle");
    }

    KDE_EXPORT void init_style()
    {
        uint flags = KRdbExportQtSettings | KRdbExportQtColors | KRdbExportXftSettings;
        TDEConfig config("kcmdisplayrc", true /*readonly*/, false /*don't read kdeglobals etc.*/);
        config.setGroup("X11");

        // This key is written by the "colors" module.
        bool exportKDEColors = config.readBoolEntry("exportKDEColors", true);
        if (exportKDEColors)
            flags |= KRdbExportColors;
        runRdb( flags );

        // Write some Qt root property.
#ifndef __osf__      // this crashes under Tru64 randomly -- will fix later
        TQByteArray properties;
        TQDataStream d(properties, IO_WriteOnly);
        d.setVersion( 3 );      // Qt2 apps need this.
        d << kapp->palette() << TDEGlobalSettings::generalFont();
        Atom a = XInternAtom(tqt_xdisplay(), "_QT_DESKTOP_PROPERTIES", false);

        // do it for all root windows - multihead support
        int screen_count = ScreenCount(tqt_xdisplay());
        for (int i = 0; i < screen_count; i++)
            XChangeProperty(tqt_xdisplay(),  RootWindow(tqt_xdisplay(), i),
                            a, a, 8, PropModeReplace,
                            (unsigned char*) properties.data(), properties.size());
#endif
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, TQWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kcmstyle, GeneralFactory )
*/


KCMStyle::KCMStyle( TQWidget* parent, const char* name )
	: TDECModule( parent, name ), appliedStyle(NULL)
{
    setQuickHelp( i18n("<h1>Style</h1>"
			"This module allows you to modify the visual appearance "
			"of user interface elements, such as the widget style "
			"and effects."));

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	TDEGlobal::dirs()->addResourceType("themes",
		TDEStandardDirs::kde_default("data") + "kstyle/themes");

	TDEAboutData *about =
		new TDEAboutData( I18N_NOOP("kcmstyle"),
						I18N_NOOP("TDE Style Module"),
						0, 0, TDEAboutData::License_GPL,
						I18N_NOOP("(c) 2002 Karol Szwed, Daniel Molkentin"));

	about->addAuthor("Karol Szwed", 0, "gallium@kde.org");
	about->addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
	about->addAuthor("Ralf Nolden", 0, "nolden@kde.org");
	setAboutData( about );

	// Setup pages and mainLayout
	mainLayout = new TQVBoxLayout( this );
	tabWidget  = new TQTabWidget( this );
	mainLayout->addWidget( tabWidget );

	page1 = new TQWidget( tabWidget );
	page1Layout = new TQVBoxLayout( page1, KDialog::marginHint(), KDialog::spacingHint() );
	page2 = new TQWidget( tabWidget );
	page2Layout = new TQVBoxLayout( page2, KDialog::marginHint(), KDialog::spacingHint() );
	page3 = new TQWidget( tabWidget );
	page3Layout = new TQVBoxLayout( page3, KDialog::marginHint(), KDialog::spacingHint() );

	// Add Page1 (Style)
	// -----------------
	gbWidgetStyle = new TQGroupBox( i18n("Widget Style"), page1, "gbWidgetStyle" );
	gbWidgetStyle->setColumnLayout( 0, Qt::Vertical );
	gbWidgetStyle->layout()->setMargin( KDialog::marginHint() );
	gbWidgetStyle->layout()->setSpacing( KDialog::spacingHint() );

	gbWidgetStyleLayout = new TQVBoxLayout( gbWidgetStyle->layout() );
	gbWidgetStyleLayout->setAlignment( Qt::AlignTop );
	hbLayout = new TQHBoxLayout( KDialog::spacingHint(), "hbLayout" );

	cbStyle = new KComboBox( gbWidgetStyle, "cbStyle" );
	cbStyle->setEditable( FALSE );
	hbLayout->addWidget( cbStyle );

	pbConfigStyle = new TQPushButton( i18n("Con&figure..."), gbWidgetStyle );
	pbConfigStyle->setSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Minimum );
	pbConfigStyle->setEnabled( FALSE );
	hbLayout->addWidget( pbConfigStyle );

	gbWidgetStyleLayout->addLayout( hbLayout );

	lblStyleDesc = new TQLabel( gbWidgetStyle );
	lblStyleDesc->setTextFormat(TQt::RichText);
	gbWidgetStyleLayout->addWidget( lblStyleDesc );

	cbIconsOnButtons = new TQCheckBox( i18n("Sho&w icons on buttons"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbIconsOnButtons );
	cbScrollablePopupMenus = new TQCheckBox( i18n("Enable &scrolling in popup menus"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbScrollablePopupMenus );
	cbAutoHideAccelerators = new TQCheckBox( i18n("Hide &underlined characters in the menu bar when not in use"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbAutoHideAccelerators );
	cbMenuAltKeyNavigation = new TQCheckBox( i18n("&Pressing only the menu bar activator key selects the menu bar"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbMenuAltKeyNavigation );
	cbEnableTooltips = new TQCheckBox( i18n("E&nable tooltips"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbEnableTooltips );
	cbTearOffHandles = new TQCheckBox( i18n("Show tear-off handles in &popup menus"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbTearOffHandles );
	cbTearOffHandles->hide(); // reenable when the corresponding Qt method is virtual and properly reimplemented

	TQGroupBox *gbPreview = new TQGroupBox( i18n( "Preview" ), page1 );
	gbPreview->setColumnLayout( 0, Qt::Vertical );
	gbPreview->layout()->setMargin( 0 );
	gbPreview->layout()->setSpacing( KDialog::spacingHint() );
	gbPreview->setFlat( true );
	stylePreview = new StylePreview( gbPreview );
	gbPreview->layout()->add( stylePreview );

	page1Layout->addWidget( gbWidgetStyle );
	page1Layout->addWidget( gbPreview );

	// Connect all required stuff
	connect( cbStyle, TQT_SIGNAL(activated(int)), this, TQT_SLOT(styleChanged()) );
	connect( cbStyle, TQT_SIGNAL(activated(int)), this, TQT_SLOT(updateConfigButton()));
	connect( pbConfigStyle, TQT_SIGNAL(clicked()), this, TQT_SLOT(styleSpecificConfig()));

	// Add Page2 (Effects)
	// -------------------
	cbEnableEffects = new TQCheckBox( i18n("&Enable GUI effects"), page2 );
	containerFrame = new TQFrame( page2 );
	containerFrame->setFrameStyle( TQFrame::NoFrame | TQFrame::Plain );
	containerFrame->setMargin(0);
	containerLayout = new TQGridLayout( containerFrame, 1, 1,	// rows, columns
		KDialog::marginHint(), KDialog::spacingHint() );

	comboComboEffect = new TQComboBox( FALSE, containerFrame );
	comboComboEffect->insertItem( i18n("Disable") );
	comboComboEffect->insertItem( i18n("Animate") );
	lblComboEffect = new TQLabel( i18n("Combobo&x effect:"), containerFrame );
	lblComboEffect->setBuddy( comboComboEffect );
	containerLayout->addWidget( lblComboEffect, 0, 0 );
	containerLayout->addWidget( comboComboEffect, 0, 1 );

	comboTooltipEffect = new TQComboBox( FALSE, containerFrame );
	comboTooltipEffect->insertItem( i18n("Disable") );
	comboTooltipEffect->insertItem( i18n("Animate") );
	comboTooltipEffect->insertItem( i18n("Fade") );
	lblTooltipEffect = new TQLabel( i18n("&Tool tip effect:"), containerFrame );
	lblTooltipEffect->setBuddy( comboTooltipEffect );
	containerLayout->addWidget( lblTooltipEffect, 1, 0 );
	containerLayout->addWidget( comboTooltipEffect, 1, 1 );

	comboRubberbandEffect = new TQComboBox( FALSE, containerFrame );
	comboRubberbandEffect->insertItem( i18n("Disable") );
	comboRubberbandEffect->insertItem( i18n("Make translucent") );
	lblRubberbandEffect = new TQLabel( i18n("&Rubberband effect:"), containerFrame );
	lblRubberbandEffect->setBuddy( comboRubberbandEffect );
	containerLayout->addWidget( lblRubberbandEffect, 2, 0 );
	containerLayout->addWidget( comboRubberbandEffect, 2, 1 );
	
	comboMenuEffect = new TQComboBox( FALSE, containerFrame );
	comboMenuEffect->insertItem( i18n("Disable") );
	comboMenuEffect->insertItem( i18n("Animate") );
	comboMenuEffect->insertItem( i18n("Fade") );
	comboMenuEffect->insertItem( i18n("Make Translucent") );
	lblMenuEffect = new TQLabel( i18n("&Menu effect:"), containerFrame );
	lblMenuEffect->setBuddy( comboMenuEffect );
	containerLayout->addWidget( lblMenuEffect, 3, 0 );
	containerLayout->addWidget( comboMenuEffect, 3, 1 );

	comboMenuHandle = new TQComboBox( FALSE, containerFrame );
	comboMenuHandle->insertItem( i18n("Disable") );
	comboMenuHandle->insertItem( i18n("Application Level") );
//	comboMenuHandle->insertItem( i18n("Enable") );
	lblMenuHandle = new TQLabel( i18n("Me&nu tear-off handles:"), containerFrame );
	lblMenuHandle->setBuddy( comboMenuHandle );
	containerLayout->addWidget( lblMenuHandle, 4, 0 );
	containerLayout->addWidget( comboMenuHandle, 4, 1 );

	cbMenuShadow = new TQCheckBox( i18n("Menu &drop shadow"), containerFrame );
	containerLayout->addWidget( cbMenuShadow, 5, 0 );

	// Push the [label combo] to the left.
	comboSpacer = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
	containerLayout->addItem( comboSpacer, 1, 2 );

	// Separator.
	TQFrame* hline = new TQFrame ( page2 );
	hline->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );

	// Now implement the Menu Transparency container.
	menuContainer = new TQFrame( page2 );
	menuContainer->setFrameStyle( TQFrame::NoFrame | TQFrame::Plain );
	menuContainer->setMargin(0);
	menuContainerLayout = new TQGridLayout( menuContainer, 1, 1,    // rows, columns
		KDialog::marginHint(), KDialog::spacingHint() );

	menuPreview = new MenuPreview( menuContainer, /* opacity */ 90, MenuPreview::Blend );

	comboMenuEffectType = new TQComboBox( FALSE, menuContainer );
	comboMenuEffectType->insertItem( i18n("Software Tint") );
	comboMenuEffectType->insertItem( i18n("Software Blend") );
#ifdef HAVE_XRENDER
	comboMenuEffectType->insertItem( i18n("XRender Blend") );
#endif

	// So much stuffing around for a simple slider..
	sliderBox = new TQVBox( menuContainer );
	sliderBox->setSpacing( KDialog::spacingHint() );
	sliderBox->setMargin( 0 );
	slOpacity = new TQSlider( 0, 100, 5, /*opacity*/ 90, Qt::Horizontal, sliderBox );
	slOpacity->setTickmarks( TQSlider::Below );
	slOpacity->setTickInterval( 10 );
	TQHBox* box1 = new TQHBox( sliderBox );
	box1->setSpacing( KDialog::spacingHint() );
	box1->setMargin( 0 );
	TQLabel* lbl = new TQLabel( i18n("0%"), box1 );
	lbl->setAlignment( AlignLeft );
	lbl = new TQLabel( i18n("50%"), box1 );
	lbl->setAlignment( AlignHCenter );
	lbl = new TQLabel( i18n("100%"), box1 );
	lbl->setAlignment( AlignRight );

	lblMenuEffectType = new TQLabel( comboMenuEffectType, i18n("Menu trans&lucency type:"), menuContainer );
	lblMenuEffectType->setAlignment( AlignBottom | AlignLeft );
	lblMenuOpacity    = new TQLabel( slOpacity, i18n("Menu &opacity:"), menuContainer );
	lblMenuOpacity->setAlignment( AlignBottom | AlignLeft );

	menuContainerLayout->addWidget( lblMenuEffectType, 0, 0 );
	menuContainerLayout->addWidget( comboMenuEffectType, 1, 0 );
	menuContainerLayout->addWidget( lblMenuOpacity, 2, 0 );
	menuContainerLayout->addWidget( sliderBox, 3, 0 );
	menuContainerLayout->addMultiCellWidget( menuPreview, 0, 3, 1, 1 );

	// Layout page2.
	page2Layout->addWidget( cbEnableEffects );
	page2Layout->addWidget( containerFrame );
	page2Layout->addWidget( hline );
	page2Layout->addWidget( menuContainer );

	TQSpacerItem* sp1 = new TQSpacerItem( 20, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
	page2Layout->addItem( sp1 );

	// Data flow stuff.
	connect( cbEnableEffects,     TQT_SIGNAL(toggled(bool)), containerFrame, TQT_SLOT(setEnabled(bool)) );
	connect( cbEnableEffects,     TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(menuEffectChanged(bool)) );
	connect( slOpacity,           TQT_SIGNAL(valueChanged(int)),menuPreview, TQT_SLOT(setOpacity(int)) );
	connect( comboMenuEffect,     TQT_SIGNAL(activated(int)), this, TQT_SLOT(menuEffectChanged()) );
	connect( comboMenuEffect,     TQT_SIGNAL(highlighted(int)), this, TQT_SLOT(menuEffectChanged()) );
	connect( comboMenuEffectType, TQT_SIGNAL(activated(int)), this, TQT_SLOT(menuEffectTypeChanged()) );
	connect( comboMenuEffectType, TQT_SIGNAL(highlighted(int)), this, TQT_SLOT(menuEffectTypeChanged()) );

	// Add Page3 (Miscellaneous)
	// -------------------------
	cbHoverButtons = new TQCheckBox( i18n("High&light buttons under mouse"), page3 );
	cbTransparentToolbars = new TQCheckBox( i18n("Transparent tool&bars when moving"), page3 );

	TQWidget * dummy = new TQWidget( page3 );
	
	TQHBoxLayout* box2 = new TQHBoxLayout( dummy, 0, KDialog::spacingHint() );
	lbl = new TQLabel( i18n("Text pos&ition:"), dummy );
	comboToolbarIcons = new TQComboBox( FALSE, dummy );
	comboToolbarIcons->insertItem( i18n("Icons Only") );
	comboToolbarIcons->insertItem( i18n("Text Only") );
	comboToolbarIcons->insertItem( i18n("Text Alongside Icons") );
	comboToolbarIcons->insertItem( i18n("Text Under Icons") );
	lbl->setBuddy( comboToolbarIcons );

	box2->addWidget( lbl );
	box2->addWidget( comboToolbarIcons );
	TQSpacerItem* sp2 = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
	box2->addItem( sp2 );
	
	page3Layout->addWidget( cbHoverButtons );
	page3Layout->addWidget( cbTransparentToolbars );
	page3Layout->addWidget( dummy );
	
	// Layout page3.
	TQSpacerItem* sp3 = new TQSpacerItem( 20, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
	page3Layout->addItem( sp3 );

	// Load settings
	load();

	// Do all the setDirty connections.
	connect(cbStyle, TQT_SIGNAL(activated(int)), this, TQT_SLOT(setStyleDirty()));
	// Page2
	connect( cbEnableEffects,     TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbEnableEffects,     TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setStyleDirty()));
	connect( comboTooltipEffect,  TQT_SIGNAL(activated(int)), this, TQT_SLOT(setEffectsDirty()));
	connect( comboRubberbandEffect, TQT_SIGNAL(activated(int)),   this, TQT_SLOT(setStyleDirty()));
	connect( comboComboEffect,    TQT_SIGNAL(activated(int)), this, TQT_SLOT(setEffectsDirty()));
	connect( comboMenuEffect,     TQT_SIGNAL(activated(int)), this, TQT_SLOT(setStyleDirty()));
	connect( comboMenuHandle,     TQT_SIGNAL(activated(int)), this, TQT_SLOT(setStyleDirty()));
	connect( comboMenuEffectType, TQT_SIGNAL(activated(int)), this, TQT_SLOT(setStyleDirty()));
	connect( slOpacity,           TQT_SIGNAL(valueChanged(int)),this, TQT_SLOT(setStyleDirty()));
	connect( cbMenuShadow,        TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setStyleDirty()));
	// Page3
	connect( cbHoverButtons,        TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setToolbarsDirty()));
	connect( cbTransparentToolbars, TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setToolbarsDirty()));
	connect( cbEnableTooltips,      TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbIconsOnButtons,      TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbScrollablePopupMenus,TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbAutoHideAccelerators,TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbMenuAltKeyNavigation,TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( cbTearOffHandles,      TQT_SIGNAL(toggled(bool)),    this, TQT_SLOT(setEffectsDirty()));
	connect( comboToolbarIcons,     TQT_SIGNAL(activated(int)), this, TQT_SLOT(setToolbarsDirty()));

	addWhatsThis();

	// Insert the pages into the tabWidget
	tabWidget->insertTab( page1, i18n("&Style"));
	tabWidget->insertTab( page2, i18n("&Effects"));
	tabWidget->insertTab( page3, i18n("&Toolbar"));

	//Enable/disable the button for the initial style
	updateConfigButton();
}


KCMStyle::~KCMStyle()
{
	delete appliedStyle;
}

void KCMStyle::updateConfigButton()
{
	if (!styleEntries[currentStyle()] || styleEntries[currentStyle()]->configPage.isEmpty()) {
		pbConfigStyle->setEnabled(false);
		return;
	}

	// We don't check whether it's loadable here -
	// lets us report an error and not waste time
	// loading things if the user doesn't click the button
	pbConfigStyle->setEnabled( true );
}

void KCMStyle::styleSpecificConfig()
{
	TQString libname = styleEntries[currentStyle()]->configPage;

	// Use KLibLoader to get the library, handling
	// any errors that arise
	KLibLoader* loader = KLibLoader::self();

	KLibrary* library = loader->library( TQFile::encodeName(libname) );
	if (!library)
	{
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
			loader->lastErrorMessage(),
			i18n("Unable to Load Dialog"));
		return;
	}

	void* allocPtr = library->symbol("allocate_kstyle_config");

	if (!allocPtr)
	{
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
			loader->lastErrorMessage(),
			i18n("Unable to Load Dialog"));
		return;
	}

	//Create the container dialog
	StyleConfigDialog* dial = new StyleConfigDialog(this, styleEntries[currentStyle()]->name);
	dial->enableButtonSeparator(true);

	typedef TQWidget*(* factoryRoutine)( TQWidget* parent );

	//Get the factory, and make the widget.
	factoryRoutine factory      = (factoryRoutine)(allocPtr); //Grmbl. So here I am on my
	//"never use C casts" moralizing streak, and I find that one can't go void* -> function ptr
	//even with a reinterpret_cast.

	TQWidget*       pluginConfig = factory( dial );

	//Insert it in...
	dial->setMainWidget( pluginConfig );

	//..and connect it to the wrapper
	connect(pluginConfig, TQT_SIGNAL(changed(bool)), dial, TQT_SLOT(setDirty(bool)));
	connect(dial, TQT_SIGNAL(defaults()), pluginConfig, TQT_SLOT(defaults()));
	connect(dial, TQT_SIGNAL(save()), pluginConfig, TQT_SLOT(save()));

	if (dial->exec() == TQDialog::Accepted  && dial->isDirty() ) {
		// Force re-rendering of the preview, to apply settings
		switchStyle(currentStyle(), true);

		//For now, ask all TDE apps to recreate their styles to apply the setitngs
		KIPC::sendMessageAll(KIPC::StyleChanged);

		// We call setStyleDirty here to make sure we force style re-creation
		setStyleDirty();
	}

	delete dial;
}

void KCMStyle::load()
{
   load( false );
}

void KCMStyle::load(bool useDefaults)
{
	TDEConfig config( "kdeglobals", true, false );

	config.setReadDefaults( useDefaults );

	// Page1 - Build up the Style ListBox
	loadStyle( config );

	// Page2 - Effects
	loadEffects( config );

	// Page3 - Misc.
	loadMisc( config );

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	emit changed( useDefaults );
}


void KCMStyle::save()
{
	// Don't do anything if we don't need to.
	if ( !(m_bToolbarsDirty | m_bEffectsDirty | m_bStyleDirty ) )
		return;

	bool allowMenuTransparency = false;
	bool allowMenuDropShadow   = false;

	// Read the TDEStyle flags to see if the style writer
	// has enabled menu translucency in the style.
	if (appliedStyle && appliedStyle->inherits("TDEStyle"))
	{
		allowMenuDropShadow = true;
		TDEStyle* style = dynamic_cast<TDEStyle*>(appliedStyle);
		if (style) {
			TDEStyle::TDEStyleFlags flags = style->styleFlags();
			if (flags & TDEStyle::AllowMenuTransparency)
				allowMenuTransparency = true;
		}
	}

	TQString warn_string( i18n("<qt>Selected style: <b>%1</b><br><br>"
		"One or more effects that you have chosen could not be applied because the selected "
		"style does not support them; they have therefore been disabled.<br>"
		"<br>" ).arg( cbStyle->currentText()) );
	bool show_warning = false;

	// Warn the user if they're applying a style that doesn't support
	// menu translucency and they enabled it.
    if ( (!allowMenuTransparency) &&
		(cbEnableEffects->isChecked()) &&
		(comboMenuEffect->currentItem() == 3) )	// Make Translucent
    {
		warn_string += i18n("Menu translucency is not available.<br>");
		comboMenuEffect->setCurrentItem(0);    // Disable menu effect.
		show_warning = true;
	}

	if (!allowMenuDropShadow && cbMenuShadow->isChecked())
	{
		warn_string += i18n("Menu drop-shadows are not available.");
		cbMenuShadow->setChecked(false);
		show_warning = true;
	}

	// Tell the user what features we could not apply on their behalf.
	if (show_warning)
		KMessageBox::information(this, warn_string);


	// Save effects.
	TDEConfig config( "kdeglobals" );
	config.setGroup("KDE");

	config.writeEntry( "EffectsEnabled", cbEnableEffects->isChecked());
	int item = comboComboEffect->currentItem();
	config.writeEntry( "EffectAnimateCombo", item == 1 );
	item = comboTooltipEffect->currentItem();
	config.writeEntry( "EffectAnimateTooltip", item == 1);
	config.writeEntry( "EffectFadeTooltip", item == 2 );
	item = comboRubberbandEffect->currentItem();
	{
		TQSettings settings;	// Only for TDEStyle stuff
		settings.writeEntry("/TDEStyle/Settings/SemiTransparentRubberband", item == 1);
	}
	item = comboMenuHandle->currentItem();
	config.writeEntry( "InsertTearOffHandle", item );
	item = comboMenuEffect->currentItem();
	config.writeEntry( "EffectAnimateMenu", item == 1 );
	config.writeEntry( "EffectFadeMenu", item == 2 );

	// Handle TDEStyle's menu effects
	TQString engine("Disabled");
	if (item == 3 && cbEnableEffects->isChecked())	// Make Translucent
		switch( comboMenuEffectType->currentItem())
		{
			case 1: engine = "SoftwareBlend"; break;
			case 2: engine = "XRender"; break;
			default:
			case 0: engine = "SoftwareTint"; break;
		}

	{	// Braces force a TQSettings::sync()
		TQSettings settings;	// Only for TDEStyle stuff
		settings.writeEntry("/TDEStyle/Settings/MenuTransparencyEngine", engine);
		settings.writeEntry("/TDEStyle/Settings/MenuOpacity", slOpacity->value()/100.0);
 		settings.writeEntry("/TDEStyle/Settings/MenuDropShadow",
					   		cbEnableEffects->isChecked() && cbMenuShadow->isChecked() );
	}

	// Misc page
	config.writeEntry( "ShowIconsOnPushButtons", cbIconsOnButtons->isChecked(), true, true );
	{       // Braces force a TQSettings::sync()
		TQSettings settings;    // Only for TDEStyle stuff
		settings.writeEntry("/TDEStyle/Settings/ScrollablePopupMenus", cbScrollablePopupMenus->isChecked() );
		settings.writeEntry("/TDEStyle/Settings/AutoHideAccelerators", cbAutoHideAccelerators->isChecked() );
		settings.writeEntry("/TDEStyle/Settings/MenuAltKeyNavigation", cbMenuAltKeyNavigation->isChecked() );
	}
	config.writeEntry( "EffectNoTooltip", !cbEnableTooltips->isChecked(), true, true );

	config.setGroup("General");
	config.writeEntry( "widgetStyle", currentStyle() );

	config.setGroup("Toolbar style");
	config.writeEntry( "Highlighting", cbHoverButtons->isChecked(), true, true );
	config.writeEntry( "TransparentMoving", cbTransparentToolbars->isChecked(), true, true );
	TQString tbIcon;
	switch( comboToolbarIcons->currentItem() )
	{
		case 1: tbIcon = "TextOnly"; break;
		case 2: tbIcon = "IconTextRight"; break;
		case 3: tbIcon = "IconTextBottom"; break;
		case 0:
		default: tbIcon = "IconOnly"; break;
	}
	config.writeEntry( "IconText", tbIcon, true, true );
	config.sync();

	// Export the changes we made to qtrc, and update all qt-only
	// applications on the fly, ensuring that we still follow the user's
	// export fonts/colors settings.
	if (m_bStyleDirty | m_bEffectsDirty)	// Export only if necessary
	{
		uint flags = KRdbExportQtSettings;
		TDEConfig tdeconfig("kcmdisplayrc", true /*readonly*/, false /*no globals*/);
		tdeconfig.setGroup("X11");
		bool exportKDEColors = tdeconfig.readBoolEntry("exportKDEColors", true);
		if (exportKDEColors)
			flags |= KRdbExportColors;
		runRdb( flags );
	}

	// Now allow TDE apps to reconfigure themselves.
	if ( m_bStyleDirty )
		KIPC::sendMessageAll(KIPC::StyleChanged);

	if ( m_bToolbarsDirty )
		// ##### FIXME - Doesn't apply all settings correctly due to bugs in
		// TDEApplication/TDEToolbar
		KIPC::sendMessageAll(KIPC::ToolbarStyleChanged);

	if (m_bEffectsDirty) {
		KIPC::sendMessageAll(KIPC::SettingsChanged);
		kapp->dcopClient()->send("twin*", "", "reconfigure()", TQString(""));
	}
        //update kicker to re-used tooltips kicker parameter otherwise, it overwritted
        //by style tooltips parameters.
        TQByteArray data;
        kapp->dcopClient()->send( "kicker", "kicker", "configure()", data );

	// Clean up
	m_bEffectsDirty  = false;
	m_bToolbarsDirty = false;
	m_bStyleDirty    = false;
	emit changed( false );
}


bool KCMStyle::findStyle( const TQString& str, int& combobox_item )
{
	StyleEntry* se   = styleEntries.find(str.lower());

	TQString     name = se ? se->name : str;

	combobox_item = 0;

	//look up name
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		if ( cbStyle->text(i) == name )
		{
			combobox_item = i;
			return TRUE;
		}
	}

	return FALSE;
}


void KCMStyle::defaults()
{
   load( true );
}

void KCMStyle::setEffectsDirty()
{
	m_bEffectsDirty = true;
	emit changed(true);
}

void KCMStyle::setToolbarsDirty()
{
	m_bToolbarsDirty = true;
	emit changed(true);
}

void KCMStyle::setStyleDirty()
{
	m_bStyleDirty = true;
	emit changed(true);
}

// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::loadStyle( TDEConfig& config )
{
	cbStyle->clear();

	// Create a dictionary of WidgetStyle to Name and Desc. mappings,
	// as well as the config page info
	styleEntries.clear();
	styleEntries.setAutoDelete(true);

	TQString strWidgetStyle;
	TQStringList list = TDEGlobal::dirs()->findAllResources("themes", "*.themerc", true, true);
	for (TQStringList::iterator it = list.begin(); it != list.end(); ++it)
	{
		KSimpleConfig config( *it, true );
		if ( !(config.hasGroup("KDE") && config.hasGroup("Misc")) )
			continue;

		config.setGroup("KDE");

		strWidgetStyle = config.readEntry("WidgetStyle");
		if (strWidgetStyle.isNull())
			continue;

		// We have a widgetstyle, so lets read the i18n entries for it...
		StyleEntry* entry = new StyleEntry;
		config.setGroup("Misc");
		entry->name = config.readEntry("Name");
		entry->desc = config.readEntry("Comment", i18n("No description available."));
		entry->configPage = config.readEntry("ConfigPage", TQString::null);

		// Check if this style should be shown
		config.setGroup("Desktop Entry");
		entry->hidden = config.readBoolEntry("Hidden", false);

		// Insert the entry into our dictionary.
		styleEntries.insert(strWidgetStyle.lower(), entry);
	}

	// Obtain all style names
	TQStringList allStyles = TQStyleFactory::keys();

	// Get translated names, remove all hidden style entries.
	TQStringList styles;
	StyleEntry* entry;
	for (TQStringList::iterator it = allStyles.begin(); it != allStyles.end(); it++)
	{
		TQString id = (*it).lower();
		// Find the entry.
		if ( (entry = styleEntries.find(id)) != 0 )
		{
			// Do not add hidden entries
			if (entry->hidden)
				continue;

			styles += entry->name;

			nameToStyleKey[entry->name] = id;
		}
		else
		{
			styles += (*it); //Fall back to the key (but in original case)
			nameToStyleKey[*it] = id;
		}
	}

	// Sort the style list, and add it to the combobox
	styles.sort();
	cbStyle->insertStringList( styles );

	// Find out which style is currently being used
	config.setGroup( "General" );
	TQString defaultStyle = TDEStyle::defaultStyle();
	TQString cfgStyle = config.readEntry( "widgetStyle", defaultStyle );

	// Select the current style
	// Do not use cbStyle->listBox() as this may be NULL for some styles when
	// they use QPopupMenus for the drop-down list!

	// ##### Since Trolltech likes to seemingly copy & paste code,
	// TQStringList::findItem() doesn't have a Qt::StringComparisonMode field.
	// We roll our own (yuck)
	cfgStyle = cfgStyle.lower();
	int item = 0;
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		TQString id = nameToStyleKey[cbStyle->text(i)];
		item = i;
		if ( id == cfgStyle )	// ExactMatch
			break;
		else if ( id.contains( cfgStyle ) )
			break;
		else if ( id.contains( TQApplication::style().className() ) )
			break;
		item = 0;
	}
	cbStyle->setCurrentItem( item );

	m_bStyleDirty = false;

	switchStyle( currentStyle() );	// make resets visible
}

TQString KCMStyle::currentStyle()
{
	return nameToStyleKey[cbStyle->currentText()];
}


void KCMStyle::styleChanged()
{
	switchStyle( currentStyle() );
}


void KCMStyle::switchStyle(const TQString& styleName, bool force)
{
	// Don't flicker the preview if the same style is chosen in the cb
	if (!force && appliedStyle && TQT_TQOBJECT(appliedStyle)->name() == styleName) 
		return;
         
	// Create an instance of the new style...
	TQStyle* style = TQStyleFactory::create(styleName);
	if (!style)
		return;

	// Prevent Qt from wrongly caching radio button images
	TQPixmapCache::clear();

	setStyleRecursive( stylePreview, style );

	// this flickers, but reliably draws the widgets correctly.
	stylePreview->resize( stylePreview->sizeHint() );

	delete appliedStyle;
	appliedStyle = style;

	// Set the correct style description
	StyleEntry* entry = styleEntries.find( styleName );
	TQString desc;
	desc = i18n("Description: %1").arg( entry ? entry->desc : i18n("No description available.") );
	lblStyleDesc->setText( desc );
}

void KCMStyle::setStyleRecursive(TQWidget* w, TQStyle* s)
{
	// Don't let broken styles kill the palette
	// for other styles being previewed. (e.g SGI style)
	w->unsetPalette();

	TQPalette newPalette(TDEApplication::createApplicationPalette());
	s->polish( newPalette );
	w->setPalette(newPalette);

	// Apply the new style.
	w->setStyle(s);

	// Recursively update all children.
	const TQObjectList children = w->childrenListObject();
	if (children.isEmpty())
		return;

	// Apply the style to each child widget.
	TQPtrListIterator<TQObject> childit(children);
	TQObject *child;
	while ((child = childit.current()) != 0)
	{
		++childit;
		if (child->isWidgetType())
			setStyleRecursive((TQWidget *) child, s);
	}
}


// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------

void KCMStyle::loadEffects( TDEConfig& config )
{
	// Load effects.
	config.setGroup("KDE");

	cbEnableEffects->setChecked( config.readBoolEntry( "EffectsEnabled", false) );

	if ( config.readBoolEntry( "EffectAnimateCombo", false) )
		comboComboEffect->setCurrentItem( 1 );
	else
		comboComboEffect->setCurrentItem( 0 );

	if ( config.readBoolEntry( "EffectAnimateTooltip", false) )
		comboTooltipEffect->setCurrentItem( 1 );
	else if ( config.readBoolEntry( "EffectFadeTooltip", false) )
		comboTooltipEffect->setCurrentItem( 2 );
	else
		comboTooltipEffect->setCurrentItem( 0 );
		
	TQSettings settings;
	bool semiTransparentRubberband = settings.readBoolEntry("/TDEStyle/Settings/SemiTransparentRubberband", false);
	comboRubberbandEffect->setCurrentItem( semiTransparentRubberband ? 1 : 0 );
	
	if ( config.readBoolEntry( "EffectAnimateMenu", false) )
		comboMenuEffect->setCurrentItem( 1 );
	else if ( config.readBoolEntry( "EffectFadeMenu", false) )
		comboMenuEffect->setCurrentItem( 2 );
	else
		comboMenuEffect->setCurrentItem( 0 );

	comboMenuHandle->setCurrentItem(config.readNumEntry("InsertTearOffHandle", 0));

	// TDEStyle Menu transparency and drop-shadow options...
	
	TQString effectEngine = settings.readEntry("/TDEStyle/Settings/MenuTransparencyEngine", "Disabled");

#ifdef HAVE_XRENDER
	if (effectEngine == "XRender") {
		comboMenuEffectType->setCurrentItem(2);
		comboMenuEffect->setCurrentItem(3);
	} else if (effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentItem(1);
		comboMenuEffect->setCurrentItem(3);
#else
	if (effectEngine == "XRender" || effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentItem(1);	// Software Blend
		comboMenuEffect->setCurrentItem(3);
#endif
	} else if (effectEngine == "SoftwareTint") {
		comboMenuEffectType->setCurrentItem(0);
		comboMenuEffect->setCurrentItem(3);
	} else
		comboMenuEffectType->setCurrentItem(0);

	if (comboMenuEffect->currentItem() != 3)	// If not translucency...
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else if (comboMenuEffectType->currentItem() == 0)
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else
		menuPreview->setPreviewMode( MenuPreview::Blend );

	slOpacity->setValue( (int)(100 * settings.readDoubleEntry("/TDEStyle/Settings/MenuOpacity", 0.90)) );

	// Menu Drop-shadows...
	cbMenuShadow->setChecked( settings.readBoolEntry("/TDEStyle/Settings/MenuDropShadow", false) );

	if (cbEnableEffects->isChecked()) {
		containerFrame->setEnabled( true );
		menuContainer->setEnabled( comboMenuEffect->currentItem() == 3 );
	} else {
		menuContainer->setEnabled( false );
		containerFrame->setEnabled( false );
	}

	m_bEffectsDirty = false;
}


void KCMStyle::menuEffectTypeChanged()
{
	MenuPreview::PreviewMode mode;

	if (comboMenuEffect->currentItem() != 3)
		mode = MenuPreview::Tint;
	else if (comboMenuEffectType->currentItem() == 0)
		mode = MenuPreview::Tint;
	else
		mode = MenuPreview::Blend;

	menuPreview->setPreviewMode(mode);

	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged()
{
	menuEffectChanged( cbEnableEffects->isChecked() );
	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged( bool enabled )
{
	if (enabled &&
		comboMenuEffect->currentItem() == 3) {
		menuContainer->setEnabled(true);
	} else
		menuContainer->setEnabled(false);
	m_bEffectsDirty = true;
}


// ----------------------------------------------------------------
// All the Miscellaneous stuff
// ----------------------------------------------------------------

void KCMStyle::loadMisc( TDEConfig& config )
{
	// TDE's Part via TDEConfig
	config.setGroup("Toolbar style");
	cbHoverButtons->setChecked(config.readBoolEntry("Highlighting", true));
	cbTransparentToolbars->setChecked(config.readBoolEntry("TransparentMoving", true));

	TQString tbIcon = config.readEntry("IconText", "IconOnly");
	if (tbIcon == "TextOnly")
		comboToolbarIcons->setCurrentItem(1);
	else if (tbIcon == "IconTextRight")
		comboToolbarIcons->setCurrentItem(2);
	else if (tbIcon == "IconTextBottom")
		comboToolbarIcons->setCurrentItem(3);
	else
		comboToolbarIcons->setCurrentItem(0);

	config.setGroup("KDE");
	cbIconsOnButtons->setChecked(config.readBoolEntry("ShowIconsOnPushButtons", false));
	cbEnableTooltips->setChecked(!config.readBoolEntry("EffectNoTooltip", false));
	cbTearOffHandles->setChecked(config.readBoolEntry("InsertTearOffHandle", false));

	TQSettings settings;
	cbScrollablePopupMenus->setChecked(settings.readBoolEntry("/TDEStyle/Settings/ScrollablePopupMenus", false));
	cbAutoHideAccelerators->setChecked(settings.readBoolEntry("/TDEStyle/Settings/AutoHideAccelerators", false));
	cbMenuAltKeyNavigation->setChecked(settings.readBoolEntry("/TDEStyle/Settings/MenuAltKeyNavigation", true));

	m_bToolbarsDirty = false;
}

void KCMStyle::addWhatsThis()
{
	// Page1
	TQWhatsThis::add( cbStyle, i18n("Here you can choose from a list of"
							" predefined widget styles (e.g. the way buttons are drawn) which"
							" may or may not be combined with a theme (additional information"
							" like a marble texture or a gradient).") );
	TQWhatsThis::add( stylePreview, i18n("This area shows a preview of the currently selected style "
							"without having to apply it to the whole desktop.") );

	// Page2
	TQWhatsThis::add( page2, i18n("This page allows you to enable various widget style effects. "
							"For best performance, it is advisable to disable all effects.") );
	TQWhatsThis::add( cbEnableEffects, i18n( "If you check this box, you can select several effects "
							"for different widgets like combo boxes, menus or tooltips.") );
	TQWhatsThis::add( comboComboEffect, i18n( "<p><b>Disable: </b>do not use any combo box effects.</p>\n"
							"<b>Animate: </b>Do some animation.") );
	TQWhatsThis::add( comboTooltipEffect, i18n( "<p><b>Disable: </b>do not use any tooltip effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<b>Fade: </b>Fade in tooltips using alpha-blending.") );
	TQWhatsThis::add( comboRubberbandEffect, i18n( "<p><b>Disable: </b>do not use any rubberband effects.</p>\n"
							"<b>Make Translucent: </b>Draw a translucent rubberband.") );
	TQWhatsThis::add( comboMenuEffect, i18n( "<p><b>Disable: </b>do not use any menu effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<p><b>Fade: </b>Fade in menus using alpha-blending.</p>\n"
							"<b>Make Translucent: </b>Alpha-blend menus for a see-through effect. (TDE styles only)") );
	TQWhatsThis::add( cbMenuShadow, i18n( "When enabled, all popup menus will have a drop-shadow, otherwise "
							"drop-shadows will not be displayed. At present, only TDE styles can have this "
							"effect enabled.") );
	TQWhatsThis::add( comboMenuEffectType, i18n( "<p><b>Software Tint: </b>Alpha-blend using a flat color.</p>\n"
							"<p><b>Software Blend: </b>Alpha-blend using an image.</p>\n"
							"<b>XRender Blend: </b>Use the XFree RENDER extension for image blending (if available). "
							"This method may be slower than the Software routines on non-accelerated displays, "
							"but may however improve performance on remote displays.</p>\n") );
	TQWhatsThis::add( slOpacity, i18n("By adjusting this slider you can control the menu effect opacity.") );

	// Page3
	TQWhatsThis::add( page3, i18n("<b>Note:</b> that all widgets in this combobox "
							"do not apply to Qt-only applications.") );
	TQWhatsThis::add( cbHoverButtons, i18n("If this option is selected, toolbar buttons will change "
							"their color when the mouse cursor is moved over them." ) );
	TQWhatsThis::add( cbTransparentToolbars, i18n("If you check this box, the toolbars will be "
							"transparent when moving them around.") );
	TQWhatsThis::add( cbEnableTooltips, i18n( "If you check this option, the TDE application "
							"will offer tooltips when the cursor remains over items in the toolbar." ) );
	TQWhatsThis::add( comboToolbarIcons, i18n( "<p><b>Icons only:</b> Shows only icons on toolbar buttons. "
							"Best option for low resolutions.</p>"
							"<p><b>Text only: </b>Shows only text on toolbar buttons.</p>"
							"<p><b>Text alongside icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned alongside the icon.</p>"
							"<b>Text under icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned below the icon.") );
	TQWhatsThis::add( cbIconsOnButtons, i18n( "If you enable this option, TDE Applications will "
							"show small icons alongside some important buttons.") );
	TQWhatsThis::add( cbScrollablePopupMenus, i18n( "If you enable this option, pop-up menus will scroll if vertical space is exhausted." ) );
	TQWhatsThis::add( cbAutoHideAccelerators, i18n( "Program drop-down menus can be used with either the mouse or "
							"keyboard. Each menu item on the menu bar that can be activated from the keyboard contains one "
							"character that is underlined. When the underlined character key is pressed concurrently with the "
							"activator key (usually Alt), the keyboard combination opens the menu or selects that menu item. The "
							"underlines can remain hidden until the activator key is pressed or remain visible at all times. "
							"Enabling this option hides the underlines until pressing the activator key. Note: some widget styles "
							"do not support this feature." ) );
	TQWhatsThis::add( cbMenuAltKeyNavigation, i18n( "When using the keyboard, program drop-down menus can be "
							"activated in one of two ways. Concurrently press the activator key (usually Alt) and the underlined "
							"character that is part of the menu name, or sequentially press and release the activator key and then "
							"press the underlined character. Enabling this option selects the latter method. The method of "
							"concurrently pressing both keys is supported in both Trinity and non Trinity programs. The choice "
							"of using either method applies to Trinity Programs only and not to non Trinity programs. Regardless "
							"of which option is preferred, after a desired menu opens, pressing only the respective underlined "
							"key of any menu item is required to select that menu item.") );
	TQWhatsThis::add( cbTearOffHandles, i18n( "If you enable this option some pop-up menus will "
							"show so called tear-off handles. If you click them, you get the menu "
							"inside a widget. This can be very helpful when performing "
							"the same action multiple times.") );
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
