/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Michael Reiher <michael.reiher@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <math.h>

#include <tqpainter.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqtoolbutton.h>
#include <tqtabbar.h>
#include <tqptrlist.h>
#include <tqpopupmenu.h>
#include <tqkeysequence.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <kprogress.h>
#include <tdelocale.h>
#include <ksqueezedtextlabel.h>
#include <networkstatusindicator.h>

#include "konq_events.h"
#include "konq_frame.h"
#include "konq_tabs.h"
#include "konq_view.h"
#include "konq_viewmgr.h"

#include <konq_pixmapprovider.h>
#include <tdestdaccel.h>
#include <assert.h>


#define DEFAULT_HEADER_HEIGHT 13

void KonqCheckBox::drawButton( TQPainter *p )
{
    //static TQPixmap indicator_anchor( UserIcon( "indicator_anchor" ) );
    static TQPixmap indicator_connect( UserIcon( "indicator_connect" ) );
    static TQPixmap indicator_noconnect( UserIcon( "indicator_noconnect" ) );

   if (isOn() || isDown())
      p->drawPixmap(0,0,indicator_connect);
   else
      p->drawPixmap(0,0,indicator_noconnect);
}

KonqFrameStatusBar::KonqFrameStatusBar( KonqFrame *_parent, const char *_name )
  : KStatusBar( _parent, _name ),
    m_pParentKonqFrame( _parent )
{
    setSizeGripEnabled( false );

    m_led = new TQLabel( this );
    m_led->setAlignment( Qt::AlignCenter );
    m_led->setSizePolicy(TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ));
    addWidget( m_led, 0, false ); // led (active view indicator)
    m_led->hide();

    m_pStatusLabel = new KSqueezedTextLabel( this );
    m_pStatusLabel->setMinimumSize( 0, 0 );
    m_pStatusLabel->setSizePolicy(TQSizePolicy( TQSizePolicy::Ignored, TQSizePolicy::Fixed ));
    m_pStatusLabel->installEventFilter(this);
    addWidget( m_pStatusLabel, 1 /*stretch*/, false ); // status label

    m_pLinkedViewCheckBox = new KonqCheckBox( this, "m_pLinkedViewCheckBox" );
    m_pLinkedViewCheckBox->setFocusPolicy(TQ_NoFocus);
    m_pLinkedViewCheckBox->setSizePolicy(TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ));
    TQWhatsThis::add( m_pLinkedViewCheckBox,
                     i18n("Checking this box on at least two views sets those views as 'linked'. "
                          "Then, when you change directories in one view, the other views "
                          "linked with it will automatically update to show the current directory. "
                          "This is especially useful with different types of views, such as a "
                          "directory tree with an icon view or detailed view, and possibly a "
                          "terminal emulator window." ) );
    addWidget( m_pLinkedViewCheckBox, 0, true /*permanent->right align*/ );
    connect( m_pLinkedViewCheckBox, TQT_SIGNAL(toggled(bool)),
            this, TQT_SIGNAL(linkedViewClicked(bool)) );

    m_progressBar = new KProgress( this );
    m_progressBar->setMaximumHeight(fontMetrics().height());
    m_progressBar->hide();
    addWidget( m_progressBar, 0, true /*permanent->right align*/ );

//     // FIXME: This was added by OpenSUSE; someone needs to figure out what it does and how to fix it!
//     StatusBarNetworkStatusIndicator * indicator = new StatusBarNetworkStatusIndicator( this, "networkstatusindicator" );
//     addWidget( indicator, 0, false );
//     indicator->init();

    fontChange(TQFont());
    installEventFilter( this );
}

KonqFrameStatusBar::~KonqFrameStatusBar()
{
}

void KonqFrameStatusBar::fontChange(const TQFont & /* oldFont */)
{
    int h = fontMetrics().height();
    if ( h < DEFAULT_HEADER_HEIGHT ) h = DEFAULT_HEADER_HEIGHT;
    m_led->setFixedHeight( h + 2 );
    m_progressBar->setFixedHeight( h + 2 );
    // This one is important. Otherwise richtext messages make it grow in height.
    m_pStatusLabel->setFixedHeight( h + 2 );

}

