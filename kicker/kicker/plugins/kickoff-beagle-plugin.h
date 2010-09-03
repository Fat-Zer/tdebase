/***************************************************************************
 *   Copyright (C) 2006 by Stephan Binner <binner@kde.org>                 *
 *   Copyright (c) 2006 Debajyoti Bera <dbera.web@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef CAPITALIZEPLUGIN_H
#define CAPITALIZEPLUGIN_H

#include "../interfaces/kickoff-search-plugin.h"
#include "beaglesearch.h"

class KickoffBeaglePlugin :public KickoffSearch::Plugin
{
    Q_OBJECT

public:
    KickoffBeaglePlugin(TQObject *parent, const char* name, const TQStringList&);

    void query(TQString, bool);
    bool daemonRunning();

protected slots:
    // to clean beaglesearchclients
    void cleanClientList ();

private:
    TQString current_query_str;

    // all beagle activity is done through the BSC object
    BeagleSearchClient *current_beagle_client;

    // used to send notification from the beagle thread to the main event loop
    virtual void customEvent (TQCustomEvent *e);

    TQPtrList<BeagleSearchClient> toclean_client_list;
    TQMutex toclean_list_mutex;

    // show the results
    void showResults (BeagleSearchResult *);
    HitMenuItem *hitToHitMenuItem (int category, Hit *hit);

    // use a different id for each bsc client, and use that to separate stale responses from current ones
    int current_beagle_client_id;

    bool genericTitle;
    TQDateTime datetimeFromString( const TQString& );
};

#endif /* CAPITALIZEPLUGIN_H */
