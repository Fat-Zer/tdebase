/*
 *
 * Copyright (c) 1998 Matthias Ettrich <ettrich@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqwhatsthis.h>
#include <tqlayout.h>
#include <tqvgroupbox.h>
#include <tqgrid.h>
#include <tqsizepolicy.h>
#include <tqbitmap.h>
#include <tqhgroupbox.h>
#include <tqtooltip.h>

#include <dcopclient.h>
#include <klocale.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kseparator.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>

#include "mouse.h"
#include "mouse.moc"


namespace {

char const * const cnf_Max[] = {
  "MaximizeButtonLeftClickCommand",
  "MaximizeButtonMiddleClickCommand",
  "MaximizeButtonRightClickCommand",
};

char const * const tbl_Max[] = {
  "Maximize",
  "Maximize (vertical only)",
  "Maximize (horizontal only)",
  "" };

TQPixmap maxButtonPixmaps[3];

void createMaxButtonPixmaps()
{
  char const * maxButtonXpms[][3 + 13] = {
    {0, 0, 0,
    "...............",
    ".......#.......",
    "......###......",
    ".....#####.....",
    "..#....#....#..",
    ".##....#....##.",
    "###############",
    ".##....#....##.",
    "..#....#....#..",
    ".....#####.....",
    "......###......",
    ".......#.......",
    "..............."},
    {0, 0, 0,
    "...............",
    ".......#.......",
    "......###......",
    ".....#####.....",
    ".......#.......",
    ".......#.......",
    ".......#.......",
    ".......#.......",
    ".......#.......",
    ".....#####.....",
    "......###......",
    ".......#.......",
    "..............."},
    {0, 0, 0,
    "...............",
    "...............",
    "...............",
    "...............",
    "..#.........#..",
    ".##.........##.",
    "###############",
    ".##.........##.",
    "..#.........#..",
    "...............",
    "...............",
    "...............",
    "..............."},
  };

  TQString baseColor(". c " + TDEGlobalSettings::baseColor().name());
  TQString textColor("# c " + TDEGlobalSettings::textColor().name());
  for (int t = 0; t < 3; ++t)
  {
    maxButtonXpms[t][0] = "15 13 2 1";
    maxButtonXpms[t][1] = baseColor.ascii();
    maxButtonXpms[t][2] = textColor.ascii();
    maxButtonPixmaps[t] = TQPixmap(maxButtonXpms[t]);
    maxButtonPixmaps[t].setMask(maxButtonPixmaps[t].createHeuristicMask());
  }
}

} // namespace

void KTitleBarActionsConfig::paletteChanged()
{
  createMaxButtonPixmaps();
  for (int b = 0; b < 3; ++b)
    for (int t = 0; t < 3; ++t)
      coMax[b]->changeItem(maxButtonPixmaps[t], t);

}

KTitleBarActionsConfig::KTitleBarActionsConfig (bool _standAlone, TDEConfig *_config, TQWidget * parent, const char *)
  : TDECModule(parent, "kcmkwm"), config(_config), standAlone(_standAlone)
{
  TQString strWin1, strWin2, strWin3, strAllKey, strAll1, strAll2, strAll3;
  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint());
  TQGrid *grid;
  TQGroupBox *box;
  TQLabel *label;
  TQString strMouseButton1, strMouseButton3, strMouseWheel;
  TQString txtButton1, txtButton3, txtButton4;
  TQStringList items;
  bool leftHandedMouse = ( TDEGlobalSettings::mouseSettings().handed == TDEGlobalSettings::KMouseSettings::LeftHanded);

/** Titlebar doubleclick ************/

  TQHBoxLayout *hlayout = new TQHBoxLayout(layout);

  label = new TQLabel(i18n("&Titlebar double-click:"), this);
  hlayout->addWidget(label);
  TQWhatsThis::add( label, i18n("Here you can customize mouse click behavior when double clicking on the"
    " titlebar of a window.") );

  TQComboBox* combo = new TQComboBox(this);
  combo->insertItem(i18n("Maximize"));
  combo->insertItem(i18n("Maximize (vertical only)"));
  combo->insertItem(i18n("Maximize (horizontal only)"));
  combo->insertItem(i18n("Minimize"));
  combo->insertItem(i18n("Shade"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("On All Desktops"));
  combo->insertItem(i18n("Nothing"));
  combo->setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::Fixed));
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  hlayout->addWidget(combo);
  coTiDbl = combo;
  TQWhatsThis::add(combo, i18n("Behavior on <em>double</em> click into the titlebar."));

  label->setBuddy(combo);