void KonqFrameStatusBar::resizeEvent( TQResizeEvent* ev )
{
    //m_progressBar->setGeometry( width()-160, 0, 140, height() );
    //m_pLinkedViewCheckBox->move( width()-15, m_yOffset ); // right justify
    KStatusBar::resizeEvent( ev );
}

// I don't think this code _ever_ gets called!
// I don't want to remove it, though.  :-)
void KonqFrameStatusBar::mousePressEvent( TQMouseEvent* event )
{
   TQWidget::mousePressEvent( event );
   if ( !m_pParentKonqFrame->childView()->isPassiveMode() )
   {
      emit clicked();
      update();
   }

   //Blocks menu of custom status bar entries
   //if (event->button()==RightButton)
   //   splitFrameMenu();
}

void KonqFrameStatusBar::splitFrameMenu()
{
   KonqMainWindow * mw = m_pParentKonqFrame->childView()->mainWindow();

   // We have to ship the remove view action ourselves,
   // since this may not be the active view (passive view)
   TDEAction actRemoveView(i18n("Close View"), "view_remove", 0, TQT_TQOBJECT(m_pParentKonqFrame), TQT_SLOT(slotRemoveView()), (TQObject*)0, "removethisview");
   //KonqView * nextView = mw->viewManager()->chooseNextView( m_pParentKonqFrame->childView() );
   actRemoveView.setEnabled( mw->mainViewsCount() > 1 || m_pParentKonqFrame->childView()->isToggleView() || m_pParentKonqFrame->childView()->isPassiveMode() );

   // For the rest, we borrow them from the main window
   // ###### might be not right for passive views !
   TDEActionCollection *actionColl = mw->actionCollection();

   TQPopupMenu menu;

   actionColl->action( "splitviewh" )->plug( &menu );
   actionColl->action( "splitviewv" )->plug( &menu );
   menu.insertSeparator();
   actionColl->action( "lock" )->plug( &menu );

   actRemoveView.plug( &menu );

   menu.exec(TQCursor::pos());
}

bool KonqFrameStatusBar::eventFilter(TQObject* o, TQEvent *e)
{
   if (TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_pStatusLabel) && e->type()==TQEvent::MouseButtonPress)
   {
      emit clicked();
      update();
      if ( TQT_TQMOUSEEVENT(e)->button() == Qt::RightButton)
         splitFrameMenu();
      return true;
   }
   else if ( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(this) && e->type() == TQEvent::ApplicationPaletteChange )
   {
      unsetPalette();
      updateActiveStatus();
      return true;
   }

   return false;
}

void KonqFrameStatusBar::message( const TQString &msg )
{
    // We don't use the message()/clear() mechanism of TQStatusBar because
    // it really looks ugly (the label border goes away, the active-view indicator
    // is hidden...)
    TQString saveMsg = m_savedMessage;
    slotDisplayStatusText( msg );
    m_savedMessage = saveMsg;
}

void KonqFrameStatusBar::slotDisplayStatusText(const TQString& text)
{
    //kdDebug(1202)<<"KonqFrameHeader::slotDisplayStatusText("<<text<<")"<<endl;
    //m_pStatusLabel->resize(fontMetrics().width(text),fontMetrics().height()+2);
    m_pStatusLabel->setText(text);
    m_savedMessage = text;
}

void KonqFrameStatusBar::slotClear()
{
    slotDisplayStatusText( m_savedMessage );
}

void KonqFrameStatusBar::slotLoadingProgress( int percent )
{
  if ( percent != -1 && percent != 100 ) // hide on 100 too
  {
    if ( !m_progressBar->isVisible() )
      m_progressBar->show();
  }
  else
    m_progressBar->hide();

  m_progressBar->setValue( percent );
}

void KonqFrameStatusBar::slotSpeedProgress( int bytesPerSecond )
{
  TQString sizeStr;

  if ( bytesPerSecond > 0 )
    sizeStr = i18n( "%1/s" ).arg( TDEIO::convertSize( bytesPerSecond ) );
  else
    sizeStr = i18n( "Stalled" );

  slotDisplayStatusText( sizeStr ); // let's share the same label...
}

