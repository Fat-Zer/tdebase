/***************************************************************************
                          keyecandypage.h  -  description
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

#ifndef KEYECANDYPAGE_H
#define KEYECANDYPAGE_H

#include "keyecandypagedlg.h"

class TQCheckListItem;
class TQColor;
class KSysInfo;

/**Abstract class for the eyecandy page. Applies  the accoring eyecandy settings
  *@author Ralf Nolden
  */

class KEyeCandyPage : public KEyeCandyPageDlg {
	Q_OBJECT
public: 
	KEyeCandyPage(TQWidget *parent=0, const char *name=0);
	~KEyeCandyPage();

//---------------------------------------------------------------------------------------------------------
	/** This should be self-explanatory, enabling/disabling the default desktop wallpaper. Level 0 disables,
	Level 1 enables this (and all levels above). */
	void enableDesktopWallpaper(bool enable, bool user=false);
	/** this function enables/disables the window effects for Shading, Minimize and Restore. The contents in moving/resized windows is set in enableWindowContens(bool ) */
	void enableDesktopWindowEffects(bool enable, bool restore= false);
	/** enable/disable window moving with contents shown */
	void enableDesktopWindowMovingContents( bool enable,bool restore= false);
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
	/** Level 0-2 disable this, Level 3 and above enable this.  */
	void enableIconZoomingPanel(bool enable);
	/** enable Icon highlighting,  Level 3 */
	void enableIconEffectGamma(bool enable, bool user);
	/** No descriptions */
	void enableIconEffectSizeDesktop(bool enable);
	/** No descriptions */
	void enableIconEffectSizePanel(bool enable);
	/** No descriptions */
	void enableIconMngAnimation(bool enable);
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
	/** Here, the background tiles/wallpapers for Kicker are set to the default values. Enabled in Level 2. */
	void enableBackgroundsPanel(bool enable);
	/** Here, the background tiles/wallpapers for Konqueror are set to the default values. Enabled in Level 2. */
	void enableBackgroundsKonqueror(bool enable);
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
	/** enables all other file previews that are available besides text and image preview. Enable in Level 9. */
	void enablePreviewOther(bool enable);
	/** enables text preview in konq/kdesktop. Enable in Level 8 */
	void enablePreviewText(bool enable);
	/** enables desktop/konqueror image previews, level 5 */
	void enablePreviewImages(bool enable);
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
	/** Enable fading tooltips in Level 7 */
	void enableFadingToolTips(bool enable);
	/** enables/disables fading menus which are off by default in TDE. Enable this in Level 9 */
	void enableFadingMenus(bool enable);
	/** Enable animated combo boxes, see styles kcontrol module. Enable in Level 4 (disabled by default anyway, so doesn't need to be
		disabled in levels below 4) */
	void enableAnimatedCombo(bool enable);
	/** Enable icons on pushbuttons in level 5 and up */
	void enablePushButtonIcons(bool enable);
	/** generally enable/disable style-Effects, depending on if one of the three is enabled. */
	void enableEffects(bool enable);
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
	/** Enable Antialiased fonts. Maybe a check if the chard can do this with xdpyinfo | grep RENDER here would bring up if
		the user can do this or not. Enable in Level 7. */
	void enableAntialiasingFonts(bool enable, bool reset);
	/** Enables the default TDE sound scheme in Level 3 */
	void enableSoundScheme(bool enable, bool user);
//---------------------------------------------------------------------------------------------------------


