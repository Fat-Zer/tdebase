/*
	Main Widget for showing system-dependent information.
	(But all functions in THIS FILE should be system independent !)

	(C) 1998-2003 by Helge Deller <deller@kde.org>

	** main.cpp includes this file ! **

	This source-file includes another system-dependet sourcefile called
	info_<systemname>.cpp
	which should define one or more of the following defines to
	indicate, that this information is really available.

        #define INFO_CPU_AVAILABLE
	#define INFO_IRQ_AVAILABLE
        #define INFO_DMA_AVAILABLE
	#define INFO_PCI_AVAILABLE
        #define INFO_IOPORTS_AVAILABLE
	#define INFO_SOUND_AVAILABLE
        #define INFO_DEVICES_AVAILABLE
	#define INFO_SCSI_AVAILABLE
        #define INFO_PARTITIONS_AVAILABLE
	#define INFO_XSERVER_AVAILABLE

	right now, there is the problem, that also the .desktop-files should
	depend on the systemname, so that only available .desktop-files will
	be copied to kde/applnk/Settings/Information !!
*/

#include <tqheader.h>
#include <tqwhatsthis.h>
#include <tqlayout.h>

#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <kdialog.h>

#include "info.h"		/* include the forward declares... */

#include <X11/Xlib.h>

/* All Functions GetInfo_xyz() can set GetInfo_ErrorString, when a special
   error-message should be shown to the user....
   If GetInfo_ErrorString is not modified in the function, the default string
   DEFAULT_ERRORSTRING will be used...
*/

static TQString *GetInfo_ErrorString;	/* should always point to:
					    KInfoListWidget::ErrorString */
static bool	sorting_allowed;	/* is sorting allowed by user ? */



#if defined(__linux__)
# define DEFAULT_ERRORSTRING TQString::null /* i18n("Maybe the proc-filesystem is not enabled in Linux-Kernel.") */
#elif defined(__hpux)
# define DEFAULT_ERRORSTRING TQString::null
#else
#define DEFAULT_ERRORSTRING  i18n("Maybe this system is not completely supported yet :-(")
#endif





/* easier to read with such a define ! */
#define I18N_MAX(txt,in,fm,maxw) \
    { int n = fm.width(txt=in); if (n>maxw) maxw=n; }

#define PIXEL_ADD	20	// add x Pixel to multicolumns..

#define HEXDIGITS (sizeof(int)*8/4)	/* 4 Bytes = 32 Bits = 8 Hex-Digits */

static const TQString Value( int val, int numbers=1 )
{
  return TDEGlobal::locale()->formatNumber(val, 0).rightJustify(numbers);
}

static const TQString HexStr(unsigned long val, int digits )
{
    TQString hexstr;
    int i;
    hexstr = TQString::fromLatin1("0x%1").arg(val, digits, 16/*=HEX*/);
    for (i=hexstr.length()-1; i>0; --i)
     if (hexstr[i]==' ')
         hexstr[i] = '0';
    return hexstr;
}

static struct _event_table {
    const char *name;
    long value;
} event_table[] = {
    { "KeyPressMask", 		KeyPressMask },
    { "KeyReleaseMask",		KeyReleaseMask },
    { "ButtonPressMask",	ButtonPressMask },
    { "ButtonReleaseMask",	ButtonReleaseMask },
    { "EnterWindowMask",	EnterWindowMask },
    { "LeaveWindowMask",	LeaveWindowMask },
    { "PointerMotionMask",	PointerMotionMask },
    { "PointerMotionHintMask",	PointerMotionHintMask },
    { "Button1MotionMask",	Button1MotionMask },
    { "Button2MotionMask",	Button2MotionMask },
    { "Button3MotionMask",	Button3MotionMask },
    { "Button4MotionMask",	Button4MotionMask },
    { "Button5MotionMask",	Button5MotionMask },
    { "ButtonMotionMask",	ButtonMotionMask },
    { "KeymapStateMask",	KeymapStateMask },
    { "ExposureMask",		ExposureMask },
    { "VisibilityChangeMask",	VisibilityChangeMask },
    { "StructureNotifyMask",	StructureNotifyMask },
    { "ResizeRedirectMask",	ResizeRedirectMask },
    { "SubstructureNotifyMask",	SubstructureNotifyMask },
    { "SubstructureRedirectMask",SubstructureRedirectMask },
    { "FocusChangeMask",	FocusChangeMask },
    { "PropertyChangeMask",	PropertyChangeMask },
    { "ColormapChangeMask",	ColormapChangeMask },
    { "OwnerGrabButtonMask",	OwnerGrabButtonMask },
    { 0L, 0 }};


