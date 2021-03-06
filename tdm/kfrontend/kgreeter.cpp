/*

Greeter widget for tdm

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kgreeter.h"
#include "kconsole.h"
#include "tdmconfig.h"
#include "tdmclock.h"
#include "tdm_greet.h"
#include "sakdlg.h"
#include "tdmadmindialog.h"
#include "themer/tdmthemer.h"
#include "themer/tdmitem.h"
#include "themer/tdmlabel.h"

#include <dmctl.h>

#include <ksslcertificate.h>

#include <tdehardwaredevices.h>
#include <tdecryptographiccarddevice.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <tdelistview.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <kdebug.h>
#include <kdialog.h>

#undef Unsorted // x headers suck - make tqdir.h work with --enable-final
#include <tqdir.h>
#include <tqfile.h>
#include <tqbuffer.h>
#include <tqmemarray.h>
#include <tqimage.h>
#include <tqmovie.h>
#include <tqpainter.h>
#include <tqpopupmenu.h>
#include <tqtimer.h>
#include <tqheader.h>
#include <tqstyle.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>
#include <tqaccel.h>
#include <tqstring.h>
#include <tqeventloop.h>
#include <tqbitmap.h>

#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#ifdef HAVE_UTMPX
# include <utmpx.h>
#else
# include <utmp.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <libgen.h>

#include <X11/Xlib.h>

#define FIFO_DIR "/tmp/tdesocket-global/tdm"
#define FIFO_FILE "/tmp/tdesocket-global/tdm/tdmctl-%1"
#define FIFO_SAK_FILE "/tmp/tdesocket-global/tdm/tdmctl-sak-%1"

class UserListView : public TDEListView {
  public:
        UserListView( bool _them, TQWidget *parent = 0, const char *name = 0 )
		: TDEListView( parent, name )
		, themed(_them), cachedSizeHint( -1, 0 )
	{
		setSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Ignored );
		header()->hide();
		addColumn( TQString::null );
		setColumnAlignment( 0, AlignVCenter );
		setResizeMode( TQListView::LastColumn );
                // FIXME: This must be configurable, so disable
                // painting of list background for now.
//		if (themed) {
//		  setBackgroundMode( Qt::NoBackground );
//		  viewport()->setBackgroundMode( Qt::NoBackground );
//		  setFrameStyle( TQFrame::NoFrame );
//		}
	}

        bool themed;
	mutable TQSize cachedSizeHint;

        int sumHeight() const 
  {
        int sum = 0;
        for (TQListViewItem *itm = firstChild(); itm; itm = itm->nextSibling()) {
            sum += itm->height();
        }
        return sum;
    }
public:
	virtual TQSize sizeHint() const
	{
	  if (themed)
            return TDEListView::sizeHint();

		if (!cachedSizeHint.isValid()) {
			constPolish();
			uint maxw = 0;
			for (TQListViewItem *itm = firstChild(); itm; itm = itm->nextSibling()) {
				uint thisw = itm->width( fontMetrics(), this, 0 );
				if (thisw > maxw)
					maxw = thisw;
			}
			cachedSizeHint.setWidth(
				style().pixelMetric( TQStyle::PM_ScrollBarExtent ) +
				frameWidth() * 2 + maxw );
		}
		return cachedSizeHint;
	}
    virtual void paintEmptyArea ( TQPainter * p, const TQRect & rect )
    {
      if (!themed)
        return TDEListView::paintEmptyArea(p, rect );

        // FIXME: This must be configurable, so disable
        // painting of list background for now.
        return TDEListView::paintEmptyArea(p, rect );

        const TQPixmap *pm = TQT_TQPIXMAP_CONST(paletteBackgroundPixmap());
        if (!pm || pm->isNull()) {
            return;
        }

        kdDebug() << "paintEmpty " << rect << endl;
        TQRect devRect = p->xForm( rect );
        kdDebug() << "paintEmpty2 " << devRect << endl;
        p->drawPixmap(0, 0, *pm, devRect.left(), devRect.top() );
    }

    TQPixmap background;
};

int KGreeter::curPlugin = -1;
PluginList KGreeter::pluginList;

KGreeter::KGreeter( bool framed )
  : inherited( framed )
  , dName( dname )
  , userView( 0 )
  , userList( 0 )
  , nNormals( 0 )
  , nSpecials( 0 )
  , curPrev( -1 )
  , curSel( -1 )
  , prevValid( true )
  , needLoad( false )
  , themed( framed )
  , showInfoMessages( true )
  , closingDown( false )
{
	stsFile = new KSimpleConfig( _stsFile );
	stsFile->setGroup( "PrevUser" );

	if (_userList) {
                readFacesList();
		userView = new UserListView( framed, this );
		connect( userView, TQT_SIGNAL(clicked( TQListViewItem * )),
		         TQT_SLOT(slotUserClicked( TQListViewItem * )) );
		connect( userView, TQT_SIGNAL(doubleClicked( TQListViewItem * )),
		         TQT_SLOT(accept()) );
	}
	if (_userCompletion) {
		userList = new TQStringList;
	}

        sessMenu = new TQPopupMenu( this );
	connect( sessMenu, TQT_SIGNAL(activated( int )),
	         TQT_SLOT(slotSessionSelected( int )) );
	insertSessions();

	if (curPlugin < 0) {
		curPlugin = 0;
		pluginList = KGVerify::init( _pluginsLogin );
	}

	mControlPipeHandlerThread = new TQEventLoopThread();
	mControlPipeHandler = new ControlPipeHandlerObject();
	mControlPipeHandler->mKGreeterParent = this;
	mControlPipeHandler->moveToThread(mControlPipeHandlerThread);
	TQObject::connect(mControlPipeHandler, SIGNAL(processCommand(TQString)), this, SLOT(processInputPipeCommand(TQString)));
	TQTimer::singleShot(0, mControlPipeHandler, SLOT(run()));
	mControlPipeHandlerThread->start();
}

KGreeter::~KGreeter()
{
	mControlPipeHandlerThread->terminate();
	mControlPipeHandlerThread->wait();
	delete mControlPipeHandler;
// 	delete mControlPipeHandlerThread;

	hide();
	delete userList;
	delete verify;
	delete stsFile;
}

void KGreeter::cryptographicCardWatcherSetup() {
	cardLoginUser = TQString::null;

	// Initialize SmartCard readers
	TDEGenericDevice *hwdevice;
	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	TDEGenericHardwareList cardReaderList = hwdevices->listByDeviceClass(TDEGenericDeviceType::CryptographicCard);
	for (hwdevice = cardReaderList.first(); hwdevice; hwdevice = cardReaderList.next()) {
		TDECryptographicCardDevice* cdevice = static_cast<TDECryptographicCardDevice*>(hwdevice);
		connect(cdevice, TQT_SIGNAL(certificateListAvailable(TDECryptographicCardDevice*)), this, TQT_SLOT(cryptographicCardInserted(TDECryptographicCardDevice*)));
		connect(cdevice, TQT_SIGNAL(cardRemoved(TDECryptographicCardDevice*)), this, TQT_SLOT(cryptographicCardRemoved(TDECryptographicCardDevice*)));
		cdevice->enableCardMonitoring(true);
	}
}

void KGreeter::done(int r) {
	closingDown = true;
	inherited::done(r);
}

void KGreeter::processInputPipeCommand(TQString command) {
	command = command.replace('\n', "");
	TQStringList commandList = TQStringList::split('\t', command, false);
	if ((*(commandList.at(0))) == "LOGIN") {
		if (verify) {
			verify->setUser( (*(commandList.at(1))) );
			verify->setPassword( (*(commandList.at(2))) );
			accept();
		}
	}
}

void KGreeter::readFacesList()
{
    FILE *f = fopen( TQFile::encodeName( _faceDir + "/.randomlist" ), "rt" );
    if ( !f )
        return;
    TQTextIStream is( f );
    while ( !is.eof() )
    {
        TQString line = is.readLine().simplifyWhiteSpace();
        if ( line.isEmpty() )
            continue;
        TQString icon;
        int index = line.find( ' ' );
        if ( index > 0 ) {
            icon = line.left( index );
            line = line.mid( index );
        } else {
            icon = line;
            line = TQString::null;
        }
        randomFaces.push_back( icon );
        TQStringList list = TQStringList::split( ' ', line );
        for ( TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
            randomFacesMap[*it] = icon;
    }
}

class UserListViewItem : public TDEListViewItem {
  public:
	UserListViewItem( UserListView *parent, const TQString &text,
	                  const TQPixmap &pixmap, const TQString &username )
		: TDEListViewItem( parent )
		, login( username )
	{
		setPixmap( 0, pixmap );
		setMultiLinesEnabled( true );
		setText( 0, text );
		parent->cachedSizeHint.setWidth( -1 );
	}

        virtual void paintCell(TQPainter *p, const TQColorGroup &cg, int column, int width, int alignment)
    {
      if (((UserListView*)listView())->themed)
        TQListViewItem::paintCell(p, cg, column, width, alignment);
      else
	TDEListViewItem::paintCell(p, cg, column, width, alignment);
    }

	TQString login;
};

#define FILE_LIMIT_ICON 20
#define FILE_LIMIT_IMAGE 200

void
KGreeter::insertUser( const TQImage &default_pix,
                      const TQString &username, struct passwd *ps )
{
        if (setegid( ps->pw_gid ))
                return;
        if (seteuid( ps->pw_uid )) {
                setegid(0);
                return;
        }

	if (userList) {
		userList->append( username );
	}
	if (!userView) {
		seteuid(0);
		setegid(0);
		return;
	}

	int dp = 0, nd = 0;
	if (_faceSource == FACE_USER_ONLY ||
	    _faceSource == FACE_PREFER_USER)
		dp = 1;
	if (_faceSource != FACE_USER_ONLY &&
	    _faceSource != FACE_ADMIN_ONLY)
		nd = 1;
	TQImage p;
	do {
		dp ^= 1;
		TQCString fn = !dp ?
		              TQCString( ps->pw_dir ) + '/' :
		              TQCString(TQFile::encodeName( _faceDir + '/' + username ));
		fn += ".face.icon";
		int fd, ico;
		if ((fd = open( fn.data(), O_RDONLY | O_NONBLOCK )) < 0) {
			fn.truncate( fn.length() - 5 );
			if ((fd = open( fn.data(), O_RDONLY | O_NONBLOCK )) < 0) {
				continue;
			}
			ico = 0;
		} else
			ico = 1;
		TQFile f;
		f.open( IO_ReadOnly, fd );
		int fs = f.size();
		if (fs > (ico ? FILE_LIMIT_ICON : FILE_LIMIT_IMAGE) * 1000) {
			LogWarn( "%s exceeds file size limit (%dkB)\n", fn.data(), ico ? FILE_LIMIT_ICON : FILE_LIMIT_IMAGE );
			continue;
		}
		TQByteArray fc( fs );
		int rfs = f.readBlock( fc.data(), fs );
		::close( fd );
		fc.resize( rfs > 0 ? rfs : 0 );
		TQBuffer buf( fc );
		buf.open( IO_ReadOnly );
		TQImageIO ir;
		ir.setIODevice( TQT_TQIODEVICE(&buf) );
		if (!ir.read()) {
			LogInfo( "%s is no valid image\n", fn.data() );
			continue;
		}
		p = ir.image();
		TQSize ns( 48, 48 );
		if (p.size() != ns) {
			p = p.convertDepth( 32 ).smoothScale( ns, TQ_ScaleMin );
		}
		break;
	} while (--nd >= 0);

        if ( p.isNull() && randomFaces.count() ) {
            TQString randomFace = randomFacesMap[username];
            if ( randomFace.isNull() ) {
                TQStringList::size_type index = 0;
                for ( size_t i = 0; i < username.length(); ++i )
                    index += ( 0x7f - username.at( i ).latin1() ) % 37;
                randomFace = randomFaces[ index % randomFaces.count() ];
            }
            p.load( _faceDir + "/../pics/users/" + randomFace + ".png" );
        }

        if ( p.isNull() ) {
            p = default_pix;
	}

	TQString realname = KStringHandler::from8Bit( ps->pw_gecos );
	realname.truncate( realname.find( ',' ) );
	if (realname.isEmpty() || realname == username)
		new UserListViewItem( userView, username, TQPixmap( p ), username );
	else {
		realname.append( "\n" ).append( username );
		new UserListViewItem( userView, realname, TQPixmap( p ), username );
	}

        seteuid( 0 );
        setegid( 0 );
}

class KCStringList : public TQValueList<TQCString> {
  public:
	bool contains( const char *str ) const
	{
		for (ConstIterator it = begin(); it != end(); ++it)
			if (*it == str)
				return true;
		return false;
	}
};

class UserList {
  public:
	UserList( char **in );
	bool hasUser( const char *str ) const { return users.contains( str ); }
	bool hasGroup( gid_t gid ) const
		{ return groups.find( gid ) != groups.end(); }
	bool hasGroups() const { return !groups.isEmpty(); }
	KCStringList users;

  private:
	TQValueList<gid_t> groups;
};

UserList::UserList( char **in )
{
	struct group *grp;

	for (; *in; in++)
		if (**in == '@') {
			if ((grp = getgrnam( *in + 1 ))) {
				for (; *grp->gr_mem; grp->gr_mem++)
					users.append( *grp->gr_mem );
				groups.append( grp->gr_gid );
			}
		} else
			users.append( *in );
}

void
KGreeter::insertUsers(int limit_users)
{
	struct passwd *ps;

	if (!(ps = getpwnam( "nobody" ))) {
		return;
	}

	TQImage default_pix;
	if (userView) {
		if (!default_pix.load( _faceDir + "/.default.face.icon" ))
			if (!default_pix.load( _faceDir + "/.default.face" ))
				LogError( "Can't open default user face\n" );
		TQSize ns( 48, 48 );
		if (default_pix.size() != ns)
			default_pix =
			  default_pix.convertDepth( 32 ).smoothScale( ns, TQ_ScaleMin );
	}
	if (_showUsers == SHOW_ALL) {
		UserList noUsers( _noUsers );
		TQDict<int> dupes( 1000 );	// Potential crash risk with buffer overrun?
                TQStringList toinsert;
                int count = 0;
		for (setpwent(); (ps = getpwent()) != 0;) {
			if (*ps->pw_dir && *ps->pw_shell &&
			    ((ps->pw_uid >= (unsigned)_lowUserId) ||
			     ((!ps->pw_uid) && _showRoot)) &&
			    (ps->pw_uid <= (unsigned)_highUserId) &&
			    (!noUsers.hasUser( ps->pw_name )) &&
			    (!noUsers.hasGroup( ps->pw_gid )))
			{
				TQString username( TQFile::decodeName( ps->pw_name ) );
				if (!dupes.find( username )) {
					dupes.insert( username, (int *)-1 );
                                        toinsert.append( username );

                                        if ( limit_users >= 0 && ++count > limit_users )
                                            break;
				}
			}
		}
		// FIXME: OpenSUSE added this code
		// For some reason it does not allow LDAP users to be listed (!),
		// therefore it was deactivated.  It should be repaired and reactivated.
//                 if ( limit_users >= 0 && ++count > limit_users ) {
//                     utmpname( _PATH_WTMP );
//                     setutxent();
//                     toinsert = TQStringList();
//                     dupes.clear();
// 
//                     for ( count = 0; count < limit_users; ) {
//                         struct utmpx * ent = getutxent();
//                         if ( !ent )
//                             break;
//                         struct passwd *ps = getpwnam( ent->ut_user );
//                         if (ps && *ps->pw_dir && *ps->pw_shell &&
// 			    (ps->pw_uid >= (unsigned)_lowUserId ||
// 			     !ps->pw_uid && _showRoot) &&
// 			    ps->pw_uid <= (unsigned)_highUserId &&
// 			    !noUsers.hasUser( ps->pw_name ) &&
// 			    !noUsers.hasGroup( ps->pw_gid ))
//                         {
//                             TQString username( TQFile::decodeName( ent->ut_user ) );
//                             if (!dupes.find( username )) {
//                                 dupes.insert( username, (int *)-1 );
//                                 toinsert.append( username );
//                                 count++;
//                             }
//                         }
// 
// 
//                     }
//                     endutxent();
//                 }

                for ( TQStringList::ConstIterator it = toinsert.begin();
                      it != toinsert.end(); ++it )
                {
                    // pretty stupid to do another lookup round, but the number is limited
                    // and caching struct passwd is pretty ugly
                    struct passwd *ps = getpwnam( TQFile::encodeName( *it ) );
                    if ( ps )
                        insertUser( default_pix, *it, ps );
                }
	} else {
		UserList users( _users );
		if (users.hasGroups()) {
			TQDict<int> dupes( 1000 );
			for (setpwent(); (ps = getpwent()) != 0;) {
				if (*ps->pw_dir && *ps->pw_shell &&
				    (ps->pw_uid >= (unsigned)_lowUserId ||
				     ((!ps->pw_uid) && _showRoot)) &&
				    ps->pw_uid <= (unsigned)_highUserId &&
				    (users.hasUser( ps->pw_name ) ||
				     users.hasGroup( ps->pw_gid )))
				{
					TQString username( TQFile::decodeName( ps->pw_name ) );
					if (!dupes.find( username )) {
						dupes.insert( username, (int *)-1 );
						insertUser( default_pix, username, ps );
					}
				}
			}
		} else {
			KCStringList::ConstIterator it = users.users.begin();
			for (; it != users.users.end(); ++it)
				if ((ps = getpwnam( (*it).data() )) &&
				    (ps->pw_uid || _showRoot))
					insertUser( default_pix, TQFile::decodeName( *it ), ps );
		}
	}
	endpwent();
	if (_sortUsers) {
		if (userView)
			userView->sort();
		if (userList)
			userList->sort();
	}
}

void
KGreeter::putSession( const TQString &type, const TQString &name, bool hid, const char *exe )
{
	int prio = exe ? (!strcmp( exe, "default" ) ? 0 :
	                  !strcmp( exe, "custom" ) ? 1 :
	                  !strcmp( exe, "failsafe" ) ? 3 : 2) : 2;
	for (uint i = 0; i < sessionTypes.size(); i++)
		if (sessionTypes[i].type == type) {
			sessionTypes[i].prio = prio;
			return;
		}
	sessionTypes.append( SessType( name, type, hid, prio ) );
}

void
KGreeter::insertSessions()
{
	for (char **dit = _sessionsDirs; *dit; ++dit) {
		TQStringList ents = TQDir( *dit ).entryList();
		for (TQStringList::ConstIterator it = ents.begin(); it != ents.end(); ++it)
		  if ((*it).endsWith( ".desktop" ) && !(*it).endsWith("admin.desktop")) {
				KSimpleConfig dsk( TQString( *dit ).append( '/' ).append( *it ) );
				dsk.setGroup( "Desktop Entry" );
				putSession( (*it).left( (*it).length() - 8 ),
				            dsk.readEntry( "Name" ),
				            (dsk.readBoolEntry( "Hidden", false ) ||
				             (dsk.hasKey( "TryExec" ) &&
				              TDEStandardDirs::findExe( dsk.readEntry( "TryExec" ) ).isEmpty())),
				            dsk.readEntry( "Exec" ).latin1() );
			}
	}
	putSession( "default", i18n("Default"), false, "default" );
	putSession( "custom", i18n("Custom"), false, "custom" );
	putSession( "failsafe", i18n("Failsafe"), false, "failsafe" );
	qBubbleSort( sessionTypes );
	for (uint i = 0; i < sessionTypes.size() && !sessionTypes[i].hid; i++) {
		sessMenu->insertItem( sessionTypes[i].name, i );
		switch (sessionTypes[i].prio) {
		case 0: case 1: nSpecials++; break;
		case 2: nNormals++; break;
		}
	}
}

void
KGreeter::slotUserEntered()
{
	if (userView) {
		TQListViewItem *item;
		for (item = userView->firstChild(); item; item = item->nextSibling())
			if (((UserListViewItem *)item)->login == curUser) {
				userView->setSelected( item, true );
				userView->ensureItemVisible( item );
				goto oke;
			}
		userView->clearSelection();
	}
  oke:
	if (isVisible())
		slotLoadPrevWM();
	else
		TQTimer::singleShot( 0, this, TQT_SLOT(slotLoadPrevWM()) );
}

void
KGreeter::slotUserClicked( TQListViewItem *item )
{
	if (item) {
		curUser = ((UserListViewItem *)item)->login;
		verify->setUser( curUser );
		slotLoadPrevWM();
	}
}

void
KGreeter::slotSessionSelected( int id )
{
	if (id != curSel) {
		sessMenu->setItemChecked( curSel, false );
		sessMenu->setItemChecked( id, true );
		curSel = id;
		verify->gplugActivity();
	}
}

void
KGreeter::reject()
{
	verify->reject();
}

void
KGreeter::accept()
{
	if (userView && userView->hasFocus())
		slotUserClicked( userView->currentItem() );
	else
		verify->accept();
}

void // private
KGreeter::setPrevWM( int wm )
{
	if (curPrev != wm) {
		if (curPrev != -1) {
			sessMenu->changeItem( curPrev, sessionTypes[curPrev].name );
		}
		if (wm != -1) {
			sessMenu->changeItem( wm, sessionTypes[wm].name + i18n(" (previous)") );
		}
		curPrev = wm;
	}
}

void
KGreeter::slotLoadPrevWM()
{
	int len, i, b;
	unsigned long crc, by;
	TQCString name;
	char *sess;

	if (verify->coreLock) {
		needLoad = true;
		return;
	}
	needLoad = false;

	prevValid = true;
	name = curUser.local8Bit();
	GSendInt( G_ReadDmrc );
	GSendStr( name.data() );
	GRecvInt(); // ignore status code ...
	if ((len = name.length())) {
		GSendInt( G_GetDmrc );
		GSendStr( "Session" );
		sess = GRecvStr();
		if (!sess) { /* no such user */
			if (!userView && !userList) { // don't fake if user list shown
				prevValid = false;
				/* simple crc32 */
				for (crc = _forgingSeed, i = 0; i < len; i++) {
					by = (crc & 255) ^ name[i];
					for (b = 0; b < 8; b++)
						by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
					crc = (crc >> 8) ^ by;
				}
				/* forge a session with this hash - default & custom more probable */
				/* XXX - this should do a statistical analysis of the real users */
#if 1
				setPrevWM( crc % (nSpecials * 2 + nNormals) % (nSpecials + nNormals) );
#else
				i = crc % (nSpecials * 2 + nNormals);
				if (i < nNormals)
					setPrevWM( i + nSpecials );
				else
					setPrevWM( (i - nNormals) / 2 );
#endif
				return;
			}
		} else {
		  if (!strcmp(sess, "admin")) {
		    // need to get the original
		    GSendInt( G_GetDmrc);
		    GSendStr( "OrigSession");
		    sess = GRecvStr();
		    if (!sess) {
		      free(sess);
		      sess = strdup("default");
		    }
		  }

			for (uint i = 0; i < sessionTypes.count() && !sessionTypes[i].hid; i++) {
				if (sessionTypes[i].type == sess) {
					free( sess );
					setPrevWM( i );
					curWMSession = sessionTypes[i].type.utf8();
					return;
				}
			}
			if (curSel == -1) {
				MsgBox( sorrybox, i18n("Your saved session type '%1' is not valid any more.\n"
				                       "Please select a new one, otherwise 'default' will be used.").arg( sess ) );
			}
			free( sess );
			prevValid = false;
		}
	}
	setPrevWM( -1 );
}