/** Mouse Wheel Events  **************/
  TQHBoxLayout *hlayoutW = new TQHBoxLayout(layout);
  strMouseWheel = i18n("Titlebar wheel event:");
  label = new TQLabel(strMouseWheel, this);
  hlayoutW->addWidget(label);
  txtButton4 = i18n("Handle mouse wheel events");
  TQWhatsThis::add( label, txtButton4);
  
  // Titlebar and frame mouse Wheel  
  TQComboBox* comboW = new TQComboBox(this);
  comboW->insertItem(i18n("Raise/Lower"));
  comboW->insertItem(i18n("Shade/Unshade"));
  comboW->insertItem(i18n("Maximize/Restore"));
  comboW->insertItem(i18n("Keep Above/Below"));  
  comboW->insertItem(i18n("Move to Previous/Next Desktop"));  
  comboW->insertItem(i18n("Change Opacity"));  
  comboW->insertItem(i18n("Nothing"));  
  comboW->setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::Fixed));
  connect(comboW, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  hlayoutW->addWidget(comboW);
  coTiAct4 = comboW;
  TQWhatsThis::add(comboW, txtButton4);
  label->setBuddy(comboW);
  
/** Titlebar and frame  **************/

  box = new TQVGroupBox( i18n("Titlebar && Frame"), this, "Titlebar and Frame");
  box->layout()->setMargin(KDialog::marginHint());
  box->layout()->setSpacing(KDialog::spacingHint());
  layout->addWidget(box);
  TQWhatsThis::add( box, i18n("Here you can customize mouse click behavior when clicking on the"
                             " titlebar or the frame of a window.") );

  grid = new TQGrid(4, Qt::Vertical, box);


  new TQLabel(grid); // dummy

  strMouseButton1 = i18n("Left button:");
  txtButton1 = i18n("In this row you can customize left click behavior when clicking into"
     " the titlebar or the frame.");

  strMouseButton3 = i18n("Right button:");
  txtButton3 = i18n("In this row you can customize right click behavior when clicking into"
     " the titlebar or the frame." );

  if ( leftHandedMouse )
  {
     tqSwap(strMouseButton1, strMouseButton3);
     tqSwap(txtButton1, txtButton3);
  }

  label = new TQLabel(strMouseButton1, grid);
  TQWhatsThis::add( label, txtButton1);

  label = new TQLabel(i18n("Middle button:"), grid);
  TQWhatsThis::add( label, i18n("In this row you can customize middle click behavior when clicking into"
    " the titlebar or the frame.") );

  label = new TQLabel(strMouseButton3, grid);
  TQWhatsThis::add( label, txtButton3);


  label = new TQLabel(i18n("Active"), grid);
  label->setAlignment(AlignCenter);
  TQWhatsThis::add( label, i18n("In this column you can customize mouse clicks into the titlebar"
                               " or the frame of an active window.") );

  // Titlebar and frame, active, mouse button 1
  combo = new TQComboBox(grid);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations Menu"));
  combo->insertItem(i18n("Toggle Raise & Lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiAct1 = combo;

  txtButton1 = i18n("Behavior on <em>left</em> click into the titlebar or frame of an "
     "<em>active</em> window.");

  txtButton3 = i18n("Behavior on <em>right</em> click into the titlebar or frame of an "
     "<em>active</em> window.");

  // Be nice to left handed users
  if ( leftHandedMouse ) tqSwap(txtButton1, txtButton3);

  TQWhatsThis::add(combo, txtButton1);

  // Titlebar and frame, active, mouse button 2

  items << i18n("Raise")
        << i18n("Lower")
        << i18n("Operations Menu")
        << i18n("Toggle Raise & Lower")
        << i18n("Nothing")
        << i18n("Shade");

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiAct2 = combo;
  TQWhatsThis::add(combo, i18n("Behavior on <em>middle</em> click into the titlebar or frame of an <em>active</em> window."));

  // Titlebar and frame, active, mouse button 3
  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiAct3 =  combo;
  TQWhatsThis::add(combo, txtButton3 );

  txtButton1 = i18n("Behavior on <em>left</em> click into the titlebar or frame of an "
     "<em>inactive</em> window.");

  txtButton3 = i18n("Behavior on <em>right</em> click into the titlebar or frame of an "
     "<em>inactive</em> window.");

  // Be nice to left handed users
  if ( leftHandedMouse ) tqSwap(txtButton1, txtButton3);

  label = new TQLabel(i18n("Inactive"), grid);
  label->setAlignment(AlignCenter);
  TQWhatsThis::add( label, i18n("In this column you can customize mouse clicks into the titlebar"
                               " or the frame of an inactive window.") );

  items.clear();
  items  << i18n("Activate & Raise")
         << i18n("Activate & Lower")
         << i18n("Activate")
         << i18n("Shade")
         << i18n("Operations Menu")
         << i18n("Raise")
         << i18n("Lower")
         << i18n("Nothing");

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiInAct1 = combo;
  TQWhatsThis::add(combo, txtButton1);

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiInAct2 = combo;
  TQWhatsThis::add(combo, i18n("Behavior on <em>middle</em> click into the titlebar or frame of an <em>inactive</em> window."));

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coTiInAct3 = combo;
  TQWhatsThis::add(combo, txtButton3);

/**  Maximize Button ******************/

  box = new TQHGroupBox(i18n("Maximize Button"), this, "Maximize Button");
  box->layout()->setMargin(KDialog::marginHint());
  box->layout()->setSpacing(KDialog::spacingHint());
  layout->addWidget(box);
  TQWhatsThis::add( box,
    i18n("Here you can customize behavior when clicking on the maximize button.") );

  TQString strMouseButton[] = {
    i18n("Left button:"),
    i18n("Middle button:"),
    i18n("Right button:")};

  TQString txtButton[] = {
    i18n("Behavior on <em>left</em> click onto the maximize button." ),
    i18n("Behavior on <em>middle</em> click onto the maximize button." ),
    i18n("Behavior on <em>right</em> click onto the maximize button." )};

  if ( leftHandedMouse ) // Be nice to lefties
  {
     tqSwap(strMouseButton[0], strMouseButton[2]);
     tqSwap(txtButton[0], txtButton[2]);
  }

  createMaxButtonPixmaps();
  for (int b = 0; b < 3; ++b)
  {
    if (b != 0) new TQWidget(box); // Spacer

    TQLabel * label = new TQLabel(strMouseButton[b], box);
    TQWhatsThis::add( label,    txtButton[b] );
    label   ->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum ));

    coMax[b] = new ToolTipComboBox(box, tbl_Max);
    for (int t = 0; t < 3; ++t) coMax[b]->insertItem(maxButtonPixmaps[t]);
    connect(coMax[b], TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
    connect(coMax[b], TQT_SIGNAL(activated(int)), coMax[b], TQT_SLOT(changed()));
    TQWhatsThis::add( coMax[b], txtButton[b] );
    coMax[b]->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum ));
  }

  connect(kapp, TQT_SIGNAL(tdedisplayPaletteChanged()), TQT_SLOT(paletteChanged()));

  layout->addStretch();

  load();
}

