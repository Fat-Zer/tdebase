/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2005 Lubos Lunak <l.lunak@kde.org>

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "client.h"

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#include <kstaticdeleter.h>

#include "server.h"

KSMClient::KSMClient( SmsConn conn)
{
    smsConn = conn;
    id = 0;
    resetState();
}

KSMClient::~KSMClient()
{
    for ( SmProp* prop = properties.first(); prop; prop = properties.next() )
        SmFreeProperty( prop );
    if (id) free((void*)id);
}

SmProp* KSMClient::property( const char* name ) const
{
    for ( TQPtrListIterator<SmProp> it( properties ); it.current(); ++it ) {
        if ( !qstrcmp( it.current()->name, name ) )
            return it.current();
    }
    return 0;
}

void KSMClient::resetState()
{
    saveYourselfDone = false;
    pendingInteraction = false;
    waitForPhase2 = false;
    wasPhase2 = false;
}

/*
 * This fakes SmsGenerateClientID() in case we can't read our own hostname.
 * In this case SmsGenerateClientID() returns NULL, but we really want a
 * client ID, so we fake one.
 */
static KStaticDeleter<TQString> smy_addr;
static char * safeSmsGenerateClientID( SmsConn /*c*/ )
{
//  Causes delays with misconfigured network :-/.
//    char *ret = SmsGenerateClientID(c);
    char* ret = NULL;
    if (!ret) {
        static TQString *my_addr = 0;
       if (!my_addr) {
//           tqWarning("Can't get own host name. Your system is severely misconfigured\n");
           smy_addr.setObject(my_addr,new TQString);

           /* Faking our IP address, the 0 below is "unknown" address format
              (1 would be IP, 2 would be DEC-NET format) */
           char hostname[ 256 ];
           if( gethostname( hostname, 255 ) != 0 )
               my_addr->sprintf("0%.8x", TDEApplication::random());
           else {
               // create some kind of hash for the hostname
               int addr[ 4 ] = { 0, 0, 0, 0 };
               int pos = 0;
               for( unsigned int i = 0;
                    i < strlen( hostname );
                    ++i, ++pos )
                  addr[ pos % 4 ] += hostname[ i ];
               *my_addr = "0";
               for( int i = 0;
                    i < 4;
                    ++i )
                  *my_addr += TQString::number( addr[ i ], 16 );
           }
       }
       /* Needs to be malloc(), to look the same as libSM */
       ret = (char *)malloc(1+my_addr->length()+13+10+4+1 + /*safeness*/ 10);
       static int sequence = 0;

       if (ret == NULL)
           return NULL;

       sprintf(ret, "1%s%.13ld%.10d%.4d", my_addr->latin1(), (long)time(NULL),
           getpid(), sequence);
       sequence = (sequence + 1) % 10000;
    }
    return ret;
}

void KSMClient::registerClient( const char* previousId )
{
    id = previousId;
    if ( !id )
        id = safeSmsGenerateClientID( smsConn );
    SmsRegisterClientReply( smsConn, (char*) id );
    SmsSaveYourself( smsConn, SmSaveLocal, false, SmInteractStyleNone, false );
    SmsSaveComplete( smsConn );
    KSMServer::self()->clientRegistered( previousId  );
}


TQString KSMClient::program() const
{
    SmProp* p = property( SmProgram );
    if ( !p || qstrcmp( p->type, SmARRAY8) || p->num_vals < 1)
        return TQString::null;
    return TQString::fromLatin1( (const char*) p->vals[0].value );
}

TQStringList KSMClient::restartCommand() const
{
    TQStringList result;
    SmProp* p = property( SmRestartCommand );
    if ( !p || qstrcmp( p->type, SmLISTofARRAY8) || p->num_vals < 1)
        return result;
    for ( int i = 0; i < p->num_vals; i++ )
        result +=TQString::fromLatin1( (const char*) p->vals[i].value );
    return result;
}

TQStringList KSMClient::discardCommand() const
{
    TQStringList result;
    SmProp* p = property( SmDiscardCommand );
    if ( !p || qstrcmp( p->type, SmLISTofARRAY8) || p->num_vals < 1)
        return result;
    for ( int i = 0; i < p->num_vals; i++ )
        result +=TQString::fromLatin1( (const char*) p->vals[i].value );
    return result;
}

int KSMClient::restartStyleHint() const
{
    SmProp* p = property( SmRestartStyleHint );
    if ( !p || qstrcmp( p->type, SmCARD8) || p->num_vals < 1)
        return SmRestartIfRunning;
    return *((int*)p->vals[0].value);
}

TQString KSMClient::userId() const
{
    SmProp* p = property( SmUserID );
    if ( !p || qstrcmp( p->type, SmARRAY8) || p->num_vals < 1)
        return TQString::null;
    return TQString::fromLatin1( (const char*) p->vals[0].value );
}