static TQListViewItem* XServer_fill_screen_info( TQListViewItem *lBox, TQListViewItem *last,
	    Display *dpy, int scr, int default_scr)
{
    unsigned	width, height;
    double	xres, yres;
    int 	i,
		ndepths,
		*depths;
    Screen 	*s = ScreenOfDisplay(dpy,scr);  /* opaque structure */
    TQListViewItem *item;

    /*
     * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
     *
     *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
     *         = N pixels / (M inch / 25.4)
     *         = N * 25.4 pixels / M inch
     */

    xres = ((double)(DisplayWidth(dpy,scr) *25.4)/DisplayWidthMM(dpy,scr) );
    yres = ((double)(DisplayHeight(dpy,scr)*25.4)/DisplayHeightMM(dpy,scr));

    item = new TQListViewItem(lBox,last, i18n("Screen # %1").arg((int)scr,-1),
		(scr==default_scr) ? i18n("(Default Screen)") : TQString::null );
    item->setExpandable(true);
    if (scr==default_scr)
	item->setOpen(true);
    last = new TQListViewItem(item, i18n("Dimensions"),
		i18n("%1 x %2 Pixel (%3 x %4 mm)")
		.arg( (int)DisplayWidth(dpy,scr) )
		.arg( (int)DisplayHeight(dpy,scr) )
		.arg( (int)DisplayWidthMM(dpy,scr) )
		.arg( (int)DisplayHeightMM (dpy,scr) ));

    last = new TQListViewItem(item, last, i18n("Resolution"),
		i18n("%1 x %2 dpi")
		.arg( (int)(xres+0.5) )
		.arg( (int)(yres+0.5) ));

    ndepths = 0;
    depths  = 0;
    depths = XListDepths (dpy, scr, &ndepths);
    if (depths) {
	TQString txt;

        for (i = 0; i < ndepths; i++) {
            txt = txt + Value(depths[i]);
            if (i < ndepths - 1)
                txt = txt + TQString::fromLatin1(", ");
        }

        last = new TQListViewItem(item, last, i18n("Depths (%1)").arg(ndepths,-1), txt);
        XFree((char *) depths);
    }

    last = new TQListViewItem(item, last, i18n("Root Window ID"),
		HexStr((unsigned long)RootWindow(dpy,scr),HEXDIGITS));
    last = new TQListViewItem(item, last, i18n("Depth of Root Window"),
		(DisplayPlanes (dpy, scr) == 1)
		?	i18n("%1 plane").arg(DisplayPlanes(dpy,scr))   /*singular*/
		:	i18n("%1 planes").arg(DisplayPlanes(dpy,scr)));/*plural*/
    last = new TQListViewItem(item, last, i18n("Number of Colormaps"),
		i18n("minimum %1, maximum %2")
		    .arg((int)MinCmapsOfScreen(s)).arg((int)MaxCmapsOfScreen(s)));
    last = new TQListViewItem(item, last, i18n("Default Colormap"),
		Value((int)DefaultColormap(dpy,scr)));
    last = new TQListViewItem(item, last, i18n("Default Number of Colormap Cells"),
		Value((int)DisplayCells(dpy, scr)));
    last = new TQListViewItem(item, last, i18n("Preallocated Pixels"),
		i18n("Black %1, White %2")
		.arg(TDEGlobal::locale()->formatNumber(BlackPixel(dpy,scr), 0))
		.arg(TDEGlobal::locale()->formatNumber(WhitePixel(dpy,scr), 0)));

    TQString YES(i18n("Yes"));
    TQString NO(i18n("No"));
    last = new TQListViewItem(item, last, i18n("Options"),
		i18n("backing-store: %1, save-unders: %2")
		.arg( (DoesBackingStore(s) == NotUseful) ? NO :
		      ((DoesBackingStore(s) == Always) ? YES : i18n("When mapped")) )
		.arg(  DoesSaveUnders(s) ? YES : NO ));

    XQueryBestSize (dpy, CursorShape, RootWindow(dpy,scr), 65535, 65535,
		    &width, &height);
    last = new TQListViewItem(item, last, i18n("Largest Cursor"),
		(width == 65535 && height == 65535)
		? i18n("unlimited") : TQString::fromLatin1("%1 x %2").arg(width).arg(height));

    last = new TQListViewItem(item, last, i18n("Current Input Event Mask"),
		HexStr((unsigned long)EventMaskOfScreen(s),HEXDIGITS));
    item = last;
    struct _event_table *etp;
    for (etp=event_table; etp->name; etp++) {
	if (EventMaskOfScreen(s) & etp->value)
	    item = new TQListViewItem(last, item,
		i18n("Event = %1").arg(HexStr(etp->value,HEXDIGITS)),
		etp->name );
    }

    return item;
}

