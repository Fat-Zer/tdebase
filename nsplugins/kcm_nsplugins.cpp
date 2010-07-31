/*
  Copyright (c) 2003 Dirk Mueller <mueller@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <tqregexp.h>
#include <tqlayout.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kurlrequester.h>

#include <tqwhatsthis.h>
#include <tqvgroupbox.h>
#include <kdebug.h>

#include <kapplication.h>
#include <dcopclient.h>
#include <kprocio.h>

#include "plugin_paths.h"

static TQDateTime lastChanged( TQString dir )
{
    TQDateTime t = TQFileInfo( dir ).lastModified();
    if( t.isNull())
        return t;
    TQStringList subdirs = TQDir( dir ).entryList();
    for( TQStringList::ConstIterator it = subdirs.begin();
         it != subdirs.end();
         ++it )
    {
        if( *it == "." || *it == ".." )
            continue;
        TQDateTime t2 = lastChanged( *it );
        if( !t2.isNull() && t2 > t )
            t = t2;
    }
    return t;
}

static bool checkSearchPathTimestamps( TQStringList paths, TQStringList timestamps )
{
    TQStringList currentTimestamps;
    bool changed = false;
    TQStringList::ConstIterator t = timestamps.begin();
    for( TQStringList::ConstIterator it = paths.begin();
         it != paths.end();
         ++it, ++t )
    {
        TQDateTime current = lastChanged( *it );
        // store non-existent directory as "N" string rather than empty string, KConfig
        // has a bug with storing a list of empty items
        if( *t == "N" ? !current.isNull() : current != TQDateTime::fromString( *t, Qt::ISODate ))
            changed = true;
        currentTimestamps.append( current.isNull() ? "N" : current.toString( Qt::ISODate ));
    }
    if( changed )
    {
        KConfig config("kcmnspluginrc");
        config.setGroup("Misc");
        config.writeEntry( "lastSearchPaths", paths );
        config.writeEntry( "lastSearchTimestamps", currentTimestamps );
        return true;
    }
    return false;
}

extern "C"
{
    KDE_EXPORT void init_nsplugin()
    {
        KConfig *config = new KConfig("kcmnspluginrc", true /* readonly */, false /* no globals*/);
        config->setGroup("Misc");
        bool scan = config->readBoolEntry( "startkdeScan", false );
        bool firstTime = config->readBoolEntry( "firstTime", true );
        
        if( !scan )
        {
        // check if plugins have changed, as just ignoring everything and requiring the user
        // to trigger the check manually is not reasonable - that probably actually obsoletes
        // both options
            TQStringList searchPaths = getSearchPaths();
            TQStringList lastSearchPaths = config->readListEntry( "lastSearchPaths" );
            TQStringList lastTimestamps = config->readListEntry ( "lastSearchTimestamps" );
            if( searchPaths != lastSearchPaths || lastTimestamps.count() != lastSearchPaths.count())
            { // count changed, set empty timestamps, still call checkSearchPathTimestamps()
              // in order to save the current timestamps for the next time
                lastSearchPaths = searchPaths;
                lastTimestamps.clear();
                lastTimestamps.insert( lastTimestamps.end(), searchPaths.count(), "N" );
                scan = true;
            }
            if( checkSearchPathTimestamps( lastSearchPaths, lastTimestamps ))
                scan = true;
        }
        delete config;

        if ( scan || firstTime )
        {
            KApplication::kdeinitExec("nspluginscan");
        }

        if (firstTime) {
            config= new KConfig("kcmnspluginrc", false);
            config->setGroup("Misc");
            config->writeEntry( "firstTime", false );
            config->sync();
            delete config;
        }
    }
}
