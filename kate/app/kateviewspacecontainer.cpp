/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//BEGIN Includes
#include "kateviewspacecontainer.h"
#include "kateviewspacecontainer.moc"

#include "katetabwidget.h"
#include "katemainwindow.h"
#include "katedocmanager.h"
#include "kateviewmanager.h"
#include "kateviewspace.h"

#include <dcopclient.h>
#include <tdeaction.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <tdediroperator.h>
#include <kdockwidget.h>
#include <kencodingfiledialog.h>
#include <kiconloader.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdetoolbar.h>
#include <tdemessagebox.h>
#include <ksimpleconfig.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <tdeglobalsettings.h>
#include <kstringhandler.h>

#include <tdetexteditor/encodinginterface.h>

#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqstringlist.h>
#include <tqvbox.h>
#include <tqtimer.h>
#include <tqfileinfo.h>

//END Includes

KateViewSpaceContainer::KateViewSpaceContainer (TQWidget *parent, KateViewManager *viewManager)
 : TQVBox  (parent)
 , m_viewManager(viewManager)
 , m_blockViewCreationAndActivation (false)
 , m_activeViewRunning (false)
 , m_pendingViewCreation(false)
{
  // no memleaks
  m_viewList.setAutoDelete(true);
  m_viewSpaceList.setAutoDelete(true);

  KateViewSpace* vs = new KateViewSpace( this, this );
  connect(this, TQT_SIGNAL(statusChanged(Kate::View *, int, int, int, bool, int, const TQString&)), vs, TQT_SLOT(slotStatusChanged(Kate::View *, int, int, int, bool, int, const TQString&)));
  vs->setActive( true );
  m_viewSpaceList.append(vs);
  connect( this, TQT_SIGNAL(viewChanged()), this, TQT_SLOT(slotViewChanged()) );
  connect(KateDocManager::self(), TQT_SIGNAL(initialDocumentReplaced()), this, TQT_SIGNAL(viewChanged()));

  connect(KateDocManager::self(),TQT_SIGNAL(documentCreated(Kate::Document *)),this,TQT_SLOT(documentCreated(Kate::Document *)));
  connect(KateDocManager::self(),TQT_SIGNAL(documentDeleted(uint)),this,TQT_SLOT(documentDeleted(uint)));
}

KateViewSpaceContainer::~KateViewSpaceContainer ()
{
  m_viewList.setAutoDelete(false);
  m_viewSpaceList.setAutoDelete(false);
}

void KateViewSpaceContainer::documentCreated (Kate::Document *doc)
{
  if (m_blockViewCreationAndActivation) return;

  if (!activeView())
    activateView (doc->documentNumber());
}

void KateViewSpaceContainer::documentDeleted (uint)
{
  if (m_blockViewCreationAndActivation) return;

  // just for the case we close a document out of many and this was the active one
  // if all docs are closed, this will be handled by the documentCreated
  if (!activeView() && (KateDocManager::self()->documents() > 0))
    createView (KateDocManager::self()->document(KateDocManager::self()->documents()-1));
}

bool KateViewSpaceContainer::createView ( Kate::Document *doc )
{
  if (m_blockViewCreationAndActivation) return false;

  // create doc
  if (!doc)
    doc = KateDocManager::self()->createDoc ();

  // create view
  Kate::View *view = (Kate::View *) doc->createView (this, 0L);

  m_viewList.append (view);

  // disable settings dialog action
  view->actionCollection()->remove (view->actionCollection()->action( "set_confdlg" ));

  // popup menu
  view->installPopup ((TQPopupMenu*)(mainWindow()->factory()->container("tdetexteditor_popup", mainWindow())) );

  connect(view->getDoc(),TQT_SIGNAL(nameChanged(Kate::Document *)),this,TQT_SLOT(statusMsg()));
  connect(view,TQT_SIGNAL(cursorPositionChanged()),this,TQT_SLOT(statusMsg()));
  connect(view,TQT_SIGNAL(newStatus()),this,TQT_SLOT(statusMsg()));
  connect(view->getDoc(), TQT_SIGNAL(undoChanged()), this, TQT_SLOT(statusMsg()));
  connect(view,TQT_SIGNAL(dropEventPass(TQDropEvent *)), mainWindow(),TQT_SLOT(slotDropEvent(TQDropEvent *)));
  connect(view,TQT_SIGNAL(gotFocus(Kate::View *)),this,TQT_SLOT(activateSpace(Kate::View *)));

  activeViewSpace()->addView( view );
  activateView( view );
  connect( doc, TQT_SIGNAL(modifiedOnDisc(Kate::Document *, bool, unsigned char)),
      activeViewSpace(), TQT_SLOT(modifiedOnDisc(Kate::Document *, bool, unsigned char)) );

  return true;
}

