#include <tqregexp.h>
#include <tqfileinfo.h>

#include <kdebug.h>
#include <kdesktopfile.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>

#include "prefs.h"

#include "docentry.h"

using namespace KHC;

DocEntry::DocEntry()
{
  init();
}

DocEntry::DocEntry( const TQString &name, const TQString &url,
                    const TQString &icon )
{
  init();

  mName = name;
  mUrl = url;
  mIcon = icon;
}

void DocEntry::init()
{
  mWeight = 0;
  mSearchEnabled = false;
  mDirectory = false;
  mParent = 0;
  mNextSibling = 0;
}

void DocEntry::setName( const TQString &name )
{
  mName = name;
}

TQString DocEntry::name() const
{
  return mName;
}

void DocEntry::setSearch( const TQString &search )
{
  mSearch = search;
}

TQString DocEntry::search() const
{
  return mSearch;
}

void DocEntry::setIcon( const TQString &icon )
{
  mIcon = icon;
}

TQString DocEntry::icon() const
{
  if ( !mIcon.isEmpty() ) return mIcon;

  if ( !docExists() ) return "unknown";

  if ( isDirectory() ) return "contents2";
  else return "text-x-generic-template";
}

void DocEntry::setUrl( const TQString &url )
{
  mUrl = url;
}

TQString DocEntry::url() const
{
  if ( !mUrl.isEmpty() ) return mUrl;

  if ( identifier().isEmpty() ) return TQString::null;

  return "khelpcenter:" + identifier();
}

void DocEntry::setInfo( const TQString &info )
{
  mInfo = info;
}

TQString DocEntry::info() const
{
  return mInfo;
}

void DocEntry::setLang( const TQString &lang )
{
  mLang = lang;
}

TQString DocEntry::lang() const
{
  return mLang;
}

void DocEntry::setIdentifier( const TQString &identifier )
{
  mIdentifier = identifier;
}

TQString DocEntry::identifier() const
{
  if ( mIdentifier.isEmpty() ) mIdentifier = TDEApplication::randomString( 15 );
  return mIdentifier;
}

void DocEntry::setIndexer( const TQString &indexer )
{
  mIndexer = indexer;
}

TQString DocEntry::indexer() const
{
  return mIndexer;
}

void DocEntry::setIndexTestFile( const TQString &indexTestFile )
{
  mIndexTestFile = indexTestFile;
}

TQString DocEntry::indexTestFile() const
{
  return mIndexTestFile;
}

void DocEntry::setWeight( int weight )
{
  mWeight = weight;
}

int DocEntry::weight() const
{
  return mWeight;
}

void DocEntry::setSearchMethod( const TQString &method )
{
  mSearchMethod = method;
}

TQString DocEntry::searchMethod() const
{
  return mSearchMethod;
}

void DocEntry::setDocumentType( const TQString &str )
{
  mDocumentType = str;
}

TQString DocEntry::documentType() const
{
  return mDocumentType;
}

TQString DocEntry::khelpcenterSpecial() const
{
  return mKhelpcenterSpecial;
}

void DocEntry::enableSearch( bool enabled )
{
  mSearchEnabled = enabled;
}

bool DocEntry::searchEnabled() const
{
  return mSearchEnabled;
}

void DocEntry::setSearchEnabledDefault( bool enabled )
{
  mSearchEnabledDefault = enabled;
}

bool DocEntry::searchEnabledDefault() const
{
  return mSearchEnabledDefault;
}

void DocEntry::setDirectory( bool dir )
{
  mDirectory = dir;
}

bool DocEntry::isDirectory() const
{
  return mDirectory;
}

