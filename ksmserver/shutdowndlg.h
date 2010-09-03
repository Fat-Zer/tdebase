/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <tqpixmap.h>
#include <tqimage.h>
#include <tqdatetime.h>
#include <tqdialog.h>
#include <kpushbutton.h>
#include <tqpushbutton.h>
#include <tqframe.h>
#include <kguiitem.h>
#include <tqtoolbutton.h>

class TQPushButton;
class TQVButtonGroup;
class TQPopupMenu;
class TQTimer;
class TQPainter;
class TQString;
class KAction;


#include "timed.h"
#include <kapplication.h>
#include <kpixmapio.h>

/* We acknowledge the the dbus API is unstable */
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/connection.h>
#include <libhal.h>

// The (singleton) widget that makes/fades the desktop gray.
class KSMShutdownFeedback : public QWidget
{
    Q_OBJECT

public:
    static void start() { s_pSelf = new KSMShutdownFeedback(); }
    static void stop() { if ( s_pSelf != 0L ) s_pSelf->fadeBack(); delete s_pSelf; s_pSelf = 0L; }
    static KSMShutdownFeedback * self() { return s_pSelf; }

protected:
    ~KSMShutdownFeedback() {}

private slots:
    void slotPaintEffect();

private:
    static KSMShutdownFeedback * s_pSelf;
    KSMShutdownFeedback();
    int m_currentY;
    TQPixmap m_root;
    void fadeBack( void );
    float  m_grayOpacity;
    float  m_compensation;
    bool   m_fadeBackwards;
    bool   m_readDelayComplete;
    TQImage m_unfadedImage;
    TQImage m_grayImage;
    TQTime  m_fadeTime;
    int    m_rowsDone;
    KPixmapIO m_pmio;

};


// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype, TQString& bopt );

public slots:
    void slotLogout();
    void slotHalt();
    void slotReboot();
    void slotReboot(int);
    void slotSuspend();
    void slotHibernate();

protected:
    ~KSMShutdownDlg();

private:
    KSMShutdownDlg( TQWidget* parent, bool maysd, KApplication::ShutdownType sdtype );
    KApplication::ShutdownType m_shutdownType;
    TQString m_bootOption;
    TQPopupMenu *targets;
    TQStringList rebootOptions;
    LibHalContext* m_halCtx;
    DBusConnection *m_dbusConn;
    bool m_lockOnResume;
};

class KSMDelayedPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMDelayedPushButton( const KGuiItem &item, TQWidget *parent, const char *name = 0 );
  void setPopup( TQPopupMenu *pop);

private slots:
  void slotTimeout();
  void slotPressed();
  void slotReleased();

private:
  TQPopupMenu *pop;
  TQTimer *popt;
};

class KSMPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMPushButton( const KGuiItem &item, TQWidget *parent, const char *name = 0 );

protected:
  virtual void keyPressEvent(TQKeyEvent*e);
  virtual void keyReleaseEvent(TQKeyEvent*e);

private:

 bool m_pressed;

};



class FlatButton : public QToolButton
{
  Q_OBJECT

 public:

  FlatButton( TQWidget *parent = 0, const char *name = 0 );
  ~FlatButton();

 protected:
  virtual void keyPressEvent(TQKeyEvent*e);
  virtual void keyReleaseEvent(TQKeyEvent*e);

 private slots:
  
 private:
  void init();
  
  bool m_pressed;
  TQString m_text;
  TQPixmap m_pixmap;
 
};




class TQLabel;

class  KSMDelayedMessageBox : public TimedLogoutDlg
{
    Q_OBJECT

public:
    KSMDelayedMessageBox( KApplication::ShutdownType sdtype, const TQString &bootOption, int confirmDelay );
    static bool showTicker( KApplication::ShutdownType sdtype, const TQString &bootOption, int confirmDelay );

protected slots:
    void updateText();

private:
    TQString m_template;
    int m_remaining;
};

#endif
