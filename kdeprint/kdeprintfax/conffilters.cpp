/*
 *   kdeprintfax - a small fax utility
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

#include "conffilters.h"
#include "filterdlg.h"

#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqheader.h>

#include <klocale.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

ConfFilters::ConfFilters(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	m_filters = new KListView(this);
	m_filters->addColumn(i18n("Mime Type"));
	m_filters->addColumn(i18n("Command"));
	m_filters->setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	m_filters->setLineWidth(1);
	m_filters->setSorting(-1);
	m_filters->header()->setStretchEnabled(true, 1);
	connect(m_filters, TQT_SIGNAL(doubleClicked(TQListViewItem*)), TQT_SLOT(slotChange()));

	m_add = new TQPushButton(this);
	m_add->setPixmap(BarIcon("filenew"));
	m_remove = new TQPushButton(this);
	m_remove->setIconSet(BarIconSet("remove"));
	m_change = new TQPushButton(this);
	m_change->setIconSet(BarIconSet("filter"));
	m_up = new TQPushButton(this);
	m_up->setIconSet(BarIconSet("up"));
	m_down = new TQPushButton(this);
	m_down->setIconSet(BarIconSet("down"));
	connect(m_add, TQT_SIGNAL(clicked()), TQT_SLOT(slotAdd()));
	connect(m_change, TQT_SIGNAL(clicked()), TQT_SLOT(slotChange()));
	connect(m_remove, TQT_SIGNAL(clicked()), TQT_SLOT(slotRemove()));
	connect(m_up, TQT_SIGNAL(clicked()), TQT_SLOT(slotUp()));
	connect(m_down, TQT_SIGNAL(clicked()), TQT_SLOT(slotDown()));
	TQToolTip::add(m_add, i18n("Add filter"));
	TQToolTip::add(m_change, i18n("Modify filter"));
	TQToolTip::add(m_remove, i18n("Remove filter"));
	TQToolTip::add(m_up, i18n("Move filter up"));
	TQToolTip::add(m_down, i18n("Move filter down"));

	QHBoxLayout	*l0 = new TQHBoxLayout(this, 10, 10);
	QVBoxLayout	*l1 = new TQVBoxLayout(0, 0, 0);
	l0->addWidget(m_filters, 1);
	l0->addLayout(l1, 0);
	l1->addWidget(m_add);
	l1->addWidget(m_change);
	l1->addWidget(m_remove);
	l1->addSpacing(10);
	l1->addWidget(m_up);
	l1->addWidget(m_down);
	l1->addStretch(1);
	updateButton();
	connect(m_filters, TQT_SIGNAL(selectionChanged ()),TQT_SLOT(updateButton()));
}

void ConfFilters::load()
{
	QFile	f(locate("data","kdeprintfax/faxfilters"));
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;
		int		p(-1);
		QListViewItem	*item(0);
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if ((p=line.find(TQRegExp("\\s"))) != -1)
			{
				QString	mime(line.left(p)), cmd(line.right(line.length()-p-1).stripWhiteSpace());
				if (!mime.isEmpty() && !cmd.isEmpty())
					item = new TQListViewItem(m_filters, item, mime, cmd);
			}
		}
	}
}

void ConfFilters::save()
{
	QListViewItem	*item = m_filters->firstChild();
	QFile	f(locateLocal("data","kdeprintfax/faxfilters"));
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		while (item)
		{
			t << item->text(0) << ' ' << item->text(1) << endl;
			item = item->nextSibling();
		}
	}
}

void ConfFilters::slotAdd()
{
	QString	mime, cmd;
	if (FilterDlg::doIt(this, &mime, &cmd))
		if (!mime.isEmpty() && !cmd.isEmpty())
		  {
		    new TQListViewItem(m_filters, m_filters->currentItem(), mime, cmd);
		    updateButton();
		  }
		else
			KMessageBox::error(this, i18n("Empty parameters."));
}

void ConfFilters::slotRemove()
{
	QListViewItem	*item = m_filters->currentItem();
	if (item)
		delete item;
	updateButton();
}

void ConfFilters::slotChange()
{
	QListViewItem	*item = m_filters->currentItem();
	if (item)
	{
		QString	mime(item->text(0)), cmd(item->text(1));
		if (FilterDlg::doIt(this, &mime, &cmd))
		{
			item->setText(0, mime);
			item->setText(1, cmd);
		}
	}
}

void ConfFilters::slotUp()
{
	QListViewItem	*item = m_filters->currentItem();
	if (item && item->itemAbove())
	{
		m_filters->moveItem(item, 0, item->itemAbove()->itemAbove());
		m_filters->setCurrentItem(item);
		updateButton();
	}
}

void ConfFilters::slotDown()
{
	QListViewItem	*item = m_filters->currentItem();
	if (item && item->itemBelow())
	{
		m_filters->moveItem(item, 0, item->itemBelow());
		m_filters->setCurrentItem(item);
		updateButton();
	}
}

void ConfFilters::updateButton()
{
  QListViewItem	*item = m_filters->currentItem();

  bool state=item && item->itemBelow();
  m_remove->setEnabled(item);
  m_down->setEnabled(state);
  state=item && item->itemAbove();
  m_up->setEnabled(state);
  m_change->setEnabled(item);
}

#include "conffilters.moc"
