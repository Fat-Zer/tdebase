/*
 *
 *	KDE2 Default configuration widget
 *
 *	Copyright (c) 2001
 *		Karol Szwed <gallium@kde.org>
 *		http://gallium.n3.net/
 */

#ifndef _KDE_DEFAULT_CONFIG_H
#define _KDE_DEFAULT_CONFIG_H

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <kconfig.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqvbox.h>

class KDEDefaultConfig: public TQObject
{
	Q_OBJECT

	public:
		KDEDefaultConfig( KConfig* conf, TQWidget* parent );
		~KDEDefaultConfig();

	// These public signals/slots work similar to KCM modules
	signals:
		void changed();

	public slots:
		void load( KConfig* conf );
		void save( KConfig* conf );
		void defaults();

	protected slots:
		void slotSelectionChanged();	// Internal use

	private:
		TQCheckBox* cbShowStipple;
		TQCheckBox* cbShowGrabBar;
		TQCheckBox* cbUseGradients;
		TQVBox* gb;
		bool 	   highcolor;
};

#endif
// vim: ts=4
