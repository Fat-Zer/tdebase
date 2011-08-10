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

#include "query.h"
#include <kdebug.h>

Query::Query()
{
   alternatives.setAutoDelete(true);
}

void Query::clear()
{
   query_term = TQString::null;
   alternatives.clear();
}

void Query::set(const TQString &term)
{
   query_term = term;
   alternatives.clear();

   current_alternative = new Alternative;
   current_part = TQString::null;
   within_quotes = false;
   exclude_part = false;

   for (uint index=0;index<term.length();index++) {
      if (current_part.isEmpty() && query_term[index]=='-')
         exclude_part = true;
      else if (term[index]=='\'' || term[index]=='"') {
         if (within_quotes)
            add_term();
         else
            within_quotes = true;
      }
      else if (!within_quotes && query_term[index]==' ')
         add_term();
      else if (!exclude_part && !within_quotes && query_term[index]=='O' && index+1<term.length() && query_term[index+1]=='R') {
         index++;
         alternatives.append(current_alternative);
         current_alternative = new Alternative;
         within_quotes = false;
         exclude_part = false;
         current_part = TQString::null;
     }
     else
        current_part+=term[index];
   }
   add_term();
   alternatives.append(current_alternative);

#if 0
   for (Alternative* alt=alternatives.first(); alt; alt=alternatives.next()) {
      kdDebug() << "---" << endl;
      kdDebug() << "*** includes = " << alt->includes << endl;
      kdDebug() << "*** excludes = " << alt->excludes << endl;
   }
#endif
}

void Query::add_term() {
   if (!current_part.isEmpty()) {
      if (current_part.startsWith("*"))
         current_part=current_part.mid(1);

      if (current_part.endsWith("*"))
         current_part=current_part.mid(0,current_part.length()-1);

      if (exclude_part)
         current_alternative->excludes+=current_part.lower();
      else
         current_alternative->includes+=current_part.lower();
   }
   within_quotes = false;
   exclude_part = false;
   current_part = TQString::null;
}

TQString Query::get() const
{
   return query_term;
}

bool Query::matches(const TQString &term)
{
   TQString lower_term = term.lower();

   for (Alternative* alt=alternatives.first(); alt; alt=alternatives.next()) {
      if (!alt->includes.count())
         continue;

      bool next_alternative = false;

      for ( TQStringList::ConstIterator it = alt->excludes.begin(); it != alt->excludes.end(); ++it ) {
         if ( lower_term.find(*it)!=-1 ) {
            next_alternative = true;
            continue;
         }
      }
      if (next_alternative)
         continue;

      for ( TQStringList::ConstIterator it = alt->includes.begin(); it != alt->includes.end(); ++it ) {
         if ( lower_term.find(*it)==-1 ) {
            next_alternative = true;
            continue;
         }
      }
      if (next_alternative)
         continue;

//kdDebug() << "Found hit in '" << term << "'" << endl;
      return true;
   }

   return false;
}
