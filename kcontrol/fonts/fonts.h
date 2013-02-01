//-----------------------------------------------------------------------------
//
// kdisplay, fonts tab
//
// Copyright (c)  Mark Donohoe 1997
//                Lars Knoll 1999

#ifndef FONTS_H
#define FONTS_H

#include <tqobject.h>

#include <tdecmodule.h>
#include <kdialogbase.h>
#include <kfontdialog.h>
#include <kfontrequester.h>

#include "kxftconfig.h"

class TQCheckBox;
class TQComboBox;
class KDoubleNumInput;
class FontAASettings;

class FontUseItem : public TDEFontRequester
{
  Q_OBJECT

public:
    FontUseItem(TQWidget * parent, const TQString &name, const TQString &grp, 
        const TQString &key, const TQString &rc, const TQFont &default_fnt, 
        bool fixed = false);

    void readFont( bool useDefaults );
    void writeFont();
    void setDefault();
    void applyFontDiff(const TQFont &fnt, int fontDiffFlags);

    const TQString& rcFile() { return _rcfile; }
    const TQString& rcGroup() { return _rcgroup; }
    const TQString& rcKey() { return _rckey; }

private:
    TQString _rcfile;
    TQString _rcgroup;
    TQString _rckey;
    TQFont _default;
};

class FontAASettings : public KDialogBase
{
  Q_OBJECT

public:

    FontAASettings(TQWidget *parent);

    bool save( bool useAA );
    bool load();
    bool load( bool useDefaults );
    void defaults();
    int getIndex(KXftConfig::SubPixel::Type spType);
    KXftConfig::SubPixel::Type getSubPixelType();
#ifdef HAVE_FONTCONFIG
    int getIndex(KXftConfig::Hint::Style hStyle);
    KXftConfig::Hint::Style getHintStyle();
#endif
    void enableWidgets();
    int exec();

protected slots:

    void changed();

private:

    TQCheckBox *excludeRange;
    TQCheckBox *useSubPixel;
    KDoubleNumInput *excludeFrom;
    KDoubleNumInput *excludeTo;
    TQComboBox *subPixelType;
#ifdef HAVE_FONTCONFIG
    TQComboBox *hintingStyle;
#endif
    TQLabel    *excludeToLabel;
    bool      changesMade;
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class TDEFonts : public TDECModule
{
    Q_OBJECT

public:
    TDEFonts(TQWidget *parent, const char *name, const TQStringList &);
    ~TDEFonts();

    virtual void load();
    virtual void load( bool useDefaults );
    virtual void save();
    virtual void defaults();

protected slots:
    void fontSelected();
    void slotApplyFontDiff(); 
    void slotUseAntiAliasing();
    void slotCfgAa();

private:
    enum AASetting { AAEnabled, AASystem, AADisabled };
    enum DPISetting { DPINone, DPI96, DPI120 };
    AASetting useAA, useAA_original;
    DPISetting dpi_original;
    TQComboBox *cbAA;
    TQComboBox* comboForceDpi;
    TQPushButton *aaSettingsButton;
    TQPtrList <FontUseItem> fontUseList;
    FontAASettings *aaSettings;
};

#endif