void // protected
KGreeter::pluginSetup()
{
	int field = 0;
	TQString ent, pn( verify->pluginName() ), dn( dName + '_' + pn );

	if (_preselUser != PRESEL_PREV)
		stsFile->deleteEntry( verify->entitiesLocal() ? dName : dn, false );
	if (_preselUser != PRESEL_NONE && verify->entityPresettable()) {
		if (verify->entitiesLocal())
			ent = _preselUser == PRESEL_PREV ?
				stsFile->readEntry( dName ) : _defaultUser;
		else
			ent = _preselUser == PRESEL_PREV ?
				stsFile->readEntry( dn ) :
				verify->getConf( 0, (pn + ".DefaultEntity").latin1(), TQVariant() ).toString();
		field = verify->entitiesFielded() ?
			verify->getConf( 0, (pn + ".FocusField").latin1(), TQVariant( 0 ) ).toInt() :
			_focusPasswd;
	}
	verify->presetEntity( ent, field );
	if (userList)
		verify->loadUsers( *userList );
}

void
KGreeter::verifyPluginChanged( int id )
{
	curPlugin = id;
	pluginSetup();
}

void
KGreeter::verifyClear()
{
	curUser = TQString::null;
	slotUserEntered();
	slotSessionSelected( -1 );
}

