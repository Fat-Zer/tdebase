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
#include "katemainwindow.h"
#include "katemainwindow.moc"

#include "kateconfigdialog.h"
#include "kateconsole.h"
#include "katedocmanager.h"
#include "katepluginmanager.h"
#include "kateconfigplugindialogpage.h"
#include "kateviewmanager.h"
#include "kateapp.h"
#include "katefileselector.h"
#include "katefilelist.h"
#include "kategrepdialog.h"
#include "katemailfilesdialog.h"
#include "katemainwindowiface.h"
#include "kateexternaltools.h"
#include "katesavemodifieddialog.h"
#include "katemwmodonhddialog.h"
#include "katesession.h"
#include "katetabwidget.h"

#include "../interfaces/mainwindow.h"
#include "../interfaces/toolviewmanager.h"

#include <dcopclient.h>
#include <kinstance.h>
#include <tdeaboutdata.h>
#include <tdeaction.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <tdediroperator.h>
#include <kdockwidget.h>
#include <kedittoolbar.h>
#include <tdefiledialog.h>
#include <kglobalaccel.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kmimetype.h>
#include <kopenwith.h>
#include <tdepopupmenu.h>
#include <ksimpleconfig.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <kuniqueapplication.h>
#include <kurldrag.h>
#include <kdesktopfile.h>
#include <khelpmenu.h>
#include <tdemultitabbar.h>
#include <ktip.h>
#include <tdemenubar.h>
#include <kstringhandler.h>
#include <tqlayout.h>
#include <tqptrvector.h>

#include <assert.h>
#include <unistd.h>
//END

uint KateMainWindow::uniqueID = 1;

KateMainWindow::KateMainWindow (TDEConfig *sconfig, const TQString &sgroup)
  : KateMDI::MainWindow (0,(TQString(TQString("__KateMainWindow#%1").arg(uniqueID))).latin1())
{
  // first the very important id
  myID = uniqueID;
  uniqueID++;

  m_modignore = false;

  console = 0;
  greptool = 0;

  // here we go, set some usable default sizes
  if (!initialGeometrySet())
  {
    int scnum = TQApplication::desktop()->screenNumber(parentWidget());
    TQRect desk = TQApplication::desktop()->screenGeometry(scnum);

    TQSize size;

    // try to load size
    if (sconfig)
    {
      sconfig->setGroup (sgroup);
      size.setWidth (sconfig->readNumEntry( TQString::fromLatin1("Width %1").arg(desk.width()), 0 ));
      size.setHeight (sconfig->readNumEntry( TQString::fromLatin1("Height %1").arg(desk.height()), 0 ));
    }

    // if thats fails, try to reuse size
    if (size.isEmpty())
    {
      // first try to reuse size known from current or last created main window ;=)
      if (KateApp::self()->mainWindows () > 0)
      {
        KateMainWindow *win = KateApp::self()->activeMainWindow ();

        if (!win)
          win = KateApp::self()->mainWindow (KateApp::self()->mainWindows ()-1);

        size = win->size();
      }
      else // now fallback to hard defaults ;)
      {
        // first try global app config
        KateApp::self()->config()->setGroup ("MainWindow");
        size.setWidth (KateApp::self()->config()->readNumEntry( TQString::fromLatin1("Width %1").arg(desk.width()), 0 ));
        size.setHeight (KateApp::self()->config()->readNumEntry( TQString::fromLatin1("Height %1").arg(desk.height()), 0 ));

        if (size.isEmpty())
          size = TQSize (kMin (700, desk.width()), kMin(480, desk.height()));
      }

      resize (size);
    }
  }

  // start session restore if needed
  startRestore (sconfig, sgroup);

  m_mainWindow = new Kate::MainWindow (this);
  m_toolViewManager = new Kate::ToolViewManager (this);

  m_dcop = new KateMainWindowDCOPIface (this);

  // setup the most important widgets
  setupMainWindow();

  // setup the actions
  setupActions();

  setStandardToolBarMenuEnabled( true );
  setXMLFile( "kateui.rc" );
  createShellGUI ( true );

  KatePluginManager::self()->enableAllPluginsGUI (this);

  if ( KateApp::self()->authorize("shell_access") )
    Kate::Document::registerCommand(KateExternalToolsCommand::self());

  // connect documents menu aboutToshow
  documentMenu = (TQPopupMenu*)factory()->container("documents", this);
  connect(documentMenu, TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(documentMenuAboutToShow()));

  // caption update
  for (uint i = 0; i < KateDocManager::self()->documents(); i++)
    slotDocumentCreated (KateDocManager::self()->document(i));

  connect(KateDocManager::self(),TQT_SIGNAL(documentCreated(Kate::Document *)),this,TQT_SLOT(slotDocumentCreated(Kate::Document *)));

  readOptions();

  if (sconfig)
    m_viewManager->restoreViewConfiguration (sconfig, sgroup);

  finishRestore ();

  setAcceptDrops(true);
}

