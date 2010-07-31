#ifndef __KDE_MODSYSTEMCONFIG_H
#define __KDE_MODSYSTEMCONFIG_H

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqlayout.h>
#include <tqvbox.h>
#include <tqslider.h>
#include <tqlabel.h>

class ModernSysConfig : public QObject
{
	Q_OBJECT

	public:
		ModernSysConfig(KConfig* conf, TQWidget* parent);
		~ModernSysConfig();

	// These public signals/slots work similar to KCM modules
	signals:
		void		changed();

	public slots:
		void		load(KConfig* conf);	
		void		save(KConfig* conf);
		void		defaults();

	protected slots:
		void		slotSelectionChanged();	// Internal use

	private:
		KConfig   	*clientrc;
		QWidget		*mainw;
		QVBoxLayout	*vbox;
		TQWidget         *handleBox;
		TQCheckBox 	*cbShowHandle;
		QVBox		*sliderBox;
		QSlider		*handleSizeSlider;
		QHBox		*hbox;
		QLabel		*label1;
		QLabel		*label2;
		QLabel		*label3;
		
		unsigned  	handleWidth;
		unsigned  	handleSize;

};


#endif