	/** save function to enable/disable the according settings that are made in the QCheckListItems of the
		Eyecandy page by default. If currSettings is false, the user's default settings will be restored*/
	void save(bool currSettings=true);
	/** sets the slider to the default value of Level 4 (TDE Default) and the checklistboxes on
		that belong to this level */
	void setDefaults();
	/** as the preview entries in the rc files (konqiconviewrc and kdesktoprc) are a string list and
		this list would be overwritten by the three different possibilities, we just set bool values
		and ask them here, set the according string list here.  If currSettings is true, take the chosen ones, else take the user's ones*/
	void enablePreview(bool currSettings);
	/** calls all enable functions with the state of the checkboxes. This is needed for save() only,
		as in case the user quits, we have to set these states again in saveUserDefaults to what they were
		prior to running kpersonalizer */
	void saveCheckState(bool currSettings);
	/** retrieves the user's local values. In case he doesn't have these set, use the default values of TDE, level 4. */
	void getUserDefaults();
	void getUserSoundScheme();

public slots:
	/** enables/disables the QCheckListItems in the klv_features
		according to the level the slider moved. */
	void slotEyeCandySliderMoved(int value);
	/** show the details-box */
	void slotEyeCandyShowDetails(bool details);
	/** to be connected to the OS page. changes default values of features according
		to the desktop selection*/
	void slotPresetSlider(const TQString& style);

private:
	// DEFAULT VALUES SET BY USER
	int panelsize; // kicker panelsize 0,1,2,3 before the big icons are set to reset that
	int desktopiconsize;
	TQString osStyle;  //stores OS-Style selection (page 2)
	bool b_EffectFadeMenu, b_EffectAnimateCombo, b_EffectFadeTooltip, b_EnableIconZoom,
		 b_AnimateMinimize, b_AnimateShade, b_MoveResizeMaximizedWindows,
		 b_ShadeHover, b_useXft, b_enableXft, b_PushButtonIcons, b_EffectsEnabled,
		 b_iconMngAnim, b_konq_prev_sound, b_konq_prev_enable, b_OpaqueResize;

	TQString konqbgimage, s_ResizeMode, s_MoveMode, deskbgimage;
	TQStringList konq_dont_prev, kdesktop_prev;
	TQColor desktopTextColor;

	struct st_Gamma{
		TQString EffectDesktop;
		TQString EffectPanel;
		TQString ValueDesktop;
		TQString ValuePanel;
	} st_UserGamma;

	struct st_Wallpaper{
		bool CommonDesktop;
		TQString MultiWallpaperMode;
		TQString WallpaperMode;
		TQString Wallpaper;
	} st_UserWallpaper;

	struct st_Sound {
		int desktop1;
		int desktop2;
		int desktop3;
		int desktop4;
		int desktop5;
		int desktop6;
		int desktop7;
		int desktop8;
		int notold;
		int close;
		int transnew;
		int transdelete;
		int iconify;
		int deiconify;
		int maximize;
		int unmaximize;
		int shadeup;
		int shadedown;
		int sticky;
		int unsticky;
	} st_UserSnd;
  // DEFAULT VALLUES SET BY USER (END)

	TDEConfig* twinconf;
	TDEConfig* twineventconf;
	TDEConfig* kickerconf;
	TDEConfig* konquerorconf;
	TDEConfig* konqiconconf;
	TDEConfig* kdesktopconf;
	TDEConfig* kdeglobals;

	KSysInfo* sys;

	bool prevImage, prevText, prevOther;

	TQCheckListItem* alpha_blending_desktop;
	TQCheckListItem* alpha_blending_panel;

	TQCheckListItem* animated_combo;

	TQCheckListItem* antialiasing_fonts;

	TQCheckListItem* backgrounds_konqueror;
	TQCheckListItem* backgrounds_panel;

	TQCheckListItem* desktop_wallpaper;
	TQCheckListItem* desktop_window_effects;
	TQCheckListItem* desktop_window_moving_contents;

	TQCheckListItem* icon_effect_gamma;
	TQCheckListItem* icon_effect_size_desktop;
	TQCheckListItem* icon_effect_size_panel;

	TQCheckListItem* icon_zooming_panel;
	TQCheckListItem* icon_mng_animation;

	TQCheckListItem* fading_menus;
	TQCheckListItem* fading_tooltips;
  
	TQCheckListItem* pushbutton_icons;

	TQCheckListItem* preview_text;
	TQCheckListItem* preview_images;
	TQCheckListItem* preview_other;

	TQCheckListItem* sound_scheme;
};

#endif