void
KGreeter::verifyOk()
{
	if (_preselUser == PRESEL_PREV && verify->entityPresettable())
		stsFile->writeEntry( verify->entitiesLocal() ?
		                       dName :
		                       dName + '_' + verify->pluginName(),
		                     verify->getEntity() );
	if (curSel != -1) {
		GSendInt( G_PutDmrc );
		GSendStr( "Session" );
		GSendStr( sessionTypes[curSel].type.utf8() );
		curWMSession = sessionTypes[curSel].type.utf8();
	} else if (!prevValid) {
		GSendInt( G_PutDmrc );
		GSendStr( "Session" );
		GSendStr( "default" );
	}
	GSendInt( G_Ready );
	closingDown = true;
	done( ex_exit );
}

void
KGreeter::verifyFailed()
{
	if (needLoad) {
		slotLoadPrevWM();
	}
}

void
KGreeter::verifySetUser( const TQString &user )
{
	curUser = user;
	slotUserEntered();
}

void KGreeter::cryptographicCardInserted(TDECryptographicCardDevice* cdevice) {
	TQString login_name = TQString::null;
	X509CertificatePtrList certList = cdevice->cardX509Certificates();
	if (certList.count() > 0) {
		KSSLCertificate* card_cert = NULL;
		card_cert = KSSLCertificate::fromX509(certList[0]);
		TQStringList cert_subject_parts = TQStringList::split("/", card_cert->getSubject(), false);
		for (TQStringList::Iterator it = cert_subject_parts.begin(); it != cert_subject_parts.end(); ++it ) {
			TQString lcpart = (*it).lower();
			if (lcpart.startsWith("cn=")) {
				login_name = lcpart.right(lcpart.length() - strlen("cn="));
			}
		}
		delete card_cert;
	}

	if (login_name != "") {
		if (verify->cardLoginInProgress) {
			return;
		}
		verify->cardLoginInProgress = true;
		verify->cardLoginDevice = cdevice;

		DM dm;
		SessList sess;
		bool vt_active = false;
		bool user_active = false;
		if (dm.localSessions(sess)) {
			TQString user, loc;
			for (SessList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
				DM::sess2Str2(*it, user, loc);
				if (user.startsWith(login_name + ": ")) {
					// Found active session
					user_active = true;
				}
				if ((*it).self) {
					if ((*it).vt == dm.activeVT()) {
						vt_active = true;
					}
				}
			}
		}

		if (!user_active && vt_active) {
			// Select the correct user
			verify->setUser(login_name);
			verifySetUser(login_name);
			verify->lockUserEntry(true);

			// FIXME
			// pam_pkcs11 is extremely chatty with no apparent way to disable the unwanted messages
			verify->setInfoMessageDisplay(false);

			// Set up password prompt
			cardLoginUser = login_name;
			verify->setPasswordPrompt(i18n("PIN:"));

			// Bypass initial password prompt
			verify->start();
			verify->setPassword("");
			verify->accept();
		}
	}
}

