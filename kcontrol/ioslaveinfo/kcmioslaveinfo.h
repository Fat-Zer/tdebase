/*
 * kcmioslaveinfo.h
 *
 * Copyright 2001 Alexander Neundorf <alexander.neundorf@rz.tu-ilmenau.de>
 * Copyright 2001 George Staikos  <staikos@kde.org>
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
#ifndef kcmioslaveinfo_h_included
#define kcmioslaveinfo_h_included

#include <tqlistbox.h>
#include <tqstring.h>

#include <kaboutdata.h>
#include <tdecmodule.h>
#include <kio/job.h>
#include <klistbox.h>
#include <ktextbrowser.h>

class KIOTimeoutControl;
class TQTabWidget;
class TQSpinBox;
class TDEConfig;

class KCMIOSlaveInfo : public TDECModule
{
    Q_OBJECT
public:
    KCMIOSlaveInfo(TQWidget *parent = 0L, const char *name = 0L, const TQStringList &lits=TQStringList() );

protected:
    KListBox *m_ioslavesLb;
    KTextBrowser *m_info;
    TQCString helpData;
    TDEIO::Job *m_tfj;

protected slots:

    void showInfo(const TQString& protocol);
    void showInfo(TQListBoxItem *item);
    void slaveHelp( TDEIO::Job *, const TQByteArray &data);
    void slotResult( TDEIO::Job * );

};
#endif
