#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstring.h>

#include <kpixmap.h>
#include <kio/job.h>

#include "docentrytraverser.h"

class TQWidget;
class KProcess;
class KConfig;
class KHTMLPart;

namespace KHC {

class Formatter;
class SearchEngine;
class View;
class SearchHandler;

class SearchTraverser : public TQObject, public DocEntryTraverser
{
    Q_OBJECT
  public:
    SearchTraverser( SearchEngine *engine, int level );
    ~SearchTraverser();

    void process( DocEntry * );
    
    void startProcess( DocEntry * );

    DocEntryTraverser *createChild( DocEntry * );

    DocEntryTraverser *parentTraverser();

    void deleteTraverser();

    void finishTraversal();

  protected:
    void connectHandler( SearchHandler *handler );
    void disconnectHandler( SearchHandler *handler );

  protected slots:
    void showSearchResult( SearchHandler *, DocEntry *, const TQString &result );
    void showSearchError( SearchHandler *, DocEntry *, const TQString &error );

  private:
    const int mMaxLevel;
  
    SearchEngine *mEngine;
    int mLevel;

    DocEntry *mEntry;
    TQString mJobData;
    
    TQString mResult;
    
    TQMap<SearchHandler *, int> mConnectCount;
};


class SearchEngine : public TQObject
{
    Q_OBJECT
  public:
    enum Operation { And, Or };

    SearchEngine( View * );
    ~SearchEngine();

    bool initSearchHandlers();

    bool search( TQString words, TQString method = "and", int matches = 5,
                 TQString scope = "" );

    Formatter *formatter() const; 
    View *view() const;

    TQString substituteSearchQuery( const TQString &query );

    static TQString substituteSearchQuery( const TQString &query,
      const TQString &identifier, const TQStringList &words, int maxResults,
      Operation operation, const TQString &lang );

    void finishSearch();

    /**
      Append error message to error log.
    */
    void logError( DocEntry *entry, const TQString &msg );

    /**
      Return error log.
    */
    TQString errorLog() const;

    bool isRunning() const;

    SearchHandler *handler( const TQString &documentType ) const;

    TQStringList words() const;
    int maxResults() const;
    Operation operation() const;

    bool canSearch( DocEntry * );
    bool needsIndex( DocEntry * );

  signals:
    void searchFinished();

  protected slots:
    void searchStdout(KProcess *proc, char *buffer, int buflen);
    void searchStderr(KProcess *proc, char *buffer, int buflen);
    void searchExited(KProcess *proc);

  protected:
    void processSearchQueue();
    
  private:
    KProcess *mProc;
    bool mSearchRunning;
    TQString mSearchResult;

    TQString mStderr;

    View *mView;
    
    TQString mWords;
    int mMatches;
    TQString mMethod;
    TQString mLang;
    TQString mScope;

    TQStringList mWordList;
    int mMaxResults;
    Operation mOperation;
    
    DocEntryTraverser *mRootTraverser;

    TQMap<TQString, SearchHandler *> mHandlers;
};

}

#endif
// vim:ts=2:sw=2:et
