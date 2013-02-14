/*****************************************************************

Copyright (c) 2006 Stephan Kulow <coolo@novell.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "media_watcher.h"
#include <tdeapplication.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <dcopref.h>

MediaWatcher::MediaWatcher( TQObject *parent ) :
    TQObject( parent ), DCOPObject("mediawatcher")
{
    connectDCOPSignal( "kded", "mediamanager", "mediumAdded(TQString,bool)",
                       "slotMediumAdded(TQString,bool)", true );
    connectDCOPSignal( "kded", "mediamanager", "mediumRemoved(TQString,bool)",
                       "slotMediumAdded(TQString,bool)", true );
    connectDCOPSignal( "kded", "mediamanager", "mediumChanged(TQString,bool)",
                       "slotMediumAdded(TQString,bool)", true );

    updateDevices();
}

void MediaWatcher::updateDevices()
{
    DCOPRef nsd( "kded", "mediamanager" );
    nsd.setDCOPClient( kapp->dcopClient() );
    m_devices = nsd.call( "fullList" );
}

void MediaWatcher::slotMediumAdded( TQString item, bool a )
{
    updateDevices();

    emit mediumChanged();
}

#include "media_watcher.moc"