KTitleBarActionsConfig::~KTitleBarActionsConfig()
{
  if (standAlone)
    delete config;
}

// do NOT change the texts below, they are written to config file
// and are not shown in the GUI
// they have to match the order of items in GUI elements though
const char* const tbl_TiDbl[] = {
    "Maximize",
    "Maximize (vertical only)",
    "Maximize (horizontal only)",
    "Minimize",
    "Shade",
    "Lower",
    "OnAllDesktops",
    "Nothing",
    "" };

const char* const tbl_TiAc[] = {
    "Raise",
    "Lower",
    "Operations menu",
    "Toggle raise and lower",
    "Nothing",
    "Shade",
    "" };

const char* const tbl_TiInAc[] = {
    "Activate and raise",
    "Activate and lower",
    "Activate",
    "Shade",
    "Operations menu",
    "Raise",
    "Lower",
    "Nothing",
    "" };

const char* const tbl_Win[] = {
    "Activate, raise and pass click",
    "Activate and pass click",
    "Activate",
    "Activate and raise",
    "" };

const char* const tbl_AllKey[] = {
    "Meta",
    "Alt",
    "" };

const char* const tbl_All[] = {
    "Move",
    "Activate, raise and move",
    "Toggle raise and lower",
    "Resize",
    "Raise",
    "Lower",
    "Minimize",
    "Nothing",
    "" };

