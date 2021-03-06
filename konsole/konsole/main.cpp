/*
    This file is part of Konsole, an X terminal.
    Copyright (C) 1996 by Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/
/* The material contained in here more or less directly orginates from    */
/* kvt, which is copyright (c) 1996 by Matthias Ettrich <ettrich@kde.org> */
/*                                                                        */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <tqdir.h>
#include <tqsessionmanager.h>
#include <tqwidgetlist.h>

#include <dcopclient.h>

#include <tdelocale.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <kimageio.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <tdeglobalsettings.h>
#include <tdeio/netaccess.h>
#include <tdemessagebox.h>

#include <config.h>

#include "konsole.h"

#if defined(Q_WS_X11) && defined(HAVE_XRENDER) && TQT_VERSION >= 0x030300
#define COMPOSITE
#endif

#ifdef COMPOSITE
# include <X11/Xlib.h>
# include <X11/extensions/Xrender.h>
# include <fixx11h.h>
# include <dlfcn.h>
#endif

static const char description[] =
  I18N_NOOP("X terminal for use with TDE.");

//   { "T <title>",       0, 0 },
static TDECmdLineOptions options[] =
{
   { "name <name>",     I18N_NOOP("Set window class"), 0 },
   { "ls",              I18N_NOOP("Start login shell"), 0 },
   { "T <title>",       I18N_NOOP("Set the window title"), 0 },
   { "tn <terminal>",   I18N_NOOP("Specify terminal type as set in the TERM\nenvironment variable"), "xterm" },
   { "noclose",         I18N_NOOP("Do not close Konsole when command exits"), 0 },
   { "nohist",          I18N_NOOP("Do not save lines in history"), 0 },
   { "nomenubar",       I18N_NOOP("Do not display menubar"), 0 },
   { "notabbar",        0, 0 },
   { "notoolbar",       I18N_NOOP("Do not display tab bar"), 0 },
   { "noframe",         I18N_NOOP("Do not display frame"), 0 },
   { "noscrollbar",     I18N_NOOP("Do not display scrollbar"), 0 },
   { "noxft",           I18N_NOOP("Do not use Xft (anti-aliasing)"), 0 },
   { "vt_sz CCxLL",     I18N_NOOP("Terminal size in columns x lines"), 0 },
   { "noresize",        I18N_NOOP("Terminal size is fixed"), 0 },
   { "type <type>",     I18N_NOOP("Start with given session type"), 0 },
   { "types",           I18N_NOOP("List available session types"), 0 },
   { "keytab <name>",   I18N_NOOP("Set keytab to 'name'"), 0 },
   { "keytabs",         I18N_NOOP("List available keytabs"), 0 },
   { "profile <name>",  I18N_NOOP("Start with given session profile"), 0 },
   { "profiles",        I18N_NOOP("List available session profiles"), 0 },
   { "schema <name> | <file>",   I18N_NOOP("Set schema to 'name' or use 'file'"), 0 },
   { "schemas",         0, 0 },
   { "schemata",        I18N_NOOP("List available schemata"), 0 },
   { "script",          I18N_NOOP("Enable extended DCOP Qt functions"), 0 },
   { "workdir <dir>",   I18N_NOOP("Change working directory to 'dir'"), 0 },
   { "!e <command>",    I18N_NOOP("Execute 'command' instead of shell"), 0 },
   // WABA: All options after -e are treated as arguments.
   { "+[args]",         I18N_NOOP("Arguments for 'command'"), 0 },
   TDECmdLineLastOption
};

static bool has_noxft = false;
static bool login_shell = false;
static bool full_script = false;
static bool auto_close = true;
static bool fixed_size = false;

bool argb_visual = false;

const char *konsole_shell(TQStrList &args)
{
  const char* shell = getenv("SHELL");
  if (shell == NULL || *shell == '\0') shell = "/bin/sh";
  if (login_shell)
  {
    char* t = const_cast<char*>(strrchr(shell,'/'));
    if (t) // see sh(1)
    {
      t = strdup(t);
      *t = '-';
      args.append(t);
      free(t);
    }
    else
      args.append(shell);
  }
  else
    args.append(shell);
  return shell;
}

/**
   The goal of this class is to add "--noxft" and "--ls" to the restoreCommand
   if konsole was started with any of those options.
 */
class KonsoleSessionManaged: public KSessionManaged {
public:
    bool saveState( TQSessionManager&sm) {
        TQStringList restartCommand = sm.restartCommand();
        if (has_noxft)
            restartCommand.append("--noxft");
        if (login_shell)
            restartCommand.append("--ls");
        if (full_script)
            restartCommand.append("--script");
        if (!auto_close)
            restartCommand.append("--noclose");
        if (fixed_size)
            restartCommand.append("--noresize");
        sm.setRestartCommand(restartCommand);
        return true;
    }
};



