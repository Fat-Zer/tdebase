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

*/

#include <tdeapplication.h>
#include <kiconloader.h>
#include <tdelocale.h>

#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include <tqfile.h>

#include "SensorLogger.moc"
#include "SensorLoggerSettings.h"

SLListViewItem::SLListViewItem(TQListView *parent)
	: TQListViewItem(parent)
{
}

LogSensor::LogSensor(TQListView *parent)
	: timerID( NONE ), lowerLimitActive( 0 ), upperLimitActive( 0 ),
	  lowerLimit( 0 ), upperLimit( 0 )
{
	TQ_CHECK_PTR(parent);

	monitor = parent;

	lvi = new SLListViewItem(monitor);
	TQ_CHECK_PTR(lvi);

	pixmap_running = UserIcon( "running" );
	pixmap_waiting = UserIcon( "waiting" );

	lvi->setPixmap(0, pixmap_waiting);
	lvi->setTextColor(monitor->colorGroup().text());

	monitor->insertItem(lvi);
}

LogSensor::~LogSensor(void)
{
	if ((lvi) && (monitor))
		delete lvi;
}

void
LogSensor::startLogging(void)
{
	lvi->setPixmap(0, pixmap_running);
	timerOn();
}

void
LogSensor::stopLogging(void)
{
	lvi->setPixmap(0, pixmap_waiting);
	lvi->setTextColor(monitor->colorGroup().text());
	lvi->repaint();
	timerOff();
}

void
LogSensor::timerEvent(TQTimerEvent*)
{
	KSGRD::SensorMgr->sendRequest(hostName, sensorName, (KSGRD::SensorClient*) this, 42);
}

void
LogSensor::answerReceived(int id, const TQString& answer)
{
	TQFile logFile(fileName);

	if (!logFile.open(IO_ReadWrite | IO_Append))
	{
		stopLogging();
		return;
	}

	switch (id)
	{
		case 42: {
			TQTextStream stream(&logFile);
			double value = answer.toDouble();

			if (lowerLimitActive && value < lowerLimit)
			{
				timerOff();
				lowerLimitActive = false;
				lvi->setTextColor(monitor->colorGroup().foreground());
				lvi->repaint();
				KNotifyClient::event(monitor->winId(), "sensor_alarm", TQString("sensor '%1' at '%2' reached lower limit").arg(sensorName).arg(hostName));
				timerOn();
			} else if (upperLimitActive && value > upperLimit)
			{
				timerOff();
				upperLimitActive = false;
				lvi->setTextColor(monitor->colorGroup().foreground());
				lvi->repaint();
				KNotifyClient::event(monitor->winId(), "sensor_alarm", TQString("sensor '%1' at '%2' reached upper limit").arg(sensorName).arg(hostName));
				timerOn();
			}
			TQDate date = TQT_TQDATE_OBJECT(TQDateTime::currentDateTime().date());
			TQTime time = TQT_TQTIME_OBJECT(TQDateTime::currentDateTime().time());

			stream << TQString("%1 %2 %3 %4 %5: %6\n").arg(date.shortMonthName(date.month())).arg(date.day()).arg(time.toString()).arg(hostName).arg(sensorName).arg(value);
		}
	}

	logFile.close();
}

SensorLogger::SensorLogger(TQWidget *parent, const char *name, const TQString& title)
	: KSGRD::SensorDisplay(parent, name, title)
{
	monitor = new TQListView(this, "monitor");
	TQ_CHECK_PTR(monitor);

	monitor->addColumn(i18n("Logging"));
	monitor->addColumn(i18n("Timer Interval"));
	monitor->addColumn(i18n("Sensor Name"));
	monitor->addColumn(i18n("Host Name"));
	monitor->addColumn(i18n("Log File"));

	TQColorGroup cgroup = monitor->colorGroup();
	cgroup.setColor(TQColorGroup::Text, KSGRD::Style->firstForegroundColor());
	cgroup.setColor(TQColorGroup::Base, KSGRD::Style->backgroundColor());
	cgroup.setColor(TQColorGroup::Foreground, KSGRD::Style->alarmColor());
	monitor->setPalette(TQPalette(cgroup, cgroup, cgroup));
	monitor->setSelectionMode(TQListView::NoSelection);

	connect(monitor, TQT_SIGNAL(rightButtonClicked(TQListViewItem*, const TQPoint&, int)), this, TQT_SLOT(RMBClicked(TQListViewItem*, const TQPoint&, int)));

	setTitle(i18n("Sensor Logger"));

	logSensors.setAutoDelete(true);

	setPlotterWidget(monitor);

	setMinimumSize(50, 25);
	setModified(false);
}

