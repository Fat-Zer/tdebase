/***************************************************************************
                          kstylepage.h  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTYLEPAGE_H
#define KSTYLEPAGE_H

#include <tqcolor.h>
#include "kstylepagedlg.h"

/**Abstract class for the style page
  *@author Ralf Nolden
  */
class TQListViewItem;

class KStylePage : public KStylePageDlg  {
	Q_OBJECT

public:
	KStylePage(TQWidget *parent=0, const char *name=0);
	~KStylePage();
	void save(bool curSettings=true);
	/** resets to KDE style as default */
	void setDefaults();
	/** set the preview-widgets' style to the currently selected */
	void switchPrevStyle();

private:
	TQString origStyle;
	TQString origKWinStyle;
	TQString origIcons;
	TQString defaultKWinStyle;
	TQString currentStyle;
	KConfig* ckwin;
	struct colorSet {
		TQString colorFile, bgMode;
		int contrast;
		TQColor usrCol1, usrCol2;
		TQColor foreground;
		TQColor background;
		TQColor windowForeground;
		TQColor windowBackground;
		TQColor selectForeground;
		TQColor selectBackground;
		TQColor buttonForeground;
		TQColor buttonBackground;
		TQColor linkColor;
		TQColor visitedLinkColor;
		TQColor activeForeground;
		TQColor inactiveForeground;
		TQColor activeBackground;
		TQColor inactiveBackground;
		TQColor activeBlend;
		TQColor inactiveBlend;
		TQColor activeTitleBtnBg;
		TQColor inactiveTitleBtnBg;
                TQColor alternateBackground;
	} usrColors, currentColors;
	// first, the KDE 2 default color values
	TQColor widget;
	TQColor kde34Blue;
        TQColor inactiveBackground;
        TQColor activeBackground;
	TQColor button;
	TQColor link;
	TQColor visitedLink;
        TQColor activeBlend;
        TQColor activeTitleBtnBg;
        TQColor inactiveTitleBtnBg;
        TQColor inactiveForeground;
        TQColor alternateBackground;

	TQListViewItem * kde;
	TQListViewItem * classic;
	TQListViewItem * keramik;
	TQListViewItem * cde;
	TQListViewItem * win;
	TQListViewItem * platinum;

	TQStyle *appliedStyle;

	// widget-style existence
	bool kde_hc_exist, kde_def_exist, kde_keramik_exist, kde_light_exist,
		cde_exist, win_exist, platinum_exist, kde_plastik_exist;

	// kwin-style-existence
	bool kwin_keramik_exist, kwin_default_exist, kwin_system_exist,
		kwin_win_exist, kwin_cde_exist, kwin_quartz_exist, kwin_plastik_exist;

	// icon-theme-existence
	bool icon_crystalsvg_exist, icon_kdeclassic_exist, icon_Locolor_exist;

public slots: // Public slots
	/** to be connected to the OS page. Catches either KDE, CDE, win or mac and pre-sets the style.  */
	void presetStyle(const TQString& style);

private:
	void saveColors(bool curSettings=true);
	void saveStyle(bool curSettings=true);
	void saveKWin(bool curSettings=true);
	void saveIcons(bool curSettings=true);
	void getAvailability();
	void getUserDefaults();
	void initColors();
	void liveUpdate();
	void getColors(colorSet *set, bool colorfile );
	void setStyleRecursive(TQWidget* w, TQPalette &, TQStyle* s);
	void changeCurrentStyle();
	TQPalette createPalette();

private slots:
	void slotCurrentChanged();
};

#endif