const char* tbl_TiWAc[] = {
    "Raise/Lower",
    "Shade/Unshade",
    "Maximize/Restore",
    "Above/Below",
    "Previous/Next Desktop",
    "Change Opacity",
    "Nothing",
    "" };

const char* tbl_AllW[] = {
    "Raise/Lower",
    "Shade/Unshade",
    "Maximize/Restore",
    "Above/Below",
    "Previous/Next Desktop",
    "Change Opacity",
    "Nothing",
    "" };

static const char* tbl_num_lookup( const char* const arr[], int pos )
{
    for( int i = 0;
         arr[ i ][ 0 ] != '\0' && pos >= 0;
         ++i )
    {
        if( pos == 0 )
            return arr[ i ];
        --pos;
    }
    abort(); // should never happen this way
}

static int tbl_txt_lookup( const char* const arr[], const char* txt )
{
    int pos = 0;
    for( int i = 0;
         arr[ i ][ 0 ] != '\0';
         ++i )
    {
        if( tqstricmp( txt, arr[ i ] ) == 0 )
            return pos;
        ++pos;
    }
    return 0;
}

void KTitleBarActionsConfig::setComboText( TQComboBox* combo, const char*txt )
{
    if( combo == coTiDbl )
        combo->setCurrentItem( tbl_txt_lookup( tbl_TiDbl, txt ));
    else if( combo == coTiAct1 || combo == coTiAct2 || combo == coTiAct3 )
        combo->setCurrentItem( tbl_txt_lookup( tbl_TiAc, txt ));
    else if( combo == coTiInAct1 || combo == coTiInAct2 || combo == coTiInAct3 )
        combo->setCurrentItem( tbl_txt_lookup( tbl_TiInAc, txt ));
    else if( combo == coTiAct4 )
        combo->setCurrentItem( tbl_txt_lookup( tbl_TiWAc, txt ));	
    else if( combo == coMax[0] || combo == coMax[1] || combo == coMax[2] )
    {
        combo->setCurrentItem( tbl_txt_lookup( tbl_Max, txt ));
        static_cast<ToolTipComboBox *>(combo)->changed();
    }
    else
        abort();
}

const char* KTitleBarActionsConfig::functionTiDbl( int i )
{
    return tbl_num_lookup( tbl_TiDbl, i );
}

const char* KTitleBarActionsConfig::functionTiAc( int i )
{
    return tbl_num_lookup( tbl_TiAc, i );
}

const char* KTitleBarActionsConfig::functionTiInAc( int i )
{
    return tbl_num_lookup( tbl_TiInAc, i );
}

const char* KTitleBarActionsConfig::functionTiWAc(int i)
{
    return tbl_num_lookup( tbl_TiWAc, i );
}

const char* KTitleBarActionsConfig::functionMax( int i )
{
    return tbl_num_lookup( tbl_Max, i );
}

void KTitleBarActionsConfig::load()
{
  config->setGroup("Windows");
  setComboText(coTiDbl, config->readEntry("TitlebarDoubleClickCommand","Shade").ascii());
  for (int t = 0; t < 3; ++t)
    setComboText(coMax[t],config->readEntry(cnf_Max[t], tbl_Max[t]).ascii());

  config->setGroup( "MouseBindings");
  setComboText(coTiAct1,config->readEntry("CommandActiveTitlebar1","Raise").ascii());
  setComboText(coTiAct2,config->readEntry("CommandActiveTitlebar2","Lower").ascii());
  setComboText(coTiAct3,config->readEntry("CommandActiveTitlebar3","Operations menu").ascii());
  setComboText(coTiAct4,config->readEntry("CommandTitlebarWheel","Nothing").ascii());  
  setComboText(coTiInAct1,config->readEntry("CommandInactiveTitlebar1","Activate and raise").ascii());
  setComboText(coTiInAct2,config->readEntry("CommandInactiveTitlebar2","Activate and lower").ascii());
  setComboText(coTiInAct3,config->readEntry("CommandInactiveTitlebar3","Operations menu").ascii());
}

