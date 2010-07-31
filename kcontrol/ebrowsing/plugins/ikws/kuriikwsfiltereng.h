/*  This file is part of the KDE project

    Copyright (C) 2002,2003 Dawit Alemayehu <adawit@kde.org>
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 1999 Yves Arrouye <yves@realnames.com>

    Advanced web shortcuts
    Copyright (C) 2001 Andreas Hochsteger <e9625392@student.tuwien.ac.at>

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

#ifndef __KURISearchFILTERENGINE_H__
#define __KURISearchFILTERENGINE_H__

#include <tqvaluelist.h>
#include <tqstringlist.h>

#include <kservice.h>

class KURL;


class KURISearchFilterEngine
{
public:
  typedef TQMap <TQString, TQString> SubstMap;
  
  KURISearchFilterEngine();
  ~KURISearchFilterEngine() {};

  TQCString name() const;
  
  TQString webShortcutQuery (const TQString&) const;
  
  TQString autoWebSearchQuery (const TQString&) const;
  
  bool verbose() const { return m_bVerbose; }

  void loadConfig();
  
  static KURISearchFilterEngine *self();

protected:
  TQString formatResult (const TQString& url, const TQString& cset1, const TQString& cset2,
                        const TQString& query, bool isMalformed) const;
  
  TQString formatResult (const TQString& url, const TQString& cset1, const TQString& cset2,
                        const TQString& query, bool isMalformed, SubstMap& map) const;

private:
  TQStringList modifySubstitutionMap (SubstMap& map, const TQString& query) const;  
  
  TQString substituteQuery (const TQString& url, SubstMap &map, 
                           const TQString& userquery, const int encodingMib) const;
  
  bool m_bVerbose;  
  bool m_bWebShortcutsEnabled;
  char m_cKeywordDelimiter;

  TQString m_defaultSearchEngine;
  static KURISearchFilterEngine *s_pSelf;
};

#endif
