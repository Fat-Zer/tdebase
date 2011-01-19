/*
    localdomainfilter.cpp

    This file is part of the KDE project
    Copyright (C) 2002 Lubos Lunak <llunak@suse.cz>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <config.h>

#include "localdomainurifilter.h"

#include <kprocess.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <tqregexp.h>
#include <tqfile.h>

#define HOSTPORT_PATTERN "[a-zA-Z0-9][a-zA-Z0-9+-]*(?:\\:[0-9]{1,5})?(?:/[\\w:@&=+$,-.!~*'()]*)*"

/**
 * IMPORTANT: If you change anything here, please run the regression test
 * kdelibs/kio/tests/kurifiltertest
 */
 
LocalDomainURIFilter::LocalDomainURIFilter( TQObject *parent, const char *name,
                                            const TQStringList & /*args*/ )
    : KURIFilterPlugin( parent, name ? name : "localdomainurifilter", 1.0 ),
      DCOPObject( "LocalDomainURIFilterIface" ),
      last_time( 0 ),
      m_hostPortPattern( TQString::tqfromLatin1(HOSTPORT_PATTERN) )
{
    configure();
}

bool LocalDomainURIFilter::filterURI( KURIFilterData& data ) const
{
    KURL url = data.uri();
    TQString cmd = url.url();
    
    kdDebug() << "LocalDomainURIFilter::filterURI: " << url << endl;
    
    if( m_hostPortPattern.exactMatch( cmd ) && 
        isLocalDomainHost( cmd ) )
    {
        cmd.prepend( TQString::tqfromLatin1("http://") );
        setFilteredURI( data, KURL( cmd ) );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        
        kdDebug() << "FilteredURI: " << data.uri() << endl;
        return true;
    }

    return false;
}

// if it's e.g. just 'www', try if it's a hostname in the local search domain
bool LocalDomainURIFilter::isLocalDomainHost( TQString& cmd ) const
{
    // find() returns -1 when no match -> left()/truncate() are noops then
    TQString host( cmd.left( cmd.tqfind( '/' ) ) );
    host.truncate( host.tqfind( ':' ) ); // Remove port number

    if( !(host == last_host && last_time > time( NULL ) - 5 ) ) {

        TQString helper = KStandardDirs::findExe(TQString::tqfromLatin1( "klocaldomainurifilterhelper" ));
        if( helper.isEmpty())
            return last_result = false;

        m_fullname = TQString::null;

        KProcess proc;
        proc << helper << host;
        connect( &proc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),
                 TQT_SLOT(receiveOutput(KProcess *, char *, int)) );
        if( !proc.start( KProcess::NotifyOnExit, KProcess::Stdout ))
            return last_result = false;

        last_host = host;
        last_time = time( (time_t *)0 );

        last_result = proc.wait( 1 ) && proc.normalExit() && !proc.exitStatus();

        if( !m_fullname.isEmpty() )
            cmd.tqreplace( 0, host.length(), m_fullname );
    }

    return last_result;
}

void LocalDomainURIFilter::receiveOutput( KProcess *, char *buf, int )
{
    m_fullname = TQFile::decodeName( buf );
}

void LocalDomainURIFilter::configure()
{
    // nothing
}

K_EXPORT_COMPONENT_FACTORY( liblocaldomainurifilter,
                            KGenericFactory<LocalDomainURIFilter>( "kcmkurifilt" ) )

#include "localdomainurifilter.moc"
