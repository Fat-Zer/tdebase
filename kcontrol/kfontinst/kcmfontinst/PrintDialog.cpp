////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CPrintDialog
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 12/05/2005
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include "PrintDialog.h"
#include <tqlayout.h>
#include <tqframe.h>
#include <tqlabel.h>
#include <klocale.h>

namespace KFI
{

CPrintDialog::CPrintDialog(TQWidget *parent)
            : KDialogBase(Plain, i18n("Print Font Samples"), Ok|Cancel, Ok, parent, NULL, true, false)
{
    TQFrame      *page=plainPage();
    TQGridLayout *tqlayout=new TQGridLayout(page, 1, 1, 0, spacingHint());

    tqlayout->addWidget(new TQLabel(i18n("Output:"), page), 0, 0);
    itsOutput=new TQComboBox(page);
    itsOutput->insertItem(i18n("All Fonts"), 0);
    itsOutput->insertItem(i18n("Selected Fonts"), 1);
    tqlayout->addWidget(itsOutput, 0, 1);
    tqlayout->addWidget(new TQLabel(i18n("Font size:"), page), 1, 0);
    itsSize=new TQComboBox(page);
    itsSize->insertItem(i18n("Waterfall"), 0);
    itsSize->insertItem(i18n("12pt"), 1);
    itsSize->insertItem(i18n("18pt"), 2);
    itsSize->insertItem(i18n("24pt"), 3);
    itsSize->insertItem(i18n("36pt"), 4);
    itsSize->insertItem(i18n("48pt"), 5);
    tqlayout->addWidget(itsSize, 1, 1);
    tqlayout->addItem(new TQSpacerItem(2, 2), 2, 1);
}

bool CPrintDialog::exec(bool select, int size)
{
    if(!select)
    {
        itsOutput->setCurrentItem(0);
        itsOutput->setEnabled(false);
    }
    else
        itsOutput->setCurrentItem(1);
    itsSize->setCurrentItem(size);
    return TQDialog::Accepted==TQDialog::exec();
}

}