bool KateViewSpaceContainer::deleteView (Kate::View *view, bool delViewSpace)
{
  if (!view) return true;

  KateViewSpace *viewspace = (KateViewSpace *)view->parentWidget()->parentWidget();

  viewspace->removeView (view);

  mainWindow()->guiFactory ()->removeClient (view);

  // remove view from list and memory !!
  m_viewList.remove (view);

  if (delViewSpace)
    if ( viewspace->viewCount() == 0 )
      removeViewSpace( viewspace );

  return true;
}

KateViewSpace* KateViewSpaceContainer::activeViewSpace ()
{
  TQPtrListIterator<KateViewSpace> it(m_viewSpaceList);

  for (; it.current(); ++it)
  {
    if ( it.current()->isActiveSpace() )
      return it.current();
  }

  if (m_viewSpaceList.count() > 0)
  {
    m_viewSpaceList.first()->setActive( true );
    return m_viewSpaceList.first();
  }

  return 0L;
}

Kate::View* KateViewSpaceContainer::activeView ()
{
  if (m_activeViewRunning)
    return 0L;

  m_activeViewRunning = true;

  for (TQPtrListIterator<Kate::View> it(m_viewList); it.current(); ++it)
  {
    if ( it.current()->isActive() )
    {
      m_activeViewRunning = false;
      return it.current();
    }
  }

  // if we get to here, no view isActive()
  // first, try to get one from activeViewSpace()
  KateViewSpace* vs;
  if ( (vs = activeViewSpace()) )
  {
    if ( vs->currentView() )
    {
      activateView (vs->currentView());

      m_activeViewRunning = false;
      return vs->currentView();
    }
  }

  // last attempt: just pick first
  if (m_viewList.count() > 0)
  {
    activateView (m_viewList.first());

    m_activeViewRunning = false;
    return m_viewList.first();
  }

  m_activeViewRunning = false;

  // no views exists!
  return 0L;
}

void KateViewSpaceContainer::setActiveSpace ( KateViewSpace* vs )
{
   if (activeViewSpace())
     activeViewSpace()->setActive( false );

   vs->setActive( true, viewSpaceCount() > 1 );
}

void KateViewSpaceContainer::setActiveView ( Kate::View* view )
{
  if (activeView())
    activeView()->setActive( false );

  view->setActive( true );
}

void KateViewSpaceContainer::activateSpace (Kate::View* v)
{
  if (!v) return;

  KateViewSpace* vs = (KateViewSpace*)v->parentWidget()->parentWidget();

  if (!vs->isActiveSpace()) {
    setActiveSpace (vs);
    activateView(v);
  }
}

void KateViewSpaceContainer::reactivateActiveView() {
  Kate::View *view=activeView();
  if (view) {
    view->setActive(false);
    activateView(view);
  } else if (m_pendingViewCreation) {
    m_pendingViewCreation=false;
    disconnect(m_pendingDocument,TQT_SIGNAL(nameChanged(Kate::Document *)),this,TQT_SLOT(slotPendingDocumentNameChanged()));
    createView(m_pendingDocument);
  }
}