KateMainWindow::~KateMainWindow()
{
  // first, save our fallback window size ;)
  KateApp::self()->config()->setGroup ("MainWindow");
  saveWindowSize (KateApp::self()->config());

  // save other options ;=)
  saveOptions();

  KateApp::self()->removeMainWindow (this);

  KatePluginManager::self()->disableAllPluginsGUI (this);

  delete m_dcop;
}

void KateMainWindow::setupMainWindow ()
{
  setToolViewStyle( KMultiTabBar::KDEV3ICON );

  m_tabWidget = new KateTabWidget (centralWidget());

  m_viewManager = new KateViewManager (this);

  KateMDI::ToolView *ft = createToolView("kate_filelist", KMultiTabBar::Left, SmallIcon("application-vnd.tde.tdemultiple"), i18n("Documents"));
  filelist = new KateFileList (this, m_viewManager, ft, "filelist");
  filelist->readConfig(KateApp::self()->config(), "Filelist");

  KateMDI::ToolView *t = createToolView("kate_fileselector", KMultiTabBar::Left, SmallIcon("document-open"), i18n("Filesystem Browser"));
  fileselector = new KateFileSelector( this, m_viewManager, t, "operator");
  connect(fileselector->dirOperator(),TQT_SIGNAL(fileSelected(const KFileItem*)),this,TQT_SLOT(fileSelected(const KFileItem*)));

  // ONLY ALLOW SHELL ACCESS IF ALLOWED ;)
  if (KateApp::self()->authorize("shell_access"))
  {
    t = createToolView("kate_greptool", KMultiTabBar::Bottom, SmallIcon("filefind"), i18n("Find in Files") );
    greptool = new GrepTool( t, "greptool" );
    connect(greptool, TQT_SIGNAL(itemSelected(const TQString &,int)), this, TQT_SLOT(slotGrepToolItemSelected(const TQString &,int)));
    connect(t,TQT_SIGNAL(visibleChanged(bool)),this, TQT_SLOT(updateGrepDir (bool)));
    // WARNING HACK - anders: showing the greptool seems to make the menu accels work
    greptool->show();

    t = createToolView("kate_console", KMultiTabBar::Bottom, SmallIcon("konsole"), i18n("Terminal"));
    console = new KateConsole (this, t);
  }

  // make per default the filelist visible, if we are in session restore, katemdi will skip this ;)
  showToolView (ft);
}

