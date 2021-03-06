/* kastasker.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
#include <tqapplication.h>
#include <tqtimer.h>

#include <tdeactionclasses.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>
#include <kiconloader.h>

//#include <tdeconfiggroupsaver.h>

#include <taskmanager.h>

#include "kasaboutdlg.h"
#include "kastaskitem.h"
#include "kasprefsdlg.h"
#include "kasstartupitem.h"
#include "kasgroupitem.h"
#include "kasgrouper.h"
#include "kasclockitem.h"
#include "kasloaditem.h"

#include "kastasker.h"
#include "kastasker.moc"

static const int SWITCH_DESKTOPS_REGROUP_DELAY = 50;

KasTasker::KasTasker( Orientation o, TQWidget* parent, const char* name, WFlags f )
  : KasBar( o, parent, name, f ),
    menu( 0 ),
    conf( 0 ),
    grouper( 0 ),
    standalone_( false ),
    enableThumbs_( true ),
    embedThumbs_( false ),
    thumbnailSize_( 0.2 ),
    enableNotifier_( true ),
    showModified_( true ),
    showProgress_( false ),
    showAllWindows_( true ),
    thumbUpdateDelay_( 10 ),
    groupWindows_( false ),
    groupInactiveDesktops_( false ),
    showAttention_( true ),
    showClock_( false ),
    clockItem(0),
    showLoad_( false ),
    loadItem(0)
{
   setAcceptDrops( true );
   connect(TaskManager::the(), TQT_SIGNAL(taskAdded(Task::Ptr)), TQT_SLOT(addTask(Task::Ptr)));
   connect(TaskManager::the(), TQT_SIGNAL(taskRemoved(Task::Ptr)), TQT_SLOT(removeTask(Task::Ptr)));
   connect(TaskManager::the(), TQT_SIGNAL(startupAdded(Startup::Ptr)), TQT_SLOT(addStartup(Startup::Ptr)));
   connect(TaskManager::the(), TQT_SIGNAL(startupRemoved(Startup::Ptr)), TQT_SLOT(removeStartup(Startup::Ptr)));
   connect(TaskManager::the(), TQT_SIGNAL(desktopChanged(int)), TQT_SLOT(refreshAllLater()));
//   connect( manager, TQT_SIGNAL( windowChanged( Task::Ptr ) ), TQT_SLOT( refreshAllLater() ) );

   connect( this, TQT_SIGNAL( itemSizeChanged( int ) ), TQT_SLOT( refreshAll() ) );

   connect( this, TQT_SIGNAL( detachedPositionChanged(const TQPoint &) ), TQT_SLOT( writeLayout() ) );
   connect( this, TQT_SIGNAL( directionChanged() ), TQT_SLOT( writeLayout() ) );
}

KasTasker::KasTasker( Orientation o, KasTasker *master, TQWidget* parent, const char* name, WFlags f )
  : KasBar( o, master, parent, name, f ),
    menu( 0 ),
    conf( 0 ),
    grouper( 0 ),
    standalone_( master->standalone_ ),
    enableThumbs_( master->enableThumbs_ ),
    embedThumbs_( master->embedThumbs_ ),
    thumbnailSize_( master->thumbnailSize_ ),
    enableNotifier_( master->enableNotifier_ ),
    showModified_( master->showModified_ ),
    showProgress_( master->showProgress_ ),
    showAllWindows_( master->showAllWindows_ ),
    thumbUpdateDelay_( master->thumbUpdateDelay_ ),
    groupWindows_( false ),
    groupInactiveDesktops_( false ),
    showAttention_( master->showAttention_ ),
    showClock_( false ),
    clockItem(0),
    showLoad_( false ),
    loadItem(0)
{
  setAcceptDrops( true );
}

KasTasker::~KasTasker()
{
    delete menu;
    delete grouper;
}

TDEPopupMenu *KasTasker::contextMenu()
{
    if ( !menu ) {
	menu = new TDEPopupMenu;

	showAllWindowsAction = new TDEToggleAction( i18n("Show &All Windows"), TDEShortcut(),
						  TQT_TQOBJECT(this), "toggle_show_all_windows" );
	showAllWindowsAction->setChecked( showAllWindows() );
	showAllWindowsAction->plug( menu );
	connect( showAllWindowsAction, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setShowAllWindows(bool)) );
	connect( TQT_TQOBJECT(this), TQT_SIGNAL(showAllWindowsChanged(bool)), showAllWindowsAction, TQT_SLOT(setChecked(bool)) );

	groupWindowsAction = new TDEToggleAction( i18n("&Group Windows"), TDEShortcut(),
						TQT_TQOBJECT(this), "toggle_group_windows" );
	groupWindowsAction->setChecked( groupWindows() );
	groupWindowsAction->plug( menu );
	connect( groupWindowsAction, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setGroupWindows(bool)) );
	connect( TQT_TQOBJECT(this), TQT_SIGNAL(groupWindowsChanged(bool)), groupWindowsAction, TQT_SLOT(setChecked(bool)) );

	showClockAction = new TDEToggleAction( i18n("Show &Clock"), TDEShortcut(), TQT_TQOBJECT(this), "toggle_show_clock" );
	showClockAction->setChecked( showClock() );
	showClockAction->plug( menu );
	connect( showClockAction, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setShowClock(bool)) );
	connect( TQT_TQOBJECT(this), TQT_SIGNAL(showClockChanged(bool)), showClockAction, TQT_SLOT(setChecked(bool)) );

	showLoadAction = new TDEToggleAction( i18n("Show &Load Meter"), TDEShortcut(), TQT_TQOBJECT(this), "toggle_show_load" );
	showLoadAction->setChecked( showLoad() );
	showLoadAction->plug( menu );
	connect( showLoadAction, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setShowLoad(bool)) );
	connect( TQT_TQOBJECT(this), TQT_SIGNAL(showLoadChanged(bool)), showLoadAction, TQT_SLOT(setChecked(bool)) );

	menu->insertSeparator();

	if ( !standalone_ ) {
	    toggleDetachedAction = new TDEToggleAction( i18n("&Floating"), TDEShortcut(), TQT_TQOBJECT(this), "toggle_detached" );
	    toggleDetachedAction->setChecked( isDetached() );
	    toggleDetachedAction->plug( menu );
	    connect( toggleDetachedAction, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setDetached(bool)) );
	    connect( TQT_TQOBJECT(this), TQT_SIGNAL(detachedChanged(bool)), toggleDetachedAction, TQT_SLOT(setChecked(bool)) );
	}

	rotateBarAction = new TDEAction( i18n("R&otate Bar"), TQString("rotate"), TDEShortcut(),
				       TQT_TQOBJECT(this), TQT_SLOT( toggleOrientation() ),
				       TQT_TQOBJECT(this), "rotate_bar" );
	rotateBarAction->plug( menu );
	connect( TQT_TQOBJECT(this), TQT_SIGNAL(detachedChanged(bool)), rotateBarAction, TQT_SLOT(setEnabled(bool)) );
	connect( rotateBarAction, TQT_SIGNAL(activated()), TQT_SLOT(writeConfigLater()) );

	menu->insertItem( SmallIcon("reload"), i18n("&Refresh"), TQT_TQOBJECT(this), TQT_SLOT( refreshAll() ) );

	menu->insertSeparator();

	menu->insertItem( SmallIcon("configure"), i18n("&Configure Kasbar..."), TQT_TQOBJECT(this), TQT_SLOT( showPreferences() ) );

	// Help menu
	TDEPopupMenu *help = new TDEPopupMenu;
	help->insertItem( SmallIcon("about"), i18n("&About Kasbar"), TQT_TQOBJECT(this), TQT_SLOT( showAbout() ) );
	menu->insertItem( SmallIcon("help"), i18n("&Help"), help );

	if ( standalone_ ) {
	    menu->insertSeparator();
	    menu->insertItem( SmallIcon("system-log-out"), i18n("&Quit"), tqApp, TQT_SLOT( quit() ) );
	}
    }

    return menu;
}

KasTasker *KasTasker::createChildBar( Orientation o, TQWidget *parent, const char *name )
{
    KasTasker *child = new KasTasker( o, this, parent, name );
    child->conf =  this->conf;
    return child;
}

KasTaskItem *KasTasker::findItem( Task::Ptr t )
{
   KasTaskItem *result = 0;
   for ( uint i = 0; i < itemCount(); i++ ) {
      if ( itemAt(i)->inherits( "KasTaskItem" ) ) {
	 KasTaskItem *curr = static_cast<KasTaskItem *> (itemAt( i ));
	 if ( curr->task() == t ) {
	    result = curr;
	    break;
	 }
      }
   }
   return result;
}

KasStartupItem *KasTasker::findItem( Startup::Ptr s )
{
   KasStartupItem *result = 0;
   for ( uint i = 0; i < itemCount(); i++ ) {
      if ( itemAt(i)->inherits( "KasStartupItem" ) ) {
	 KasStartupItem *curr = static_cast<KasStartupItem *> (itemAt( i ));
	 if ( curr->startup() == s ) {
	    result = curr;
	    break;
	 }
      }
   }
   return result;
}

void KasTasker::addTask( Task::Ptr t )
{
   KasItem *item = 0;

   if ( onlyShowMinimized_ && !t->isMinimized() )
       return;

   if ( showAllWindows_ || t->isOnCurrentDesktop() ) {
      if ( grouper )
	  item = grouper->maybeGroup( t );
      if ( !item ) {
	  item = new KasTaskItem( this, t );
	  append( item );
      }

      //
      // Ensure the window manager knows where we put the icon.
      //
      TQPoint p = mapToGlobal( itemPos( item ) );
      TQSize s( itemExtent(), itemExtent() );
      t->publishIconGeometry( TQRect( p, s ) );
   }
}

void KasTasker::removeTask( Task::Ptr t )
{
   KasTaskItem *i = findItem( t );
   if ( !i )
     return;

   remove( i );
   refreshIconGeometry();
}

KasGroupItem *KasTasker::convertToGroup( Task::Ptr t )
{
  KasTaskItem *ti = findItem( t );
  int i = indexOf( ti );
  KasGroupItem *gi = new KasGroupItem( this );
  gi->addTask( t );
  removeTask( t );
  insert( i, gi );

  connect(TaskManager::the(), TQT_SIGNAL(taskRemoved(Task::Ptr)), gi, TQT_SLOT(removeTask(Task::Ptr)));

  return gi;
}

void KasTasker::moveToMain( KasGroupItem *gi, Task::Ptr t )
{
  int i = indexOf( gi );
  if ( i != -1 ) {
    remove( gi );
    insert( i, new KasTaskItem( this, t ) );
  }
  else
    append( new KasTaskItem( this, t ) );

  refreshIconGeometry();
}

void KasTasker::moveToMain( KasGroupItem *gi )
{
   bool updates = isUpdatesEnabled();
   setUpdatesEnabled( false );

   int i = indexOf( gi );

   for ( int ti = 0 ; ti < gi->taskCount() ; ti++ ) {
       Task::Ptr t = gi->task( ti );
       insert( i, new KasTaskItem( this, t ) );
   }

   gi->hidePopup();
   remove( gi );

   setUpdatesEnabled( updates );
   updateLayout();
}

void KasTasker::addStartup( Startup::Ptr s )
{
   if ( enableNotifier_ )
      append( new KasStartupItem( this, s ) );
}

void KasTasker::removeStartup( Startup::Ptr s )
{
   KasStartupItem *i = findItem( s );
   remove( i );
}

void KasTasker::refreshAll()
{
   bool updates = isUpdatesEnabled();
   setUpdatesEnabled( false );

   clear();

   if ( showClock_ ) {
       showClock_ = false;
       setShowClock( true );
   }

   if ( showLoad_ ) {
       showLoad_ = false;
       setShowLoad( true );
   }

   Task::Dict l = TaskManager::the()->tasks();
   for ( Task::Dict::iterator t = l.begin(); t != l.end(); ++t ) {
      addTask( t.data() );
   }

   setUpdatesEnabled( updates );
   updateLayout();
}

void KasTasker::refreshAllLater()
{
    TQTimer::singleShot( SWITCH_DESKTOPS_REGROUP_DELAY, this, TQT_SLOT( refreshAll() ) );
}

void KasTasker::refreshIconGeometry()
{
   for ( uint i = 0; i < itemCount(); i++ ) {
      if ( itemAt(i)->inherits( "KasTaskItem" ) ) {
	 KasTaskItem *curr = static_cast<KasTaskItem *> (itemAt( i ));

	 TQPoint p = mapToGlobal( itemPos( curr ) );
	 TQSize s( itemExtent(), itemExtent() );
	 curr->task()->publishIconGeometry( TQRect( p, s ) );
      }
   }
}

void KasTasker::setNotifierEnabled( bool enable )
{
   enableNotifier_ = enable;
}

void KasTasker::setThumbnailSize( double size )
{
  thumbnailSize_ = size;
}

void KasTasker::setThumbnailSize( int percent )
{
   double amt = (double) percent / 100.0;
   setThumbnailSize( amt );
}

void KasTasker::setThumbnailsEnabled( bool enable )
{
   enableThumbs_ = enable;
}

void KasTasker::setShowModified( bool enable )
{
   showModified_ = enable;
   update();
}

void KasTasker::setShowProgress( bool enable )
{
   showProgress_ = enable;
   update();
}

void KasTasker::setShowAttention( bool enable )
{
   showAttention_ = enable;
   update();
}

void KasTasker::setShowAllWindows( bool enable )
{
   if ( showAllWindows_ != enable ) {
      showAllWindows_ = enable;
      refreshAll();
      if ( !showAllWindows_ ) {
	connect(TaskManager::the(), TQT_SIGNAL(desktopChanged(int)), TQT_SLOT(refreshAll()));
//	connect( manager, TQT_SIGNAL( windowChanged( Task::Ptr ) ), TQT_SLOT( refreshAll() ) );
      }
      else {
	disconnect(TaskManager::the(), TQT_SIGNAL(desktopChanged(int)), this, TQT_SLOT(refreshAll()));
//	disconnect( manager, TQT_SIGNAL( windowChanged( Task::Ptr ) ), this, TQT_SLOT( refreshAll() ) );
      }

      emit showAllWindowsChanged( enable );
   }
}

void KasTasker::setThumbnailUpdateDelay( int secs )
{
  thumbUpdateDelay_ = secs;
}

void KasTasker::setEmbedThumbnails( bool enable )
{
  if ( embedThumbs_ == enable )
      return;

  embedThumbs_ = enable;
  update();
}

void KasTasker::setShowClock( bool enable )
{
  if ( showClock_ == enable )
      return;

  showClock_ = enable;

  if ( enable ) {
      clockItem = new KasClockItem( this );
      insert( 0, clockItem );
  }
  else if ( clockItem ) {
      remove( clockItem );
      clockItem = 0;
  }


  emit showClockChanged( showClock_ );
  writeConfigLater();
}

void KasTasker::setShowLoad( bool enable )
{
  if ( showLoad_ == enable )
      return;

  showLoad_ = enable;

  if ( enable ) {
      loadItem = new KasLoadItem( this );
      insert( showClock_ ? 1 : 0, loadItem );
  }
  else if ( loadItem ) {
      remove( loadItem );
      loadItem = 0;
  }

  emit showLoadChanged( showLoad_ );
  writeConfigLater();
}

void KasTasker::setGroupWindows( bool enable )
{
   if ( groupWindows_ != enable ) {
      groupWindows_ = enable;
      if ( enable && (!grouper) )
	  grouper = new KasGrouper( this );
      refreshAll();

      emit groupWindowsChanged( enable );
   }
}

void KasTasker::setGroupInactiveDesktops( bool enable )
{
   if ( groupInactiveDesktops_ != enable ) {
      groupInactiveDesktops_ = enable;
      if ( enable && (!grouper) )
	  grouper = new KasGrouper( this );

      refreshAll();
   }
}

void KasTasker::setOnlyShowMinimized( bool enable )
{
   if ( onlyShowMinimized_ != enable ) {
      onlyShowMinimized_ = enable;
      refreshAll();
   }
}

void KasTasker::setStandAlone( bool enable )
{ 
    standalone_ = enable;
}

//
// Configuration Loader
//

void KasTasker::setConfig( TDEConfig *conf )
{
    this->conf = conf;
}

void KasTasker::readConfig()
{
   readConfig(conf);
}

void KasTasker::writeConfigLater()
{
   TQTimer::singleShot( 10, this, TQT_SLOT( writeConfig() ) );
}

void KasTasker::writeConfig()
{
   writeConfig(conf);
}

void KasTasker::readConfig( TDEConfig *conf )
{
    if ( !conf ) {
	kdWarning() << "KasTasker::readConfig() got a null TDEConfig" << endl;
	return;
    }

    if ( master() ) {
	kdWarning() << "KasTasker::readConfig() for child bar" << endl;
	return;
    }

    bool updates = isUpdatesEnabled();
    setUpdatesEnabled( false );


   //
   // Appearance Settings.
   //
   TDEConfigGroupSaver saver( conf, "Appearance" );

   int ext = conf->readNumEntry( "ItemExtent", -1 );
   if ( ext > 0 )
       setItemExtent( ext );
   else
       setItemSize( conf->readNumEntry( "ItemSize", KasBar::Medium ) );

   setTint( conf->readBoolEntry( "EnableTint", false ) );
   setTintColor( conf->readColorEntry( "TintColor", &TQt::black ) );
   setTintAmount( conf->readDoubleNumEntry( "TintAmount", 0.1 ) );
   setTransparent( conf->readBoolEntry( "Transparent", true ) );
   setPaintInactiveFrames( conf->readBoolEntry( "PaintInactiveFrames", true ) );

   //
   // Painting colors
   //
   conf->setGroup("Colors");

   KasResources *res = resources();
   res->setLabelPenColor( conf->readColorEntry( "LabelPenColor", &TQt::white ) );
   res->setLabelBgColor( conf->readColorEntry( "LabelBgColor", &TQt::black ) );
   res->setInactivePenColor( conf->readColorEntry( "InactivePenColor", &TQt::black ) );
   res->setInactiveBgColor( conf->readColorEntry( "InactiveBgColor", &TQt::white ) );
   res->setActivePenColor( conf->readColorEntry( "ActivePenColor", &TQt::black ) );
   res->setActiveBgColor( conf->readColorEntry( "ActiveBgColor", &TQt::white ) );
   res->setProgressColor( conf->readColorEntry( "ProgressColor", &TQt::green ) );
   res->setAttentionColor( conf->readColorEntry( "AttentionColor", &TQt::red ) );

   //
   // Thumbnail Settings
   //
   conf->setGroup("Thumbnails");
   setThumbnailsEnabled( conf->readBoolEntry( "Thumbnails", true ) );
   setThumbnailSize( conf->readDoubleNumEntry( "ThumbnailSize", 0.2 ) );
   setThumbnailUpdateDelay( conf->readNumEntry( "ThumbnailUpdateDelay", 10 ) );
   setEmbedThumbnails( conf->readBoolEntry( "EmbedThumbnails", false ) );

   //
   // Behaviour Settings
   //
   conf->setGroup("Behaviour");
   setNotifierEnabled( conf->readBoolEntry( "StartupNotifier", true ) );
   setShowModified( conf->readBoolEntry( "ModifiedIndicator", true ) );
   setShowProgress( conf->readBoolEntry( "ProgressIndicator", false ) );
   setShowAttention( conf->readBoolEntry( "AttentionIndicator", true ) );
   setShowAllWindows( conf->readBoolEntry( "ShowAllWindows", true ) );
   setGroupWindows( conf->readBoolEntry( "GroupWindows", true ) );
   setGroupInactiveDesktops( conf->readBoolEntry( "GroupInactiveDesktops", false ) );
   setOnlyShowMinimized( conf->readBoolEntry( "OnlyShowMinimized", false ) );

   //
   // Layout Settings
   //
   conf->setGroup("Layout");

   setDirection( (Direction) conf->readNumEntry( "Direction", TQBoxLayout::LeftToRight ) );
   setOrientation( (Qt::Orientation) conf->readNumEntry( "Orientation", Qt::Horizontal ) );
   setMaxBoxes( conf->readUnsignedNumEntry( "MaxBoxes", 0 ) );

   TQPoint pos(100, 100);
   setDetachedPosition( conf->readPointEntry( "DetachedPosition", &pos ) );
   setDetached( conf->readBoolEntry( "Detached", false ) );

   //
   // Custom Items
   //
   conf->setGroup("Custom Items");
   setShowClock( conf->readBoolEntry( "ShowClock", true ) );
   setShowLoad( conf->readBoolEntry( "ShowLoad", true ) );

   //    fillBg = conf->readBoolEntry( "FillIconBackgrounds", /*true*/ false );
   //    fillActiveBg = conf->readBoolEntry( "FillActiveIconBackground", true );
   //    enablePopup = conf->readBoolEntry( "EnablePopup", true );

   setUpdatesEnabled( updates );
   emit configChanged();
}

