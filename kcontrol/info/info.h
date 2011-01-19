#ifndef _INFO_H_
#define _INFO_H_

#include <tqwidget.h>
#include <tqwidgetstack.h>
#include <tqframe.h>
#include <tqlabel.h>
#include <tqtabdialog.h>
#include <tqpushbutton.h>
#include <tqtimer.h>
#include <tqlistview.h>
#include <tqfile.h>
#include <tqevent.h>

#include <kcmodule.h>
#include <kaboutdata.h>

#include "config.h"

/* function call-back-prototypes... */

bool GetInfo_CPU( TQListView *lBox );
bool GetInfo_IRQ( TQListView *lBox );
bool GetInfo_DMA( TQListView *lBox );
bool GetInfo_PCI( TQListView *lBox );
bool GetInfo_IO_Ports( TQListView *lBox );
bool GetInfo_Sound( TQListView *lBox );
bool GetInfo_Devices( TQListView *lBox );
bool GetInfo_SCSI( TQListView *lBox );
bool GetInfo_Partitions( TQListView *lBox );
bool GetInfo_XServer_and_Video( TQListView *lBox );
bool GetInfo_CD_ROM( TQListView *lBox ); /* New CD-ROM Info */
extern bool GetInfo_OpenGL( TQListView *lBox );

class KInfoListWidget : public KCModule
{
public:
  KInfoListWidget(const TQString &_title, TQWidget *parent, const char *name=0, bool _getlistbox (TQListView *)=0);

  virtual void load();
  virtual TQString quickHelp() const;
  
private:
  TQListView 	*lBox;
  bool 		(*getlistbox) (TQListView *);
  TQString title;
  
  TQLabel	*NoInfoText;
  TQString	ErrorString;
  TQWidgetStack  *widgetStack;
};

#endif