void KateViewSpaceContainer::activateView ( Kate::View *view )
{
  if (!view) return;

  if (!view->isActive())
  {
    if ( !activeViewSpace()->showView (view) )
    {
      // since it wasn't found, give'em a new one
      createView ( view->getDoc() );
      return;
    }

    setActiveView (view);
    m_viewList.findRef (view);

    mainWindow()->toolBar ()->setUpdatesEnabled (false);

    if (m_viewManager->guiMergedView)
      mainWindow()->guiFactory()->removeClient (m_viewManager->guiMergedView );

    m_viewManager->guiMergedView = view;

    if (!m_blockViewCreationAndActivation)
      mainWindow()->guiFactory ()->addClient( view );

    mainWindow()->toolBar ()->setUpdatesEnabled (true);

    statusMsg();

    emit viewChanged ();
  }

  KateDocManager::self()->setActiveDocument(view->getDoc());
}

void KateViewSpaceContainer::activateView( uint documentNumber )
{
  if ( activeViewSpace()->showView(documentNumber) ) {
    activateView( activeViewSpace()->currentView() );
  }
  else
  {
    TQPtrListIterator<Kate::View> it(m_viewList);
    for ( ;it.current(); ++it)
    {
      if ( it.current()->getDoc()->documentNumber() == documentNumber  )
      {
        createView( it.current()->getDoc() );
        return;
      }
    }

    Kate::Document *d = (Kate::Document *)KateDocManager::self()->documentWithID(documentNumber);
    createView (d);
  }
}

uint KateViewSpaceContainer::viewCount ()
{
  return m_viewList.count();
}

uint KateViewSpaceContainer::viewSpaceCount ()
{
  return m_viewSpaceList.count();
}

void KateViewSpaceContainer::slotViewChanged()
{
  if ( activeView() && !activeView()->hasFocus())
    activeView()->setFocus();
}

void KateViewSpaceContainer::activateNextView()
{
  uint i = m_viewSpaceList.find (activeViewSpace())+1;

  if (i >= m_viewSpaceList.count())
    i=0;

  setActiveSpace (m_viewSpaceList.at(i));
  activateView(m_viewSpaceList.at(i)->currentView());
}

void KateViewSpaceContainer::activatePrevView()
{
  int i = m_viewSpaceList.find (activeViewSpace())-1;

  if (i < 0)
    i=m_viewSpaceList.count()-1;

  setActiveSpace (m_viewSpaceList.at(i));
  activateView(m_viewSpaceList.at(i)->currentView());
}

void KateViewSpaceContainer::closeViews(uint documentNumber)
{
    TQPtrList<Kate::View> closeList;

    for (uint z=0 ; z < m_viewList.count(); z++)
    {
      Kate::View* current = m_viewList.at(z);
      if ( current->getDoc()->documentNumber() == documentNumber )
      {
        closeList.append (current);
      }
    }

    while ( !closeList.isEmpty() )
    {
      Kate::View *view = closeList.first();
      deleteView (view, true);
      closeList.removeFirst();
    }

  if (m_blockViewCreationAndActivation) return;
  TQTimer::singleShot(0,this,TQT_SIGNAL(viewChanged()));
  //emit m_viewManager->viewChanged ();
}

void KateViewSpaceContainer::slotPendingDocumentNameChanged() {
          TQString c;
          if (m_pendingDocument->url().isEmpty() || (!showFullPath))
          {
            c = m_pendingDocument->docName();
          }
          else
          {
            c = m_pendingDocument->url().prettyURL();
          }
          setCaption(KStringHandler::lsqueeze(c,32));
}