SensorLogger::~SensorLogger(void)
{
}

bool
SensorLogger::addSensor(const TQString& hostName, const TQString& sensorName, const TQString& sensorType, const TQString&)
{
	if (sensorType != "integer" && sensorType != "float")
		return (false);

	sld = new SensorLoggerDlg(this, "SensorLoggerDlg");
	TQ_CHECK_PTR(sld);

	if (sld->exec()) {
		if (!sld->fileName().isEmpty()) {
			LogSensor *sensor = new LogSensor(monitor);
			TQ_CHECK_PTR(sensor);

			sensor->setHostName(hostName);
			sensor->setSensorName(sensorName);
			sensor->setFileName(sld->fileName());
			sensor->setTimerInterval(sld->timerInterval());
			sensor->setLowerLimitActive(sld->lowerLimitActive());
			sensor->setUpperLimitActive(sld->upperLimitActive());
			sensor->setLowerLimit(sld->lowerLimit());
			sensor->setUpperLimit(sld->upperLimit());

			logSensors.append(sensor);

			setModified(true);
		}
	}

	delete sld;
	sld = 0;

	return (true);
}

bool
SensorLogger::editSensor(LogSensor* sensor)
{
	sld = new SensorLoggerDlg(this, "SensorLoggerDlg");
	TQ_CHECK_PTR(sld);

	sld->setFileName(sensor->getFileName());
	sld->setTimerInterval(sensor->getTimerInterval());
	sld->setLowerLimitActive(sensor->getLowerLimitActive());
	sld->setLowerLimit(sensor->getLowerLimit());
	sld->setUpperLimitActive(sensor->getUpperLimitActive());
	sld->setUpperLimit(sensor->getUpperLimit());

	if (sld->exec()) {
		if (!sld->fileName().isEmpty()) {
			sensor->setFileName(sld->fileName());
			sensor->setTimerInterval(sld->timerInterval());
			sensor->setLowerLimitActive(sld->lowerLimitActive());
			sensor->setUpperLimitActive(sld->upperLimitActive());
			sensor->setLowerLimit(sld->lowerLimit());
			sensor->setUpperLimit(sld->upperLimit());

			setModified(true);
		}
	}

	delete sld;
	sld = 0;

	return (true);
}

void
SensorLogger::configureSettings()
{
	TQColorGroup cgroup = monitor->colorGroup();

	sls = new SensorLoggerSettings(this, "SensorLoggerSettings");
	TQ_CHECK_PTR(sls);

	connect( sls, TQT_SIGNAL( applyClicked() ), TQT_SLOT( applySettings() ) );

	sls->setTitle(title());
	sls->setForegroundColor(cgroup.text());
	sls->setBackgroundColor(cgroup.base());
	sls->setAlarmColor(cgroup.foreground());

	if (sls->exec())
		applySettings();

	delete sls;
	sls = 0;
}

void
SensorLogger::applySettings()
{
	TQColorGroup cgroup = monitor->colorGroup();

	setTitle(sls->title());

	cgroup.setColor(TQColorGroup::Text, sls->foregroundColor());
	cgroup.setColor(TQColorGroup::Base, sls->backgroundColor());
	cgroup.setColor(TQColorGroup::Foreground, sls->alarmColor());
	monitor->setPalette(TQPalette(cgroup, cgroup, cgroup));

	setModified(true);
}

void
SensorLogger::applyStyle(void)
{
	TQColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(TQColorGroup::Text, KSGRD::Style->firstForegroundColor());
	cgroup.setColor(TQColorGroup::Base, KSGRD::Style->backgroundColor());
	cgroup.setColor(TQColorGroup::Foreground, KSGRD::Style->alarmColor());
	monitor->setPalette(TQPalette(cgroup, cgroup, cgroup));

	setModified(true);
}