bool DocEntry::readFromFile( const TQString &fileName )
{
  KDesktopFile file( fileName );

  mName = file.readName();
  mSearch = file.readEntry( "X-DOC-Search" );
  mIcon = file.readIcon();
  mUrl = file.readPathEntry( "X-DocPath" );
  mInfo = file.readEntry( "Info" );
  if ( mInfo.isNull() ) mInfo = file.readEntry( "Comment" );
  mLang = file.readEntry( "Lang", "en" );
  mIdentifier = file.readEntry( "X-DOC-Identifier" );
  if ( mIdentifier.isEmpty() ) {
    TQFileInfo fi( fileName );
    mIdentifier = fi.baseName( true );
  }
  mIndexer = file.readEntry( "X-DOC-Indexer" );
  mIndexer.replace( "%f", fileName );
  mIndexTestFile = file.readEntry( "X-DOC-IndexTestFile" );
  mSearchEnabledDefault = file.readBoolEntry( "X-DOC-SearchEnabledDefault",
                                              false );
  mSearchEnabled = mSearchEnabledDefault;
  mWeight = file.readNumEntry( "X-DOC-Weight", 0 );
  mSearchMethod = file.readEntry( "X-DOC-SearchMethod" );
  mDocumentType = file.readEntry( "X-DOC-DocumentType" );

  mKhelpcenterSpecial = file.readEntry("X-TDE-KHelpcenter-Special");

  return true;
}

bool DocEntry::indexExists( const TQString &indexDir )
{
  TQString testFile;
  if ( mIndexTestFile.isEmpty() ) {
    testFile = identifier() + ".exists";
  } else {
    testFile = mIndexTestFile;
  }

  if ( !testFile.startsWith( "/" ) ) testFile = indexDir + "/" + testFile;

  return TQFile::exists( testFile );
}

bool DocEntry::docExists() const
{
  if ( !mUrl.isEmpty() ) {
    KURL docUrl( mUrl );
    if ( docUrl.isLocalFile() && !TDEStandardDirs::exists( docUrl.path() ) ) {
//      kdDebug(1400) << "URL not found: " << docUrl.url() << endl;
      return false;
    }
  }

  return true;
}

void DocEntry::addChild( DocEntry *entry )
{
  entry->setParent( this );

  uint i;
  for( i = 0; i < mChildren.count(); ++i ) {
    if ( i == 0 ) {
      if ( entry->weight() < mChildren.first()->weight() ) {
        entry->setNextSibling( mChildren.first() );
        mChildren.prepend( entry );
        break;
      }
    }
    if ( i + 1 < mChildren.count() ) {
      if ( entry->weight() >= mChildren[ i ]->weight() &&
           entry->weight() < mChildren[ i + 1 ]->weight() ) {
        entry->setNextSibling( mChildren[ i + 1 ] );
        mChildren[ i ]->setNextSibling( entry );
        mChildren.insert( mChildren.at( i + 1 ), entry );
        break;
      }
    }
  }
  if ( i == mChildren.count() ) {
    if ( i > 0 ) {
      mChildren.last()->setNextSibling( entry );
    }
    mChildren.append( entry );
  }
}

bool DocEntry::hasChildren()
{
  return mChildren.count();
}

DocEntry *DocEntry::firstChild()
{
  return mChildren.first();
}

DocEntry::List DocEntry::children()
{
  return mChildren;
}

void DocEntry::setParent( DocEntry *parent )
{
  mParent = parent;
}

DocEntry *DocEntry::parent()
{
  return mParent;
}

void DocEntry::setNextSibling( DocEntry *next )
{
  mNextSibling = next;
}

DocEntry *DocEntry::nextSibling()
{
  return mNextSibling;
}

bool DocEntry::isSearchable()
{
  return !search().isEmpty() && docExists() &&
    indexExists( Prefs::indexDirectory() );
}

void DocEntry::dump() const
{
  kdDebug() << "  <docentry>" << endl;
  kdDebug() << "    <name>" << mName << "</name>" << endl;
  kdDebug() << "    <searchmethod>" << mSearchMethod << "</searchmethod>" << endl;
  kdDebug() << "    <search>" << mSearch << "</search>" << endl;
  kdDebug() << "    <indexer>" << mIndexer << "</indexer>" << endl;
  kdDebug() << "    <indextestfile>" << mIndexTestFile << "</indextestfile>" << endl;
  kdDebug() << "    <icon>" << mIcon << "</icon>" << endl;
  kdDebug() << "    <url>" << mUrl << "</url>" << endl;
  kdDebug() << "    <documenttype>" << mDocumentType << "</documenttype>" << endl; 
  kdDebug() << "  </docentry>" << endl;
}
// vim:ts=2:sw=2:et
