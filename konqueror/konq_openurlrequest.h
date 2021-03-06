#ifndef __konqopenurlrequest_h
#define __konqopenurlrequest_h

#include <tqstringlist.h>

#include <tdeparts/browserextension.h>

struct KonqOpenURLRequest {

  KonqOpenURLRequest() :
      followMode(false), newTab(false), newTabInFront(false),
      openAfterCurrentPage(false), forceAutoEmbed(false),
      tempFile(false), userRequestedReload(false) {}

  KonqOpenURLRequest( const TQString & url ) :
    typedURL(url), followMode(false), newTab(false), newTabInFront(false),
    openAfterCurrentPage(false), forceAutoEmbed(false),
    tempFile(false), userRequestedReload(false) {}

  TQString debug() const {
#ifndef NDEBUG
      TQStringList s;
      if ( !args.frameName.isEmpty() )
          s << "frameName=" + args.frameName;
      if ( !nameFilter.isEmpty() )
          s << "nameFilter=" + nameFilter;
      if ( !typedURL.isEmpty() )
          s << "typedURL=" + typedURL;
      if ( followMode )
          s << "followMode";
      if ( newTab )
          s << "newTab";
      if ( newTabInFront )
          s << "newTabInFront";
      if ( openAfterCurrentPage )
          s << "openAfterCurrentPage";
      if ( forceAutoEmbed )
          s << "forceAutoEmbed";
      if ( tempFile )
          s << "tempFile";
      if ( userRequestedReload )
          s << "userRequestedReload";
      return "[" + s.join(" ") + "]";
#else
      return TQString::null;
#endif
  }

  TQString typedURL; // empty if URL wasn't typed manually
  TQString nameFilter; // like *.cpp, extracted from the URL
  bool followMode; // true if following another view - avoids loops
  bool newTab; // open url in new tab
  bool newTabInFront; // new tab in front or back
  bool openAfterCurrentPage;
  bool forceAutoEmbed; // if true, override the user's FMSettings for embedding
  bool tempFile; // if true, the url should be deleted after use
  bool userRequestedReload; // args.reload because the user requested it, not a website
  KParts::URLArgs args;
  TQStringList filesToSelect; // files to select in a konqdirpart

  static KonqOpenURLRequest null;
};

#endif