void KGreeter::cryptographicCardRemoved(TDECryptographicCardDevice* cdevice) {
	cardLoginUser = TQString::null;
	verify->lockUserEntry(false);
        verify->requestAbort();
        verify->setPasswordPrompt(TQString::null);

	// Restore information message display settings
        verify->setInfoMessageDisplay(showInfoMessages);

	verify->cardLoginInProgress = false;
	verify->cardLoginDevice = NULL;
}

KStdGreeter::KStdGreeter()
  : KGreeter()
  , clock( 0 )
  , pixLabel( 0 )
{
	TQBoxLayout *main_box;
#ifdef WITH_TDM_XCONSOLE
	if (consoleView) {
		TQBoxLayout *ex_box = new TQVBoxLayout( this, 10, 10 );
		main_box = new TQHBoxLayout( ex_box, 10 );
		ex_box->addWidget( consoleView );
	} else
#endif
		main_box = new TQHBoxLayout( this, 10, 10 );

	if (userView)
		main_box->addWidget( userView );

	TQBoxLayout *inner_box = new TQVBoxLayout( main_box, 10 );

	if (!_authorized && _authComplain) {
		TQLabel *complainLabel = new TQLabel(
			i18n("Warning: this is an unsecured session"), this );
		TQToolTip::add( complainLabel,
		               i18n("This display requires no X authorization.\n"
		                    "This means that anybody can connect to it,\n"
		                    "open windows on it or intercept your input.") );
		complainLabel->setAlignment( AlignCenter );
		complainLabel->setFont( _failFont );
		complainLabel->setPaletteForegroundColor( Qt::red );
		inner_box->addWidget( complainLabel );
	}
	if (_logoArea == LOGO_NONE) {
		KSMModalDialogHeader *theader = new KSMModalDialogHeader(this);
		inner_box->addWidget( theader, AlignCenter );
	}
	if (!_greetString.isEmpty()) {
		TQLabel *welcomeLabel = new TQLabel( _greetString, this );
		welcomeLabel->setAlignment( AlignCenter );
		welcomeLabel->setFont( _greetFont );
		inner_box->addWidget( welcomeLabel );
		if (_logoArea == LOGO_NONE) {
			// Match caps lock warning font size
			TQLabel *spacerLabel = new TQLabel( " ", this );
			spacerLabel->setFont( _failFont );
			inner_box->addWidget( spacerLabel, AlignCenter );
		}
	}

	switch (_logoArea) {
		case LOGO_CLOCK:
			clock = new KdmClock( this, "clock" );
			break;
		case LOGO_LOGO:
			{
				TQMovie movie( _logo );
				kapp->eventLoop()->processEvents( TQEventLoop::ExcludeUserInput | TQEventLoop::ExcludeSocketNotifiers, 100 );
				TQPixmap pixmap;
				if (!movie.framePixmap().isNull() || pixmap.load( _logo )) {
					pixLabel = new TQLabel( this );
					if (!movie.framePixmap().isNull()) {
						pixLabel->setMovie( movie );
						if (!movie.framePixmap().hasAlpha())
							pixLabel->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
					} else {
						pixLabel->setPixmap( pixmap );
						if (!pixmap.hasAlpha())
							pixLabel->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
					}
					pixLabel->setIndent( 0 );
				}
			}
			break;
	}

	if (userView) {
		if (clock)
			inner_box->addWidget( clock, 0, AlignCenter );
		else if (pixLabel)
			inner_box->addWidget( pixLabel, 0, AlignCenter );
	} else {
		if (clock)
			main_box->addWidget( clock, 0, AlignCenter );
		else if (pixLabel)
			main_box->addWidget( pixLabel, 0, AlignCenter );
	}

	goButton = new TQPushButton( i18n("L&ogin"), this );
	goButton->setDefault( true );
	connect( goButton, TQT_SIGNAL(clicked()), TQT_SLOT(accept()) );
	menuButton = new TQPushButton( i18n("&Menu"), this );
	//helpButton

	TQWidget *prec;
	if (userView)
		prec = userView;
#ifdef WITH_TDM_XCONSOLE
	else if (consoleView)
		prec = consoleView;
#endif
	else
		prec = menuButton;
	KGStdVerify *sverify =
		new KGStdVerify( this, this, prec, TQString::null,
		                 pluginList, KGreeterPlugin::Authenticate,
		                 KGreeterPlugin::Login );
	inner_box->addLayout( sverify->getLayout() );
	TQPopupMenu *plugMenu = sverify->getPlugMenu();
	sverify->selectPlugin( curPlugin );
	verify = sverify;

	inner_box->addWidget( new KSeparator( KSeparator::HLine, this ) );

	TQBoxLayout *hbox2 = new TQHBoxLayout( inner_box, 10 );
	hbox2->addWidget( goButton );
	hbox2->addStretch( 1 );
	hbox2->addWidget( menuButton );
	hbox2->addStretch( 1 );

	if (sessMenu->count() > 1) {
		inserten( i18n("Session &Type"), 0, sessMenu );
		needSep = true;
	}

	if (plugMenu) {
		inserten( i18n("&Authentication Method"), 0, plugMenu );
		needSep = true;
	}

#ifdef XDMCP
	completeMenu( LOGIN_LOCAL_ONLY, ex_choose, i18n("&Remote Login"), 0 );
#else
	completeMenu();
#endif

        if (userView || userList)
		insertUsers();

	if (optMenu)
		menuButton->setPopup( optMenu );
	else
		menuButton->hide();

	pluginSetup();

        verify->setInfoMessageDisplay(showInfoMessages);
	verify->start();

	TQTimer::singleShot(0, this, SLOT(cryptographicCardWatcherSetup()));
}

