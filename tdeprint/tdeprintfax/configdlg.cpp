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

#include "configdlg.h"
#include "confgeneral.h"
#include "conffax.h"
#include "confsystem.h"
#include "conffilters.h"

#include <tqvbox.h>
#include <tdelocale.h>
#include <kiconloader.h>

ConfigDlg::ConfigDlg(TQWidget *parent, const char *name)
: KDialogBase(IconList, i18n("Configuration"), Ok|Cancel, Ok, parent, name, true)
{
	TQVBox	*page1 = addVBoxPage(i18n("Personal"), i18n("Personal Settings"), DesktopIcon("tdmconfig"));
	m_general = new ConfGeneral(page1, "Personal");

	TQVBox	*page2 = addVBoxPage(i18n("Page setup"), i18n("Page Setup"), DesktopIcon("edit-copy"));
	m_fax = new ConfFax(page2, "Fax");

	TQVBox	*page3 = addVBoxPage(i18n("System"), i18n("Fax System Selection"), DesktopIcon("tdeprintfax"));
	m_system = new ConfSystem(page3, "System");

	TQVBox	*page4 = addVBoxPage(i18n("Filters"), i18n("Filters Configuration"), DesktopIcon("filter"));
	m_filters = new ConfFilters(page4, "Filters");

	resize(450, 300);
}

void ConfigDlg::load()
{
	m_general->load();
	m_fax->load();
	m_system->load();
	m_filters->load();
}

void ConfigDlg::save()
{
	m_general->save();
	m_fax->save();
	m_system->save();
	m_filters->save();
}

bool ConfigDlg::configure(TQWidget *parent)
{
	ConfigDlg	dlg(parent);
	dlg.load();
	if (dlg.exec())
	{
		dlg.save();
		return true;
	}
	return false;
}
