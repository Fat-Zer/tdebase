#include <string.h>
#include <errno.h>

#include <tqstring.h>
#include <tqmap.h>
#include <tqfile.h>
#include <tqdir.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKM.h>

#include "extension.h"


TQMap<TQString, FILE*> XKBExtension::fileCache;	//TODO: move to class?


static TQString getLayoutKey(const TQString& tqlayout, const TQString& variant)
{
	return tqlayout + "." + variant;
}

TQString XKBExtension::getPrecompiledLayoutFilename(const TQString& tqlayoutKey)
{
	TQString compiledLayoutFileName = m_tempDir + tqlayoutKey + ".xkm";
	return compiledLayoutFileName;
}

XKBExtension::XKBExtension(Display *d)
{
	if ( d == NULL )
		d = qt_xdisplay();
	m_dpy = d;
	
//	TQStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
//	m_tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0];
	m_tempDir = locateLocal("tmp", "");
}

bool XKBExtension::init()
{
    // Verify the Xlib has matching XKB extension.

    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;
	
    if (!XkbLibraryVersion(&major, &minor))
    {
        kdError() << "Xlib XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion << endl;
        return false;
    }

    // Verify the X server has matching XKB extension.

    int opcode_rtrn;
    int error_rtrn;
    int xkb_opcode;
    if (!XkbQueryExtension(m_dpy, &opcode_rtrn, &xkb_opcode, &error_rtrn,
                         &major, &minor))
    {
        kdError() << "X server XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion << endl;
        return false;
    }

    // Do it, or face horrible memory corrupting bugs
    ::XkbInitAtoms(NULL);

    return true;
}

void XKBExtension::reset()
{
	for(TQMap<TQString, FILE*>::ConstIterator it = fileCache.begin(); it != fileCache.end(); it++) {
		fclose(*it);
//		remove( TQFile::encodeName(getPrecompiledLayoutFileName(*it)) );
	}
	fileCache.clear();
}

XKBExtension::~XKBExtension()
{
/*	if( m_compiledLayoutFileNames.isEmpty() == false )
		deletePrecompiledLayouts();*/
}

bool XKBExtension::setXkbOptions(const TQString& options, bool resetOld)
{
    if (options.isEmpty())
        return true;

    TQString exe = KGlobal::dirs()->findExe("setxkbmap");
    if (exe.isEmpty())
        return false;

    KProcess p;
    p << exe;
    if( resetOld )
        p << "-option";
    p << "-option" << options;

    p.start(KProcess::Block);

    return p.normalExit() && (p.exitStatus() == 0);
}

bool XKBExtension::setLayout(const TQString& model,
		const TQString& tqlayout, const TQString& variant,
		const TQString& includeGroup, bool useCompiledLayouts)
{
	if( useCompiledLayouts == false ) {
		return setLayoutInternal( model, tqlayout, variant, includeGroup );
	}
	
	const TQString tqlayoutKey = getLayoutKey(tqlayout, variant);
	
	bool res;
	if( fileCache.tqcontains(tqlayoutKey) ) {
		res = setCompiledLayout( tqlayoutKey );
		kdDebug() << "setCompiledLayout " << tqlayoutKey << ": " << res << endl;

		if( res )
			return res;
	}
//	else {
		res = setLayoutInternal( model, tqlayout, variant, includeGroup );
		kdDebug() << "setRawLayout " << tqlayoutKey << ": " << res << endl;
		if( res )
			compileCurrentLayout( tqlayoutKey );
		
//	}
	return res;
}

// private
bool XKBExtension::setLayoutInternal(const TQString& model,
		const TQString& tqlayout, const TQString& variant,
		const TQString& includeGroup)
{
    if ( tqlayout.isEmpty() )
        return false;

	TQString exe = KGlobal::dirs()->findExe("setxkbmap");
	if( exe.isEmpty() ) {
		kdError() << "Can't find setxkbmap" << endl;
		return false;
	}

    TQString fullLayout = tqlayout;
    TQString fullVariant = variant;
	if( includeGroup.isEmpty() == false ) {
        fullLayout = includeGroup;
        fullLayout += ",";
        fullLayout += tqlayout;
		
//    fullVariant = baseVar;
        fullVariant = ",";
        fullVariant += variant;
    }
 
    KProcess p;
    p << exe;
//  p << "-rules" << rule;
	if( model.isEmpty() == false )
		p << "-model" << model;
    p << "-tqlayout" << fullLayout;
    if( !fullVariant.isNull() && !fullVariant.isEmpty() )
        p << "-variant" << fullVariant;

    p.start(KProcess::Block); 

    // reload system-wide hotkey-setup keycode -> keysym maps
    if ( TQFile::exists( "/opt/trinity/share/apps/kxkb/system.xmodmap" ) ) {
        KProcess pXmodmap;
        pXmodmap << "xmodmap" << "/opt/trinity/share/apps/kxkb/system.xmodmap";
        pXmodmap.start(KProcess::Block);
    }

    if ( TQFile::exists( TQDir::home().path() + "/.Xmodmap" ) ) {
        KProcess pXmodmapHome;
        pXmodmapHome << "xmodmap" << TQDir::home().path() + "/.Xmodmap";
        pXmodmapHome.start(KProcess::Block);
    }

    return p.normalExit() && (p.exitStatus() == 0);
}