void
KStdGreeter::pluginSetup()
{
	inherited::pluginSetup();
	if (userView) {
		if (verify->entitiesLocal() && verify->entityPresettable())
			userView->show();
		else
			userView->hide();
	}
	adjustGeometry();
	update();
}

void
KStdGreeter::verifyFailed()
{
	goButton->setEnabled( false );
	menuButton->setEnabled( false );
	if (userView)
		userView->setEnabled( false );
	inherited::verifyFailed();
}

void
KStdGreeter::verifyRetry()
{
	goButton->setEnabled( true );
	menuButton->setEnabled( true );
	if (userView)
		userView->setEnabled( true );
}


KThemedGreeter::KThemedGreeter()
	: KGreeter( true )
	, themer( 0 )
//	, clock( 0 )
{
	// We do all painting ourselves
	setBackgroundMode( NoBackground );
	// Allow tracking the mouse position
	setMouseTracking( true );

	adjustGeometry();

	themer = new KdmThemer( _theme, "console", this );
	if (!themer->isOK()) {
		delete themer;
		themer = 0;
		return;
	}

	connect( themer, TQT_SIGNAL(activated( const TQString & )),
	         TQT_SLOT(slotThemeActivated( const TQString & )) );

	console_rect = themer->findNode( "xconsole" ); // tdm ext
	userlist_rect = themer->findNode( "userlist" );
	caps_warning = themer->findNode( "caps-lock-warning" );
	xauth_warning = themer->findNode( "xauth-warning" ); // tdm ext
	pam_error = themer->findNode( "pam-error" );
	timed_label = themer->findNode( "timed-label" );
	if (pam_error && pam_error->isA( "KdmLabel" )) {
		static_cast<KdmLabel*>(pam_error)->setText( i18n("Login Failed.") );
	}

	KdmItem *itm;
	if ((itm = themer->findNode( "pam-message" ))) // done via msgboxes
		itm->hide( true );
	if ((itm = themer->findNode( "language_button" ))) // not implemented yet
		itm->hide( true );

#ifdef WITH_TDM_XCONSOLE
	if (console_rect) {
		if (consoleView)
			console_rect->setWidget( consoleView );
		else
			console_rect->hide( true );
	}
#endif

	if (xauth_warning && (_authorized || !_authComplain))
		xauth_warning->hide( true );

        if (userView || userList)
            insertUsers( 7 ); // TODO: find out how many are a good value

//	if (!_greetString.isEmpty()) {
//	}
//	clock = new KdmClock( this, "clock" );

	TQWidget *prec;
	if (userView)
		prec = userView;
#ifdef WITH_TDM_XCONSOLE
	else if (consoleView)
		prec = consoleView;
#endif
	else
		prec = 0;
	KGThemedVerify *tverify =
		new KGThemedVerify( this, themer, this, prec, TQString::null,
		                    pluginList, KGreeterPlugin::Authenticate,
		                    KGreeterPlugin::Login );
	TQPopupMenu *plugMenu = tverify->getPlugMenu();
	tverify->selectPlugin( curPlugin );
	verify = tverify;

	session_button = 0;
	if ((itm = themer->findNode( "session_button" ))) {
		if (sessMenu->count() <= 1)
			itm->hide( true );
                else
			session_button = itm;
	} else {
		if (sessMenu->count() > 1) {
			inserten( i18n("Session &Type"), ALT+Key_T, sessMenu );
			needSep = true;
		}
	}

	admin_button = themer->findNode( "admin_button");
	if ( admin_button ) {
	  if ( !_useAdminSession )
	    admin_button->hide( true );
	}

	if (plugMenu) {
		inserten( i18n("&Authentication Method"), ALT+Key_A, plugMenu );
		needSep = true;
	}

#ifdef XDMCP
	completeMenu( LOGIN_LOCAL_ONLY, ex_choose, i18n("&Remote Login"), ALT+Key_R );
#else
	completeMenu();
#endif

	system_button = themer->findNode( "system_button" );
	TQAccel *accel = new TQAccel( this );
	accel->insertItem( ALT+Key_M, 0 );
	connect( accel, TQT_SIGNAL(activated( int )), TQT_SLOT(slotActionMenu()) );

	pluginSetup();

	verify->setInfoMessageDisplay(showInfoMessages);
	verify->start();

	TQTimer::singleShot(0, this, SLOT(cryptographicCardWatcherSetup()));
}