void KateViewSpaceContainer::statusMsg ()
{
  if (!activeView()) return;

  Kate::View* v = activeView();

  bool readOnly =  !v->getDoc()->isReadWrite();
  uint config =  v->getDoc()->configFlags();

  int ovr = 0;
  if (readOnly)
    ovr = 0;
  else
  {
    if (config & Kate::Document::cfOvr)
    {
      ovr=1;
    }
    else
    {
      ovr=2;
    }
  }

  int mod = (int)v->getDoc()->isModified();
  bool block=v->getDoc()->blockSelectionMode();

  TQString c;
  if (v->getDoc()->url().isEmpty() || (!showFullPath))
  {
    c = v->getDoc()->docName();
  }
  else
  {
    c = v->getDoc()->url().prettyURL();
  }

  m_viewManager->mainWindow()->tabWidget()->changeTab (this, KStringHandler::lsqueeze(c,32));
  emit statusChanged (v, v->cursorLine(), v->cursorColumn(), ovr,block, mod, KStringHandler::lsqueeze(c,64));
  emit statChanged ();
}

void KateViewSpaceContainer::splitViewSpace( KateViewSpace* vs,
                                      bool isHoriz,
                                      bool atTop)
{
//   kdDebug(13001)<<"splitViewSpace()"<<endl;

  if (!activeView()) return;
  if (!vs) vs = activeViewSpace();

  bool isFirstTime = vs->parentWidget() == this;

  TQValueList<int> psizes;
  if ( ! isFirstTime )
    if ( TQSplitter *ps = static_cast<TQSplitter*>(vs->parentWidget()->tqt_cast(TQSPLITTER_OBJECT_NAME_STRING)) )
      psizes = ps->sizes();

  Qt::Orientation o = isHoriz ? Qt::Vertical : Qt::Horizontal;
  KateMDI::Splitter* s = new KateMDI::Splitter(o, vs->parentWidget());
  s->setOpaqueResize( TDEGlobalSettings::opaqueResize() );

  if (! isFirstTime) {
    // anders: make sure the split' viewspace is always
    // correctly positioned.
    // If viewSpace is the first child, the new splitter must be moveToFirst'd
    if ( !((KateMDI::Splitter*)vs->parentWidget())->isLastChild( vs ) )
       ((KateMDI::Splitter*)s->parentWidget())->moveToFirst( s );
  }
  vs->reparent( s, 0, TQPoint(), true );
  KateViewSpace* vsNew = new KateViewSpace( this, s );

  if (atTop)
    s->moveToFirst( vsNew );

  if (!isFirstTime)
    if (TQSplitter *ps = static_cast<TQSplitter*>(s->parentWidget()->tqt_cast(TQSPLITTER_OBJECT_NAME_STRING)) )
      ps->setSizes( psizes );

  s->show();

  TQValueList<int> sizes;
  int space = 50;//isHoriz ? s->parentWidget()->height()/2 : s->parentWidget()->width()/2;
  sizes << space << space;
  s->setSizes( sizes );

  connect(this, TQT_SIGNAL(statusChanged(Kate::View *, int, int, int, bool, int, const TQString &)), vsNew, TQT_SLOT(slotStatusChanged(Kate::View *, int, int,int, bool, int, const TQString &)));
  m_viewSpaceList.append( vsNew );
  activeViewSpace()->setActive( false );
  vsNew->setActive( true, true );
  vsNew->show();

  createView (activeView()->getDoc());

  if (this == m_viewManager->activeContainer())
    m_viewManager->updateViewSpaceActions ();

//   kdDebug(13001)<<"splitViewSpace() - DONE!"<<endl;
}

