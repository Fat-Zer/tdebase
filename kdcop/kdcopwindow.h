/*
 * Copyright (C) 2000 by Matthias Kalle Dalheimer <kalle@kde.org>
 *
 * Licensed under the Artistic License.
 */

#ifndef __KDCOPWINDOW_H__
#define __KDCOPWINDOW_H__

class DCOPClient;
class QListViewItem;
class KAction;
class KSelectAction;
class QWidgetStack;
class QLabel;
class KDCOPListView;

#include <kmainwindow.h>
#include "kdcoplistview.h"
#include "kdcopview.h"

class KDCOPWindow : public KMainWindow
{
  Q_OBJECT

  public:

    KDCOPWindow( TQWidget* parent = 0, const char* name = 0 );

  protected slots:

    void slotCurrentChanged( TQListViewItem* item );
    void slotCallFunction();
    void slotCallFunction( TQListViewItem* item );
    void slotApplicationRegistered(const TQCString &);
    void slotApplicationUnregistered(const TQCString &);
    void slotFillApplications();
    void slotCopy();
    void slotMode();
    void slotReload();
  private:
    void fillObjects( DCOPBrowserItem*, const char* app );
    void fillFunctions( DCOPBrowserItem*, const char* app, const char* obj );

    bool getParameters
      (
       const TQString  & unNormalisedSignature,
       TQString        & normalisedSignature,
       TQStringList    & types,
       TQStringList    & names
      );

    bool demarshal(TQCString & replyType, TQDataStream & reply, TQListBox *theList);

    DCOPClient    * dcopClient;
    KAction       * exeaction;
    KSelectAction * langmode;
    kdcopview	  * mainView;
    TQVBoxLayout  * mainLayout;
};



#endif