void KTitleBarActionsConfig::save()
{
  config->setGroup("Windows");
  config->writeEntry("TitlebarDoubleClickCommand", functionTiDbl( coTiDbl->currentItem() ) );
  for (int t = 0; t < 3; ++t)
    config->writeEntry(cnf_Max[t], functionMax(coMax[t]->currentItem()));

  config->setGroup("MouseBindings");
  config->writeEntry("CommandActiveTitlebar1", functionTiAc(coTiAct1->currentItem()));
  config->writeEntry("CommandActiveTitlebar2", functionTiAc(coTiAct2->currentItem()));
  config->writeEntry("CommandActiveTitlebar3", functionTiAc(coTiAct3->currentItem()));
  config->writeEntry("CommandInactiveTitlebar1", functionTiInAc(coTiInAct1->currentItem()));
  config->writeEntry("CommandTitlebarWheel", functionTiWAc(coTiAct4->currentItem()));  
  config->writeEntry("CommandInactiveTitlebar2", functionTiInAc(coTiInAct2->currentItem()));
  config->writeEntry("CommandInactiveTitlebar3", functionTiInAc(coTiInAct3->currentItem()));
  
  if (standAlone)
  {
    config->sync();
    if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
    kapp->dcopClient()->send("twin*", "", "reconfigure()", TQString(""));
  }
}

void KTitleBarActionsConfig::defaults()
{
  setComboText(coTiDbl, "Shade");
  setComboText(coTiAct1,"Raise");
  setComboText(coTiAct2,"Lower");
  setComboText(coTiAct3,"Operations menu");
  setComboText(coTiAct4,"Nothing");    
  setComboText(coTiInAct1,"Activate and raise");
  setComboText(coTiInAct2,"Activate and lower");
  setComboText(coTiInAct3,"Operations menu");
  for (int t = 0; t < 3; ++t)
    setComboText(coMax[t], tbl_Max[t]);
}


KWindowActionsConfig::KWindowActionsConfig (bool _standAlone, TDEConfig *_config, TQWidget * parent, const char *)
  : TDECModule(parent, "kcmkwm"), config(_config), standAlone(_standAlone)
{
  TQString strWin1, strWin2, strWin3, strAllKey, strAll1, strAll2, strAll3, strAllW;
  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint());
  TQGrid *grid;
  TQGroupBox *box;
  TQLabel *label;
  TQString strMouseButton1, strMouseButton3;
  TQString txtButton1, txtButton3;
  TQStringList items;
  bool leftHandedMouse = ( TDEGlobalSettings::mouseSettings().handed == TDEGlobalSettings::KMouseSettings::LeftHanded);

