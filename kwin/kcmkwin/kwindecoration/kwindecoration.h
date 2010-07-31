/*
	This is the new kwindecoration kcontrol module

	Copyright (c) 2001
		Karol Szwed <gallium@kde.org>
		http://gallium.n3.net/

	Supports new kwin configuration plugins, and titlebar button position
	modification via dnd interface.

	Based on original "kwintheme" (Window Borders) 
	Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef KWINDECORATION_H
#define KWINDECORATION_H

#include <kcmodule.h>
#include <dcopobject.h>
#include <buttons.h>
#include <kconfig.h>
#include <klibloader.h>

#include <kdecoration.h>

#include "kwindecorationIface.h"

class KComboBox;
class QCheckBox;
class QLabel;
class QTabWidget;
class QVBox;
class QSlider;

class KDecorationPlugins;
class KDecorationPreview;

// Stores themeName and its corresponding library Name
struct DecorationInfo
{
	TQString name;
	TQString libraryName;
};


class KWinDecorationModule : public KCModule, virtual public KWinDecorationIface, public KDecorationDefines
{
	Q_OBJECT

	public:
		KWinDecorationModule(TQWidget* parent, const char* name, const TQStringList &);
		~KWinDecorationModule();

		virtual void load();
		virtual void save();
		virtual void defaults();

		TQString quickHelp() const;

		virtual void dcopUpdateClientList();

	signals:
		void pluginLoad( KConfig* conf );
		void pluginSave( KConfig* conf );
		void pluginDefaults();

	protected slots:
		// Allows us to turn "save" on
		void slotSelectionChanged();
		void slotChangeDecoration( const TQString &  );
		void slotBorderChanged( int );
		void slotButtonsChanged();

	private:
		void readConfig( KConfig* conf );
		void writeConfig( KConfig* conf );
		void findDecorations();
		void createDecorationList();
		void updateSelection();
		TQString decorationLibName( const TQString& name );
		TQString decorationName ( TQString& libName );
		static TQString styleToConfigLib( TQString& styleLib );
		void resetPlugin( KConfig* conf, const TQString& currentDecoName = TQString::null );
		void resetKWin();
		void checkSupportedBorderSizes();
		static int borderSizeToIndex( BorderSize size, TQValueList< BorderSize > sizes );
		static BorderSize indexToBorderSize( int index, TQValueList< BorderSize > sizes );

		TQTabWidget* tabWidget;

		// Page 1
		KComboBox* decorationList;
		TQValueList<DecorationInfo> decorations;

		KDecorationPreview* preview;
		KDecorationPlugins* plugins;
		KConfig kwinConfig;

		TQCheckBox* cbUseCustomButtonPositions;
	//	TQCheckBox* cbUseMiniWindows;
		TQCheckBox* cbShowToolTips;
		TQLabel*    lBorder;
		TQComboBox* cBorder;
		BorderSize border_size;

		TQObject* pluginObject;
		TQWidget* pluginConfigWidget;
		TQString  currentLibraryName;
		TQString  oldLibraryName;
		TQObject* (*allocatePlugin)( KConfig* conf, TQWidget* parent );

		// Page 2
		ButtonPositionWidget *buttonPositionWidget;
		TQVBox*	 buttonPage;
};


#endif
// vim: ts=4
// kate: space-indent off; tab-width 4;
