/*
 *  Copyright (c) 2005      Stefan Nikolaus <stefan.nikolaus@kdemail.net>
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

#include <tqlayout.h>
#include <tqtimer.h>

#include <tdelocale.h>
#include <kdebug.h>

#include "lookandfeeltab_impl.h"
#include "kickerSettings.h"
#include "main.h"

#include "lookandfeelconfig.h"
#include "lookandfeelconfig.moc"

LookAndFeelConfig::LookAndFeelConfig(TQWidget *parent, const char *name)
  : TDECModule(parent, name)
{
    TQVBoxLayout *layout = new TQVBoxLayout(this);
    m_widget = new LookAndFeelTab(this);
    layout->addWidget(m_widget);
    layout->addStretch();

    setQuickHelp(KickerConfig::the()->quickHelp());
    setAboutData(KickerConfig::the()->aboutData());

    addConfig(KickerSettings::self(), m_widget);

    connect(m_widget, TQT_SIGNAL(changed()),
            this, TQT_SLOT(changed()));
    connect(KickerConfig::the(), TQT_SIGNAL(aboutToNotifyKicker()),
            this, TQT_SLOT(aboutToNotifyKicker()));

    load();
    TQTimer::singleShot(0, this, TQT_SLOT(notChanged()));
}

void LookAndFeelConfig::notChanged()
{
    emit changed(false);
}

void LookAndFeelConfig::load()
{
    TDECModule::load();
    m_widget->load();
}

void LookAndFeelConfig::aboutToNotifyKicker()
{
    kdDebug() << "LookAndFeelConfig::aboutToNotifyKicker()" << endl;

    // This slot is triggered by the signal,
    // which is send before Kicker is notified.
    // See comment in save().
    TDECModule::save();
    m_widget->save();
}

void LookAndFeelConfig::save()
{
    // As we don't want to notify Kicker multiple times
    // we do not save the settings here. Instead the
    // KickerConfig object sends a signal before the
    // notification. On this signal all existing modules,
    // including this object, save their settings.
    KickerConfig::the()->notifyKicker();
}

void LookAndFeelConfig::defaults()
{
    TDECModule::defaults();
    m_widget->defaults();

    // TDEConfigDialogManager may queue an changed(false) signal,
    // so we make sure, that the module is labeled as changed,
    // while we manage some of the widgets ourselves
    TQTimer::singleShot(0, this, TQT_SLOT(changed()));
}

TQString LookAndFeelConfig::handbookSection() const
{
    return "panel-appearance";
}