/**  Inactive inner window ******************/

  box = new TQVGroupBox(i18n("Inactive Inner Window"), this, "Inactive Inner Window");
  box->layout()->setMargin(KDialog::marginHint());
  box->layout()->setSpacing(KDialog::spacingHint());
  layout->addWidget(box);
  TQWhatsThis::add( box, i18n("Here you can customize mouse click behavior when clicking on an inactive"
                             " inner window ('inner' means: not titlebar, not frame).") );

  grid = new TQGrid(3, Qt::Vertical, box);

  strMouseButton1 = i18n("Left button:");
  txtButton1 = i18n("In this row you can customize left click behavior when clicking into"
     " the titlebar or the frame.");

  strMouseButton3 = i18n("Right button:");
  txtButton3 = i18n("In this row you can customize right click behavior when clicking into"
     " the titlebar or the frame." );

  if ( leftHandedMouse )
  {
     tqSwap(strMouseButton1, strMouseButton3);
     tqSwap(txtButton1, txtButton3);
  }

  strWin1 = i18n("In this row you can customize left click behavior when clicking into"
     " an inactive inner window ('inner' means: not titlebar, not frame).");

  strWin3 = i18n("In this row you can customize right click behavior when clicking into"
     " an inactive inner window ('inner' means: not titlebar, not frame).");

  // Be nice to lefties
  if ( leftHandedMouse ) tqSwap(strWin1, strWin3);

  label = new TQLabel(strMouseButton1, grid);
  TQWhatsThis::add( label, strWin1 );

  label = new TQLabel(i18n("Middle button:"), grid);
  strWin2 = i18n("In this row you can customize middle click behavior when clicking into"
     " an inactive inner window ('inner' means: not titlebar, not frame).");
  TQWhatsThis::add( label, strWin2 );

  label = new TQLabel(strMouseButton3, grid);
  TQWhatsThis::add( label, strWin3 );

  items.clear();
  items   << i18n("Activate, Raise & Pass Click")
          << i18n("Activate & Pass Click")
          << i18n("Activate")
          << i18n("Activate & Raise");

  TQComboBox* combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coWin1 = combo;
  TQWhatsThis::add( combo, strWin1 );

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coWin2 = combo;
  TQWhatsThis::add( combo, strWin2 );

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coWin3 = combo;
  TQWhatsThis::add( combo, strWin3 );


/** Inner window, titlebar and frame **************/

  box = new TQVGroupBox(i18n("Inner Window, Titlebar && Frame"), this, "Inner Window, Titlebar and Frame");
  box->layout()->setMargin(KDialog::marginHint());
  box->layout()->setSpacing(KDialog::spacingHint());
  layout->addWidget(box);
  TQWhatsThis::add( box, i18n("Here you can customize TDE's behavior when clicking somewhere into"
                             " a window while pressing a modifier key."));

  grid = new TQGrid(5, Qt::Vertical, box);

  // Labels
  label = new TQLabel(i18n("Modifier key:"), grid);

  strAllKey = i18n("Here you select whether holding the Meta key or Alt key "
    "will allow you to perform the following actions.");
  TQWhatsThis::add( label, strAllKey );


  strMouseButton1 = i18n("Modifier key + left button:");
  strAll1 = i18n("In this row you can customize left click behavior when clicking into"
                 " the titlebar or the frame.");

  strMouseButton3 = i18n("Modifier key + right button:");
  strAll3 = i18n("In this row you can customize right click behavior when clicking into"
                 " the titlebar or the frame." );

  if ( leftHandedMouse )
  {
     tqSwap(strMouseButton1, strMouseButton3);
     tqSwap(strAll1, strAll3);
  }

  label = new TQLabel(strMouseButton1, grid);
  TQWhatsThis::add( label, strAll1);

  label = new TQLabel(i18n("Modifier key + middle button:"), grid);
  strAll2 = i18n("Here you can customize TDE's behavior when middle clicking into a window"
                 " while pressing the modifier key.");
  TQWhatsThis::add( label, strAll2 );

  label = new TQLabel(strMouseButton3, grid);
  TQWhatsThis::add( label, strAll3);

  label = new TQLabel(i18n("Modifier key + mouse wheel:"), grid);
  strAllW = i18n("Here you can customize TDE's behavior when scrolling with the mouse wheel"
      "  in a window while pressing the modifier key.");
  TQWhatsThis::add( label, strAllW);

  // Combo's
  combo = new TQComboBox(grid);
  combo->insertItem(i18n("Meta"));
  combo->insertItem(i18n("Alt"));
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coAllKey = combo;
  TQWhatsThis::add( combo, strAllKey );

  items.clear();
  items << i18n("Move")
        << i18n("Activate, Raise and Move")
        << i18n("Toggle Raise & Lower")
        << i18n("Resize")
        << i18n("Raise")
        << i18n("Lower")
        << i18n("Minimize")
        << i18n("Nothing");

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coAll1 = combo;
  TQWhatsThis::add( combo, strAll1 );

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coAll2 = combo;
  TQWhatsThis::add( combo, strAll2 );

  combo = new TQComboBox(grid);
  combo->insertStringList(items);
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coAll3 =  combo;
  TQWhatsThis::add( combo, strAll3 );

  combo = new TQComboBox(grid);
  combo->insertItem(i18n("Raise/Lower"));
  combo->insertItem(i18n("Shade/Unshade"));
  combo->insertItem(i18n("Maximize/Restore"));
  combo->insertItem(i18n("Keep Above/Below"));  
  combo->insertItem(i18n("Move to Previous/Next Desktop"));  
  combo->insertItem(i18n("Change Opacity"));  
  combo->insertItem(i18n("Nothing"));  
  connect(combo, TQT_SIGNAL(activated(int)), TQT_SLOT(changed()));
  coAllW =  combo;
  TQWhatsThis::add( combo, strAllW );

  layout->addStretch();

  load();
}

