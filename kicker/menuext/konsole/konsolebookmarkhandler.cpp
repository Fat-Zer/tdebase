// Born as tdelibs/tdeio/tdefile/tdefilebookmarkhandler.cpp

#include <stdio.h>
#include <stdlib.h>

#include <tqtextstream.h>

#include <kbookmarkimporter.h>
#include <kmimetype.h>
#include <kpopupmenu.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include "konsole_mnu.h"
#include "konsolebookmarkmenu.h"
#include "konsolebookmarkhandler.h"

KonsoleBookmarkHandler::KonsoleBookmarkHandler( KonsoleMenu *konsole, bool )
    : TQObject( konsole, "KonsoleBookmarkHandler" ),
      KBookmarkOwner(),
      m_konsole( konsole ),
      m_importStream( 0L )
{
    m_menu = new TDEPopupMenu( konsole, "bookmark menu" );

    TQString file = locate( "data", "konsole/bookmarks.xml" );
    if ( file.isEmpty() )
        file = locateLocal( "data", "konsole/bookmarks.xml" );

    // import old bookmarks
    if ( !TDEStandardDirs::exists( file ) ) {
        TQString oldFile = locate( "data", "tdefile/bookmarks.html" );
        if ( !oldFile.isEmpty() )
            importOldBookmarks( oldFile, file );
    }

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);
    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    connect( manager, TQT_SIGNAL( changed(const TQString &, const TQString &) ),
             TQT_SLOT( slotBookmarksChanged(const TQString &, const TQString &) ) );
    m_bookmarkMenu = new KonsoleBookmarkMenu( manager, this, m_menu,
                             NULL, false, /*Not toplevel*/
			     false /*No 'Add Bookmark'*/ );
}

TQString KonsoleBookmarkHandler::currentURL() const
{
    return m_konsole->baseURL().url();
}

void KonsoleBookmarkHandler::importOldBookmarks( const TQString& path,
                                                 const TQString& destinationPath )
{
    KSaveFile file( destinationPath );
    if ( file.status() != 0 )
        return;

    m_importStream = file.textStream();
    *m_importStream << "<!DOCTYPE xbel>\n<xbel>\n";

    KNSBookmarkImporter importer( path );
    connect( &importer,
             TQT_SIGNAL( newBookmark( const TQString&, const TQCString&, const TQString& )),
             TQT_SLOT( slotNewBookmark( const TQString&, const TQCString&, const TQString& )));
    connect( &importer,
             TQT_SIGNAL( newFolder( const TQString&, bool, const TQString& )),
             TQT_SLOT( slotNewFolder( const TQString&, bool, const TQString& )));
    connect( &importer, TQT_SIGNAL( newSeparator() ), TQT_SLOT( newSeparator() ));
    connect( &importer, TQT_SIGNAL( endMenu() ), TQT_SLOT( endMenu() ));

    importer.parseNSBookmarks( false );

    *m_importStream << "</xbel>";

    file.close();
    m_importStream = 0L;
}

void KonsoleBookmarkHandler::slotNewBookmark( const TQString& /*text*/,
                                            const TQCString& url,
                                            const TQString& additionalInfo )
{
    *m_importStream << "<bookmark icon=\"" << KMimeType::iconForURL( KURL( url ) );
    *m_importStream << "\" href=\"" << TQString::fromUtf8(url) << "\">\n";
    *m_importStream << "<title>" << (additionalInfo.isEmpty() ? TQString(TQString::fromUtf8(url)) : additionalInfo) << "</title>\n</bookmark>\n";
}

void KonsoleBookmarkHandler::slotNewFolder( const TQString& text, bool /*open*/,
                                          const TQString& /*additionalInfo*/ )
{
    *m_importStream << "<folder icon=\"bookmark_folder\">\n<title=\"";
    *m_importStream << text << "\">\n";
}

void KonsoleBookmarkHandler::slotBookmarksChanged( const TQString &,
                                                   const TQString & )
{
    // This is called when someone changes bookmarks in konsole....
    m_bookmarkMenu->slotBookmarksChanged("");
}

void KonsoleBookmarkHandler::newSeparator()
{
    *m_importStream << "<separator/>\n";
}

void KonsoleBookmarkHandler::endFolder()
{
    *m_importStream << "</folder>\n";
}

void KonsoleBookmarkHandler::virtual_hook( int id, void* data )
{ KBookmarkOwner::virtual_hook( id, data ); }

#include "konsolebookmarkhandler.moc"
