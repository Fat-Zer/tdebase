//
//  Copyright (C) 2004 Stephan Binner <binner@kde.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the7 implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <kprogress.h>
#include "progressdialogiface.h"

class ProgressDialog : public KProgressDialog, virtual public ProgressDialogIface
{
    Q_OBJECT

    public:
      ProgressDialog(TQWidget* parent = 0, const TQString& caption = TQString::null, const TQString& text = TQString::null, int totalSteps = 100);
      
      void setTotalSteps( int );
      int totalSteps() const;
    
      void setProgress( int );
      int progress() const;
      
      void setLabel(const TQString&);
    
      void showCancelButton(bool show);
      bool wasCancelled() const;
      void ignoreCancel();

      void setAutoClose( bool );
      bool autoClose() const;
                  
      void close();
};

#endif // PROGRESSDIALOG_H