void KateMainWindow::setupActions()
{
  TDEAction *a;

  KStdAction::openNew( TQT_TQOBJECT(m_viewManager), TQT_SLOT( slotDocumentNew() ), actionCollection(), "file_new" )->setWhatsThis(i18n("Create a new document"));
  KStdAction::open( TQT_TQOBJECT(m_viewManager), TQT_SLOT( slotDocumentOpen() ), actionCollection(), "file_open" )->setWhatsThis(i18n("Open an existing document for editing"));

  fileOpenRecent = KStdAction::openRecent (TQT_TQOBJECT(m_viewManager), TQT_SLOT(openURL (const KURL&)), actionCollection());
  fileOpenRecent->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

  a=new TDEAction( i18n("Save A&ll"),"save_all", CTRL+Key_L, KateDocManager::self(), TQT_SLOT( saveAll() ), actionCollection(), "file_save_all" );
  a->setWhatsThis(i18n("Save all open, modified documents to disk."));

  KStdAction::close( TQT_TQOBJECT(m_viewManager), TQT_SLOT( slotDocumentClose() ), actionCollection(), "file_close" )->setWhatsThis(i18n("Close the current document."));

  a=new TDEAction( i18n( "Clos&e All" ), 0, TQT_TQOBJECT(this), TQT_SLOT( slotDocumentCloseAll() ), actionCollection(), "file_close_all" );
  a->setWhatsThis(i18n("Close all open documents."));

  KStdAction::mail( TQT_TQOBJECT(this), TQT_SLOT(slotMail()), actionCollection() )->setWhatsThis(i18n("Send one or more of the open documents as email attachments."));

  KStdAction::quit( TQT_TQOBJECT(this), TQT_SLOT( slotFileQuit() ), actionCollection(), "file_quit" )->setWhatsThis(i18n("Close this window"));

  a=new TDEAction(i18n("&New Window"), "window-new", 0, TQT_TQOBJECT(this), TQT_SLOT(newWindow()), actionCollection(), "view_new_view");
  a->setWhatsThis(i18n("Create a new Kate view (a new window with the same document list)."));

  if ( KateApp::self()->authorize("shell_access") )
  {
    externalTools = new KateExternalToolsMenuAction( i18n("External Tools"), actionCollection(), "tools_external", this );
    externalTools->setWhatsThis( i18n("Launch external helper applications") );
  }

  TDEToggleAction* showFullScreenAction = KStdAction::fullScreen( 0, 0, actionCollection(),this);
  connect( showFullScreenAction,TQT_SIGNAL(toggled(bool)), this,TQT_SLOT(slotFullScreen(bool)));

  documentOpenWith = new TDEActionMenu(i18n("Open W&ith"), actionCollection(), "file_open_with");
  documentOpenWith->setWhatsThis(i18n("Open the current document using another application registered for its file type, or an application of your choice."));
  connect(documentOpenWith->popupMenu(), TQT_SIGNAL(aboutToShow()), this, TQT_SLOT(mSlotFixOpenWithMenu()));
  connect(documentOpenWith->popupMenu(), TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotOpenWithMenuAction(int)));

  a=KStdAction::keyBindings(TQT_TQOBJECT(this), TQT_SLOT(editKeys()), actionCollection());
  a->setWhatsThis(i18n("Configure the application's keyboard shortcut assignments."));

  a=KStdAction::configureToolbars(TQT_TQOBJECT(this), TQT_SLOT(slotEditToolbars()), actionCollection());
  a->setWhatsThis(i18n("Configure which items should appear in the toolbar(s)."));

  TDEAction* settingsConfigure = KStdAction::preferences(TQT_TQOBJECT(this), TQT_SLOT(slotConfigure()), actionCollection(), "settings_configure");
  settingsConfigure->setWhatsThis(i18n("Configure various aspects of this application and the editing component."));

  // pipe to terminal action
  if (KateApp::self()->authorize("shell_access"))
    new TDEAction(i18n("&Pipe to Console"), "pipe", 0, TQT_TQOBJECT(console), TQT_SLOT(slotPipeToConsole()), actionCollection(), "tools_pipe_to_terminal");

  // tip of the day :-)
  KStdAction::tipOfDay( TQT_TQOBJECT(this), TQT_SLOT( tipOfTheDay() ), actionCollection() )->setWhatsThis(i18n("This shows useful tips on the use of this application."));

  if (KatePluginManager::self()->pluginList().count() > 0)
  {
    a=new TDEAction(i18n("&Plugins Handbook"), 0, TQT_TQOBJECT(this), TQT_SLOT(pluginHelp()), actionCollection(), "help_plugins_contents");
    a->setWhatsThis(i18n("This shows help files for various available plugins."));
  }

  connect(m_viewManager,TQT_SIGNAL(viewChanged()),TQT_TQOBJECT(this),TQT_SLOT(slotWindowActivated()));
  connect(m_viewManager,TQT_SIGNAL(viewChanged()),TQT_TQOBJECT(this),TQT_SLOT(slotUpdateOpenWith()));

  slotWindowActivated ();

  // session actions
  new TDEAction(i18n("Menu entry Session->New", "&New"), "document-new", 0, TQT_TQOBJECT(KateSessionManager::self()), TQT_SLOT(sessionNew()), actionCollection(), "sessions_new");
  new TDEAction(i18n("&Open..."), "document-open", 0, TQT_TQOBJECT(KateSessionManager::self()), TQT_SLOT(sessionOpen()), actionCollection(), "sessions_open");
  new TDEAction(i18n("&Save"), "document-save", 0, TQT_TQOBJECT(KateSessionManager::self()), TQT_SLOT(sessionSave()), actionCollection(), "sessions_save");
  new TDEAction(i18n("Save &As..."), "document-save-as", 0, TQT_TQOBJECT(KateSessionManager::self()), TQT_SLOT(sessionSaveAs()), actionCollection(), "sessions_save_as");
  new TDEAction(i18n("&Manage..."), "view_choose", 0, TQT_TQOBJECT(KateSessionManager::self()), TQT_SLOT(sessionManage()), actionCollection(), "sessions_manage");

  // quick open menu ;)
  new KateSessionsAction (i18n("&Quick Open"), actionCollection(), "sessions_list");
}

