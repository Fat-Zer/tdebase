/**
 * Copyright (c) 2001 Dawit Alemayehu <adawit@kde.org>
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

#ifndef __UAPROVIDERDLG_H___
#define __UAPROVIDERDLG_H___


#include <kdialog.h>
#include <klineedit.h>


class FakeUASProvider;
class UAProviderDlgUI;

class UALineEdit : public KLineEdit
{
  Q_OBJECT

public:
  UALineEdit( TQWidget *parent, const char *name=0 );

protected:
  virtual void keyPressEvent( TQKeyEvent * );
};

class UAProviderDlg : public KDialog
{
  Q_OBJECT

public:
  UAProviderDlg( const TQString& caption, TQWidget *parent = 0,
                 FakeUASProvider* provider = 0, const char *name = 0 );
  ~UAProviderDlg();

  void setSiteName( const TQString& );
  void setIdentity( const TQString& );

  TQString siteName();
  TQString identity();
  TQString alias();

protected slots:
  void slotActivated( const TQString& );
  void slotTextChanged( const TQString& );

protected:
  void init();

private:
  FakeUASProvider* m_provider;
  UAProviderDlgUI* dlg;
};
#endif
