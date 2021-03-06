/* This file is part of the KDE projects
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __konqdirpart_h
#define __konqdirpart_h

#include <tqstring.h>
#include <tdeparts/part.h>
#include <tdeparts/browserextension.h>
#include <tdefileitem.h>
#include <kdatastream.h>
#include <tdeio/global.h>
#include <libkonq_export.h>

class KDirLister;
namespace KParts { class BrowserExtension; }
class KonqPropsView;
class TQScrollView;
class TDEAction;
class TDEToggleAction;
class KonqDirPartBrowserExtension;

class LIBKONQ_EXPORT KonqDirPart: public KParts::ReadOnlyPart
{
    Q_OBJECT

    friend class KonqDirPartBrowserExtension;

public:
    KonqDirPart( TQObject *parent, const char *name );

    virtual ~KonqDirPart();

    /**
     * The derived part should call this in its constructor
     */
    void setBrowserExtension( KonqDirPartBrowserExtension * extension )
      { m_extension = extension; }

    KonqDirPartBrowserExtension * extension()
      { return m_extension; }

    /**
     * The derived part should call this in its constructor
     */
    void setDirLister( KDirLister* lister );
    // TODO KDE4 create the KDirLister here and simplify the parts?

    TQScrollView * scrollWidget();

    virtual void saveState( TQDataStream &stream );
    virtual void restoreState( TQDataStream &stream );

    /** Called when LMB'ing an item in a directory view.
     * @param fileItem must be set
     * @param widget is only set as parent pointer for dialog boxes */
    void lmbClicked( KFileItem * fileItem );

    /** Called when MMB'ing an item in a directory view.
     * @param fileItem if 0 it means we MMB'ed the background. */
    void mmbClicked( KFileItem * fileItem );

    void setNameFilter( const TQString & nameFilter ) { m_nameFilter = nameFilter; }

    TQString nameFilter() const { return m_nameFilter; }

    void setFilesToSelect( const TQStringList & filesToSelect ) { m_filesToSelect = filesToSelect; }

    /**
     * Sets per directory mime-type based filtering.
     *
     * This method causes only the items matching the mime-type given
     * by @p filters to be displayed. You can supply multiple mime-types
     * by separating them with a space, eg. "text/plain image/x-png".
     * To clear all the filters set for the current url simply call this
     * function with a null or empty argument.
     *
     * NOTE: the filter(s) specified here only apply to the current
     * directory as returned by @ref #url().
     *
     * @param filter mime-type(s) to filter directory by.
     */
    void setMimeFilter (const TQStringList& filters);

    /**
     * Completely clears the internally stored list of mime filters
     * set by call to @ref #setMimeFilter.
     */
    TQStringList mimeFilter() const;


    KonqPropsView * props() const { return m_pProps; }

    /**
     * "Cut" icons : disable those whose URL is in lst, enable the others
     */
    virtual void disableIcons( const KURL::List & lst ) = 0;

    /**
     * This class takes care of the counting of items, size etc. in the
     * current directory. Call this in slotClear.
     */
    void resetCount();

    /**
     * Update the counts for those new items
     */
    void newItems( const KFileItemList & entries );

    /**
     * Update the counts with this item being deleted
     */
    void deleteItem( KFileItem * fileItem );

    /**
     * Refresh the items
     */
    void refreshItems(const KFileItemList &entries);

    /**
     * Show the counts for the directory in the status bar
     */
    void emitTotalCount();

    // ##### TODO KDE 4: remove!
    /**
     * Show the counts for the list of items in the status bar.
     * If none are provided emitTotalCount() is called to display
     * the counts for the whole directory. However, that does not work
     * for a treeview.
     * 
     * @deprecated
     */
    void emitCounts( const KFileItemList & lst, bool selectionChanged );
    
    /**
     * Show the counts for the list of items in the status bar. The list
     * can be empty.
     * 
     * @param lst the list of fileitems for which to display the counts
     * @since 3.4
     */
    void emitCounts( const KFileItemList & lst );

    void emitMouseOver( const KFileItem * item );

    /**
     * Enables or disables the paste action. This depends both on
     * the data in the clipboard and the number of files selected
     * (pasting is only possible if not more than one file is selected).
     */
    void updatePasteAction();

    /**
     * Change the icon size of the view.
     * The view should call it initially.
     * The view should also reimplement it, to update the icons.
     */
    virtual void newIconSize( int size );

    /**
     * This is called by the actions that change the icon size.
     * It stores the new size and calls newIconSize.
     */
    void setIconSize( int size );

    /**
     * This is called by konqueror itself, when the "find" functionality is activated
     */
    void setFindPart( KParts::ReadOnlyPart * part );

    KParts::ReadOnlyPart * findPart() const { return m_findPart; }

    virtual const KFileItem * currentItem() = 0; // { return 0L; }

    virtual KFileItemList selectedFileItems() { return KFileItemList(); }

    /**
     * Re-implemented for internal reasons.  API is unaffected.  All inheriting
     * classes should re-implement @ref doCloseURL() instead instead of this one.
     */
    bool closeURL ();

