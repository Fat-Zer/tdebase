/***************************************************************************
                          kfindlanguage.h  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2002 by Carsten Wolff
    email                : wolff@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDLANGUAGE_H
#define KFINDLANGUAGE_H

class KFindLanguage {
public:
	KFindLanguage();
	~KFindLanguage();
	TQStringList getLangList() const;
	TQMap<TQString,TQString> getLangMap() const;
	TQString getBestLang() const;
	TQString getOldLang() const;
	TQString getCountry() const;
private:
	TQStringList m_langlist;          // stores tags like "en_US"
	TQMap<TQString,TQString> m_langmap; // stores tag -> name pairs
	TQString m_country;
	TQString m_oldlang;
	TQString m_bestlang;
};

#endif
