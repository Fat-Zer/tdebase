/* This file is part of the KDE project
   Copyright (C) 1998-2000 David Faure <faure@kde.org>
                 2003      Sven Leiber <s.leiber@web.de>

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

#ifndef __knewmenu_h
#define __knewmenu_h

#include <tqintdict.h>
#include <tqstringlist.h>

#include <tdeaction.h>
#include <kdialogbase.h>
#include <kurl.h>
#include <libkonq_export.h>

namespace TDEIO { class Job; }

class KDirWatch;
class KLineEdit;
class KURLRequester;
class TQPopupMenu;

/**
 * The 'New' submenu, both for the File menu and the RMB popup menu.
 * (The same instance can be used by both).
 * Fills it with 'Folder' and one item per Template.
 * For this you need to connect aboutToShow() of the File menu with slotCheckUpToDate()
 * and to call slotCheckUpToDate() before showing the RMB popupmenu.
 *
 * KNewMenu automatically updates the list of templates if templates are
 * added/updated/deleted.
 *
 * @author David Faure <faure@kde.org>
 * Ideas and code for the new template handling mechanism ('link' desktop files)
 * from Christoph Pickart <pickart@iam.uni-bonn.de>
 */
class LIBKONQ_EXPORT KNewMenu : public TDEActionMenu
{
  Q_OBJECT
public:

    /**
     * Constructor
     */
    KNewMenu( TDEActionCollection * _collec, const char *name=0L );
    KNewMenu( TDEActionCollection * _collec, TQWidget *parentWidget, const char *name=0L );
    virtual ~KNewMenu();

    /**
     * Set the files the popup is shown for
     * Call this before showing up the menu
     */
    void setPopupFiles(KURL::List & _files) {
        popupFiles = _files;
    }
    void setPopupFiles(const KURL & _file) {
        popupFiles.clear();
        popupFiles.append( _file );
    }

public slots:
    /**
     * Checks if updating the list is necessary
     * IMPORTANT : Call this in the slot for aboutToShow.
     */
    void slotCheckUpToDate( );

protected slots:
    /**
     * Called when New->Directory... is clicked
     */
    void slotNewDir();

    /**
     * Called when New->* is clicked
     */
    void slotNewFile();

    /**
     * Fills the templates list.
     */
    void slotFillTemplates();

    void slotResult( TDEIO::Job * );
    // Special case (filename conflict when creating a link=url file)
    void slotRenamed( TDEIO::Job *, const KURL&, const KURL& );

private:

    /**
     * Fills the menu from the templates list.
     */
    void fillMenu();

    /**
     * Opens the desktop files and completes the Entry list
     * Input: the entry list. Output: the entry list ;-)
     */
    void parseFiles();

    /**
     * Make the main menus on the startup.
     */
    void makeMenus();

    /**
     * For entryType
     * LINKTOTEMPLATE: a desktop file that points to a file or dir to copy
     * TEMPLATE: a real file to copy as is (the KDE-1.x solution)
     * SEPARATOR: to put a separator in the menu
     * 0 means: not parsed, i.e. we don't know
     */
    enum { LINKTOTEMPLATE = 1, TEMPLATE, SEPARATOR };

    struct Entry {
        TQString text;
        TQString filePath; // empty for SEPARATOR
        TQString templatePath; // same as filePath for TEMPLATE
        TQString icon;
        int entryType;
        TQString comment;
    };
    // NOTE: only filePath is known before we call parseFiles

    /**
     * List of all template files. It is important that they are in
     * the same order as the 'New' menu.
     */
    static TQValueList<Entry> * s_templatesList;

    class KNewMenuPrivate;
    KNewMenuPrivate* d;

    /**
     * Is increased when templatesList has been updated and
     * menu needs to be re-filled. Menus have their own version and compare it
     * to templatesVersion before showing up
     */
    static int s_templatesVersion;

    /**
     * Set back to false each time new templates are found,
     * and to true on the first call to parseFiles
     */
    static bool s_filesParsed;

    int menuItemsVersion;

    /**
     * When the user pressed the right mouse button over an URL a popup menu
     * is displayed. The URL belonging to this popup menu is stored here.
     */
    KURL::List popupFiles;

    /**
     * True when a desktop file with Type=URL is being copied
     */
    bool m_isURLDesktopFile;
    TQString m_linkURL; // the url to put in the file

    static KDirWatch * s_pDirWatch;
};

/**
 * @internal
 * Dialog to ask for a filename and a URL, when creating a link to a URL.
 * Basically a merge of KLineEditDlg and KURLRequesterDlg ;)
 * @author David Faure <faure@kde.org>
 */
class KURLDesktopFileDlg : public KDialogBase
{
    Q_OBJECT
public:
    KURLDesktopFileDlg( const TQString& textFileName, const TQString& textUrl );
    KURLDesktopFileDlg( const TQString& textFileName, const TQString& textUrl, TQWidget *parent );
    virtual ~KURLDesktopFileDlg() {}

    /**
     * @return the filename the user entered (no path)
     */
    TQString fileName() const;
    /**
     * @return the URL the user entered
     */
    TQString url() const;

protected slots:
    void slotClear();
    void slotNameTextChanged( const TQString& );
    void slotURLTextChanged( const TQString& );
private:
    void initDialog( const TQString& textFileName, const TQString& defaultName, const TQString& textUrl, const TQString& defaultUrl );

    /**
     * The line edit widget for the fileName
     */
    KLineEdit *m_leFileName;
    /**
     * The URL requester for the URL :)
     */
    KURLRequester *m_urlRequester;

    /**
     * True if the filename was manually edited.
     */
    bool m_fileNameEdited;
};

#endif