bool
KThemedGreeter::event( TQEvent *e )
{
	if (themer)
		themer->widgetEvent( e );
	return inherited::event( e );
}

void
KThemedGreeter::pluginSetup()
{
	inherited::pluginSetup();

	if (userView && verify->entitiesLocal() && verify->entityPresettable() && userlist_rect) {
//		userView->setMaximumHeight( userView->sumHeight() );
		userlist_rect->setWidget( userView );
	} else {
		if (userView)
			userView->hide();
		if (userlist_rect)
			userlist_rect->hide( true );
	}

	update();
}

void
KThemedGreeter::verifyFailed()
{
//	goButton->setEnabled( false );
	inherited::verifyFailed();
	if (userView)
		userView->setEnabled(false);
}

void
KThemedGreeter::verifyRetry()
{
//	goButton->setEnabled( true );
        if (userView)
                userView->setEnabled(true);

}

TQString KThemedGreeter::timedUser = TQString::null;
int KThemedGreeter::timedDelay = -1;

void
KThemedGreeter::updateStatus( bool fail, bool caps, int timedleft )
{
	if (pam_error) {
		if (fail)
			pam_error->show( true );
		else
			pam_error->hide( true );
	}
	if (caps_warning) {
		if (caps)
			caps_warning->show( true );
		else
			caps_warning->hide( true );
	}
	if (timed_label) {
		if (timedleft) {
			if (timedleft != timedDelay) {
				timedDelay = timedleft;
				timedUser = curUser;
				timed_label->show( true );
				timed_label->update();
			}
		} else {
			timedDelay = -1;
			timed_label->hide( true );
		}
	}

	if (cardLoginUser != TQString::null) {
		verify->setPasswordPrompt(i18n("PIN:"));
	}
}