static const TQString Order( int order )
{
    if (order==LSBFirst) return i18n("LSBFirst"); else
    if (order==MSBFirst) return i18n("MSBFirst"); else
	return i18n("Unknown Order %1").arg(order);
}

static const TQString BitString( unsigned long n )
{
    return i18n("1 Bit", "%n Bits", n); // singular & plural form of "%d Bit"
}

static const TQString ByteString( unsigned long n )
{
    /* explanation in BR #52640 (http://bugs.kde.org/show_bug.cgi?id=52640) */
    if (n == 1)
	return i18n("1 Byte"); // singular form: "1 Byte" (yes, it's "1", not "%1"!)

    return i18n("%1 Bytes")  // plural form: "%1 Bytes"
		.arg(TDEGlobal::locale()->formatNumber(n,0));
}

static bool GetInfo_XServer_Generic( TQListView *lBox )
{
    /* Many parts of this source are taken from the X11-program "xdpyinfo" */

    int i,n;
    long req_size;

    Display *dpy;
    XPixmapFormatValues *pmf;

    TQString str,txt;
    TQListViewItem *last, *item, *next;

    dpy = XOpenDisplay(0);
    if (!dpy)
        return false;

    lBox->addColumn(i18n("Information") );
    lBox->addColumn(i18n("Value") );
    sorting_allowed = false;

    next = new TQListViewItem(lBox, i18n("Server Information"));
    next->setPixmap(0, SmallIcon("kcmx"));
    next->setOpen(true);
    next->setSelectable(false);
    next->setExpandable(false);

    last = new TQListViewItem(next, i18n("Name of the Display"),
		DisplayString(dpy));

    last = new TQListViewItem(next, last, i18n("Vendor String"), TQString::fromLatin1(ServerVendor(dpy)));
    last = new TQListViewItem(next, last, i18n("Vendor Release Number"),
		Value((int)VendorRelease(dpy)));

    last = new TQListViewItem(next, last, i18n("Version Number"),
		TQString::fromLatin1("%1.%2").arg((int)ProtocolVersion(dpy))
    		                  .arg((int)ProtocolRevision(dpy)));

    last = item = new TQListViewItem(next, last, i18n("Available Screens"));
    last->setOpen(true);
    last->setExpandable(true);
    for (i=0; i<ScreenCount(dpy); i++) {
        item = XServer_fill_screen_info(last, item, dpy, i, (int)DefaultScreen(dpy));
	if (i==0) item->setOpen(true);
    }

    last = new TQListViewItem( next, last, i18n("Supported Extensions") );
    item = last;

    int extCount;
    char **extensions = XListExtensions( dpy, &extCount );
    for ( i = 0; i < extCount; i++ ) {
       item = new TQListViewItem( last, item, TQString::fromLatin1( extensions[i] ) );
    }
    XFreeExtensionList( extensions );

    pmf = XListPixmapFormats(dpy, &n);
    last = item = new TQListViewItem(next, last, i18n("Supported Pixmap Formats"));
    if (pmf) {
	last->setExpandable(true);
	for (i=0; i<n; i++) {
	    item = new TQListViewItem(last, item,
			i18n("Pixmap Format #%1").arg(i+1),
			i18n("%1 BPP, Depth: %2, Scanline padding: %3")
				.arg(pmf[i].bits_per_pixel)
				.arg(BitString(pmf[i].depth))
				.arg(BitString(pmf[i].scanline_pad)));
	}
	XFree ((char *)pmf);
    }

    req_size = XExtendedMaxRequestSize(dpy);
    if (!req_size) req_size = XMaxRequestSize(dpy);
    last = new TQListViewItem(next, last, i18n("Maximum Request Size"),
		ByteString(req_size*4));
    last = new TQListViewItem(next, last, i18n("Motion Buffer Size"),
		ByteString(XDisplayMotionBufferSize(dpy)));

    last = item = new TQListViewItem(next, last, i18n("Bitmap"));
    last->setExpandable(true);
    item = new TQListViewItem(last, item, i18n("Unit"),
		Value(BitmapUnit(dpy)) );
    item = new TQListViewItem(last, item, i18n("Order"),
		Order(BitmapBitOrder(dpy)));
    item = new TQListViewItem(last, item, i18n("Padding"),
		Value(BitmapPad(dpy)));

    last = new TQListViewItem(next, last, i18n("Image Byte Order"),
		Order(ImageByteOrder(dpy)));

    XCloseDisplay (dpy);
    return true;
}



/*
***************************************************************************
***************************************************************************
***************************************************************************
*/