void KateViewSpaceContainer::removeViewSpace (KateViewSpace *viewspace)
{
  // abort if viewspace is 0
  if (!viewspace) return;

  // abort if this is the last viewspace
  if (m_viewSpaceList.count() < 2) return;

  KateMDI::Splitter* p = (KateMDI::Splitter*)viewspace->parentWidget();

  // find out if it is the first child for repositioning
  // see below
  bool pIsFirst = false;

  // save some size information
  KateMDI::Splitter* pp=0L;
  TQValueList<int> ppsizes;
  if (m_viewSpaceList.count() > 2 && p->parentWidget() != this)
  {
    pp = (KateMDI::Splitter*)p->parentWidget();
    ppsizes = pp->sizes();
    pIsFirst = !pp->isLastChild( p ); // simple logic, right-
  }

  // Figure out where to put views that are still needed
  KateViewSpace* next;
  if (m_viewSpaceList.find(viewspace) == 0)
    next = m_viewSpaceList.next();
  else
    next = m_viewSpaceList.prev();

  // Reparent views in viewspace that are last views, delete the rest.
  int vsvc = viewspace->viewCount();
  while (vsvc > 0)
  {
    if (viewspace->currentView())
    {
      Kate::View* v = viewspace->currentView();

      if (v->isLastView())
      {
        viewspace->removeView(v);
        next->addView( v, false );
      }
      else
      {
        deleteView( v, false );
      }
    }
    vsvc = viewspace->viewCount();
  }

  m_viewSpaceList.remove( viewspace );

  // reparent the other sibling of the parent.
  while (!p->childrenListObject().isEmpty())
  {
    TQWidget* other = ((TQWidget *)(( TQPtrList<TQObject>)p->childrenListObject()).first());

    other->reparent( p->parentWidget(), 0, TQPoint(), true );
    // We also need to find the right viewspace to become active
    if (pIsFirst)
       ((KateMDI::Splitter*)p->parentWidget())->moveToFirst( other );
    if ( other->isA("KateViewSpace") ) {
      setActiveSpace( (KateViewSpace*)other );
    }
    else {
      TQObjectList* l = other->queryList( "KateViewSpace" );
      if ( l->first() != 0 ) { // I REALLY hope so!
        setActiveSpace( (KateViewSpace*)l->first() );
      }
      delete l;
    }
  }

  delete p;

  if (!ppsizes.isEmpty())
    pp->setSizes( ppsizes );

  // find the view that is now active.
  Kate::View* v = activeViewSpace()->currentView();
  if ( v )
    activateView( v );

  if (this == m_viewManager->activeContainer())
    m_viewManager->updateViewSpaceActions ();

  emit viewChanged();
}

void KateViewSpaceContainer::slotCloseCurrentViewSpace()
{
  removeViewSpace(activeViewSpace());
}

void KateViewSpaceContainer::setShowFullPath( bool enable )
{
  showFullPath = enable;
  statusMsg ();
  //m_mainWindow->slotWindowActivated ();
}

/**
 * session config functions
 */

void KateViewSpaceContainer::saveViewConfiguration(TDEConfig *config,const TQString& group)
{
  bool weHaveSplittersAlive (viewSpaceCount() > 1);

  config->setGroup (group); //"View Configuration");
  config->writeEntry ("Splitters", weHaveSplittersAlive);

  // no splitters around
  if (!weHaveSplittersAlive)
  {
    config->writeEntry("Active Viewspace", 0);
    m_viewSpaceList.first()->saveConfig ( config, 0,group );

    return;
  }

  // I need the first splitter, the one which has this as parent.
  KateMDI::Splitter* s;
  TQObjectList *l = queryList("KateMDI::Splitter", 0, false, false);
  TQObjectListIt it( *l );

  if ( (s = (KateMDI::Splitter*)it.current()) != 0 )
    saveSplitterConfig( s, 0, config , group);

  delete l;
}

void KateViewSpaceContainer::restoreViewConfiguration (TDEConfig *config, const TQString& group)
{
  config->setGroup(group);
  //config->setGroup ("View Configuration");

  // no splitters around, ohhh :()
  if (!config->readBoolEntry ("Splitters"))
  {
    // only add the new views needed, let the old stay, won't hurt if one around
    m_viewSpaceList.first ()->restoreConfig (this, config, TQString(group+"-ViewSpace 0"));
  }
  else
  {
    // send all views + their gui to **** ;)
    for (uint i=0; i < m_viewList.count(); i++)
      mainWindow()->guiFactory ()->removeClient (m_viewList.at(i));

    m_viewList.clear ();

    // cu viewspaces
    m_viewSpaceList.clear();

    // call restoreSplitter for Splitter 0
    restoreSplitter( config, TQString(group+"-Splitter 0"), this,group );
  }

  // finally, make the correct view active.
  config->setGroup (group);
/*
  KateViewSpace *vs = m_viewSpaceList.at( config->readNumEntry("Active ViewSpace") );
  if ( vs )
    activateSpace( vs->currentView() );
  */
}