void KonqFrameStatusBar::slotConnectToNewView(KonqView *, KParts::ReadOnlyPart *,KParts::ReadOnlyPart *newOne)
{
   if (newOne!=0)
      connect(newOne,TQT_SIGNAL(setStatusBarText(const TQString &)),this,TQT_SLOT(slotDisplayStatusText(const TQString&)));
   slotDisplayStatusText( TQString::null );
}

void KonqFrameStatusBar::showActiveViewIndicator( bool b )
{
    m_led->setShown( b );
    updateActiveStatus();
}

void KonqFrameStatusBar::showLinkedViewIndicator( bool b )
{
    m_pLinkedViewCheckBox->setShown( b );
}

void KonqFrameStatusBar::setLinkedView( bool b )
{
    m_pLinkedViewCheckBox->blockSignals( true );
    m_pLinkedViewCheckBox->setChecked( b );
    m_pLinkedViewCheckBox->blockSignals( false );
}

void KonqFrameStatusBar::updateActiveStatus()
{
    if ( !m_led->isShown() )
    {
        unsetPalette();
        return;
    }

    bool hasFocus = m_pParentKonqFrame->isActivePart();

    const TQColorGroup& activeCg = kapp->palette().active();
    setPaletteBackgroundColor( hasFocus ? activeCg.midlight() : activeCg.mid() );

    static TQPixmap indicator_viewactive( UserIcon( "indicator_viewactive" ) );
    static TQPixmap indicator_empty( UserIcon( "indicator_empty" ) );
    m_led->setPixmap( hasFocus ? indicator_viewactive : indicator_empty );
}

//###################################################################

void KonqFrameBase::printFrameInfo(const TQString& spaces)
{
    kdDebug(1202) << spaces << "KonqFrameBase " << this << " printFrameInfo not implemented in derived class!" << endl;
}

//###################################################################

KonqFrame::KonqFrame( TQWidget* parent, KonqFrameContainerBase *parentContainer, const char *name )
:TQWidget(parent,name)
{
   //kdDebug(1202) << "KonqFrame::KonqFrame()" << endl;

   m_pLayout = 0L;
   m_pView = 0L;

   // the frame statusbar
   m_pStatusBar = new KonqFrameStatusBar( this, "KonquerorFrameStatusBar");
   m_pStatusBar->setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
   connect(m_pStatusBar, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotStatusBarClicked()));
   connect( m_pStatusBar, TQT_SIGNAL( linkedViewClicked( bool ) ), this, TQT_SLOT( slotLinkedViewClicked( bool ) ) );
   m_separator = 0;
   m_pParentContainer = parentContainer;
}

KonqFrame::~KonqFrame()
{
   //kdDebug(1202) << "KonqFrame::~KonqFrame() " << this << endl;
}

bool KonqFrame::isActivePart()
{
  return ( m_pView &&
           static_cast<KonqView*>(m_pView) == m_pView->mainWindow()->currentView() );
}

void KonqFrame::listViews( ChildViewList *viewList )
{
  viewList->append( childView() );
}

void KonqFrame::saveConfig( TDEConfig* config, const TQString &prefix, bool saveURLs, KonqFrameBase* docContainer, int /*id*/, int /*depth*/ )
{
  if (saveURLs)
    config->writePathEntry( TQString::fromLatin1( "URL" ).prepend( prefix ),
                        childView()->url().url() );
  config->writeEntry( TQString::fromLatin1( "ServiceType" ).prepend( prefix ), childView()->serviceType() );
  config->writeEntry( TQString::fromLatin1( "ServiceName" ).prepend( prefix ), childView()->service()->desktopEntryName() );
  config->writeEntry( TQString::fromLatin1( "PassiveMode" ).prepend( prefix ), childView()->isPassiveMode() );
  config->writeEntry( TQString::fromLatin1( "LinkedView" ).prepend( prefix ), childView()->isLinkedView() );
  config->writeEntry( TQString::fromLatin1( "ToggleView" ).prepend( prefix ), childView()->isToggleView() );
  config->writeEntry( TQString::fromLatin1( "LockedLocation" ).prepend( prefix ), childView()->isLockedLocation() );
  //config->writeEntry( TQString::fromLatin1( "ShowStatusBar" ).prepend( prefix ), statusbar()->isVisible() );
  if (this == docContainer) config->writeEntry( TQString::fromLatin1( "docContainer" ).prepend( prefix ), true );

  KonqConfigEvent ev( config, prefix+"_", true/*save*/);
  TQApplication::sendEvent( childView()->part(), &ev );
}