void
KThemedGreeter::slotThemeActivated( const TQString &id )
{
	if (id == "login_button")
		accept();
	else if (id == "session_button")
		slotSessMenu();
	else if (id == "system_button")
		slotActionMenu();
	else if (id == "admin_button")
	        slotAskAdminPassword();
}

void
KThemedGreeter::slotSessMenu()
{
	sessMenu->popup( mapToGlobal( session_button->rect().center() ) );
}

void
KThemedGreeter::slotActionMenu()
{
	if (system_button)
		optMenu->popup( mapToGlobal( system_button->rect().center() ) );
	else
		optMenu->popup( mapToGlobal( rect().center() ) );
}

void
KThemedGreeter::keyPressEvent( TQKeyEvent *e )
{
	inherited::keyPressEvent( e );
	if (!(e->state() & KeyButtonMask) &&
	    (e->key() == Key_Return || e->key() == Key_Enter))
		accept();
}

void
KThemedGreeter::slotAskAdminPassword()
{
  TDMAdmin k(curUser, this);
  if (k.exec()) {
	GSendInt(G_Ready);
	hide();
	closingDown = true;
	done(ex_exit);
   }
}

//===========================================================================
//
// TDM control pipe handler
//
ControlPipeHandlerObject::ControlPipeHandlerObject() : TQObject() {
	mPipe_fd = -1;
	mKGreeterParent = NULL;
	mSAKDlgParent = NULL;
}

