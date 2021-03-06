/*
 *   tdeprintfax - a small fax utility
 *   Copyright (C) 2001  Michael Goffioul
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "conffax.h"

#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqprinter.h>

#include <tdeglobal.h>
#include <tdeconfig.h>
#include <tdelocale.h>
#include <kseparator.h>

#include <stdlib.h>

ConfFax::ConfFax(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	m_resolution = new TQComboBox(this);
	m_resolution->setMinimumHeight(25);
	m_pagesize = new TQComboBox(this);
	m_pagesize->setMinimumHeight(25);
	m_resolution->insertItem(i18n("High (204x196 dpi)"));
	m_resolution->insertItem(i18n("Low (204x98 dpi)"));
	m_pagesize->insertItem(i18n("A4"));
	m_pagesize->insertItem(i18n("Letter"));
	m_pagesize->insertItem(i18n("Legal"));
	TQLabel	*m_resolutionlabel = new TQLabel(i18n("&Resolution:"), this);
	m_resolutionlabel->setBuddy(m_resolution);
	TQLabel	*m_pagesizelabel = new TQLabel(i18n("&Paper size:"), this);
	m_pagesizelabel->setBuddy(m_pagesize);

	TQGridLayout	*l0 = new TQGridLayout(this, 3, 2, 10, 10);
	l0->setColStretch(1, 1);
	l0->setRowStretch(2, 1);
	l0->addWidget(m_resolutionlabel, 0, 0);
	l0->addWidget(m_pagesizelabel, 1, 0);
	l0->addWidget(m_resolution, 0, 1);
	l0->addWidget(m_pagesize, 1, 1);
}

void ConfFax::load()
{
	TDEConfig	*conf = TDEGlobal::config();
	conf->setGroup("Fax");
	TQString	v = conf->readEntry("Page", TDEGlobal::locale()->pageSize() == TQPrinter::A4 ? "a4" : "letter");
	if (v == "letter") m_pagesize->setCurrentItem(1);
	else if (v == "legal") m_pagesize->setCurrentItem(2);
	else m_pagesize->setCurrentItem(0);
	v = conf->readEntry("Resolution", "High");
	m_resolution->setCurrentItem((v == "Low" ? 1 : 0));
}

void ConfFax::save()
{
	TDEConfig	*conf = TDEGlobal::config();
	conf->setGroup("Fax");
	conf->writeEntry("Resolution", (m_resolution->currentItem() == 0 ? "High" : "Low"));
	conf->writeEntry("Page", (m_pagesize->currentItem() == 0 ? "a4" : (m_pagesize->currentItem() == 1 ? "letter" : "legal")));
}
