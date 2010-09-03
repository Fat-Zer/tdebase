/*****************************************************************

   Copyright (c) 2006 Stephan Binner <binner@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

******************************************************************/

#ifndef QUERY_H
#define QUERY_H

#include <tqstringlist.h>
#include <tqptrlist.h>

class Alternative
{
public:
    TQStringList includes;
    TQStringList excludes;
};

class Query
{
  public:
    Query();
    void clear();
    void set(const TQString &);
    TQString get() const;
    bool matches(const TQString &);

  private:
    TQString query_term;
    TQPtrList<Alternative> alternatives;

    void add_term();
    TQString current_part;
    Alternative *current_alternative;
    bool within_quotes;
    bool exclude_part;
};

#endif