KateTabWidget *KateMainWindow::tabWidget ()
{
  return m_tabWidget;
}

void KateMainWindow::slotDocumentCloseAll() {
  if (queryClose_internal())
    KateDocManager::self()->closeAllDocuments(false);
}

bool KateMainWindow::queryClose_internal() {
   uint documentCount=KateDocManager::self()->documents();

  if ( ! showModOnDiskPrompt() )
    return false;

  TQPtrList<Kate::Document> modifiedDocuments=KateDocManager::self()->modifiedDocumentList();
  bool shutdown=(modifiedDocuments.count()==0);

  if (!shutdown) {
    shutdown=KateSaveModifiedDialog::queryClose(this,modifiedDocuments);
  }

  if ( KateDocManager::self()->documents() > documentCount ) {
    KMessageBox::information (this,
                              i18n ("New file opened while trying to close Kate, closing aborted."),
                              i18n ("Closing Aborted"));
    shutdown=false;
  }

  return shutdown;
}

/**
 * queryClose(), take care that after the last mainwindow the stuff is closed
 */
bool KateMainWindow::queryClose()
{
  // session saving, can we close all views ?
  // just test, not close them actually
  if (KateApp::self()->sessionSaving())
  {
    return queryClose_internal ();
  }

  // normal closing of window
  // allow to close all windows until the last without restrictions
  if ( KateApp::self()->mainWindows () > 1 )
    return true;

  // last one: check if we can close all documents, try run
  // and save docs if we really close down !
  if ( queryClose_internal () )
  {
    KateApp::self()->sessionManager()->saveActiveSession(true, true);

    // detach the dcopClient
    KateApp::self()->dcopClient()->detach();

    return true;
  }

  return false;
}

void KateMainWindow::newWindow ()
{
  KateApp::self()->newMainWindow ();
}

void KateMainWindow::slotEditToolbars()
{
  saveMainWindowSettings( KateApp::self()->config(), "MainWindow" );
  KEditToolbar dlg( factory() );
  connect( &dlg, TQT_SIGNAL(newToolbarConfig()), this, TQT_SLOT(slotNewToolbarConfig()) );
  dlg.exec();
}

void KateMainWindow::slotNewToolbarConfig()
{
  applyMainWindowSettings( KateApp::self()->config(), "MainWindow" );
}

