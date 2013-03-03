/**
  * kprivacymanager.h
  *
  * Copyright (c) 2003 Ralf Hoelzer <ralf@well.com>
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU Lesser General Public License as published
  *  by the Free Software Foundation; either version 2.1 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU Lesser General Public License for more details.
  *
  *  You should have received a copy of the GNU Lesser General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  */

#ifndef KPRIVACYMANAGER_H
#define KPRIVACYMANAGER_H

#include <tqobject.h>

/**
@author Ralf Hoelzer
*/

class KPrivacyManager : public TQObject
{
Q_OBJECT
public:
    KPrivacyManager();

    ~KPrivacyManager();

    bool clearAllCookies() const;
    bool clearSavedClipboardContents();
    bool clearThumbnails();
    bool clearRunCommandHistory() const;
    bool clearFormCompletion() const;
    bool clearWebHistory();
    bool clearWebCache() const;
    bool clearQuickStartMenu() const;
    bool clearRecentDocuments() const;
    bool clearFavIcons();
    bool clearKPDFDocData();

private:
    bool isApplicationRegistered(const TQString &appName);
    bool m_error;

};

#endif
