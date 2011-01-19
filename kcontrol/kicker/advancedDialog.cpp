/*
 *  advancedDialog.cpp
 *
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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
 */

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqslider.h>

#include <kcolorbutton.h>
#include <klocale.h>

#include "advancedDialog.h"
#include "advancedOptions.h"
#include "main.h"

advancedDialog::advancedDialog(TQWidget* parent, const char* name)
    : KDialogBase(KDialogBase::Plain,
                  i18n("Advanced Options"),
                  Ok|Apply|Cancel,
                  Cancel,
                  parent,
                  name,
                  false, false)
{
    connect(this, TQT_SIGNAL(applyClicked()),
            this, TQT_SLOT(save()));
    connect(this, TQT_SIGNAL(okClicked()),
            this, TQT_SLOT(save()));

    TQFrame* page = plainPage();
    TQVBoxLayout* layout = new TQVBoxLayout(page);
    m_advancedWidget = new advancedKickerOptions(page);
    layout->addWidget(m_advancedWidget);
    layout->addStretch();

    setMinimumSize( tqsizeHint() );

    connect(m_advancedWidget->handles, TQT_SIGNAL(clicked(int)),
            this, TQT_SLOT(changed()));
    connect(m_advancedWidget->hideButtonSize, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(changed()));
    connect(m_advancedWidget->tintColorB, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(changed()));
    connect(m_advancedWidget->tintSlider, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(changed()));
    connect(m_advancedWidget->menubarPanelTransparent, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(changed()));
    load();
}

advancedDialog::~advancedDialog()
{
}

void advancedDialog::load()
{
    KConfig c(KickerConfig::the()->configName(), false, false);
    c.setGroup("General");

    bool fadedOut = c.readBoolEntry("FadeOutAppletHandles", true);
    bool hideHandles = c.readBoolEntry("HideAppletHandles", false);
    if (hideHandles)
        m_advancedWidget->hideHandles->setChecked(true);
    else if (fadedOut)
        m_advancedWidget->fadeOutHandles->setChecked(true);
    else
        m_advancedWidget->visibleHandles->setChecked(true);

    int defaultHideButtonSize = c.readNumEntry("HideButtonSize", 14);
    m_advancedWidget->hideButtonSize->setValue(defaultHideButtonSize);
    TQColor color = c.readColorEntry( "TintColor", &tqcolorGroup().mid() );
    m_advancedWidget->tintColorB->setColor( color );
    int tintValue = c.readNumEntry( "TintValue", 33 );
    m_advancedWidget->tintSlider->setValue( tintValue );
    
    bool transparentMenubarPanel = c.readBoolEntry("MenubarPanelTransparent", false);
    m_advancedWidget->menubarPanelTransparent->setChecked( transparentMenubarPanel );

    enableButtonApply(false);
}

void advancedDialog::save()
{
    KConfig c(KickerConfig::the()->configName(), false, false);

    c.setGroup("General");
    c.writeEntry("FadeOutAppletHandles",
                 m_advancedWidget->fadeOutHandles->isChecked());
    c.writeEntry("HideAppletHandles",
                 m_advancedWidget->hideHandles->isChecked());
    c.writeEntry("HideButtonSize",
                 m_advancedWidget->hideButtonSize->value());
    c.writeEntry("TintColor",
                 m_advancedWidget->tintColorB->color());
    c.writeEntry("TintValue",
                 m_advancedWidget->tintSlider->value());
    c.writeEntry("MenubarPanelTransparent",
                 m_advancedWidget->menubarPanelTransparent->isChecked());

    TQStringList elist = c.readListEntry("Extensions2");
    for (TQStringList::Iterator it = elist.begin(); it != elist.end(); ++it)
    {
        // extension id
        TQString group(*it);

        // is there a config group for this extension?
        if(!c.hasGroup(group) ||
           group.tqcontains("Extension") < 1)
        {
            continue;
        }

        // set config group
        c.setGroup(group);
        KConfig extConfig(c.readEntry("ConfigFile"));
        extConfig.setGroup("General");
        extConfig.writeEntry("FadeOutAppletHandles",
                             m_advancedWidget->fadeOutHandles->isChecked());
        extConfig.writeEntry("HideAppletHandles",
                             m_advancedWidget->hideHandles->isChecked());
        extConfig.writeEntry("HideButtonSize",
                             m_advancedWidget->hideButtonSize->value());
        extConfig.writeEntry("TintColor",
                             m_advancedWidget->tintColorB->color());
        extConfig.writeEntry("TintValue",
                             m_advancedWidget->tintSlider->value());
        extConfig.writeEntry("MenubarPanelTransparent",
                             m_advancedWidget->menubarPanelTransparent->isChecked());

        extConfig.sync();
    }
   
    c.sync();

    KickerConfig::the()->notifyKicker();
    enableButtonApply(false);
}

void advancedDialog::changed()
{
    enableButtonApply(true);
}

#include "advancedDialog.moc"