void KateMainWindow::slotFileQuit()
{
  KateApp::self()->shutdownKate (this);
}

void KateMainWindow::readOptions ()
{
  TDEConfig *config = KateApp::self()->config ();

  config->setGroup("General");
  syncKonsole =  config->readBoolEntry("Sync Konsole", true);
  useInstance =  config->readBoolEntry("UseInstance", false);
  modNotification = config->readBoolEntry("Modified Notification", false);
  KateDocManager::self()->setSaveMetaInfos(config->readBoolEntry("Save Meta Infos", true));
  KateDocManager::self()->setDaysMetaInfos(config->readNumEntry("Days Meta Infos", 30));

  m_viewManager->setShowFullPath(config->readBoolEntry("Show Full Path in Title", false));

  fileOpenRecent->setMaxItems( config->readNumEntry("Number of recent files", fileOpenRecent->maxItems() ) );
  fileOpenRecent->loadEntries(config, "Recent Files");

  fileselector->readConfig(config, "fileselector");
}

void KateMainWindow::saveOptions ()
{
  TDEConfig *config = KateApp::self()->config ();
  config->setGroup("General");

  if (console)
    config->writeEntry("Show Console", console->isVisible());
  else
    config->writeEntry("Show Console", false);

  config->writeEntry("Save Meta Infos", KateDocManager::self()->getSaveMetaInfos());
  config->writeEntry("Days Meta Infos", KateDocManager::self()->getDaysMetaInfos());
  config->writeEntry("Show Full Path in Title", m_viewManager->getShowFullPath());
  config->writeEntry("Sync Konsole", syncKonsole);
  config->writeEntry("UseInstance", useInstance);
  
  fileOpenRecent->saveEntries(config, "Recent Files");
  fileselector->writeConfig(config, "fileselector");
  filelist->writeConfig(config, "Filelist");

  config->sync();
}

void KateMainWindow::slotWindowActivated ()
{
  if (m_viewManager->activeView())
  {
    if (console && syncKonsole)
    {
      static TQString path;
      TQString newPath = m_viewManager->activeView()->getDoc()->url().directory();

      if ( newPath != path )
      {
        path = newPath;
        console->cd (KURL( path ));
      }
    }

    updateCaption (m_viewManager->activeView()->getDoc());
  }

  // update proxy
  centralWidget()->setFocusProxy (m_viewManager->activeView());
}

void KateMainWindow::slotUpdateOpenWith()
{
  if (m_viewManager->activeView())
    documentOpenWith->setEnabled(!m_viewManager->activeView()->document()->url().isEmpty());
  else
    documentOpenWith->setEnabled(false);
}

void KateMainWindow::documentMenuAboutToShow()
{
  // remove documents
  while (documentMenu->count() > 3)
    documentMenu->removeItemAt (3);

  TQListViewItem * item = filelist->firstChild();
  while( item ) {
    // would it be saner to use the screen width as a limit that some random number??
    TQString name = KStringHandler::rsqueeze( ((KateFileListItem *)item)->document()->docName(), 150 ); 
    Kate::Document* doc = ((KateFileListItem *)item)->document();
    documentMenu->insertItem (
          doc->isModified() ? i18n("'document name [*]', [*] means modified", "%1 [*]").arg(name) : name,
          m_viewManager, TQT_SLOT (activateView (int)), 0,
          ((KateFileListItem *)item)->documentNumber () );

    item = item->nextSibling();
  }
  if (m_viewManager->activeView())
    documentMenu->setItemChecked ( m_viewManager->activeView()->getDoc()->documentNumber(), true);
}

void KateMainWindow::slotGrepToolItemSelected(const TQString &filename,int linenumber)
{
  KURL fileURL;
  fileURL.setPath( filename );
  m_viewManager->openURL( fileURL );
  if ( m_viewManager->activeView() == 0 ) return;
  m_viewManager->activeView()->gotoLineNumber( linenumber );
  raise();
  setActiveWindow();
}