void KonqFrame::copyHistory( KonqFrameBase *other )
{
    assert( other->frameType() == "View" );
    childView()->copyHistory( static_cast<KonqFrame *>( other )->childView() );
}

void KonqFrame::printFrameInfo( const TQString& spaces )
{
   TQString className = "NoPart";
   if (part()) className = part()->widget()->className();
   kdDebug(1202) << spaces << "KonqFrame " << this << " visible=" << TQString("%1").arg(isVisible()) << " containing view "
                 << childView() << " visible=" << TQString("%1").arg(isVisible())
                 << " and part " << part() << " whose widget is a " << className << endl;
}

KParts::ReadOnlyPart *KonqFrame::attach( const KonqViewFactory &viewFactory )
{
   KonqViewFactory factory( viewFactory );

   // Note that we set the parent to 0.
   // We don't want that deleting the widget deletes the part automatically
   // because we already have that taken care of in KParts...

   m_pPart = factory.create( this, "view widget", 0, "" );

   assert( m_pPart->widget() );

   attachInternal();

   m_pStatusBar->slotConnectToNewView(0, 0,m_pPart);

   return m_pPart;
}

void KonqFrame::attachInternal()
{
   //kdDebug(1202) << "KonqFrame::attachInternal()" << endl;
   delete m_pLayout;

   m_pLayout = new TQVBoxLayout( this, 0, -1, "KonqFrame's TQVBoxLayout" );

   m_pLayout->addWidget( m_pPart->widget(), 1 );

   m_pLayout->addWidget( m_pStatusBar, 0 );
   m_pPart->widget()->show();

   m_pLayout->activate();

   m_pPart->widget()->installEventFilter(this);
}

bool KonqFrame::eventFilter(TQObject* /*obj*/, TQEvent *ev)
{
   if (ev->type()==TQEvent::KeyPress)
   {
      TQKeyEvent * keyEv = TQT_TQKEYEVENT(ev);
      if ((keyEv->key()==Key_Tab) && (keyEv->state()==ControlButton))
      {
         emit ((KonqFrameContainer*)parent())->ctrlTabPressed();
         return true;
      }
   }
   return false;
}

void KonqFrame::insertTopWidget( TQWidget * widget )
{
    assert(m_pLayout);
    m_pLayout->insertWidget( 0, widget );
    if (widget!=0)
       widget->installEventFilter(this);
}

void KonqFrame::setView( KonqView* child )
{
   m_pView = child;
   if (m_pView)
   {
     connect(m_pView,TQT_SIGNAL(sigPartChanged(KonqView *, KParts::ReadOnlyPart *,KParts::ReadOnlyPart *)),
             m_pStatusBar,TQT_SLOT(slotConnectToNewView(KonqView *, KParts::ReadOnlyPart *,KParts::ReadOnlyPart *)));
   }
}

void KonqFrame::setTitle( const TQString &title , TQWidget* /*sender*/)
{
  //kdDebug(1202) << "KonqFrame::setTitle( " << title << " )" << endl;
  m_title = title;
  if (m_pParentContainer) m_pParentContainer->setTitle( title , this);
}

void KonqFrame::setTabIcon( const KURL &url, TQWidget* /*sender*/ )
{
  //kdDebug(1202) << "KonqFrame::setTabIcon( " << url << " )" << endl;
  if (m_pParentContainer) m_pParentContainer->setTabIcon( url, this );
}

void KonqFrame::reparentFrame( TQWidget* parent, const TQPoint & p, bool showIt )
{
   TQWidget::reparent( parent, p, showIt );
}

void KonqFrame::slotStatusBarClicked()
{
  if ( !isActivePart() && m_pView && !m_pView->isPassiveMode() )
    m_pView->mainWindow()->viewManager()->setActivePart( part() );
}

void KonqFrame::slotLinkedViewClicked( bool mode )
{
  if ( m_pView->mainWindow()->linkableViewsCount() == 2 )
    m_pView->mainWindow()->slotLinkView();
  else
    m_pView->setLinkedView( mode );
}