/* --| main |------------------------------------------------------ */
extern "C" int KDE_EXPORT kdemain(int argc, char* argv[])
{
  setgid(getgid()); setuid(getuid()); // drop privileges

  // deal with shell/command ////////////////////////////
  bool histon = true;
  bool menubaron = true;
  bool tabbaron = true;
  bool frameon = true;
  bool scrollbaron = true;
  bool showtip = true;

  TDEAboutData aboutData( "konsole", I18N_NOOP("Konsole"),
    KONSOLE_VERSION, description, TDEAboutData::License_GPL_V2,
    "Copyright (c) 2011-2014, The Trinity Desktop project\nCopyright (c) 1997-2006, Lars Doelle");
  aboutData.addAuthor( "Timothy Pearson", I18N_NOOP("Maintainer, Trinity bugfixes"), "kb9vqf@pearsoncomputing.net" );
  aboutData.addAuthor("Robert Knight",I18N_NOOP("Previous Maintainer"), "robertknight@gmail.com");
  aboutData.addAuthor("Lars Doelle",I18N_NOOP("Author"), "lars.doelle@on-line.de");
  aboutData.addCredit("Kurt V. Hindenburg",
    I18N_NOOP("bug fixing and improvements"),
    "kurt.hindenburg@gmail.com");
  aboutData.addCredit("Waldo Bastian",
    I18N_NOOP("bug fixing and improvements"),
    "bastian@kde.org");
  aboutData.addCredit("Stephan Binner",
    I18N_NOOP("bug fixing and improvements"),
    "binner@kde.org");
  aboutData.addCredit("Chris Machemer",
    I18N_NOOP("bug fixing"),
    "machey@ceinetworks.com");
  aboutData.addCredit("Stephan Kulow",
    I18N_NOOP("Solaris support and work on history"),
    "coolo@kde.org");
  aboutData.addCredit("Alexander Neundorf",
    I18N_NOOP("faster startup, bug fixing"),
    "neundorf@kde.org");
  aboutData.addCredit("Peter Silva",
    I18N_NOOP("decent marking"),
    "peter.silva@videotron.ca");
  aboutData.addCredit("Lotzi Boloni",
    I18N_NOOP("partification\n"
    "Toolbar and session names"),
    "boloni@cs.purdue.edu");
  aboutData.addCredit("David Faure",
    I18N_NOOP("partification\n"
    "overall improvements"),
    "David.Faure@insa-lyon.fr");
  aboutData.addCredit("Antonio Larrosa",
    I18N_NOOP("transparency"),
    "larrosa@kde.org");
  aboutData.addCredit("Matthias Ettrich",
    I18N_NOOP("most of main.C donated via kvt\n"
    "overall improvements"),
    "ettrich@kde.org");
  aboutData.addCredit("Warwick Allison",
    I18N_NOOP("schema and selection improvements"),
    "warwick@troll.no");
  aboutData.addCredit("Dan Pilone",
    I18N_NOOP("SGI Port"),
    "pilone@slac.com");
  aboutData.addCredit("Kevin Street",
    I18N_NOOP("FreeBSD port"),
    "street@iname.com");
  aboutData.addCredit("Sven Fischer",
    I18N_NOOP("bug fixing"),
    "herpes@kawo2.rwth-aachen.de");
  aboutData.addCredit("Dale M. Flaven",
    I18N_NOOP("bug fixing"),
    "dflaven@netport.com");
  aboutData.addCredit("Martin Jones",
    I18N_NOOP("bug fixing"),
    "mjones@powerup.com.au");
  aboutData.addCredit("Lars Knoll",
    I18N_NOOP("bug fixing"),
    "knoll@mpi-hd.mpg.de");
  aboutData.addCredit("",I18N_NOOP("Thanks to many others.\n"
    "The above list only reflects the contributors\n"
    "I managed to keep track of."));

  TDECmdLineArgs::init( argc, argv, &aboutData );
  TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  //1.53 sec
  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  TDECmdLineArgs *qtargs = TDECmdLineArgs::parsedArgs("qt");
  has_noxft = !args->isSet("xft");
  TEWidget::setAntialias( !has_noxft );
  TEWidget::setStandalone( true );

  // The following Qt options have no effect; warn users.
  if( qtargs->isSet("background") )
      kdWarning() << "The Qt option -bg, --background has no effect." << endl;
  if( qtargs->isSet("foreground") )
      kdWarning() << "The Qt option -fg, --foreground has no effect." << endl;
  if( qtargs->isSet("button") )
      kdWarning() << "The Qt option -btn, --button has no effect." << endl;
  if( qtargs->isSet("font") )
      kdWarning() << "The Qt option -fn, --font has no effect." << endl;

  TDEApplication* a = NULL;
#ifdef COMPOSITE
  a = new TDEApplication(TDEApplication::openX11RGBADisplay());
  argb_visual = a->isX11CompositionAvailable();
#else
  a = new TDEApplication;
#endif

  TQString dataPathBase = TDEStandardDirs::kde_default("data").append("konsole/");
  TDEGlobal::dirs()->addResourceType("wallpaper", dataPathBase + "wallpapers");

  KImageIO::registerFormats(); // add io for additional image formats
  //2.1 secs

  TQString title;
  if(args->isSet("T")) {
    title = TQFile::decodeName(args->getOption("T"));
  }
  if(qtargs->isSet("title")) {
    title = TQFile::decodeName(qtargs->getOption("title"));
  }

  TQString term = "";
  if(args->isSet("tn")) {
    term=TQString::fromLatin1(args->getOption("tn"));
  }
  login_shell = args->isSet("ls");

  TQStrList eargs;

  const char* shell = 0;
  if (!args->getOption("e").isEmpty())
  {
     if (args->isSet("ls"))
        TDECmdLineArgs::usage(i18n("You can't use BOTH -ls and -e.\n"));
     shell = strdup(args->getOption("e"));
     eargs.append(shell);
     for(int i=0; i < args->count(); i++)
       eargs.append( args->arg(i) );

     if (title.isEmpty() &&
         (kapp->caption() == kapp->aboutData()->programName()))
     {
        title = TQFile::decodeName(shell);  // program executed in the title bar
     }
     showtip = false;
  }

  TQCString sz = "";
  sz = args->getOption("vt_sz");
  histon = args->isSet("hist");
  menubaron = args->isSet("menubar");
  tabbaron = args->isSet("tabbar") && args->isSet("toolbar");
  frameon = args->isSet("frame");
  scrollbaron = args->isSet("scrollbar");
  TQCString wname = qtargs->getOption("name");
  full_script = args->isSet("script");
  auto_close = args->isSet("close");
  fixed_size = !args->isSet("resize");

  if (!full_script)
	a->dcopClient()->setQtBridgeEnabled(false);

  TQCString type = "";

  if(args->isSet("type")) {
    type = args->getOption("type");
  }
  if(args->isSet("types")) {
    TQStringList types = TDEGlobal::dirs()->findAllResources("appdata", "*.desktop", false, true);
    types.sort();
    for(TQStringList::ConstIterator it = types.begin();
        it != types.end(); ++it)
    {
       TQString file = *it;
       file = file.mid(file.findRev('/')+1);
       if (file.endsWith(".desktop"))
          file = file.left(file.length()-8);
       printf("%s\n", TQFile::encodeName(file).data());
    }
    return 0;
  }
  if(args->isSet("schemas") || args->isSet("schemata")) {
    ColorSchemaList colors;
    colors.checkSchemas();
    for(int i = 0; i < (int) colors.count(); i++)
    {
       ColorSchema *schema = colors.find(i);
       TQString relPath = schema->relPath();
       if (!relPath.isEmpty())
          printf("%s\n", TQFile::encodeName(relPath).data());
    }
    return 0;
  }

  if(args->isSet("keytabs")) {
    TQStringList lst = TDEGlobal::dirs()->findAllResources("data", "konsole/*.keytab");

    printf("default\n");   // 'buildin' keytab
    lst.sort();
    for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
    {
      TQFileInfo fi(*it);
      TQString file = fi.baseName();
      printf("%s\n", TQFile::encodeName(file).data());
    }
    return 0;
  }

  TQString workDir = TQFile::decodeName( args->getOption("workdir") );

  TQString keytab = "";
  if (args->isSet("keytab"))
    keytab = TQFile::decodeName(args->getOption("keytab"));

  TQString schema = "";
  if (args->isSet("schema"))
    schema = args->getOption("schema");

  TDEConfig * sessionconfig = 0;
  TQString profile = "";
  if (args->isSet("profile")) {
    profile = args->getOption("profile");
    TQString path = locate( "data", "konsole/profiles/" + profile );
    if ( TQFile::exists( path ) )
      sessionconfig=new TDEConfig( path, true );
    else
      profile = "";
  }
  if (args->isSet("profiles"))
  {
     TQStringList profiles = TDEGlobal::dirs()->findAllResources("data", "konsole/profiles/*", false, true);
     profiles.sort();
     for(TQStringList::ConstIterator it = profiles.begin();
         it != profiles.end(); ++it)
     {
        TQString file = *it;
        file = file.mid(file.findRev('/')+1);
        printf("%s\n", TQFile::encodeName(file).data());
     }
     return 0;
  }


  //FIXME: more: font

  args->clear();

  int c = 0, l = 0;
  if ( !sz.isEmpty() )
  {
    char *ls = strchr( sz.data(), 'x' );
    if ( ls != NULL )
    {
       *ls='\0';
       ls++;
       c=atoi(sz.data());
       l=atoi(ls);
    }
    else
    {
       TDECmdLineArgs::usage(i18n("expected --vt_sz <#columns>x<#lines> e.g. 80x40\n"));
    }
  }

  if (!kapp->authorizeTDEAction("size"))
    fixed_size = true;

  // ///////////////////////////////////////////////

  // Ignore SIGHUP so that we don't get killed when
  // our parent-shell gets closed.
  signal(SIGHUP, SIG_IGN);

  putenv((char*)"COLORTERM="); // to trigger mc's color detection
  KonsoleSessionManaged ksm;

  if (a->isRestored() || !profile.isEmpty())
  {
    if (!shell)
       shell = konsole_shell(eargs);

    if (profile.isEmpty())
      sessionconfig = a->sessionConfig();
    sessionconfig->setDesktopGroup();
    int n = 1;

    TQString key;
    TQString sTitle;
    TQString sPgm;
    TQString sTerm;
    TQString sIcon;
    TQString sCwd;
    int     n_tabbar;

    // TODO: Session management stores everything in same group,
    // should use one group / mainwindow
    while (TDEMainWindow::canBeRestored(n) || !profile.isEmpty())
    {
        sessionconfig->setGroup(TQString("%1").arg(n));
        if (!sessionconfig->hasKey("Pgm0"))
            sessionconfig->setDesktopGroup(); // Backwards compatible

        int session_count = sessionconfig->readNumEntry("numSes");
        int counter = 0;

        wname = sessionconfig->readEntry("class",wname).latin1();

        sPgm = sessionconfig->readEntry("Pgm0", shell);
        sessionconfig->readListEntry("Args0", eargs);
        sTitle = sessionconfig->readEntry("Title0", title);
        sTerm = sessionconfig->readEntry("Term0");
        sIcon = sessionconfig->readEntry("Icon0","konsole");
        sCwd = sessionconfig->readPathEntry("Cwd0");
        workDir = sessionconfig->readPathEntry("workdir");
	n_tabbar = TQMIN(sessionconfig->readUnsignedNumEntry("tabbar",Konsole::TabBottom),2);
        Konsole *m = new Konsole(wname,histon,menubaron,tabbaron,frameon,scrollbaron,0/*type*/,true,n_tabbar, workDir);

        m->newSession(sPgm, eargs, sTerm, sIcon, sTitle, sCwd);

        m->enableFullScripting(full_script);
        m->enableFixedSize(fixed_size);
	m->restore(n);
        sessionconfig->setGroup(TQString("%1").arg(n));
        if (!sessionconfig->hasKey("Pgm0"))
            sessionconfig->setDesktopGroup(); // Backwards compatible
        m->makeGUI();
        m->setEncoding(sessionconfig->readNumEntry("Encoding0"));
        m->setSchema(sessionconfig->readEntry("Schema0"));
        // Use konsolerc default as tmpFont instead?
        TQFont tmpFont = TDEGlobalSettings::fixedFont();
        m->initSessionFont(sessionconfig->readFontEntry("SessionFont0", &tmpFont));
        m->initSessionKeyTab(sessionconfig->readEntry("KeyTab0"));
        m->initMonitorActivity(sessionconfig->readBoolEntry("MonitorActivity0",false));
        m->initMonitorSilence(sessionconfig->readBoolEntry("MonitorSilence0",false));
        m->initMasterMode(sessionconfig->readBoolEntry("MasterMode0",false));
        m->initTabColor(sessionconfig->readColorEntry("TabColor0"));
        // -1 will be changed to the default history in konsolerc
        m->initHistory(sessionconfig->readNumEntry("History0", -1), 
                       sessionconfig->readBoolEntry("HistoryEnabled0", true));
        counter++;

        // show() before 2nd+ sessions are created allows --profile to
        //  initialize the TE size correctly.
        m->show();

        while (counter < session_count)
        {
          key = TQString("Title%1").arg(counter);
          sTitle = sessionconfig->readEntry(key, title);
          key = TQString("Args%1").arg(counter);
          sessionconfig->readListEntry(key, eargs);

          key = TQString("Pgm%1").arg(counter);
          
          // if the -e option is passed on the command line, this overrides the program specified 
          // in the profile file
          if ( args->isSet("e") )
            sPgm = (shell ? TQFile::decodeName(shell) : TQString());
          else
            sPgm = sessionconfig->readEntry(key, shell);

          key = TQString("Term%1").arg(counter);
          sTerm = sessionconfig->readEntry(key);
          key = TQString("Icon%1").arg(counter);
          sIcon = sessionconfig->readEntry(key,"konsole");
          key = TQString("Cwd%1").arg(counter);
          sCwd = sessionconfig->readPathEntry(key);
          m->newSession(sPgm, eargs, sTerm, sIcon, sTitle, sCwd);
          m->setSessionTitle(sTitle);  // Use title as is
          key = TQString("Schema%1").arg(counter);
          m->setSchema(sessionconfig->readEntry(key));
          key = TQString("Encoding%1").arg(counter);
          m->setEncoding(sessionconfig->readNumEntry(key));
          key = TQString("SessionFont%1").arg(counter);
          TQFont tmpFont = TDEGlobalSettings::fixedFont();
          m->initSessionFont(sessionconfig->readFontEntry(key, &tmpFont));
          key = TQString("KeyTab%1").arg(counter);
          m->initSessionKeyTab(sessionconfig->readEntry(key));
          key = TQString("MonitorActivity%1").arg(counter);
          m->initMonitorActivity(sessionconfig->readBoolEntry(key,false));
          key = TQString("MonitorSilence%1").arg(counter);
          m->initMonitorSilence(sessionconfig->readBoolEntry(key,false));
          key = TQString("MasterMode%1").arg(counter);
          m->initMasterMode(sessionconfig->readBoolEntry(key,false));
          key = TQString("TabColor%1").arg(counter);
          m->initTabColor(sessionconfig->readColorEntry(key));
          // -1 will be changed to the default history in konsolerc
          key = TQString("History%1").arg(counter);
          TQString key2 = TQString("HistoryEnabled%1").arg(counter);
          m->initHistory(sessionconfig->readNumEntry(key, -1), 
                         sessionconfig->readBoolEntry(key2, true));
          counter++;
        }
        m->setDefaultSession( sessionconfig->readEntry("DefaultSession","shell.desktop") );

        m->initFullScreen();
        if ( !profile.isEmpty() ) {
          m->callReadPropertiesInternal(sessionconfig,1);
          profile = "";
          // Hack to work-around sessions initialized with minimum size
          for (int i=0;i<counter;i++)
            m->activateSession( i );
          m->setColLin(c,l); // will use default height and width if called with (0,0)
        }
	// works only for the first one, but there won't be more.
        n++;
        m->activateSession( sessionconfig->readNumEntry("ActiveSession",0) );
	m->setAutoClose(auto_close);
    }
  }
  else
  {
    Konsole*  m = new Konsole(wname,histon,menubaron,tabbaron,frameon,scrollbaron,type, false, 0, workDir);
    m->newSession((shell ? TQFile::decodeName(shell) : TQString()), eargs, term, TQString(), title, workDir);
    m->enableFullScripting(full_script);
    m->enableFixedSize(fixed_size);
    //3.8 :-(
    //exit(0);

    if (!keytab.isEmpty())
      m->initSessionKeyTab(keytab);

    if (!schema.isEmpty()) {
      if (schema.right(7)!=".schema")
        schema+=".schema";
      m->setSchema(schema);
      m->activateSession(0); // Fixes BR83162, transp. schema + notabbar
    }

    m->setColLin(c,l); // will use default height and width if called with (0,0)

    m->initFullScreen();
    m->show();
    if (showtip)
      m->showTipOnStart();
    m->setAutoClose(auto_close);
  }

  int ret = a->exec();

 //// Temporary code, waiting for Qt to do this properly

  // Delete all toplevel widgets that have WDestructiveClose
  TQWidgetList *list = TQApplication::topLevelWidgets();
  // remove all toplevel widgets that have a parent (i.e. they
  // got WTopLevel explicitly), they'll be deleted by the parent
  list->first();
  while( list->current())
  {
    if( list->current()->parentWidget() != NULL || !list->current()->testWFlags( TQt::WDestructiveClose ) )
    {
        list->remove();
        continue;
    }
    list->next();
  }
  TQWidgetListIt it(*list);
  TQWidget * w;
  while( (w=it.current()) != 0 ) {
     ++it;
     delete w;
  }
  delete list;
  
  delete a;

  return ret;
}
