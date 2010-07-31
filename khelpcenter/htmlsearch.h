#ifndef KHC_HTMLSEARCH_H
#define KHC_HTMLSEARCH_H

#include <tqobject.h>
#include <tqstring.h>

class KConfig;

namespace KHC {

class DocEntry;

class HTMLSearch : public QObject
{
    Q_OBJECT
  public:
    HTMLSearch();
    ~HTMLSearch();

    void setupDocEntry( KHC::DocEntry * );

    TQString defaultSearch( KHC::DocEntry * );
    TQString defaultIndexer( KHC::DocEntry * );
    TQString defaultIndexTestFile( KHC::DocEntry * );

  private:
    KConfig *mConfig;
};

}

#endif
// vim:ts=2:sw=2:et
