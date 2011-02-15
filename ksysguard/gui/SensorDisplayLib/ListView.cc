/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <tqdom.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "ListView.h"
#include "ListView.moc"
#include "ListViewSettings.h"

PrivateListViewItem::PrivateListViewItem(PrivateListView *parent)
	: TQListViewItem(parent)
{
	_parent = parent;
}

int PrivateListViewItem::compare( TQListViewItem *item, int col, bool ascending ) const
{
  int type = ((PrivateListView*)listView())->columnType( col );

  if ( type == PrivateListView::Int ) {
    int prev = (int)KGlobal::locale()->readNumber( key( col, ascending ) );
    int next = (int)KGlobal::locale()->readNumber( item->key( col, ascending ) );
    if ( prev < next )
      return -1;
    else if ( prev == next )
      return 0;
    else
      return 1;
  } else if ( type == PrivateListView::Float ) {
    double prev = KGlobal::locale()->readNumber( key( col, ascending ) );
    double next = KGlobal::locale()->readNumber( item->key( col, ascending ) );
    if ( prev < next )
      return -1;
    else
      return 1;
  } else if ( type == PrivateListView::Time ) {
    int hourPrev, hourNext, minutesPrev, minutesNext;
    sscanf( key( col, ascending ).latin1(), "%d:%d", &hourPrev, &minutesPrev );
    sscanf( item->key( col, ascending ).latin1(), "%d:%d", &hourNext, &minutesNext );
    int prev = hourPrev * 60 + minutesPrev;
    int next = hourNext * 60 + minutesNext;
    if ( prev < next )
      return -1;
    else if ( prev == next )
      return 0;
    else
      return 1;
  } else if ( type == PrivateListView::DiskStat ) {
    TQString prev = key( col, ascending );
    TQString next = item->key( col, ascending );
    TQString prevKey, nextKey;
    
    uint counter = prev.length();
    for ( uint i = 0; i < counter; ++i )
      if ( prev[ i ].isDigit() ) {
        prevKey.sprintf( "%s%016d", prev.left( i ).latin1(), prev.mid( i ).toInt() );
        break;
      }

    counter = next.length();
    for ( uint i = 0; i < counter; ++i )
      if ( next[ i ].isDigit() ) {
        nextKey.sprintf( "%s%016d", next.left( i ).latin1(), next.mid( i ).toInt() );
        break;
      }

    return prevKey.compare( nextKey );
  } else
    return key( col, ascending ).localeAwareCompare( item->key( col, ascending ) );
}

PrivateListView::PrivateListView(TQWidget *parent, const char *name)
	: TQListView(parent, name)
{
	TQColorGroup cg = tqcolorGroup();

	cg.setColor(TQColorGroup::Link, KSGRD::Style->firstForegroundColor());
	cg.setColor(TQColorGroup::Text, KSGRD::Style->secondForegroundColor());
	cg.setColor(TQColorGroup::Base, KSGRD::Style->backgroundColor());

	setPalette(TQPalette(cg, cg, cg));
}

void PrivateListView::update(const TQString& answer)
{
	tqsetUpdatesEnabled(false);
	viewport()->tqsetUpdatesEnabled(false);

	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

	clear();

	KSGRD::SensorTokenizer lines(answer, '\n');
	for (uint i = 0; i < lines.count(); i++) {
		PrivateListViewItem *item = new PrivateListViewItem(this);
		KSGRD::SensorTokenizer records(lines[i], '\t');
		for (uint j = 0; j < records.count(); j++) {
      if ( mColumnTypes[ j ] == "f" )
        item->setText(j, KGlobal::locale()->formatNumber( records[j].toFloat() ) );
      else if ( mColumnTypes[ j ] == "D" )
        item->setText(j, KGlobal::locale()->formatNumber( records[j].toDouble(), 0 ) );
      else
			  item->setText(j, records[j]);
    }

		insertItem(item);
	}

	verticalScrollBar()->setValue(vpos);
	horizontalScrollBar()->setValue(hpos);

	viewport()->tqsetUpdatesEnabled(true);
	tqsetUpdatesEnabled(true);

	triggerUpdate();
}

int PrivateListView::columnType( uint pos ) const
{
  if ( pos >= mColumnTypes.count() )
    return 0;

  if ( mColumnTypes[ pos ] == "d" || mColumnTypes[ pos ] == "D" )
    return Int;
  else if ( mColumnTypes[ pos ] == "f" || mColumnTypes[ pos ] == "F" )
    return Float;
  else if ( mColumnTypes[ pos ] == "t" )
    return Time;
  else if ( mColumnTypes[ pos ] == "M" )
    return DiskStat;
  else
    return Text;
}

void PrivateListView::removeColumns(void)
{
	for (int i = columns() - 1; i >= 0; --i)
		removeColumn(i);
}

void PrivateListView::addColumn(const TQString& label, const TQString& type)
{
	TQListView::addColumn( label );
  int col = columns() - 1;

  if (type == "s" || type == "S")
    setColumnAlignment(col, AlignLeft);
	else if (type == "d" || type == "D")
		setColumnAlignment(col, AlignRight);
	else if (type == "t")
		setColumnAlignment(col, AlignRight);
	else if (type == "f")
		setColumnAlignment(col, AlignRight);
	else if (type == "M")
		setColumnAlignment(col, AlignLeft);
	else
	{
		kdDebug(1215) << "Unknown type " << type << " of column " << label
				  << " in ListView!" << endl;
		return;
	}

  mColumnTypes.append( type );

	/* Just use some sensible default values as initial setting. */
	TQFontMetrics fm = fontMetrics();
	setColumnWidth(col, fm.width(label) + 10);
}

