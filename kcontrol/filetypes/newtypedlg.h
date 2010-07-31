#ifndef _NEWTYPEDLG_H
#define _NEWTYPEDLG_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <kdialogbase.h>

class KLineEdit;
class QComboBox;

/**
 * A dialog for creating a new file type, with
 * a combobox for choosing the group and a line-edit
 * for entering the name of the file type
 */
class NewTypeDialog : public KDialogBase
{
public:
  NewTypeDialog(TQStringList groups, TQWidget *parent = 0, 
		const char *name = 0);
  TQString group() const;
  TQString text() const;
private:
  KLineEdit *typeEd;
  TQComboBox *groupCombo;
};

#endif