void KateMainWindow::dragEnterEvent( TQDragEnterEvent *event )
{
  event->accept(KURLDrag::canDecode(event));
}

void KateMainWindow::dropEvent( TQDropEvent *event )
{
  slotDropEvent(event);
}

void KateMainWindow::slotDropEvent( TQDropEvent * event )
{
  KURL::List textlist;
  if (!KURLDrag::decode(event, textlist)) return;

  for (KURL::List::Iterator i=textlist.begin(); i != textlist.end(); ++i)
  {
    m_viewManager->openURL (*i);
  }
}

void KateMainWindow::editKeys()
{
  KKeyDialog dlg ( false, this );

  TQPtrList<KXMLGUIClient> clients = guiFactory()->clients();

  for( TQPtrListIterator<KXMLGUIClient> it( clients ); it.current(); ++it )
    dlg.insert ( (*it)->actionCollection(), (*it)->instance()->aboutData()->programName() );

  dlg.insert( externalTools->actionCollection(), i18n("External Tools") );

  dlg.configure();

  TQPtrList<Kate::Document>  l=KateDocManager::self()->documentList();
  for (uint i=0;i<l.count();i++) {
//     kdDebug(13001)<<"reloading Keysettings for document "<<i<<endl;
    l.at(i)->reloadXML();
    TQPtrList<class KTextEditor::View> l1=l.at(i)->views ();//KTextEditor::Document
    for (uint i1=0;i1<l1.count();i1++) {
      l1.at(i1)->reloadXML();
//       kdDebug(13001)<<"reloading Keysettings for view "<<i<<"/"<<i1<<endl;
    }
  }

  externalTools->actionCollection()->writeShortcutSettings( "Shortcuts", new TDEConfig("externaltools", false, false, "appdata") );
}

void KateMainWindow::openURL (const TQString &name)
{
  m_viewManager->openURL (KURL(name));
}

void KateMainWindow::slotConfigure()
{
  if (!m_viewManager->activeView())
    return;

  KateConfigDialog* dlg = new KateConfigDialog (this, m_viewManager->activeView());
  dlg->exec();

  delete dlg;
}

KURL KateMainWindow::activeDocumentUrl()
{
  // anders: i make this one safe, as it may be called during
  // startup (by the file selector)
  Kate::View *v = m_viewManager->activeView();
  if ( v )
    return v->getDoc()->url();
  return KURL();
}

void KateMainWindow::fileSelected(const KFileItem * /*file*/)
{
  const KFileItemList *list=fileselector->dirOperator()->selectedItems();
  KFileItem *tmp;
  for (KFileItemListIterator it(*list); (tmp = it.current()); ++it)
  {
    m_viewManager->openURL(tmp->url());
    fileselector->dirOperator()->view()->setSelected(tmp,false);
  }
}

// TODO make this work
void KateMainWindow::mSlotFixOpenWithMenu()
{
  //kdDebug(13001)<<"13000"<<"fixing open with menu"<<endl;
  documentOpenWith->popupMenu()->clear();
  // get a list of appropriate services.
  KMimeType::Ptr mime = KMimeType::findByURL( m_viewManager->activeView()->getDoc()->url() );
  //kdDebug(13001)<<"13000"<<"url: "<<m_viewManager->activeView()->getDoc()->url().prettyURL()<<"mime type: "<<mime->name()<<endl;
  // some checking goes here...
  TDETrader::OfferList offers = TDETrader::self()->query(mime->name(), "Type == 'Application'");
  // for each one, insert a menu item...
  for(TDETrader::OfferList::Iterator it = offers.begin(); it != offers.end(); ++it) {
    if ((*it)->name() == "Kate") continue;
    documentOpenWith->popupMenu()->insertItem( SmallIcon( (*it)->icon() ), (*it)->name() );
  }
  // append "Other..." to call the TDE "open with" dialog.
  documentOpenWith->popupMenu()->insertItem(i18n("&Other..."));
}