bool
SensorLogger::restoreSettings(TQDomElement& element)
{
	TQColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(TQColorGroup::Text, restoreColor(element, "textColor", Qt::green));
	cgroup.setColor(TQColorGroup::Base, restoreColor(element, "backgroundColor", Qt::black));
	cgroup.setColor(TQColorGroup::Foreground, restoreColor(element, "alarmColor", Qt::red));
	monitor->setPalette(TQPalette(cgroup, cgroup, cgroup));

	logSensors.clear();

	TQDomNodeList dnList = element.elementsByTagName("logsensors");
	for (uint i = 0; i < dnList.count(); i++) {
		TQDomElement element = dnList.item(i).toElement();
		LogSensor* sensor = new LogSensor(monitor);
		TQ_CHECK_PTR(sensor);

		sensor->setHostName(element.attribute("hostName"));
		sensor->setSensorName(element.attribute("sensorName"));
		sensor->setFileName(element.attribute("fileName"));
		sensor->setTimerInterval(element.attribute("timerInterval").toInt());
		sensor->setLowerLimitActive(element.attribute("lowerLimitActive").toInt());
		sensor->setLowerLimit(element.attribute("lowerLimit").toDouble());
		sensor->setUpperLimitActive(element.attribute("upperLimitActive").toInt());
		sensor->setUpperLimit(element.attribute("upperLimit").toDouble());

		logSensors.append(sensor);
	}

	SensorDisplay::restoreSettings(element);

	setModified(false);

	return (true);
}

bool
SensorLogger::saveSettings(TQDomDocument& doc, TQDomElement& element, bool save)
{
	saveColor(element, "textColor", monitor->colorGroup().text());
	saveColor(element, "backgroundColor", monitor->colorGroup().base());
	saveColor(element, "alarmColor", monitor->colorGroup().foreground());

	for (LogSensor* sensor = logSensors.first(); sensor != 0; sensor = logSensors.next())
	{
		TQDomElement log = doc.createElement("logsensors");
		log.setAttribute("sensorName", sensor->getSensorName());
		log.setAttribute("hostName", sensor->getHostName());
		log.setAttribute("fileName", sensor->getFileName());
		log.setAttribute("timerInterval", sensor->getTimerInterval());
		log.setAttribute("lowerLimitActive", TQString("%1").arg(sensor->getLowerLimitActive()));
		log.setAttribute("lowerLimit", TQString("%1").arg(sensor->getLowerLimit()));
		log.setAttribute("upperLimitActive", TQString("%1").arg(sensor->getUpperLimitActive()));
		log.setAttribute("upperLimit", TQString("%1").arg(sensor->getUpperLimit()));

		element.appendChild(log);
	}

	SensorDisplay::saveSettings(doc, element);

	if (save)
		setModified(false);

	return (true);
}

void
SensorLogger::answerReceived(int, const TQString&)
{
 // we do not use this, since all answers are received by the LogSensors
}

void
SensorLogger::resizeEvent(TQResizeEvent*)
{
	frame()->setGeometry(0, 0, this->width(), this->height());
	monitor->setGeometry(10, 20, this->width() - 20, this->height() - 30);
}

LogSensor*
SensorLogger::getLogSensor(TQListViewItem* item)
{
	for (LogSensor* sensor = logSensors.first(); sensor != 0; sensor = logSensors.next())
	{
		if (item == sensor->getListViewItem()) {
			return sensor;
		}
	}

	return NULL;
}

void
SensorLogger::RMBClicked(TQListViewItem* item, const TQPoint& point, int)
{
	TQPopupMenu pm;
	if (hasSettingsDialog())
		pm.insertItem(i18n("&Properties"), 1);
	pm.insertItem(i18n("&Remove Display"), 2);
	pm.insertSeparator(-1);
	pm.insertItem(i18n("&Remove Sensor"), 3);
	pm.insertItem(i18n("&Edit Sensor..."), 4);

	if ( !item )
	{
		pm.setItemEnabled( 3, false );
		pm.setItemEnabled( 4, false );
	}
	else
	{
		LogSensor* sensor = getLogSensor(item);

		if ( sensor->isLogging() )
			pm.insertItem(i18n("St&op Logging"), 6);
		else
			pm.insertItem(i18n("S&tart Logging"), 5);
	}

	switch (pm.exec(point))
	{
	case 1:
		configureSettings();
		break;
	case 2: {
		TQCustomEvent* ev = new TQCustomEvent(TQEvent::User);
		ev->setData(this);
		kapp->postEvent(parent(), ev);
		break;
		}
	case 3:	{
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			logSensors.remove(sensor);
		break;
		}
	case 4: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			editSensor(sensor);
		break;
		}
	case 5: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			sensor->startLogging();
		break;
		}
	case 6: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			sensor->stopLogging();
		break;
		}
	}
}