void KInfoListWidget::load()
{
    bool ok = false;

    lBox->clear();

    /*  Delete the user-visible ErrorString, before calling the
        retrieve-function. If the function wants the widget to show
        another string, then it change *GetInfo_ErrorString ! */
    ErrorString = i18n("No information available about %1.").arg(title)
		    + TQString::fromLatin1("\n\n") + DEFAULT_ERRORSTRING;
    GetInfo_ErrorString = &ErrorString;  /* save the address of ErrorString */

    sorting_allowed = true; 	/* the functions may set that */
    lBox->setSorting(-1);   	/* No Sorting per default */

    if (getlistbox)
        ok = (*getlistbox)(lBox);	/* retrieve the information */

    if (lBox->header()->count()<=1)
        lBox->addColumn(title);	/* set default title */

    /* is the user allowed to use sorting ? */
    lBox->header()->setClickEnabled(sorting_allowed);
    lBox->header()->setFont(TDEGlobalSettings::generalFont());
    lBox->setShowSortIndicator(sorting_allowed);

    if (ok)
    {
        widgetStack->raiseWidget(lBox);
    }
    else
    {
        NoInfoText->setText(ErrorString);
        widgetStack->raiseWidget(NoInfoText);
    }

    emit changed(false);
}


TQString KInfoListWidget::quickHelp() const
{
  return i18n("<h1>System Information</h1>"
	" All the information modules return information about a certain"
	" aspect of your computer hardware or your operating system."
	" Not all modules are available on all hardware architectures"
	" and/or operating systems." );
}


KInfoListWidget::KInfoListWidget(const TQString &_title, TQWidget *parent, const char *name,
                                 bool _getlistbox(TQListView *lbox))
    : TDECModule(parent, name),
      title(_title)
{
    TDEAboutData *about =
    new TDEAboutData(I18N_NOOP("kcminfo"),
		I18N_NOOP("TDE Panel System Information Control Module"),
		0, 0, TDEAboutData::License_GPL,
		I18N_NOOP("(c) 1998 - 2002 Helge Deller"));

    about->addAuthor("Helge Deller", 0, "deller@kde.org");
    setAboutData( about );

    setButtons(TDECModule::Help);
    getlistbox 	= _getlistbox;
    GetInfo_ErrorString = 0;
    TQHBoxLayout *layout = new TQHBoxLayout(this, 0, KDialog::spacingHint());
    widgetStack = new TQWidgetStack(this);
    layout->addWidget(widgetStack);
    lBox 	= new TQListView(widgetStack);
    widgetStack->addWidget(lBox, 0);
    lBox->setMinimumSize(200,120);
    lBox->setFont(TDEGlobalSettings::generalFont()); /* default font */
    lBox->setAllColumnsShowFocus(true);
    TQWhatsThis::add( lBox, i18n( "This list displays system information on the selected category." ) );
    NoInfoText  = new TQLabel(widgetStack);
    widgetStack->addWidget(NoInfoText, 1);
    NoInfoText->setAlignment(AlignCenter | WordBreak);
    widgetStack->raiseWidget(NoInfoText);
    load();
}



/* Helper-function to read output from an external program */
static int GetInfo_ReadfromPipe( TQListView *lBox, const char *FileName, bool WithEmptyLines = true )
{
    FILE *pipe;
    TQListViewItem* olditem = 0L;
    TQString s;

    if ((pipe = popen(FileName, "r")) == NULL) {
	pclose(pipe);
	return 0;
    }

    TQTextStream t(pipe, IO_ReadOnly);

    while (!t.atEnd()) {
	s = t.readLine();
	if (!WithEmptyLines && s.length()==0)
		continue;
       	olditem = new TQListViewItem(lBox, olditem, s);
    }

    pclose(pipe);

    return (lBox->childCount());
}

/*
***************************************************************************
**  Include system-specific code					 **
***************************************************************************
*/

#ifdef __linux__
#include "info_linux.cpp"
#elif defined(sgi) && sgi
#include "info_sgi.cpp"
#elif defined(__FreeBSD__) || defined (__DragonFly__)
#include "info_fbsd.cpp"
#elif __hpux
#include "info_hpux.cpp"
#elif __NetBSD__
#include "info_netbsd.cpp"
#elif __OpenBSD__
#include "info_openbsd.cpp"
#elif defined(__svr4__) && defined(sun)
#include "info_solaris.cpp"
#elif __svr4__
#include "info_svr4.cpp"
#elif _AIX
#include "info_aix.cpp"
#elif defined(__APPLE__)
#include "info_osx.cpp"
#else
#include "info_generic.cpp"	/* Default for unsupportet systems.... */
#endif

/*
***************************************************************************
**  End of: Include system-specific code				 **
***************************************************************************
*/