bool XKBExtension::setGroup(unsigned int group)
{
	kdDebug() << "Setting group " << group << endl;
	return XkbLockGroup( m_dpy, XkbUseCoreKbd, group );
}

unsigned int XKBExtension::getGroup() const
{
	XkbStateRec xkbState;
	XkbGetState( m_dpy, XkbUseCoreKbd, &xkbState );
	return xkbState.group;
}

/**
 * @brief Gets the current tqlayout in its binary compiled form
 *		and write it to the file specified by 'fileName'
 * @param[in] fileName file to store compiled tqlayout to
 * @return true if no problem, false otherwise
 */
bool XKBExtension::compileCurrentLayout(const TQString &tqlayoutKey)
{
    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
    result.type = XkmKeymapFile;
    XkbReadFromServer(m_dpy, XkbAllMapComponentsMask, XkbAllMapComponentsMask, &result);
	 
	const TQString fileName = getPrecompiledLayoutFilename(tqlayoutKey);

	kdDebug() << "compiling tqlayout " << this << " cache size: " << fileCache.count() << endl;
	if( fileCache.tqcontains(tqlayoutKey) ) {
		kdDebug() << "trashing old compiled tqlayout for " << fileName << endl;
		if( fileCache[ tqlayoutKey ] != NULL )
			fclose( fileCache[ tqlayoutKey ] );	// recompiling - trash the old file
		fileCache.remove(fileName);
	}

	FILE *output = fopen(TQFile::encodeName(fileName), "w");
		
    if ( output == NULL )
    {
		kdWarning() << "Could not open " << fileName << " to precompile: " << strerror(errno) << endl;
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        return false;
    }

	if( !XkbWriteXKMFile(output, &result) ) {
		kdWarning() << "Could not write compiled tqlayout to " << fileName << endl;
		fclose(output);
        return false;
	}
	
	fclose(output);	// TODO: can we change mode w/out reopening?
	FILE *input = fopen(TQFile::encodeName(fileName), "r");
	fileCache[ tqlayoutKey ] = input;

	XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}

/**
 * @brief takes tqlayout from its compiled binary snapshot in file 
 *	and sets it as current
 * TODO: cache tqlayout in memory rather than in file
 */
bool XKBExtension::setCompiledLayout(const TQString &tqlayoutKey)
{
	FILE *input = NULL;
	
	if( fileCache.tqcontains(tqlayoutKey) ) {
		input = fileCache[ tqlayoutKey ];
	}
	
	if( input == NULL ) {
		kdWarning() << "setCompiledLayout trying to reopen xkb file" << endl;	// should never happen
		const TQString fileName = getPrecompiledLayoutFilename(tqlayoutKey);
		input = fopen(TQFile::encodeName(fileName), "r");
		
		// 	FILE *input = fopen(TQFile::encodeName(fileName), "r");
		if ( input == NULL ) {
			kdDebug() << "Unable to open " << fileName << ": " << strerror(errno) << endl;
			fileCache.remove(tqlayoutKey);
			return false;
		}
	}
	else {
		rewind(input);
	}

    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
	if ((result.xkb = XkbAllocKeyboard())==NULL) {
		kdWarning() << "Unable to allocate memory for keyboard description" << endl;
//      fclose(input);
//		fileCache.remove(tqlayoutKey);
    	return false;
	}
	
    unsigned retVal = XkmReadFile(input, 0, XkmKeymapLegal, &result);
    if (retVal == XkmKeymapLegal)
    {
        // this means reading the Xkm didn't manage to read any section
        kdWarning() << "Unable to load map from file" << endl;
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        fclose(input);
		fileCache.remove(tqlayoutKey);
		return false;
    }

	//    fclose(input);	// don't close - goes in cache

    if (XkbChangeKbdDisplay(m_dpy, &result) == Success)
    {
        if (!XkbWriteToServer(&result))
        {
            kdWarning() << "Unable to write the keyboard tqlayout to X display" << endl;
            XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
            return false;
        }
    }
    else
    {
        kdWarning() << "Unable prepare the keyboard tqlayout for X display" << endl;
    }

    XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}


// Deletes the precompiled tqlayouts stored in temporary files
// void XKBExtension::deletePrecompiledLayouts()
// {
// 	TQMapConstIterator<LayoutUnit, TQString> it, end;
// 	end = m_compiledLayoutFileNames.end();
// 	for (it = m_compiledLayoutFileNames.begin(); it != end; ++it)
// 	{
// 		unlink(TQFile::encodeName(it.data()));
// 	}
// 	m_compiledLayoutFileNames.clear();
// }
