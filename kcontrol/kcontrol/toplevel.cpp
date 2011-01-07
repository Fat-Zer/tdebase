/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.
*/

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kbugreport.h>
#include <kaboutapplication.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kaction.h>
#include <kwin.h>
#include <kiconloader.h>
#include <klineedit.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include "indexwidget.h"
#include "searchwidget.h"
#include "helpwidget.h"
#include "aboutwidget.h"
#include "proxywidget.h"
#include "moduletreeview.h"
#include <stdio.h>

#include "toplevel.h"
#include "toplevel.moc"

TopLevel::TopLevel(const char* name)
  : KMainWindow( 0, name, WStyle_ContextHelp  )
  , _active(0), dummyAbout(0)
{
  setCaption(TQString::null);

  report_bug = 0;

  // read settings
  KConfig *config = KGlobal::config();
  config->setGroup("Index");
  TQString viewmode = config->readEntry("ViewMode", "Tree");

  if (viewmode == "Tree")
    KCGlobal::setViewMode(Tree);
  else
    KCGlobal::setViewMode(Icon);

  TQString size = config->readEntry("IconSize", "Medium");
  if (size == "Small")
    KCGlobal::setIconSize(KIcon::SizeSmall);
  else if (size == "Large")
    KCGlobal::setIconSize(KIcon::SizeLarge);
  else if (size == "Huge")
    KCGlobal::setIconSize(KIcon::SizeHuge);
  else
    KCGlobal::setIconSize(KIcon::SizeMedium);

  // initialize the entries
  _modules = new ConfigModuleList();
  _modules->readDesktopEntries();

  for ( ConfigModule* m = _modules->first(); m; m = _modules->next() ) {
      connect( m, TQT_SIGNAL( handbookRequest() ), this, TQT_SLOT( slotHandbookRequest() ) );
      connect( m, TQT_SIGNAL( helpRequest() ), this, TQT_SLOT( slotHelpRequest() ) );
  }

  // create the layout box
  _splitter = new TQSplitter( TQSplitter::Horizontal, this );

  TQFrame* leftFrame = new TQFrame ( _splitter );
  TQBoxLayout *leftFrameLayout = new TQVBoxLayout( leftFrame );

  TQFrame* mSearchFrame = new TQFrame( leftFrame );
  leftFrameLayout->addWidget( mSearchFrame );

  TQBoxLayout *searchLayout = new TQHBoxLayout( mSearchFrame );
  searchLayout->setSpacing( KDialog::spacingHint() );
  searchLayout->setMargin( 6 );

  TQPushButton *clearButton = new TQPushButton( mSearchFrame );
  clearButton->setIconSet( KApplication::reverseLayout() ?
    SmallIconSet( "clear_left" ) : SmallIconSet("locationbar_erase") );
  searchLayout->addWidget( clearButton );
  TQToolTip::add( clearButton, i18n("Clear search") );

  TQLabel *label = new TQLabel(i18n("Search:"), mSearchFrame );
  searchLayout->addWidget( label );
  
  KLineEdit *searchEdit = new KLineEdit( mSearchFrame );
  clearButton->setFixedHeight( searchEdit->height() );
  connect( clearButton, TQT_SIGNAL( clicked() ), searchEdit, TQT_SLOT( clear() ) );
  label->setBuddy( searchEdit );
  searchLayout->addWidget( searchEdit );
  connect( searchEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
           TQT_SLOT( slotSearchChanged(const TQString &) ) );

  // create the left hand side under search
  _stack = new TQWidgetStack( leftFrame );
  leftFrameLayout->addWidget( _stack );

  // index tab
  _index = new IndexWidget(_modules, this);
  connect(_index, TQT_SIGNAL(moduleActivated(ConfigModule*)),
                  this, TQT_SLOT(activateModule(ConfigModule*)));
  _stack->addWidget( _index );

  connect(_index, TQT_SIGNAL(categorySelected(TQListViewItem*)),
                  this, TQT_SLOT(categorySelected(TQListViewItem*)));

  // search tab
  _search = new SearchWidget(this);
  _search->populateKeywordList(_modules);
  connect(_search, TQT_SIGNAL(moduleSelected(ConfigModule *)),
                  this, TQT_SLOT(activateModule(ConfigModule *)));

  _stack->addWidget( _search );

  // set up the right hand side (the docking area)
  _dock = new DockContainer( _splitter );

  // help widget
  _help = new HelpWidget(_dock);

  _stack->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding ) );

 // Restore sizes
  config->setGroup("General");
  TQValueList<int> sizes = config->readIntListEntry(  "SplitterSizes" );
  if (!sizes.isEmpty())
     _splitter->setSizes(sizes);

  // That one does the trick ...
  _splitter->setResizeMode( leftFrame, TQSplitter::KeepSize );

  connect(_dock, TQT_SIGNAL(newModule(const TQString&, const TQString&, const TQString&)),
                  this, TQT_SLOT(newModule(const TQString&, const TQString&, const TQString&)));
  connect(_dock, TQT_SIGNAL(changedModule(ConfigModule*)),
          TQT_SLOT(changedModule(ConfigModule*)));

  // set the main view
  setCentralWidget( _splitter );

  // initialize the GUI actions
  setupActions();

  // activate defaults
  if (KCGlobal::viewMode() == Tree)   {
    activateTreeView();
    tree_view->setChecked(true);
  }
  else {
    activateIconView();
    icon_view->setChecked(true);
  }

  // insert the about widget
  if (KCGlobal::isInfoCenter())
  {
      AboutWidget *aw = new AboutWidget( this, 0, _index->firstTreeViewItem());
      connect( aw, TQT_SIGNAL( moduleSelected( ConfigModule * ) ),
               TQT_SLOT( activateModule( ConfigModule * ) ) );
      _dock->setBaseWidget( aw );
      KWin::setIcons(  winId(),
		       KGlobal::iconLoader()->loadIcon("hwinfo",  KIcon::NoGroup,  32 ),
		       KGlobal::iconLoader()->loadIcon("hwinfo",  KIcon::NoGroup,  16 ) );
  }
  else
  {
      AboutWidget *aw = new AboutWidget(this);
      connect( aw, TQT_SIGNAL( moduleSelected( ConfigModule * ) ),
                   TQT_SLOT( activateModule( ConfigModule * ) ) );
      _dock->setBaseWidget(aw);
  }
}

