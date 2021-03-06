/*  -*- c++ -*-
    capabilities.cc

    This file is part of tdeio_smtp, the KDE SMTP tdeioslave.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config.h>

#include "capabilities.h"

#include "response.h"

#include <tqstrlist.h>

namespace KioSMTP {

  Capabilities Capabilities::fromResponse( const Response & ehlo ) {
    Capabilities c;

    // first, check whether the response was valid and indicates success:
    if ( !ehlo.isOk()
	 || ehlo.code() / 10 != 25 // ### restrict to 250 only?
	 || ehlo.lines().empty() )
      return c;

    QCStringList l = ehlo.lines();

    for ( QCStringList::const_iterator it = ++l.begin() ; it != l.end() ; ++it )
      c.add( *it );

    return c;
  }

  void Capabilities::add( const TQString & cap, bool replace ) {
    TQStringList tokens = TQStringList::split( ' ', cap.upper() );
    if ( tokens.empty() )
      return;
    TQString name = tokens.front(); tokens.pop_front();
    add( name, tokens, replace );
  }

  void Capabilities::add( const TQString & name, const TQStringList & args, bool replace ) {
    if ( replace )
      mCapabilities[name] = args;
    else
      mCapabilities[name] += args;
  }

  TQString Capabilities::asMetaDataString() const {
    TQString result;
    for ( TQMap<TQString,TQStringList>::const_iterator it = mCapabilities.begin() ; it != mCapabilities.end() ; ++it ) {
      result += it.key();
      if ( !it.data().empty() )
	result += ' ' + it.data().join( " " );
      result += '\n';
    }
    return result;
  }

  TQString Capabilities::authMethodMetaData() const {
    TQStringList sl = saslMethodsQSL();
    TQString result;
    for ( TQStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it )
      result += "SASL/" + *it + '\n';
    return result;
  }

  TQStrIList Capabilities::saslMethods() const {
    TQStrIList result( true ); // deep copies to be safe
    TQStringList sl = saslMethodsQSL();
    for ( TQStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it )
      result.append( (*it).latin1() );
    return result;
  }

  TQString Capabilities::createSpecialResponse( bool tls ) const {
    TQStringList result;
    if ( tls )
      result.push_back( "STARTTLS" );
    result += saslMethodsQSL();
    if ( have( "PIPELINING" ) )
      result.push_back( "PIPELINING" );
    if ( have( "8BITMIME" ) )
      result.push_back( "8BITMIME" );
    if ( have( "SIZE" ) ) {
      bool ok = false;
      unsigned int size = mCapabilities["SIZE"].front().toUInt( &ok );
      if ( ok && !size )
	result.push_back( "SIZE=*" ); // any size
      else if ( ok )
	result.push_back( "SIZE=" + TQString::number( size ) ); // fixed max
      else
	result.push_back( "SIZE" ); // indetermined
    }
    return result.join( " " );
  }

  TQStringList Capabilities::saslMethodsQSL() const {
    TQStringList result;
    for ( TQMap<TQString,TQStringList>::const_iterator it = mCapabilities.begin() ; it != mCapabilities.end() ; ++it ) {
      if ( it.key() == "AUTH" )
	result += it.data();
      else if ( it.key().startsWith( "AUTH=" ) ) {
	result.push_back( it.key().mid( tqstrlen("AUTH=") ) );
	result += it.data();
      }
    }
    result.sort();
    TQStringList::iterator it = result.begin();
    for (TQStringList::iterator ot = it++; it != result.end(); ot = it++)
        if (*ot == *it) result.remove(ot);
    return result;
  }



} // namespace KioSMTP

