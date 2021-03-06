/*
 * commandShortcuts.h
 *
 * Copyright (c) 2003 Aaron J. Seigo
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "commandShortcuts.h"
#include "treeview.h"

#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqwhatsthis.h>

#include <kactivelabel.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <kdialog.h>
#include <khotkeys.h>
#include <kkeybutton.h>
#include <tdelocale.h>

static bool treeFilled = false;
CommandShortcutsModule::CommandShortcutsModule( TQWidget *parent, const char *name )
: TQWidget( parent, name )
{
    treeFilled = false;
    initGUI();
}

CommandShortcutsModule::~CommandShortcutsModule()
{
}

// Called when [Reset] is pressed
void CommandShortcutsModule::load()
{
    defaults();
}

void CommandShortcutsModule::save()
{
    for (treeItemListIterator it(m_changedItems); it.current(); ++it)
    {
        KHotKeys::changeMenuEntryShortcut(it.current()->storageId(), it.current()->accel());
    }
    m_changedItems.clear();
}

void CommandShortcutsModule::defaults()
{
    m_tree->clear();
    m_tree->fill();
}

TQString CommandShortcutsModule::quickHelp() const
{
  return i18n("<h1>Command Shortcuts</h1> Using key bindings you can configure applications "
    "and commands to be triggered when you press a key or a combination of keys.");
}

void CommandShortcutsModule::initGUI()
{
    TQVBoxLayout* mainLayout = new TQVBoxLayout(this, KDialog::marginHint());
    mainLayout->addSpacing( KDialog::marginHint() );

    KActiveLabel* label = new KActiveLabel(this);
    label->setText(i18n("<qt>Below is a list of known commands which you may assign keyboard shortcuts to. "
                        "To edit, add or remove entries from this list use the "
                        "<a href=\"launchMenuEditor\">TDE menu editor</a>.</qt>"));
    label->setSizePolicy(TQSizePolicy::Preferred, TQSizePolicy::Minimum);
    disconnect(label, TQT_SIGNAL(linkClicked(const TQString &)), label, TQT_SLOT(openLink(const TQString &)));
    connect(label, TQT_SIGNAL(linkClicked(const TQString &)), this, TQT_SLOT(launchMenuEditor()));
    mainLayout->addWidget(label);

    m_tree = new AppTreeView(this, "appTreeView");
    m_tree->setSizePolicy(TQSizePolicy::Preferred, TQSizePolicy::Expanding);
    mainLayout->setStretchFactor(m_tree, 10);
    mainLayout->addWidget(m_tree);
    TQWhatsThis::add(m_tree,
                    i18n("This is a list of all the desktop applications and commands "
                         "currently defined on this system. Click to select a command to "
                         "assign a keyboard shortcut to. Complete management of these "
                         "entries can be done via the menu editor program."));
    connect(m_tree, TQT_SIGNAL(entrySelected(const TQString&, const TQString &, bool)),
            this, TQT_SLOT(commandSelected(const TQString&, const TQString &, bool)));
    connect(m_tree, TQT_SIGNAL(doubleClicked(TQListViewItem *, const TQPoint &, int)),
            this, TQT_SLOT(commandDoubleClicked(TQListViewItem *, const TQPoint &, int)));
    m_shortcutBox = new TQButtonGroup(i18n("Shortcut for Selected Command"), this);
    mainLayout->addWidget(m_shortcutBox);
    TQHBoxLayout* buttonLayout = new TQHBoxLayout(m_shortcutBox, KDialog::marginHint() * 2);
    buttonLayout->addSpacing( KDialog::marginHint() );

    m_noneRadio = new TQRadioButton(i18n("no key", "&None"), m_shortcutBox);
    TQWhatsThis::add(m_noneRadio, i18n("The selected command will not be associated with any key."));
    buttonLayout->addWidget(m_noneRadio);
    m_customRadio = new TQRadioButton(i18n("C&ustom"), m_shortcutBox);
    TQWhatsThis::add(m_customRadio,
                    i18n("If this option is selected you can create a customized key binding for the"
                         " selected command using the button to the right.") );
    buttonLayout->addWidget(m_customRadio);
    m_shortcutButton = new KKeyButton(m_shortcutBox);
    TQWhatsThis::add(m_shortcutButton,
                    i18n("Use this button to choose a new shortcut key. Once you click it, "
                         "you can press the key-combination which you would like to be assigned "
                         "to the currently selected command."));
    buttonLayout->addSpacing(KDialog::spacingHint() * 2);
    buttonLayout->addWidget(m_shortcutButton);
    connect(m_shortcutButton, TQT_SIGNAL(capturedShortcut(const TDEShortcut&)),
            this, TQT_SLOT(shortcutChanged(const TDEShortcut&)));
    connect(m_customRadio, TQT_SIGNAL(toggled(bool)), m_shortcutButton, TQT_SLOT(setEnabled(bool)));
    connect(m_noneRadio, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(shortcutRadioToggled(bool)));
    buttonLayout->addStretch(1);
}

void CommandShortcutsModule::launchMenuEditor()
{
    if ( TDEApplication::startServiceByDesktopName( "kmenuedit",
                                                  TQString() /*url*/,
                                                  0 /*error*/,
                                                  0 /*dcopservice*/,
                                                  0 /*pid*/,
                                                  "" /*startup_id*/,
                                                  true /*nowait*/ ) != 0 )
    {
        KMessageBox::error(this,
                           i18n("The TDE menu editor (kmenuedit) could not be launched.\n"
                           "Perhaps it is not installed or not in your path."),
                           i18n("Application Missing"));
    }
}