KWindowActionsConfig::~KWindowActionsConfig()
{
  if (standAlone)
    delete config;
}

void KWindowActionsConfig::setComboText( TQComboBox* combo, const char*txt )
{
    if( combo == coWin1 || combo == coWin2 || combo == coWin3 )
        combo->setCurrentItem( tbl_txt_lookup( tbl_Win, txt ));
    else if( combo == coAllKey )
        combo->setCurrentItem( tbl_txt_lookup( tbl_AllKey, txt ));
    else if( combo == coAll1 || combo == coAll2 || combo == coAll3 )
        combo->setCurrentItem( tbl_txt_lookup( tbl_All, txt ));
    else if( combo == coAllW )
        combo->setCurrentItem( tbl_txt_lookup( tbl_AllW, txt ));	
    else
        abort();
}

const char* KWindowActionsConfig::functionWin( int i )
{
    return tbl_num_lookup( tbl_Win, i );
}

const char* KWindowActionsConfig::functionAllKey( int i )
{
    return tbl_num_lookup( tbl_AllKey, i );
}

const char* KWindowActionsConfig::functionAll( int i )
{
    return tbl_num_lookup( tbl_All, i );
}

const char* KWindowActionsConfig::functionAllW(int i)
{
    return tbl_num_lookup( tbl_AllW, i );
}

void KWindowActionsConfig::load()
{
  config->setGroup( "MouseBindings");
  setComboText(coWin1,config->readEntry("CommandWindow1","Activate, raise and pass click").ascii());
  setComboText(coWin2,config->readEntry("CommandWindow2","Activate and pass click").ascii());
  setComboText(coWin3,config->readEntry("CommandWindow3","Activate and pass click").ascii());
  setComboText(coAllKey,config->readEntry("CommandAllKey","Alt").ascii());
  setComboText(coAll1,config->readEntry("CommandAll1","Move").ascii());
  setComboText(coAll2,config->readEntry("CommandAll2","Toggle raise and lower").ascii());
  setComboText(coAll3,config->readEntry("CommandAll3","Resize").ascii());
  setComboText(coAllW,config->readEntry("CommandAllWheel","Nothing").ascii());
}

void KWindowActionsConfig::save()
{
  config->setGroup("MouseBindings");
  config->writeEntry("CommandWindow1", functionWin(coWin1->currentItem()));
  config->writeEntry("CommandWindow2", functionWin(coWin2->currentItem()));
  config->writeEntry("CommandWindow3", functionWin(coWin3->currentItem()));
  config->writeEntry("CommandAllKey", functionAllKey(coAllKey->currentItem()));
  config->writeEntry("CommandAll1", functionAll(coAll1->currentItem()));
  config->writeEntry("CommandAll2", functionAll(coAll2->currentItem()));
  config->writeEntry("CommandAll3", functionAll(coAll3->currentItem()));
  config->writeEntry("CommandAllWheel", functionAllW(coAllW->currentItem()));
  
  if (standAlone)
  {
    config->sync();
    if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
    kapp->dcopClient()->send("twin*", "", "reconfigure()", TQString(""));
  }
}

void KWindowActionsConfig::defaults()
{
  setComboText(coWin1,"Activate, raise and pass click");
  setComboText(coWin2,"Activate and pass click");
  setComboText(coWin3,"Activate and pass click");
  setComboText(coAllKey,"Alt");
  setComboText (coAll1,"Move");
  setComboText(coAll2,"Toggle raise and lower");
  setComboText(coAll3,"Resize");
  setComboText(coAllW,"Nothing");
}
