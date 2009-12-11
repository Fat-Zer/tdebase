/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <qpixmap.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <kpushbutton.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <kguiitem.h>
#include <qtoolbutton.h>

class QPushButton;
class QVButtonGroup;
class QPopupMenu;
class QTimer;
class QPainter;
class QString;
class KAction;


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
    QPixmap m_root;
    void fadeBack( void );
    float  m_grayOpacity;
    float  m_compensation;
    bool   m_fadeBackwards;
    bool   m_readDelayComplete;
    QImage m_unfadedImage;
    QImage m_grayImage;
    QTime  m_fadeTime;
    int    m_rowsDone;
    KPixmapIO m_pmio;

};


// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown( bool maysd, KApplication::ShutdownType& sdtype, QString& bopt );

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
    KSMShutdownDlg( QWidget* parent, bool maysd, KApplication::ShutdownType sdtype );
    KApplication::ShutdownType m_shutdownType;
    QString m_bootOption;
    QPopupMenu *targets;
    QStringList rebootOptions;
    LibHalContext* m_halCtx;
    DBusConnection *m_dbusConn;
    bool m_lockOnResume;
};

class KSMDelayedPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMDelayedPushButton( const KGuiItem &item, QWidget *parent, const char *name = 0 );
  void setPopup( QPopupMenu *pop);

private slots:
  void slotTimeout();
  void slotPressed();
  void slotReleased();

private:
  QPopupMenu *pop;
  QTimer *popt;
};

class KSMPushButton : public KPushButton
{
  Q_OBJECT

public:

  KSMPushButton( const KGuiItem &item, QWidget *parent, const char *name = 0 );

protected:
  virtual void keyPressEvent(QKeyEvent*e);
  virtual void keyReleaseEvent(QKeyEvent*e);

private:

 bool m_pressed;

};



class FlatButton : public QToolButton
{
  Q_OBJECT

 public:

  FlatButton( QWidget *parent = 0, const char *name = 0 );
  ~FlatButton();

 protected:
  virtual void keyPressEvent(QKeyEvent*e);
  virtual void keyReleaseEvent(QKeyEvent*e);

 private slots:
  
 private:
  void init();
  
  bool m_pressed;
  QString m_text;
  QPixmap m_pixmap;
 
};




#endif