void CommandShortcutsModule::shortcutRadioToggled(bool remove)
{
    AppTreeItem *item = static_cast<AppTreeItem*>(m_tree->currentItem());
    if (!item || item->isDirectory())
    {
        return;
    }

    if (remove)
    {
        m_shortcutButton->setShortcut(TQString(), false);
        item->setAccel(TQString());
        if (m_changedItems.findRef(item) == -1)
        {
            m_changedItems.append(item);
        }
        emit changed(true);
    }
    else
    {
        m_shortcutButton->captureShortcut();
    }
}

void CommandShortcutsModule::shortcutChanged(const TDEShortcut& shortcut)
{
    AppTreeItem *item = static_cast<AppTreeItem*>(m_tree->currentItem());
    if (!item || item->isDirectory())
    {
        return;
    }

    TQString accel = shortcut.toString();
    bool hasAccel = !accel.isEmpty();
    m_noneRadio->blockSignals(true);
    m_noneRadio->setChecked(!hasAccel);
    m_customRadio->setChecked(hasAccel);
    m_shortcutButton->setShortcut(accel, false);
    item->setAccel(accel);
    m_noneRadio->blockSignals(false);
    if (m_changedItems.findRef(item) == -1)
    {
       m_changedItems.append(item);
    }

    emit changed( true );
}

void CommandShortcutsModule::showing(TQWidget* w)
{
    if (w != this || treeFilled)
    {
        return;
    }

    m_tree->fill();
    if (m_tree->firstChild())
    {
        m_tree->setSelected(m_tree->firstChild(), true);
    }
    else
    {
        m_shortcutBox->setEnabled(false);
    }
    treeFilled = true;
}

void CommandShortcutsModule::commandSelected(const TQString& /* path */, const TQString & accel, bool isDirectory)
{
    m_noneRadio->blockSignals(true);
    m_shortcutBox->setEnabled(!isDirectory);
    if (!isDirectory)
    {
        bool hasAccel = !accel.isEmpty();
        m_noneRadio->setChecked(!hasAccel);
        m_customRadio->setChecked(hasAccel);
        m_shortcutButton->setShortcut(accel, false);
    }
    m_noneRadio->blockSignals(false);
}

void CommandShortcutsModule::commandDoubleClicked(TQListViewItem *item, const TQPoint &, int)
{
    if (!item)
    {
        return;
    }
    AppTreeItem *rl_item = static_cast<AppTreeItem*>(item);
    if ( rl_item->isDirectory())
        return;

    m_shortcutButton->captureShortcut();
}

#include "commandShortcuts.moc"
