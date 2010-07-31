/**
 * kcmaccess.h
 *
 * Copyright (c) 2000 Matthias H�zer-Klpfel <hoelzer@kde.org>
 *
 */

#ifndef __kcmaccess_h__
#define __kcmaccess_h__


#include <kcmodule.h>
#include <knuminput.h>


class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class KColorButton;
class QSlider;
class KNumInput;
class KAboutData;

class ExtendedIntNumInput : public KIntNumInput
{
	Q_OBJECT

	public:
    /**
	  * Constructs an input control for integer values
	  * with base 10 and initial value 0.
	  */
		ExtendedIntNumInput(TQWidget *parent=0, const char *name=0);
		
    /**
	  * Destructor
	  */
		virtual ~ExtendedIntNumInput();

    /**
	  * @param min  minimum value
	  * @param max  maximum value
	  * @param step step size for the QSlider
	  * @param slider whether the slider is created or not
	  */
		void setRange(int min, int max, int step=1, bool slider=true);

	private slots:
		void slotSpinValueChanged(int);
		void slotSliderValueChanged(int);
	
	private:
		int min, max;
		int sliderMax;
};

class KAccessConfig : public KCModule
{
  Q_OBJECT

public:

  KAccessConfig(TQWidget *parent = 0L, const char *name = 0L);
  virtual ~KAccessConfig();
  
  void load();
  void load(bool useDefaults);
  void save();
  void defaults();

protected slots:

  void configChanged();
  void checkAccess();
  void invertClicked();
  void flashClicked();
  void selectSound();
  void changeFlashScreenColor();
  void configureKNotify();

private:

  TQCheckBox *systemBell, *customBell, *visibleBell;
  TQRadioButton *invertScreen, *flashScreen;
  TQLabel    *soundLabel, *colorLabel;
  TQLineEdit *soundEdit;
  TQPushButton *soundButton;
  KColorButton *colorButton;
  ExtendedIntNumInput *durationSlider;

  TQCheckBox *stickyKeys, *stickyKeysLock, *stickyKeysAutoOff;
  TQCheckBox *stickyKeysBeep, *toggleKeysBeep, *kNotifyModifiers;
  TQPushButton *kNotifyModifiersButton;

  TQCheckBox *slowKeys, *bounceKeys;
  ExtendedIntNumInput *slowKeysDelay, *bounceKeysDelay;
  TQCheckBox *slowKeysPressBeep, *slowKeysAcceptBeep;
  TQCheckBox *slowKeysRejectBeep, *bounceKeysRejectBeep;

  TQCheckBox *gestures, *gestureConfirmation;
  TQCheckBox *timeout;
  KIntNumInput *timeoutDelay;
  TQCheckBox *accessxBeep, *kNotifyAccessX;
  TQPushButton *kNotifyAccessXButton;
};


#endif
