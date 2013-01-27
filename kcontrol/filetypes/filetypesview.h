#ifndef _FILETYPESVIEW_H
#define _FILETYPESVIEW_H

#include <tqptrlist.h>
#include <tqmap.h>

#include <tdeconfig.h>
#include <tdecmodule.h>

#include "typeslistitem.h"

class TQLabel;
class KListView;
class TQListViewItem;
class TQListBox;
class TQPushButton;
class KIconButton;
class TQLineEdit;
class TQComboBox;
class FileTypeDetails;
class FileGroupDetails;
class TQWidgetStack;

class FileTypesView : public TDECModule
{
  Q_OBJECT
public:
  FileTypesView(TQWidget *p = 0, const char *name = 0);
  ~FileTypesView();

  void load();
  void save();
  void defaults();

protected slots:
  /** fill in the various graphical elements, set up other stuff. */
  void init();

  void addType();
  void removeType();
  void updateDisplay(TQListViewItem *);
  void slotDoubleClicked(TQListViewItem *);
  void slotFilter(const TQString &patternFilter);
  void setDirty(bool state);

  void slotDatabaseChanged();
  void slotEmbedMajor(const TQString &major, bool &embed);

protected:
  void readFileTypes();
  bool sync( TQValueList<TypesListItem *>& itemsModified );

private:
  KListView *typesLV;
  TQPushButton *m_removeTypeB;

  TQWidgetStack * m_widgetStack;
  FileTypeDetails * m_details;
  FileGroupDetails * m_groupDetails;
  TQLabel * m_emptyWidget;

  TQLineEdit *patternFilterLE;
  TQStringList removedList;
  bool m_dirty;
  TQMap<TQString,TypesListItem*> m_majorMap;
  TQPtrList<TypesListItem> m_itemList;

  TQValueList<TypesListItem *> m_itemsModified;

  KSharedConfig::Ptr m_konqConfig;
};

#endif