TopLevel::~TopLevel()
{
  KConfig *config = KGlobal::config();
  config->setGroup("Index");
  if (KCGlobal::viewMode() == Tree)
    config->writeEntry("ViewMode", "Tree");
  else
    config->writeEntry("ViewMode", "Icon");

  switch (KCGlobal::iconSize())
    {
    case KIcon::SizeSmall:
      config->writeEntry("IconSize", "Small");
      break;
    case KIcon::SizeLarge:
      config->writeEntry("IconSize", "Large");
      break;
    case KIcon::SizeHuge:
      config->writeEntry("IconSize", "Huge");
      break;
    default:
      config->writeEntry("IconSize", "Medium");
      break;
    }

  config->setGroup("General");
  config->writeEntry("SplitterSizes", _splitter->sizes());

  config->sync();

  delete _modules;
}

bool TopLevel::queryClose()
{
  return _dock->dockModule(0);
}

void TopLevel::setupActions()
{
  KStdAction::quit(this, TQT_SLOT(close()), actionCollection());
  KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()),
actionCollection());
  icon_view = new KRadioAction
    (i18n("&Icon View"), 0, this, TQT_SLOT(activateIconView()),
     actionCollection(), "activate_iconview");
  icon_view->setExclusiveGroup( "viewmode" );

  tree_view = new KRadioAction
    (i18n("&Tree View"), 0, this, TQT_SLOT(activateTreeView()),
     actionCollection(), "activate_treeview");
  tree_view->setExclusiveGroup( "viewmode" );

  icon_small = new KRadioAction
    (i18n("&Small"), 0, this, TQT_SLOT(activateSmallIcons()),
     actionCollection(), "activate_smallicons");
  icon_small->setExclusiveGroup( "iconsize" );

  icon_medium = new KRadioAction
    (i18n("&Medium"), 0, this, TQT_SLOT(activateMediumIcons()),
     actionCollection(), "activate_mediumicons");
  icon_medium->setExclusiveGroup( "iconsize" );

  icon_large = new KRadioAction
    (i18n("&Large"), 0, this, TQT_SLOT(activateLargeIcons()),
     actionCollection(), "activate_largeicons");
  icon_large->setExclusiveGroup( "iconsize" );

  icon_huge = new KRadioAction
    (i18n("&Huge"), 0, this, TQT_SLOT(activateHugeIcons()),
     actionCollection(), "activate_hugeicons");
  icon_huge->setExclusiveGroup( "iconsize" );

  about_module = new KAction(i18n("About Current Module"), 0, this, TQT_SLOT(aboutModule()), actionCollection(), "help_about_module");
  about_module->setEnabled(false);

  // I need to add this so that each module can get a bug reported,
  // and not just KControl
  if (KCGlobal::isInfoCenter())
    createGUI("kinfocenterui.rc");
  else
    createGUI("kcontrolui.rc");

  report_bug = actionCollection()->action("help_report_bug");
  report_bug->setText(i18n("&Report Bug..."));
  report_bug->disconnect();
  connect(report_bug, TQT_SIGNAL(activated()), TQT_SLOT(reportBug()));
}

