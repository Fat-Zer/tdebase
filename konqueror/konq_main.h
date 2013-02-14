#ifndef __konq_main_h
#define __konq_main_h

#include <tdeapplication.h>

// This is used to know if we are being closed by session management
// or by the user. See KonqMainWindow::~KonqMainWindow.
// Credits to Matthias Ettrich for the idea.
class KonquerorApplication : public TDEApplication
{
public:
  KonquerorApplication() : TDEApplication(),
      closed_by_sm( false ) {}

  bool closedByUser() const { return !closed_by_sm; }
  void commitData(TQSessionManager& sm) {
    closed_by_sm = true;
    TDEApplication::commitData( sm );
    closed_by_sm = false;
  }
 
private:
  bool closed_by_sm;
 
};

#endif