ControlPipeHandlerObject::~ControlPipeHandlerObject() {
	if (mPipe_fd != -1) {
		if (mKGreeterParent) mKGreeterParent->closingDown = true;
		if (mSAKDlgParent) mSAKDlgParent->closingDown = true;
		::close(mPipe_fd);
		::unlink(mPipeFilename.ascii());
	}
}

void ControlPipeHandlerObject::run(void) {
	while (1) {
		if ((mKGreeterParent && (mKGreeterParent->closingDown)) || (mSAKDlgParent && (mSAKDlgParent->closingDown))) {
			::unlink(mPipeFilename.ascii());
			TQApplication::eventLoop()->exit(-1);
			return;
		}

		if ((mKGreeterParent && (mKGreeterParent->isShown())) || (mSAKDlgParent && (mSAKDlgParent->isShown()))) {
			char readbuf[2048];
			int displayNumber;
			TQString currentDisplay;
			currentDisplay = TQString(getenv("DISPLAY"));
			currentDisplay = currentDisplay.replace(":", "");
			displayNumber = currentDisplay.toInt();
			if (mKGreeterParent) {
				mPipeFilename = TQString(FIFO_FILE).arg(displayNumber);
				::unlink((TQString(FIFO_SAK_FILE).arg(displayNumber)).ascii());
			}
			if (mSAKDlgParent) {
				mPipeFilename = TQString(FIFO_SAK_FILE).arg(displayNumber);
				::unlink((TQString(FIFO_FILE).arg(displayNumber)).ascii());
			}

			/* Create the FIFOs if they do not exist */
			umask(0);
			struct stat buffer;
			int status;
			char *fifo_parent_dir;
			char *fifo_dir = strdup(FIFO_DIR);
			fifo_parent_dir = dirname(fifo_dir);
			status = stat(fifo_parent_dir, &buffer);
			if (status != 0) {
				mkdir(fifo_parent_dir, 0755);
			}
			free(fifo_dir);
			status = stat(FIFO_DIR, &buffer);
			if (status == 0) {
				int dir_mode = ((buffer.st_mode & S_IRWXU) >> 6) * 100;
				dir_mode = dir_mode + ((buffer.st_mode & S_IRWXG) >> 3) * 10;
				dir_mode = dir_mode + ((buffer.st_mode & S_IRWXO) >> 0) * 1;
				if ((dir_mode != 700 && dir_mode != 600) || (buffer.st_uid != 0) || (buffer.st_gid != 0)) {
					printf("[WARNING] Possible security breach!  Please check permissions on " FIFO_DIR " (must be 700 and owned by root/root, got %d %d/%d).  Not listening for login credentials on remote control socket.\n", dir_mode, buffer.st_uid, buffer.st_gid); fflush(stdout);
					TQApplication::eventLoop()->exit(-1);
					return;
				}
			}
			else {
				mkdir(FIFO_DIR, 0700);
			}
			status = stat(mPipeFilename.ascii(), &buffer);
			if (status == 0) {
				int file_mode = ((buffer.st_mode & S_IRWXU) >> 6) * 100;
				file_mode = file_mode + ((buffer.st_mode & S_IRWXG) >> 3) * 10;
				file_mode = file_mode + ((buffer.st_mode & S_IRWXO) >> 0) * 1;
				if ((file_mode != 600) || (buffer.st_uid != 0) || (buffer.st_gid != 0)) {
					::unlink(mPipeFilename.ascii());
					printf("[WARNING] Possible security breach!  Please check permissions on %s (must be 600 and owned by root/root, got %d %d/%d).  Not listening for login credentials on remote control socket.\n", mPipeFilename.ascii(), file_mode, buffer.st_uid, buffer.st_gid); fflush(stdout);
					TQApplication::eventLoop()->exit(-1);
					return;
				}
			}
			else {
				mknod(mPipeFilename.ascii(), S_IFIFO|0600, 0);
				chmod(mPipeFilename.ascii(), 0600);
			}

			mPipe_fd = ::open(mPipeFilename.ascii(), O_RDONLY | O_NONBLOCK);
			int numread;
			int retval;
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(mPipe_fd, &rfds);
			TQString inputcommand = "";
			while ((!inputcommand.contains('\n')) && ((mKGreeterParent && (!mKGreeterParent->closingDown)) || (mSAKDlgParent && (!mSAKDlgParent->closingDown)))) {
				// Wait for mPipe_fd to receive input
				retval = select(mPipe_fd + 1, &rfds, NULL, NULL, NULL);
				if (retval < 0) {
					// ERROR
				}
				else if (retval) {
					// New data is available
					numread = ::read(mPipe_fd, readbuf, 2048);
					readbuf[numread] = 0;
					readbuf[2047] = 0;
					inputcommand += readbuf;
				}
			}
			if ((mKGreeterParent && (mKGreeterParent->closingDown)) || (mSAKDlgParent && (mSAKDlgParent->closingDown))) {
				::unlink(mPipeFilename.ascii());
				TQApplication::eventLoop()->exit(-1);
				return;
			}

			emit processCommand(inputcommand);

			if ((mKGreeterParent && (!mKGreeterParent->closingDown)) || (mSAKDlgParent && (!mSAKDlgParent->closingDown))) {
				::close(mPipe_fd);
				::unlink(mPipeFilename.ascii());
			}
			else {
				::unlink(mPipeFilename.ascii());
				TQApplication::eventLoop()->exit(-1);
				return;
			}
		}

		// Thread cancellation point
		usleep(1);
	}
	TQApplication::eventLoop()->exit(-1);
}

#include "kgreeter.moc"