void KateMainWindow::slotOpenWithMenuAction(int idx)
{
  KURL::List list;
  list.append( m_viewManager->activeView()->getDoc()->url() );
  TQString appname = documentOpenWith->popupMenu()->text(idx);

  appname = appname.remove('&'); //Remove a possible accelerator ... otherwise the application might not get found.
  if ( appname.compare(i18n("Other...")) == 0 ) {
    // display "open with" dialog
    KOpenWithDlg dlg(list);
    if (dlg.exec())
      KRun::run(*dlg.service(), list);
    return;
  }

  TQString qry = TQString("((Type == 'Application') and (Name == '%1'))").arg( appname.latin1() );
  KMimeType::Ptr mime = KMimeType::findByURL( m_viewManager->activeView()->getDoc()->url() );
  TDETrader::OfferList offers = TDETrader::self()->query(mime->name(), qry);

  if (!offers.isEmpty()) {
    KService::Ptr app = offers.first();
    KRun::run(*app, list);
  }
  else
    KMessageBox::error(this, i18n("Application '%1' not found!").arg(appname.latin1()), i18n("Application Not Found!"));
}

void KateMainWindow::pluginHelp()
{
  KateApp::self()->invokeHelp (TQString::null, "kate-plugins");
}

void KateMainWindow::slotMail()
{
  KateMailDialog *d = new KateMailDialog(this, this);
  if ( ! d->exec() )
  {
    delete d;
    return;
  }
  TQPtrList<Kate::Document> attDocs = d->selectedDocs();
  delete d;
  // Check that all selected files are saved (or shouldn't be)
  TQStringList urls; // to atthatch
  Kate::Document *doc;
  TQPtrListIterator<Kate::Document> it(attDocs);
  for ( ; it.current(); ++it ) {
    doc = it.current();
    if (!doc) continue;
    if ( doc->url().isEmpty() ) {
      // unsaved document. back out unless it gets saved
      int r = KMessageBox::questionYesNo( this,
              i18n("<p>The current document has not been saved, and "
              "cannot be attached to an email message."
              "<p>Do you want to save it and proceed?"),
              i18n("Cannot Send Unsaved File"),KStdGuiItem::saveAs(),KStdGuiItem::cancel() );
      if ( r == KMessageBox::Yes ) {
        Kate::View *v = (Kate::View*)doc->views().first();
        int sr = v->saveAs();
        if ( sr == Kate::View::SAVE_OK ) { ;
        }
        else {
          if ( sr != Kate::View::SAVE_CANCEL ) // ERROR or RETRY(?)
            KMessageBox::sorry( this, i18n("The file could not be saved. Please check "
                                        "if you have write permission.") );
          continue;
        }
      }
      else
        continue;
    }
    if ( doc->isModified() ) {
      // warn that document is modified and offer to save it before proceeding.
      int r = KMessageBox::warningYesNoCancel( this,
                i18n("<p>The current file:<br><strong>%1</strong><br>has been "
                "modified. Modifications will not be available in the attachment."
                "<p>Do you want to save it before sending it?").arg(doc->url().prettyURL()),
                i18n("Save Before Sending?"), KStdGuiItem::save(), i18n("Do Not Save") );
      switch ( r ) {
        case KMessageBox::Cancel:
          continue;
        case KMessageBox::Yes:
          doc->save();
          if ( doc->isModified() ) { // read-only docs ends here, if modified. Hmm.
            KMessageBox::sorry( this, i18n("The file could not be saved. Please check "
                                      "if you have write permission.") );
            continue;
          }
          break;
        default:
          break;
      }
    }
    // finally call the mailer
    urls << doc->url().url();
  } // check selected docs done
  if ( ! urls.count() )
    return;
  KateApp::self()->invokeMailer( TQString::null, // to
                      TQString::null, // cc
                      TQString::null, // bcc
                      TQString::null, // subject
                      TQString::null, // body
                      TQString::null, // msgfile
                      urls           // urls to atthatch
                      );
}
void KateMainWindow::tipOfTheDay()
{
  KTipDialog::showTip( /*0*/this, TQString::null, true );
}

