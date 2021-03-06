/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

// This file exists for the convenience of other KDE headers. It may change between
// different versions of KDE, and may disappear altogether. Do NOT include this
// directly in your program. We mean it.


#ifndef __WNDSTATUS_H__
#define __WNDSTATUS_H__

#include <tqhbox.h>

class TQLabel;
class KProgress;

/** @short Window displaying status and progress bar. */
class WndStatus:
      public TQHBox
{
  Q_OBJECT
public:
  WndStatus( TQPalette,
             int, // Xinerama screen
             bool, // At top?
             bool, // Progress indicator visible?
             const TQFont&, // Status bar font
             const TQColor&, const TQColor &, // Foreground/Background color
             const TQString& // Icon
           );

public slots:
  void slotSetMessage( const TQString& );
  void slotUpdateProgress( int );
  void slotUpdateSteps( int );

protected:
  TQLabel *m_label;
  KProgress *m_progress;
};
#endif