void
KonqFrame::paintEvent( TQPaintEvent* )
{
#ifdef USE_QT4
   #warning [INFO] Repaint call disabled in Qt4 to prevent recursive repaint (which otherwise occurs for unknown reasons)
#else // USE_QT4
   m_pStatusBar->repaint();
#endif // USE_QT4
}

void KonqFrame::slotRemoveView()
{
   m_pView->mainWindow()->viewManager()->removeView( m_pView );
}

void KonqFrame::activateChild()
{
  if (m_pView && !m_pView->isPassiveMode() )
    m_pView->mainWindow()->viewManager()->setActivePart( part() );
}

//###################################################################

void KonqFrameContainerBase::printFrameInfo(const TQString& spaces)
{
	kdDebug(1202) << spaces << "KonqFrameContainerBase " << this << ", this shouldn't happen!" << endl;
}

//###################################################################

KonqFrameContainer::KonqFrameContainer( Orientation o,
                                        TQWidget* parent,
                                        KonqFrameContainerBase* parentContainer,
                                        const char * name)
  : TQSplitter( o, parent, name ), m_bAboutToBeDeleted(false)
{
  m_pParentContainer = parentContainer;
  m_pFirstChild = 0L;
  m_pSecondChild = 0L;
  m_pActiveChild = 0L;
  setOpaqueResize( TDEGlobalSettings::opaqueResize() );
}

KonqFrameContainer::~KonqFrameContainer()
{
    //kdDebug(1202) << "KonqFrameContainer::~KonqFrameContainer() " << this << " - " << className() << endl;
	delete m_pFirstChild;
	delete m_pSecondChild;
}

void KonqFrameContainer::listViews( ChildViewList *viewList )
{
   if( m_pFirstChild )
      m_pFirstChild->listViews( viewList );

   if( m_pSecondChild )
      m_pSecondChild->listViews( viewList );
}

void KonqFrameContainer::saveConfig( TDEConfig* config, const TQString &prefix, bool saveURLs, KonqFrameBase* docContainer, int id, int depth )
{
  int idSecond = id + (int)pow( 2.0, depth );

  //write children sizes
  config->writeEntry( TQString::fromLatin1( "SplitterSizes" ).prepend( prefix ), sizes() );

  //write children
  TQStringList strlst;
  if( firstChild() )
    strlst.append( TQString::fromLatin1( firstChild()->frameType() ) + TQString::number(idSecond - 1) );
  if( secondChild() )
    strlst.append( TQString::fromLatin1( secondChild()->frameType() ) + TQString::number( idSecond ) );

  config->writeEntry( TQString::fromLatin1( "Children" ).prepend( prefix ), strlst );

  //write orientation
  TQString o;
  if( orientation() == Qt::Horizontal )
    o = TQString::fromLatin1("Horizontal");
  else if( orientation() == Qt::Vertical )
    o = TQString::fromLatin1("Vertical");
  config->writeEntry( TQString::fromLatin1( "Orientation" ).prepend( prefix ), o );

  //write docContainer
  if (this == docContainer) config->writeEntry( TQString::fromLatin1( "docContainer" ).prepend( prefix ), true );

  if (m_pSecondChild == m_pActiveChild) config->writeEntry( TQString::fromLatin1( "activeChildIndex" ).prepend( prefix ), 1 );
  else config->writeEntry( TQString::fromLatin1( "activeChildIndex" ).prepend( prefix ), 0 );

  //write child configs
  if( firstChild() ) {
    TQString newPrefix = TQString::fromLatin1( firstChild()->frameType() ) + TQString::number(idSecond - 1);
    newPrefix.append( '_' );
    firstChild()->saveConfig( config, newPrefix, saveURLs, docContainer, id, depth + 1 );
  }

  if( secondChild() ) {
    TQString newPrefix = TQString::fromLatin1( secondChild()->frameType() ) + TQString::number( idSecond );
    newPrefix.append( '_' );
    secondChild()->saveConfig( config, newPrefix, saveURLs, docContainer, idSecond, depth + 1 );
  }
}

