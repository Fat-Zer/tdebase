/*
 * kcmsambastatistics.h
 *
 * Copyright (c) 2000 Alexander Neundorf <alexander.neundorf@rz.tu-ilmenau.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef kcmsambastatistics_h_included
#define kcmsambastatistics_h_included
 
#include <tqwidget.h>
#include <tqptrlist.h>

class TQListView;
class TQLabel;
class TQComboBox;
class TQCheckBox;
class TQLineEdit;
class TQPushButton;

class KConfig;

class SmallLogItem
{
 public:
  SmallLogItem():name(""),count(0){};
  SmallLogItem(TQString n):name(n),count(1){};
  TQString name;
  int count;
};

class LogItem
{
 public:
  LogItem():name(""), accessed(),count(0) {};
  LogItem(TQString n, TQString a):name(n), accessed(), count(1)
	{
	  accessed.setAutoDelete(TRUE);
	  accessed.append(new SmallLogItem(a));
	};
  TQString name;
  //TQStrList accessedBy;
  TQPtrList<SmallLogItem> accessed;
  int count;
  SmallLogItem* itemInList(TQString name);
  void addItem (TQString host);
};

class SambaLog
{
 public:
  SambaLog()
	{
	  items.setAutoDelete(TRUE);
	};
  TQPtrList<LogItem> items;
  void addItem (TQString share, TQString host);
  void printItems();
 private:
  LogItem* itemInList(TQString name);
};

class StatisticsView: public QWidget
{
  Q_OBJECT
public:
  StatisticsView(TQWidget *parent=0, KConfig *config=0, const char *name=0);
  virtual ~StatisticsView() {};
  void saveSettings() {};
  void loadSettings() {};
  public slots:
	void setListInfo(TQListView *list, int nrOfFiles, int nrOfConnections);
private:
  KConfig *configFile;
  TQListView *dataList;
  TQListView* viewStatistics;
  TQLabel* connectionsL, *filesL;
  TQComboBox* eventCb;
  TQLabel* eventL;
  TQLineEdit* serviceLe;
  TQLabel* serviceL;
  TQLineEdit* hostLe;
  TQLabel* hostL;
  TQPushButton* calcButton, *clearButton;
  TQCheckBox* expandedInfoCb, *expandedUserCb;
  int connectionsCount, filesCount, calcCount;
private slots:
	void clearStatistics();
  void calculate();
};
#endif // main_included