ListView::ListView(TQWidget* parent, const char* name, const TQString& title, int, int)
	: KSGRD::SensorDisplay(parent, name, title)
{
	setBackgroundColor(KSGRD::Style->backgroundColor());

	monitor = new PrivateListView( frame() );
	Q_CHECK_PTR(monitor);
	monitor->setSelectionMode(TQListView::NoSelection);
	monitor->setItemMargin(2);

	setMinimumSize(50, 25);

	setPlotterWidget(monitor);

	setModified(false);
}

bool
ListView::addSensor(const TQString& hostName, const TQString& sensorName, const TQString& sensorType, const TQString& title)
{
	if (sensorType != "listview")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

	setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);
	sendRequest(hostName, sensorName, 19);
	setModified(true);
	return (true);
}

void
ListView::updateList()
{
	sendRequest(sensors().tqat(0)->hostName(), sensors().tqat(0)->name(), 19);
}

void
ListView::answerReceived(int id, const TQString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 100: {
			/* We have received the answer to a '?' command that contains
			 * the information about the table headers. */
			KSGRD::SensorTokenizer lines(answer, '\n');
			if (lines.count() != 2)
			{
				kdDebug(1215) << "wrong number of lines" << endl;
				return;
			}
			KSGRD::SensorTokenizer headers(lines[0], '\t');
			KSGRD::SensorTokenizer colTypes(lines[1], '\t');

			/* remove all columns from list */
			monitor->removeColumns();

			/* add the new columns */
			for (unsigned int i = 0; i < headers.count(); i++)
				/* TODO: Implement translation support for header texts */
				monitor->addColumn(headers[i], colTypes[i]);
			break;
		}
		case 19: {
			monitor->update(answer);
			break;
		}
	}
}

void
ListView::resizeEvent(TQResizeEvent*)
{
	frame()->setGeometry(0, 0, width(), height());
	monitor->setGeometry(10, 20, width() - 20, height() - 30);
}

bool
ListView::restoreSettings(TQDomElement& element)
{
	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), element.attribute("title"));

	TQColorGroup tqcolorGroup = monitor->tqcolorGroup();
	tqcolorGroup.setColor(TQColorGroup::Link, restoreColor(element, "gridColor", KSGRD::Style->firstForegroundColor()));
	tqcolorGroup.setColor(TQColorGroup::Text, restoreColor(element, "textColor", KSGRD::Style->secondForegroundColor()));
	tqcolorGroup.setColor(TQColorGroup::Base, restoreColor(element, "backgroundColor", KSGRD::Style->backgroundColor()));

	monitor->setPalette(TQPalette(tqcolorGroup, tqcolorGroup, tqcolorGroup));

	SensorDisplay::restoreSettings(element);

	setModified(false);

	return (true);
}

bool
ListView::saveSettings(TQDomDocument& doc, TQDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors().tqat(0)->hostName());
	element.setAttribute("sensorName", sensors().tqat(0)->name());
	element.setAttribute("sensorType", sensors().tqat(0)->type());

	TQColorGroup tqcolorGroup = monitor->tqcolorGroup();
	saveColor(element, "gridColor", tqcolorGroup.color(TQColorGroup::Link));
	saveColor(element, "textColor", tqcolorGroup.color(TQColorGroup::Text));
	saveColor(element, "backgroundColor", tqcolorGroup.color(TQColorGroup::Base));

	SensorDisplay::saveSettings(doc, element);

	if (save)
		setModified(false);

	return (true);
}

void
ListView::configureSettings()
{
	lvs = new ListViewSettings(this, "ListViewSettings");
	Q_CHECK_PTR(lvs);
	connect(lvs, TQT_SIGNAL(applyClicked()), TQT_SLOT(applySettings()));

	TQColorGroup tqcolorGroup = monitor->tqcolorGroup();
	lvs->setGridColor(tqcolorGroup.color(TQColorGroup::Link));
	lvs->setTextColor(tqcolorGroup.color(TQColorGroup::Text));
	lvs->setBackgroundColor(tqcolorGroup.color(TQColorGroup::Base));
	lvs->setTitle(title());

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	TQColorGroup tqcolorGroup = monitor->tqcolorGroup();
	tqcolorGroup.setColor(TQColorGroup::Link, lvs->gridColor());
	tqcolorGroup.setColor(TQColorGroup::Text, lvs->textColor());
	tqcolorGroup.setColor(TQColorGroup::Base, lvs->backgroundColor());
	monitor->setPalette(TQPalette(tqcolorGroup, tqcolorGroup, tqcolorGroup));

	setTitle(lvs->title());

	setModified(true);
}

void
ListView::applyStyle()
{
	TQColorGroup tqcolorGroup = monitor->tqcolorGroup();
	tqcolorGroup.setColor(TQColorGroup::Link, KSGRD::Style->firstForegroundColor());
	tqcolorGroup.setColor(TQColorGroup::Text, KSGRD::Style->secondForegroundColor());
	tqcolorGroup.setColor(TQColorGroup::Base, KSGRD::Style->backgroundColor());
	monitor->setPalette(TQPalette(tqcolorGroup, tqcolorGroup, tqcolorGroup));

	setModified(true);
}