void KateViewSpaceContainer::saveSplitterConfig( KateMDI::Splitter* s, int idx, TDEConfig* config, const TQString& viewConfGrp )
{
  TQString grp = TQString(viewConfGrp+"-Splitter %1").arg(idx);
  config->setGroup(grp);

  // Save sizes, orient, children for this splitter
  config->writeEntry( "Sizes", s->sizes() );
  config->writeEntry( "Orientation", s->orientation() );

  TQStringList childList;
  // a katesplitter has two children, of which one may be a KateSplitter.
  const TQObjectList l = s->childrenListObject();
  TQObjectListIt it( l );
  TQObject* obj;
  for (; it.current(); ++it) {
   obj = it.current();
   TQString n;  // name for child list, see below
   // For KateViewSpaces, ask them to save the file list.
   if ( obj->isA("KateViewSpace") ) {
     n = TQString(viewConfGrp+"-ViewSpace %1").arg( m_viewSpaceList.find((KateViewSpace*)obj) );
     ((KateViewSpace*)obj)->saveConfig ( config, m_viewSpaceList.find((KateViewSpace*)obj), viewConfGrp);
     // save active viewspace
     if ( ((KateViewSpace*)obj)->isActiveSpace() ) {
       config->setGroup(viewConfGrp);
       config->writeEntry("Active Viewspace", m_viewSpaceList.find((KateViewSpace*)obj) );
     }
   }
   // For KateSplitters, recurse
   else if ( obj->isA("KateMDI::Splitter") ) {
     idx++;
     saveSplitterConfig( (KateMDI::Splitter*)obj, idx, config,viewConfGrp);
     n = TQString(viewConfGrp+"-Splitter %1").arg( idx );
   }
   // make sure list goes in right place!
   if (!n.isEmpty()) {
     if ( childList.count() > 0 && ! s->isLastChild( (TQWidget*)obj ) )
       childList.prepend( n );
     else
       childList.append( n );
   }
  }

  // reset config group.
  config->setGroup(grp);
  config->writeEntry("Children", childList);
}

void KateViewSpaceContainer::restoreSplitter( TDEConfig* config, const TQString &group, TQWidget* parent, const TQString& viewConfGrp)
{
  config->setGroup( group );

  KateMDI::Splitter* s = new KateMDI::Splitter((Qt::Orientation)config->readNumEntry("Orientation"), parent);

  TQStringList children = config->readListEntry( "Children" );
  for (TQStringList::Iterator it=children.begin(); it!=children.end(); ++it)
  {
    // for a viewspace, create it and open all documents therein.
    if ( (*it).startsWith(viewConfGrp+"-ViewSpace") )
    {
     KateViewSpace* vs = new KateViewSpace( this, s );

     connect(this, TQT_SIGNAL(statusChanged(Kate::View *, int, int, int, bool, int, const TQString &)), vs, TQT_SLOT(slotStatusChanged(Kate::View *, int, int, int, bool, int, const TQString &)));

     if (m_viewSpaceList.isEmpty())
       vs->setActive (true);

     m_viewSpaceList.append( vs );

     vs->show();
     setActiveSpace( vs );

     vs->restoreConfig (this, config, *it);
    }
    else
    {
      // for a splitter, recurse.
      restoreSplitter( config, TQString(*it), s, viewConfGrp );
    }
  }

  // set sizes
  config->setGroup( group );
  s->setSizes( config->readIntListEntry("Sizes") );
  s->show();
}

KateMainWindow *KateViewSpaceContainer::mainWindow() {
  return m_viewManager->mainWindow();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
