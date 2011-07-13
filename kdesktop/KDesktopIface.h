
#ifndef __KDesktopIface_h__
#define __KDesktopIface_h__

#include <tqstringlist.h>
#include <dcopobject.h>
#include <dcopref.h>

class KDesktopIface : public DCOPObject
{
    K_DCOP
public:

    KDesktopIface() : DCOPObject("KDesktopIface") {}

k_dcop:
    /**
     * @internal
     */
    virtual void runAutoStart() = 0;

    /**
     * Re-arrange the desktop icons.
     */
    virtual void rearrangeIcons() = 0;
    /**
     * @deprecated
     */
    void rearrangeIcons( bool ) { rearrangeIcons(); }
    /**
     * Lineup the desktop icons.
     */
    virtual void lineupIcons() = 0;
    /**
     * Select all icons
     */
    virtual void selectAll() = 0;
    /**
     * Unselect all icons
     */
    virtual void unselectAll() = 0;
    /**
     * Refresh all icons
     */
    virtual void refreshIcons() = 0;
    /**
     * Set show desktop state
     */
    virtual void setShowDesktop( bool b ) = 0;
    /**
     * @return the show desktop state
     */
    virtual bool showDesktopState() = 0;
    /**
     * Toggle show desktop state
     */
    virtual void toggleShowDesktop() = 0;
    /**
     * @return the urls of selected icons
     */
    virtual TQStringList selectedURLs() = 0;

    /**
     * Re-read KDesktop's configuration
     */
    virtual void configure() = 0;
    /**
     * Display the "Run Command" dialog (minicli)
     */
    virtual void popupExecuteCommand() = 0;
    /**
     * Display the "Run Command" dialog (minicli) and prefill
     * @since 3.4
     */
    virtual void popupExecuteCommand(const TQString& command) = 0;
    /**
     * Get the background dcop interface (KBackgroundIface)
     */
    DCOPRef background() { return DCOPRef( "kdesktop", "KBackgroundIface" ); }
    /**
     * Get the screensaver dcop interface (KScreensaverIface)
     */
    DCOPRef screenSaver() { return DCOPRef( "kdesktop", "KScreensaverIface" ); }
    /**
     * Full refresh
     */
    virtual void refresh() = 0;
    /**
     * Bye bye
     */
    virtual void logout() = 0;
    /**
     * Returns whether KDesktop uses a virtual root.
     */
    virtual bool isVRoot() = 0;
    /**
     * Set whether KDesktop should use a virtual root.
     */
    virtual void setVRoot( bool enable )= 0;
    /**
     * Clears the command history and completion items
     */
    virtual void clearCommandHistory() = 0;
    /**
     * Returns whether icons are enabled on the desktop
     */
    virtual bool isIconsEnabled() = 0;
    /**
     * Disable icons on the desktop.
     */
    virtual void setIconsEnabled( bool enable )= 0;

    /**
     * Should be called by any application that wants to tell KDesktop
     * to switch desktops e.g.  the minipager applet on kicker.
     */
    virtual void switchDesktops( int delta ) = 0;

    /**
     * slot for kicker; called when the number or size of panels change the available
     * space for desktop icons
     */
    virtual void desktopIconsAreaChanged(const TQRect &area, int screen) = 0;

    /**
     * Find the next free place for a not yet existing icon, so it fits
     * in the user arrangement. Basicly prepare for icons to be moved in.
     * It will try to find a place in the virtual grid near col,row
     * where no other icon is.
     *
     * If you specify -1 for row or column, it will try to find the next
     * free room where no other icon follows. E.g. if you specify column
     * = -1 and row = 0, kdesktop will find the next vertical placement
     * so that the icon appears at the end of the existing icons preferable
     * in the first column. If the first column is full, it will find the
     * next free room in the second column.
     *
     * If you specify both column and row, kdesktop won't care for aligning,
     * or surrounding icons, but try to find the free place near the given
     * grid place (e.g. specify 0,0 to find the nearest place in the left
     * upper corner).
     */
    virtual TQPoint findPlaceForIcon( int column, int row) = 0;

    /// copy the desktop file in the Desktop and place it at x, y
    virtual void addIcon(const TQString &url, int x, int y) = 0;

    /// same with specific destination
    virtual void addIcon(const TQString &url, const TQString &dest, int x, int y) = 0;

    /// remove the desktop file (either full path or relative)
    virtual void removeIcon(const TQString &dest) = 0;
};

#endif

