/*
    This file is part of KHelpCenter.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "searchhandler.h"

#include "searchengine.h"
#include "prefs.h"
#include "docentry.h"

#include <kdesktopfile.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <stdlib.h>

using namespace KHC;

SearchHandler::SearchHandler()
{
  mLang = KGlobal::locale()->language().left( 2 );
}

SearchHandler *SearchHandler::initFromFile( const TQString &filename )
{
  SearchHandler *handler = new SearchHandler;

  KDesktopFile file( filename );

  handler->mSearchCommand = file.readEntry( "SearchCommand" );
  handler->mSearchUrl = file.readEntry( "SearchUrl" );
  handler->mIndexCommand = file.readEntry( "IndexCommand" );
  handler->mDocumentTypes = file.readListEntry( "DocumentTypes" );

  return handler;
}

TQStringList SearchHandler::documentTypes() const
{
  return mDocumentTypes;
}

TQString SearchHandler::indexCommand( const TQString &identifier )
{
  TQString cmd = mIndexCommand;
  cmd.tqreplace( "%i", identifier );
  cmd.tqreplace( "%d", Prefs::indexDirectory() );
  cmd.tqreplace( "%l", mLang );
  return cmd;
}

bool SearchHandler::checkPaths() const
{
  if ( !mSearchCommand.isEmpty() && !checkBinary( mSearchCommand ) )
    return false;

  if ( !mIndexCommand.isEmpty() && !checkBinary( mIndexCommand ) )
    return false;

  return true;
}

bool SearchHandler::checkBinary( const TQString &cmd ) const
{
  TQString binary;

  int pos = cmd.tqfind( ' ' );
  if ( pos < 0 ) binary = cmd;
  else binary = cmd.left( pos );

  return !KStandardDirs::findExe( binary ).isEmpty();
}

void SearchHandler::search( DocEntry *entry, const TQStringList &words,
  int maxResults,
  SearchEngine::Operation operation )
{
  kdDebug() << "SearchHandler::search(): " << entry->identifier() << endl;

  if ( !mSearchCommand.isEmpty() ) {
    TQString cmdString = SearchEngine::substituteSearchQuery( mSearchCommand,
      entry->identifier(), words, maxResults, operation, mLang );

    kdDebug() << "SearchHandler::search() CMD: " << cmdString << endl;

    KProcess *proc = new KProcess();

    TQStringList cmd = TQStringList::split( " ", cmdString );
    TQStringList::ConstIterator it;
    for( it = cmd.begin(); it != cmd.end(); ++it ) {
      TQString arg = *it;
      if ( arg.left( 1 ) == "\"" && arg.right( 1 ) =="\"" ) {
        arg = arg.mid( 1, arg.length() - 2 );
      }
      *proc << arg.utf8();
    }

    connect( proc, TQT_SIGNAL( receivedStdout( KProcess *, char *, int ) ),
             TQT_SLOT( searchStdout( KProcess *, char *, int ) ) );
    connect( proc, TQT_SIGNAL( receivedStderr( KProcess *, char *, int ) ),
             TQT_SLOT( searchStderr( KProcess *, char *, int ) ) );
    connect( proc, TQT_SIGNAL( processExited( KProcess * ) ),
             TQT_SLOT( searchExited( KProcess * ) ) );

    SearchJob *searchJob = new SearchJob;
    searchJob->mEntry = entry;
    searchJob->mProcess = proc;
    searchJob->mCmd = cmdString;

    mProcessJobs.insert( proc, searchJob );

    if ( !proc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
      TQString txt = i18n("Error executing search command '%1'.").arg( cmdString );
      emit searchFinished( this, entry, txt );
    }
  } else if ( !mSearchUrl.isEmpty() ) {
    TQString urlString = SearchEngine::substituteSearchQuery( mSearchUrl,
      entry->identifier(), words, maxResults, operation, mLang );
  
    kdDebug() << "SearchHandler::search() URL: " << urlString << endl;
  
    KIO::TransferJob *job = KIO::get( KURL( urlString ) );
    connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
             TQT_SLOT( slotJobResult( KIO::Job * ) ) );
    connect( job, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
             TQT_SLOT( slotJobData( KIO::Job *, const TQByteArray & ) ) );

    SearchJob *searchJob = new SearchJob;
    searchJob->mEntry = entry;
    searchJob->mKioJob = job;
    mKioJobs.insert( job, searchJob );
  } else {
    TQString txt = i18n("No search command or URL specified.");
    emit searchFinished( this, entry, txt );
    return;
  }
}

void SearchHandler::searchStdout( KProcess *proc, char *buffer, int len )
{
  if ( !buffer || len == 0 )
    return;

  TQString bufferStr;
  char *p;
  p = (char*) malloc( sizeof(char) * ( len + 1 ) );
  p = strncpy( p, buffer, len );
  p[len] = '\0';

  TQMap<KProcess *, SearchJob *>::ConstIterator it = mProcessJobs.tqfind( proc );
  if ( it != mProcessJobs.end() ) {
    (*it)->mResult += bufferStr.fromUtf8( p );
  }

  free( p );
}

void SearchHandler::searchStderr( KProcess *proc, char *buffer, int len )
{
  if ( !buffer || len == 0 )
    return;

  TQMap<KProcess *, SearchJob *>::ConstIterator it = mProcessJobs.tqfind( proc );
  if ( it != mProcessJobs.end() ) {
    (*it)->mError += TQString::fromUtf8( buffer, len );
  }
}

void SearchHandler::searchExited( KProcess *proc )
{
//  kdDebug() << "SearchHandler::searchExited()" << endl;

  TQString result;
  TQString error;
  DocEntry *entry = 0;

  TQMap<KProcess *, SearchJob *>::ConstIterator it = mProcessJobs.tqfind( proc );
  if ( it != mProcessJobs.end() ) {
    SearchJob *j = *it;
    entry = j->mEntry;
    result = j->mResult;
    error = "<em>" + j->mCmd + "</em>\n" + j->mError;
    
    mProcessJobs.remove( proc );
    delete j;
  } else {
    kdError() << "No search job for exited process found." << endl;
  }

  if ( proc->normalExit() && proc->exitStatus() == 0 ) {
    emit searchFinished( this, entry, result );
  } else {
    emit searchError( this, entry, error );
  }
}

void SearchHandler::slotJobResult( KIO::Job *job )
{
  TQString result;
  DocEntry *entry = 0;

  TQMap<KIO::Job *, SearchJob *>::ConstIterator it = mKioJobs.tqfind( job );
  if ( it != mKioJobs.end() ) {
    SearchJob *j = *it;

    entry = j->mEntry;
    result = j->mResult;    
    
    mKioJobs.remove( job );
    delete j;
  }

  if ( job->error() ) {
    emit searchError( this, entry, i18n("Error: %1").arg( job->errorString() ) );
  } else {
    emit searchFinished( this, entry, result );
  }
}
 
void SearchHandler::slotJobData( KIO::Job *job, const TQByteArray &data )
{
//  kdDebug() << "SearchHandler::slotJobData()" << endl;

  TQMap<KIO::Job *, SearchJob *>::ConstIterator it = mKioJobs.tqfind( job );
  if ( it != mKioJobs.end() ) {
    (*it)->mResult += data.data();
  }
}

#include "searchhandler.moc"
