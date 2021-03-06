/*
 *  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _KCM_PERF_KONQUEROR_H
#define _KCM_PERF_KONQUEROR_H

#include "konqueror_ui.h"

namespace KCMPerformance
{

class Konqueror
    : public Konqueror_ui
    {
    Q_OBJECT
    public:
        Konqueror( TQWidget* parent_P = NULL );
        void load(bool useDefaults);
        void save();
        void defaults();
    signals:
        void changed();
    private slots:
	void preload_count_changed( int );
    private:
        TQString allowed_parts;
    };

}  // namespace 

#endif