void KateMainWindow::slotFullScreen(bool t)
{
  if (t)
    showFullScreen();
  else
    showNormal();
}

void KateMainWindow::updateGrepDir (bool visible)
{
  // grepdlg gets hidden
  if (!visible)
    return;

  if ( m_viewManager->activeView() )
  {
    if ( m_viewManager->activeView()->getDoc()->url().isLocalFile() )
    {
      greptool->updateDirName( m_viewManager->activeView()->getDoc()->url().directory() );
    }
  }
}

bool KateMainWindow::event( TQEvent *e )
{
  uint type = e->type();
  if ( type == TQEvent::WindowActivate && modNotification )
  {
    showModOnDiskPrompt();
  }
  return KateMDI::MainWindow::event( e );
}

bool KateMainWindow::showModOnDiskPrompt()
{
  Kate::Document *doc;

  DocVector list( KateDocManager::self()->documents() );
  uint cnt = 0;
  for( doc = KateDocManager::self()->firstDocument(); doc; doc = KateDocManager::self()->nextDocument() )
  {
    if ( KateDocManager::self()->documentInfo( doc )->modifiedOnDisc )
    {
      list.insert( cnt, doc );
      cnt++;
    }
  }

  if ( cnt && !m_modignore )
  {
    list.resize( cnt );
    KateMwModOnHdDialog mhdlg( list, this );
    m_modignore = true;
    bool res = mhdlg.exec();
    m_modignore = false;

    return res;
  }
  return true;
}

void KateMainWindow::slotDocumentCreated (Kate::Document *doc)
{
  connect(doc,TQT_SIGNAL(modStateChanged(Kate::Document *)),this,TQT_SLOT(updateCaption(Kate::Document *)));
  connect(doc,TQT_SIGNAL(nameChanged(Kate::Document *)),this,TQT_SLOT(slotNameChanged(Kate::Document *)));
  connect(doc,TQT_SIGNAL(nameChanged(Kate::Document *)),this,TQT_SLOT(slotUpdateOpenWith()));

  updateCaption (doc);
}

void KateMainWindow::slotNameChanged(Kate::Document *doc)
{
  updateCaption(doc);
  if (!doc->url().isEmpty())
    fileOpenRecent->addURL(doc->url());
}

void KateMainWindow::updateCaption (Kate::Document *doc)
{
  if (!m_viewManager->activeView())
  {
    setCaption ("", false);
    return;
  }

  if (!(m_viewManager->activeView()->getDoc() == doc))
    return;

  TQString c;
  if (m_viewManager->activeView()->getDoc()->url().isEmpty() || (!m_viewManager->getShowFullPath()))
  {
    c = m_viewManager->activeView()->getDoc()->docName();
  }
  else
  {
    c = m_viewManager->activeView()->getDoc()->url().prettyURL();
  }

  TQString sessName = KateApp::self()->sessionManager()->activeSession()->sessionName();
  if ( !sessName.isEmpty() )
    sessName = TQString("%1: ").arg( sessName );

  setCaption( sessName + KStringHandler::lsqueeze(c,64),
      m_viewManager->activeView()->getDoc()->isModified());
}

void KateMainWindow::saveProperties(TDEConfig *config)
{
  TQString grp=config->group();

  saveSession(config, grp);
  m_viewManager->saveViewConfiguration (config, grp);

  config->setGroup(grp);
}

void KateMainWindow::readProperties(TDEConfig *config)
{
  TQString grp=config->group();

  startRestore(config, grp);
  finishRestore ();
  m_viewManager->restoreViewConfiguration (config, grp);

  config->setGroup(grp);
}

void KateMainWindow::saveGlobalProperties( TDEConfig* sessionConfig )
{
  KateDocManager::self()->saveDocumentList (sessionConfig);

  sessionConfig->setGroup("General");
  sessionConfig->writeEntry ("Last Session", KateApp::self()->sessionManager()->activeSession()->sessionFileRelative());
}

// kate: space-indent on; indent-width 2; replace-tabs on;
