// Melchior FRANZ  <mfranz@kde.org>	-- 2001-04-22

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <layout.h>
#include <tqwhatsthis.h>
#include "config.h"


extern "C"
{
	KDE_EXPORT TQObject* allocate_config(KConfig* conf, TQWidget* parent)
	{
		return(new ModernSysConfig(conf, parent));
	}
}


// 'conf'	is a pointer to the twindecoration modules open twin config,
//		and is by default set to the "Style" group.
//
// 'parent'	is the parent of the TQObject, which is a VBox inside the
//		Configure tab in twindecoration

ModernSysConfig::ModernSysConfig(KConfig* conf, TQWidget* parent) : TQObject(parent)
{	
	clientrc = new KConfig("twinmodernsysrc");
	KGlobal::locale()->insertCatalogue("twin_clients");
	mainw = new TQWidget(parent);
	vbox = new TQVBoxLayout(mainw);
	vbox->setSpacing(6);
	vbox->setMargin(0);

	handleBox = new TQWidget(mainw);
        TQGridLayout* layout = new TQGridLayout(handleBox, 0, KDialog::spacingHint());

	cbShowHandle = new TQCheckBox(i18n("&Show window resize handle"), handleBox);
	TQWhatsThis::add(cbShowHandle,
			i18n("When selected, all windows are drawn with a resize "
			"handle at the lower right corner. This makes window resizing "
			"easier, especially for trackballs and other mouse replacements "
			"on laptops."));
        layout->addMultiCellWidget(cbShowHandle, 0, 0, 0, 1);
	connect(cbShowHandle, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotSelectionChanged()));

	sliderBox = new TQVBox(handleBox);
	handleSizeSlider = new TQSlider(0, 4, 1, 0, Qt::Horizontal, sliderBox);
	TQWhatsThis::add(handleSizeSlider,
			i18n("Here you can change the size of the resize handle."));
	handleSizeSlider->setTickInterval(1);
	handleSizeSlider->setTickmarks(TQSlider::Below);
	connect(handleSizeSlider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotSelectionChanged()));

	hbox = new TQHBox(sliderBox);
	hbox->setSpacing(6);

	bool rtl = kapp->reverseLayout();
	label1 = new TQLabel(i18n("Small"), hbox);
	label1->setAlignment(rtl ? AlignRight : AlignLeft);
	label2 = new TQLabel(i18n("Medium"), hbox);
	label2->setAlignment(AlignHCenter);
	label3 = new TQLabel(i18n("Large"), hbox);
	label3->setAlignment(rtl ? AlignLeft : AlignRight);
	
	vbox->addWidget(handleBox);
	vbox->addStretch(1);

//        layout->setColSpacing(0, 30);
        layout->addItem(new TQSpacerItem(30, 10, TQSizePolicy::Fixed, TQSizePolicy::Fixed), 1, 0);
        layout->addWidget(sliderBox, 1, 1);
	
	load(conf);
	mainw->show();
}


ModernSysConfig::~ModernSysConfig()
{
	delete mainw;
	delete clientrc;
}


void ModernSysConfig::slotSelectionChanged()
{
	bool i = cbShowHandle->isChecked();
	if (i != hbox->isEnabled()) {
		hbox->setEnabled(i);
		handleSizeSlider->setEnabled(i);
	}
	emit changed();
}


void ModernSysConfig::load(KConfig* /*conf*/)
{
	clientrc->setGroup("General");
	bool i = clientrc->readBoolEntry("ShowHandle", true );
	cbShowHandle->setChecked(i);
	hbox->setEnabled(i);
	handleSizeSlider->setEnabled(i);
	handleWidth = clientrc->readUnsignedNumEntry("HandleWidth", 6);
	handleSize = clientrc->readUnsignedNumEntry("HandleSize", 30);
	handleSizeSlider->setValue(TQMIN((handleWidth - 6) / 2, 4));
	
}


void ModernSysConfig::save(KConfig* /*conf*/)
{
	clientrc->setGroup("General");
	clientrc->writeEntry("ShowHandle", cbShowHandle->isChecked());
	clientrc->writeEntry("HandleWidth", 6 + 2 * handleSizeSlider->value());
	clientrc->writeEntry("HandleSize", 30 + 4 * handleSizeSlider->value());
	clientrc->sync();
}


void ModernSysConfig::defaults()
{
	cbShowHandle->setChecked(true);
	hbox->setEnabled(true);
	handleSizeSlider->setEnabled(true);
	handleSizeSlider->setValue(0);
}

#include "config.moc"