void TopLevel::activateIconView()
{
  KCGlobal::setViewMode(Icon);
  _index->activateView(Icon);

  icon_small->setEnabled(true);
  icon_medium->setEnabled(true);
  icon_large->setEnabled(true);
  icon_huge->setEnabled(true);

  switch(KCGlobal::iconSize())
    {
    case KIcon::SizeSmall:
      icon_small->setChecked(true);
      break;
    case KIcon::SizeLarge:
      icon_large->setChecked(true);
      break;
    case KIcon::SizeHuge:
      icon_huge->setChecked(true);
      break;
    default:
      icon_medium->setChecked(true);
      break;
    }
}

void TopLevel::activateTreeView()
{
  KCGlobal::setViewMode(Tree);
  _index->activateView(Tree);

  icon_small->setEnabled(false);
  icon_medium->setEnabled(false);
  icon_large->setEnabled(false);
  icon_huge->setEnabled(false);
}

void TopLevel::activateSmallIcons()
{
  KCGlobal::setIconSize(KIcon::SizeSmall);
  _index->reload();
}

void TopLevel::activateMediumIcons()
{
  KCGlobal::setIconSize(KIcon::SizeMedium);
  _index->reload();
}

void TopLevel::activateLargeIcons()
{
  KCGlobal::setIconSize(KIcon::SizeLarge);
  _index->reload();
}

void TopLevel::activateHugeIcons()
{
  KCGlobal::setIconSize(KIcon::SizeHuge);
  _index->reload();
}

void TopLevel::newModule(const TQString &name, const TQString& docPath, const TQString &quickhelp)
{
    setCaption(name, false);

  _help->setText( docPath, quickhelp );

  if (!report_bug) return;

  if(name.isEmpty())
    report_bug->setText(i18n("&Report Bug..."));
  else
    report_bug->setText(i18n("Report Bug on Module %1...").arg( handleAmpersand( name)));
}

void TopLevel::changedModule(ConfigModule *changed)
{
    if (!changed)
        return;
    setCaption(changed->moduleName(), changed->isChanged() );
}

