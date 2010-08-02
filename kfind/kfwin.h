/***********************************************************************
 *
 *  Kfwin.h
 *
 ***********************************************************************/

#ifndef KFWIN_H
#define KFWIN_H

#include <klistview.h>
#include <kfileitem.h>
#include <kurl.h>

class KfArchiver;
class TQPixmap;
class TQFileInfo;
class KPopupMenu;
class KfindWindow;

class KfFileLVI : public QListViewItem
{
 public:
  KfFileLVI(KfindWindow* lv, const KFileItem &item,const TQString& matchingLine);
  ~KfFileLVI();

  TQString key(int column, bool) const;

  TQFileInfo *fileInfo;
  KFileItem fileitem;
};

class KfindWindow: public   KListView
{
  Q_OBJECT
public:
  KfindWindow( TQWidget * parent = 0, const char * name = 0 );

  void beginSearch(const KURL& baseUrl);
  void endSearch();

  void insertItem(const KFileItem &item, const TQString& matchingLine);

  TQString reducedDir(const TQString& fullDir);

public slots:
  void copySelection();
  void slotContextMenu(KListView *,TQListViewItem *item,const TQPoint&p);

private slots:
  void deleteFiles();
  void fileProperties();
  void openFolder();
  void saveResults();
  void openBinding();
  void selectionHasChanged();
  void slotExecute(TQListViewItem*);
  void slotOpenWith();
  
protected:
  virtual void resizeEvent(TQResizeEvent *e);

  virtual TQDragObject *dragObject();

signals:
  void resultSelected(bool);

private:
  TQString m_baseDir;
  KPopupMenu *m_menu;
  bool haveSelection;
  bool m_pressed;
  void resetColumns(bool init);
};

#endif
