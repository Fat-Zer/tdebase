/* This file is part of the TDE project
   Copyright (C) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqmultilineedit.h>
#include <tqlineedit.h>
#include <tqlabel.h>

#include <tdemessagebox.h>
#include <tdelocale.h>

#include "bugdescription.moc"
#include "bugdescription.h"

BugDescription::BugDescription(TQWidget *parent, bool modal,
                           const TDEAboutData *aboutData)
  : BugDescriptionDialog(parent, modal, aboutData)
{
}

void BugDescription::setText(const TQString &str)
{
	m_lineedit->setText(str);
	m_startstring = str.simplifyWhiteSpace();
}

TQString BugDescription::emailAddress()
{
	return m_email->text();
}

TQString BugDescription::crashDescription()
{
	return m_lineedit->text();
}

void BugDescription::fullReportViewMode( bool enabled ) {
	if (enabled) {
		m_email->hide();
		m_emailLabel->hide();
		m_descriptionLabel->hide();
		m_lineedit->setReadOnly(true);
		showButtonCancel(false);
		setCaption(i18n("Crash Report"));
	}
	else {
		m_email->show();
		m_emailLabel->show();
		m_descriptionLabel->show();
		m_lineedit->setReadOnly(false);
		showButtonCancel(true);
		setCaption(i18n("Bug Description"));
	}
}

void BugDescription::slotOk()
{
	BugDescriptionDialog::slotOk();
}