signals:

    /**
     * Emitted whenever the current URL is about to be changed.
     */
    void aboutToOpenURL();

    /**
     * We emit this if we want a find part to be created for us.
     * This happens when restoring from history
     */
    void findOpen( KonqDirPart * );

    /**
     * We emit this _after_ a find part has been created for us.
     * This also happens initially.
     */
    void findOpened( KonqDirPart * );

    /**
     * We emit this to ask konq to close the find part
     */
    void findClosed( KonqDirPart * );

    /**
     * Emitted as the part is updated with new items.
     * Useful for informing plugins of changes in view.
     */
    void itemsAdded(const KFileItemList &);

    /**
     * Emitted as the part is updated with these items.
     * Useful for informing plugins of changes in view.
     */
    void itemRemoved(const KFileItem *);

    /**
     * Emitted when items need to be refreshed (for example when
     * a file is renamed)
     */
    void itemsRefresh(const KFileItemList &);

    /**
     * Emitted with the list of filtered-out items whenever
     * a mime-based filter(s) is set.
     */
    void itemsFilteredByMime( const KFileItemList& );

public slots:

    /**
     * Re-implemented for internal reasons.  API is unaffected.  All inheriting
     * classes should re-implement @ref doOpenURL() instead instead of this one.
     */
     bool openURL (const KURL&);

    /**
     * This is called either by the part's close button, or by the
     * dir part itself, if entering a directory. It deletes the find
     * part.
     */
    void slotFindClosed();

    /**
     * Start the animated "K" during kfindpart's file search
     */
    void slotStartAnimationSearching();

    /**
     * Start the animated "K" during kfindpart's file search
     */
    void slotStopAnimationSearching();

    void slotBackgroundSettings();

    /**
     * Called when the clipboard's data changes, to update the 'cut' icons
     * Call this when the directory's listing is finished, to draw icons as cut.
     */
    void slotClipboardDataChanged();

    void slotIncIconSize();
    void slotDecIconSize();

    void slotIconSizeToggled( bool );

    // slots connected to the directory lister - or to the kfind interface
    virtual void slotStarted() = 0;
    virtual void slotCanceled() = 0;
    virtual void slotCompleted() = 0;
    virtual void slotNewItems( const KFileItemList& ) = 0;
    virtual void slotDeleteItem( KFileItem * ) = 0;
    virtual void slotRefreshItems( const KFileItemList& ) = 0;
    virtual void slotClear() = 0;
    virtual void slotRedirection( const KURL & ) = 0;

private slots:
    void slotIconChanged(int group);
protected:
    /**
     * Invoked from openURL to enable childern classes to
     * handle open URL requests.
     */
    virtual bool doOpenURL( const KURL& ) = 0;
    virtual bool doCloseURL () = 0;

protected:

    TQString m_nameFilter;
    TQStringList m_filesToSelect;

    KonqPropsView * m_pProps;

    TDEAction *m_paIncIconSize;
    TDEAction *m_paDecIconSize;
    TDEToggleAction *m_paDefaultIcons;
    TDEToggleAction *m_paHugeIcons;
    TDEToggleAction *m_paLargeIcons;
    TDEToggleAction *m_paMediumIcons;
    TDEToggleAction *m_paSmallIcons;

    KParts::ReadOnlyPart * m_findPart;
    KonqDirPartBrowserExtension * m_extension;

    // Remove all those in KDE4
    int m_iIconSize[5];
    TDEIO::filesize_t m_lDirSize;
    uint m_lFileCount;
    uint m_lDirCount;

private:
    void saveFindState( TQDataStream& );
    void restoreFindState( TQDataStream& );

    void adjustIconSizes();

    class KonqDirPartPrivate;
    KonqDirPartPrivate* d;
};

class LIBKONQ_EXPORT KonqDirPartBrowserExtension : public KParts::BrowserExtension
{
public:
    KonqDirPartBrowserExtension( KonqDirPart* dirPart )
        : KParts::BrowserExtension( dirPart )
        , m_dirPart( dirPart )
    {}

    /**
     * This calls saveState in KonqDirPart, and also takes care of the "find part".
     *
     * If your KonqDirPart-derived class needs to save and restore state,
     * you should probably override KonqDirPart::saveState
     * and KonqDirPart::restoreState, not the following methods.
     */
    virtual void saveState( TQDataStream &stream );
    virtual void restoreState( TQDataStream &stream );

private:
    KonqDirPart* m_dirPart;
};

#endif