void KasTasker::writeConfig( TDEConfig *conf )
{
    if ( !conf ) {
	kdWarning() << "KasTasker::writeConfig() got a null TDEConfig" << endl;
	return;
    }

    if ( master() ) {
	kdWarning() << "KasTasker::writeConfig() for child bar" << endl;
	return;
    }

    conf->setGroup("Appearance");
    conf->writeEntry( "ItemSize", itemSize() );
    conf->writeEntry( "ItemExtent", itemExtent() );
    conf->writeEntry( "Transparent", isTransparent() );
    conf->writeEntry( "EnableTint", hasTint() );
    conf->writeEntry( "TintColor", tintColor() );
    conf->writeEntry( "TintAmount", tintAmount() );
    conf->writeEntry( "PaintInactiveFrames", paintInactiveFrames() );

    conf->setGroup("Colors");
    conf->writeEntry( "LabelPenColor", resources()->labelPenColor() );
    conf->writeEntry( "LabelBgColor", resources()->labelBgColor() );
    conf->writeEntry( "InactivePenColor", resources()->inactivePenColor() );
    conf->writeEntry( "InactiveBgColor", resources()->inactiveBgColor() );
    conf->writeEntry( "ActivePenColor", resources()->activePenColor() );
    conf->writeEntry( "ActiveBgColor", resources()->activeBgColor() );
    conf->writeEntry( "ProgressColor", resources()->progressColor() );
    conf->writeEntry( "AttentionColor", resources()->attentionColor() );

    conf->setGroup("Thumbnails");
    conf->writeEntry( "Thumbnails", thumbnailsEnabled() );
    conf->writeEntry( "ThumbnailSize", thumbnailSize() );
    conf->writeEntry( "ThumbnailUpdateDelay", thumbnailUpdateDelay() );
    conf->writeEntry( "EmbedThumbnails", embedThumbnails() );

    conf->setGroup("Behaviour");
    conf->writeEntry( "StartupNotifier", notifierEnabled() );
    conf->writeEntry( "ModifiedIndicator", showModified() );
    conf->writeEntry( "ProgressIndicator", showProgress() );
    conf->writeEntry( "AttentionIndicator", showAttention() );
    conf->writeEntry( "ShowAllWindows", showAllWindows() );
    conf->writeEntry( "GroupWindows", groupWindows() );
    conf->writeEntry( "GroupInactiveDesktops", groupInactiveDesktops() );
    conf->writeEntry( "OnlyShowMinimized", onlyShowMinimized() );

    conf->setGroup("Layout");
    conf->writeEntry( "Orientation", orientation() );
    conf->writeEntry( "Direction", direction() );
    conf->writeEntry( "Detached", isDetached() );

    conf->setGroup("Custom Items");
    conf->writeEntry( "ShowClock", showClock() );
    conf->writeEntry( "ShowLoad", showLoad() );
}

void KasTasker::writeLayout()
{
    if ( !conf )
	return;

    conf->setGroup("Layout");
    conf->writeEntry( "Orientation", orientation() );
    conf->writeEntry( "Direction", direction() );
    conf->writeEntry( "Detached", isDetached() );
    conf->writeEntry( "DetachedPosition", detachedPosition() );
    conf->sync();
}

void KasTasker::showPreferences()
{
   KasPrefsDialog *dlg = new KasPrefsDialog( this );
   dlg->exec();
   delete dlg;

   readConfig();
}

void KasTasker::showAbout()
{
  KasAboutDialog *dlg = new KasAboutDialog( 0 );
  dlg->exec();
  delete dlg;
}