void TopLevel::categorySelected(TQListViewItem *category)
{
  if (_active)
  {
    if (_active->isChanged())
      {
        int res = KMessageBox::warningYesNoCancel(this, _active ?
             i18n("There are unsaved changes in the active module.\n"
                  "Do you want to apply the changes before running "
                  "the new module or discard the changes?") :
             i18n("There are unsaved changes in the active module.\n"
                  "Do you want to apply the changes before exiting "
                  "the Control Center or discard the changes?"),
                            i18n("Unsaved Changes"),
                            KStdGuiItem::apply(),
                            KStdGuiItem::discard());
        if (res == KMessageBox::Yes)
          _active->module()->applyClicked();
        else if (res == KMessageBox::Cancel)
          return;
      }
  }
  _dock->removeModule();
  about_module->setText( i18n( "About Current Module" ) );
  about_module->setIconSet( TQIconSet() );
  about_module->setEnabled( false );

  // insert the about widget
  TQListViewItem *firstItem = category->firstChild();
  TQString caption = static_cast<ModuleTreeItem*>(category)->caption();
  TQString icon = static_cast<ModuleTreeItem*>(category)->icon();
  if( _dock->baseWidget()->isA( "AboutWidget" ) )
  {
    static_cast<AboutWidget *>( _dock->baseWidget() )->setCategory( firstItem, icon, caption);
  }
  else
  {
    AboutWidget *aw = new AboutWidget( this, 0, firstItem, caption );
    connect( aw, TQT_SIGNAL( moduleSelected( ConfigModule * ) ),
             TQT_SLOT( activateModule( ConfigModule * ) ) );
    _dock->setBaseWidget( aw );
  }
}


void TopLevel::activateModule(ConfigModule *mod)
{
  if ( _dock->module() == mod )
     return;

  // tell the index to display the module
  _index->makeVisible(mod);

  // tell the index to mark this module as loaded
  _index->makeSelected(mod);

  // dock it
  if (!_dock->dockModule(mod))
  {
     if ( _dock->module() )
     {
       _index->makeVisible(_active);
       _index->makeSelected(_active);
     }
     return;
  }

  _active=mod;

  if (mod->aboutData())
  {
     about_module->setText(i18n("Help menu->about <modulename>", "About %1").arg(
                             handleAmpersand( mod->moduleName())));
     about_module->setIcon(mod->icon());
     about_module->setEnabled(true);
  }
  else
  {
     about_module->setText(i18n("About Current Module"));
     about_module->setIconSet(TQIconSet());
     about_module->setEnabled(false);
  }
}

void TopLevel::deleteDummyAbout()
{
  delete dummyAbout;
  dummyAbout = 0;
}

void TopLevel::slotSearchChanged(const TQString& search)
{
    if (search.isEmpty())
       _stack->raiseWidget(_index);
    else
    {
       _stack->raiseWidget(_search);
       _search->searchTextChanged(search);
    }
}

void TopLevel::slotHelpRequest()
{
    _help->display(_help->text(), TQCursor::pos ( ), _dock);
}

void TopLevel::slotHandbookRequest()
{
    _help->handbookRequest();
}

void TopLevel::reportBug()
{
    // this assumes the user only opens one bug report at a time
    static char buffer[128];

    dummyAbout = 0;
    bool deleteit = false;

    if (!_active) // report against kcontrol
        dummyAbout = const_cast<KAboutData*>(KGlobal::instance()->aboutData());
    else
    {
        if (_active->aboutData())
            dummyAbout = const_cast<KAboutData*>(_active->aboutData());
        else
        {
            snprintf(buffer, sizeof(buffer), "kcm%s", _active->library().latin1());
            dummyAbout = new KAboutData(buffer, _active->moduleName().utf8(), "2.0");
            deleteit = true;
        }
    }
    KBugReport *br = new KBugReport(this, false, dummyAbout);
    if (deleteit)
        connect(br, TQT_SIGNAL(finished()), TQT_SLOT(deleteDummyAbout()));
    else
        dummyAbout = 0;
    br->show();
}

void TopLevel::aboutModule()
{
    KAboutApplication dlg(_active->aboutData());
    dlg.exec();
}

TQString TopLevel::handleAmpersand( TQString modulename ) const
{
   if( modulename.contains( '&' )) // double it
   {
      for( int i = modulename.length();
           i >= 0;
           --i )
         if( modulename[ i ] == '&' )
             modulename.insert( i, "&" );
   }
   return modulename;
}