void KonqFrameContainer::copyHistory( KonqFrameBase *other )
{
    assert( other->frameType() == "Container" );
    if ( firstChild() )
        firstChild()->copyHistory( static_cast<KonqFrameContainer *>( other )->firstChild() );
    if ( secondChild() )
        secondChild()->copyHistory( static_cast<KonqFrameContainer *>( other )->secondChild() );
}

KonqFrameBase* KonqFrameContainer::otherChild( KonqFrameBase* child )
{
   if( firstChild() == child )
      return secondChild();
   else if( secondChild() == child )
      return firstChild();
   return 0L;
}

void KonqFrameContainer::printFrameInfo( const TQString& spaces )
{
        kdDebug(1202) << spaces << "KonqFrameContainer " << this << " visible=" << TQString("%1").arg(isVisible())
                      << " activeChild=" << m_pActiveChild << endl;
        if (!m_pActiveChild)
            kdDebug(1202) << "WARNING: " << this << " has a null active child!" << endl;
        KonqFrameBase* child = firstChild();
        if (child != 0L)
            child->printFrameInfo(spaces + "  ");
        else
            kdDebug(1202) << spaces << "  Null child" << endl;
        child = secondChild();
        if (child != 0L)
            child->printFrameInfo(spaces + "  ");
        else
            kdDebug(1202) << spaces << "  Null child" << endl;
}

void KonqFrameContainer::reparentFrame( TQWidget* parent, const TQPoint & p, bool showIt )
{
  TQWidget::reparent( parent, p, showIt );
}

void KonqFrameContainer::swapChildren()
{
  KonqFrameBase *firstCh = m_pFirstChild;
  m_pFirstChild = m_pSecondChild;
  m_pSecondChild = firstCh;
}

void KonqFrameContainer::setTitle( const TQString &title , TQWidget* sender)
{
  //kdDebug(1202) << "KonqFrameContainer::setTitle( " << title << " , " << sender << " )" << endl;
  if (m_pParentContainer && activeChild() && (sender == activeChild()->widget()))
      m_pParentContainer->setTitle( title , this);
}

void KonqFrameContainer::setTabIcon( const KURL &url, TQWidget* sender )
{
  //kdDebug(1202) << "KonqFrameContainer::setTabIcon( " << url << " , " << sender << " )" << endl;
  if (m_pParentContainer && activeChild() && (sender == activeChild()->widget()))
      m_pParentContainer->setTabIcon( url, this );
}

void KonqFrameContainer::insertChildFrame( KonqFrameBase* frame, int /*index*/  )
{
  //kdDebug(1202) << "KonqFrameContainer " << this << ": insertChildFrame " << frame << endl;

  if (frame)
  {
      if( !m_pFirstChild )
      {
          m_pFirstChild = frame;
          frame->setParentContainer(this);
          //kdDebug(1202) << "Setting as first child" << endl;
      }
      else if( !m_pSecondChild )
      {
          m_pSecondChild = frame;
          frame->setParentContainer(this);
          //kdDebug(1202) << "Setting as second child" << endl;
      }
      else
        kdWarning(1202) << this << " already has two children..."
                        << m_pFirstChild << " and " << m_pSecondChild << endl;
  } else
    kdWarning(1202) << "KonqFrameContainer " << this << ": insertChildFrame(0L) !" << endl;
}

void KonqFrameContainer::removeChildFrame( KonqFrameBase * frame )
{
  //kdDebug(1202) << "KonqFrameContainer::RemoveChildFrame " << this << ". Child " << frame << " removed" << endl;

  if( m_pFirstChild == frame )
  {
    m_pFirstChild = m_pSecondChild;
    m_pSecondChild = 0L;
  }
  else if( m_pSecondChild == frame )
    m_pSecondChild = 0L;

  else
    kdWarning(1202) << this << " Can't find this child:" << frame << endl;
}

void KonqFrameContainer::childEvent( TQChildEvent *c )
{
  // Child events cause layout changes. These are unnecassery if we are going
  // to be deleted anyway.
  if (!m_bAboutToBeDeleted)
      TQSplitter::childEvent(c);
}

void KonqFrameContainer::setRubberband( int pos  )
{
    emit setRubberbandCalled();
    TQSplitter::setRubberband( pos );
}

#include "konq_frame.moc"
