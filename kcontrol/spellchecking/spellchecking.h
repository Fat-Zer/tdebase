/*
  Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef __spellchecking_h__
#define __spellchecking_h__ 

#include "tdecmodule.h"

class KSpellConfig;

class KSpellCheckingConfig  : public TDECModule
{
  Q_OBJECT

 public:
  KSpellCheckingConfig(TQWidget *parent, const char *name, const TQStringList &);

  void load();
  void save();
  void defaults();

 protected:
  KSpellConfig *spellConfig;
};

#endif
