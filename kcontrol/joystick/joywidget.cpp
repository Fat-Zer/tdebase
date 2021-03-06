/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
 *   m.koller@surfeu.at                                                    *
 *   This file is part of the Trinity Control Center Module for Joysticks  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "joywidget.h"
#include "joydevice.h"
#include "poswidget.h"
#include "caldialog.h"

#include <tqhbox.h>
#include <tqvbox.h>
#include <tqtable.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqlistbox.h>
#include <tqcheckbox.h>
#include <tqtimer.h>
#include <tqfontmetrics.h>
#include <tqpushbutton.h>

#include <tdelocale.h>
#include <kdialog.h>
#include <tdemessagebox.h>
#include <kiconloader.h>

//--------------------------------------------------------------
static TQString PRESSED = I18N_NOOP("PRESSED");
//--------------------------------------------------------------

JoyWidget::JoyWidget(TQWidget *parent, const char *name)
 : TQWidget(parent, name), idle(0), joydev(0)
{
  TQVBox *mainVbox = new TQVBox(parent);
  mainVbox->setSpacing(KDialog::spacingHint());

  // create area to show an icon + message if no joystick was detected
  {
    messageBox = new TQHBox(mainVbox);
    messageBox->setSpacing(KDialog::spacingHint());
    TQLabel *icon = new TQLabel(messageBox);
    icon->setPixmap(TDEGlobal::iconLoader()->loadIcon("messagebox_warning", TDEIcon::NoGroup,
                                                    TDEIcon::SizeMedium, TDEIcon::DefaultState, 0, true));
    icon->setFixedSize(icon->sizeHint());
    message = new TQLabel(messageBox);
    messageBox->hide();
  }

  TQHBox *devHbox = new TQHBox(mainVbox);
  new TQLabel(i18n("Device:"), devHbox);
  device = new TQComboBox(true, devHbox);
  device->setInsertionPolicy(TQComboBox::NoInsertion);
  connect(device, TQT_SIGNAL(activated(const TQString &)), this, TQT_SLOT(deviceChanged(const TQString &)));
  devHbox->setStretchFactor(device, 3);

  TQHBox *hbox = new TQHBox(mainVbox);
  hbox->setSpacing(KDialog::spacingHint());

  TQVBox *vboxLeft = new TQVBox(hbox);
  vboxLeft->setSpacing(KDialog::spacingHint());

  new TQLabel(i18n("Position:"), vboxLeft);
  xyPos = new PosWidget(vboxLeft);
  trace = new TQCheckBox(i18n("Show trace"), mainVbox);
  connect(trace, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(traceChanged(bool)));

  TQVBox *vboxMid = new TQVBox(hbox);
  vboxMid->setSpacing(KDialog::spacingHint());

  TQVBox *vboxRight = new TQVBox(hbox);
  vboxRight->setSpacing(KDialog::spacingHint());

  // calculate the column width we need
  TQFontMetrics fm(font());
  int colWidth = TQMAX(fm.width(PRESSED), fm.width("-32767")) + 10;  // -32767 largest string

  new TQLabel(i18n("Buttons:"), vboxMid);
  buttonTbl = new TQTable(0, 1, vboxMid);
  buttonTbl->setReadOnly(true);
  buttonTbl->horizontalHeader()->setLabel(0, i18n("State"));
  buttonTbl->horizontalHeader()->setClickEnabled(false);
  buttonTbl->horizontalHeader()->setResizeEnabled(false);
  buttonTbl->verticalHeader()->setClickEnabled(false);
  buttonTbl->verticalHeader()->setResizeEnabled(false);
  buttonTbl->setColumnWidth(0, colWidth);

  new TQLabel(i18n("Axes:"), vboxRight);
  axesTbl = new TQTable(0, 1, vboxRight);
  axesTbl->setReadOnly(true);
  axesTbl->horizontalHeader()->setLabel(0, i18n("Value"));
  axesTbl->horizontalHeader()->setClickEnabled(false);
  axesTbl->horizontalHeader()->setResizeEnabled(false);
  axesTbl->verticalHeader()->setClickEnabled(false);
  axesTbl->verticalHeader()->setResizeEnabled(false);
  axesTbl->setColumnWidth(0, colWidth);

  // calibrate button
  calibrate = new TQPushButton(i18n("Calibrate"), mainVbox);
  connect(calibrate, TQT_SIGNAL(clicked()), this, TQT_SLOT(calibrateDevice()));
  calibrate->setEnabled(false);

  // set up a timer for idle processing of joystick events
  idle = new TQTimer(this);
  connect(idle, TQT_SIGNAL(timeout()), this, TQT_SLOT(checkDevice()));

  // check which devicefiles we have
  init();

  vboxLeft->adjustSize();
  vboxMid->adjustSize();
  vboxRight->adjustSize();
  hbox->adjustSize();
  mainVbox->adjustSize();

  setMinimumSize(mainVbox->size());
}

//--------------------------------------------------------------

JoyWidget::~JoyWidget()
{
  delete joydev;
}

//--------------------------------------------------------------

void JoyWidget::init()
{
  // check which devicefiles we have
  int i;
  bool first = true;
  char dev[30];

  device->clear();
  buttonTbl->setNumRows(0);
  axesTbl->setNumRows(0);

  for (i = 0; i < 5; i++)  // check the first 5 devices
  {
    sprintf(dev, "/dev/js%d", i);  // first look in /dev
    JoyDevice *joy = new JoyDevice(dev);

    if ( joy->open() != JoyDevice::SUCCESS )
    {
      delete joy;
      sprintf(dev, "/dev/input/js%d", i);  // then look in /dev/input
      joy = new JoyDevice(dev);

      if ( joy->open() != JoyDevice::SUCCESS )
      {
        delete joy;
        continue;    // try next number
      }
    }

    // we found one

    device->insertItem(TQString("%1 (%2)").arg(joy->text()).arg(joy->device()));

    // display values for first device
    if ( first )
    {
      showDeviceProps(joy);  // this sets the joy object into this->joydev
      first = false;
    }
    else
      delete joy;
  }

  /* KDE 4: Remove this check(and i18n) when all KCM wrappers properly test modules */
  if ( device->count() == 0 )
  {
    messageBox->show();
    message->setText(TQString("<qt><b>%1</b></qt>").arg(
      i18n("No joystick device automatically found on this computer.<br>"
           "Checks were done in /dev/js[0-4] and /dev/input/js[0-4]<br>"
           "If you know that there is one attached, please enter the correct device file.")));
  }
}

//--------------------------------------------------------------

void JoyWidget::traceChanged(bool state)
{
  xyPos->showTrace(state);
}

//--------------------------------------------------------------

void JoyWidget::restoreCurrDev()
{
  if ( !joydev )  // no device open
  {
    device->setCurrentText("");
    calibrate->setEnabled(false);
  }
  else
  {
    // try to find the current open device in the combobox list
    TQListBoxItem *item;
    item = device->listBox()->findItem(joydev->device(), TQt::Contains);

    if ( !item )  // the current open device is one the user entered (not in the list)
      device->setCurrentText(joydev->device());
    else
      device->setCurrentText(item->text());
  }
}

//--------------------------------------------------------------

void JoyWidget::deviceChanged(const TQString &dev)
{
  // find "/dev" in given string
  int start, stop;
  TQString devName;

  if ( (start = dev.find("/dev")) == -1 )
  {
    KMessageBox::sorry(this,
      i18n("The given device name is invalid (does not contain /dev).\n"
           "Please select a device from the list or\n"
           "enter a device file, like /dev/js0."), i18n("Unknown Device"));

    restoreCurrDev();
    return;
  }

  if ( (stop = dev.find(")", start)) != -1 )  // seems to be text selected from our list
    devName = dev.mid(start, stop - start);
  else
    devName = dev.mid(start);

  if ( joydev && (devName == joydev->device()) ) return;  // user selected the current device; ignore it

  JoyDevice *joy = new JoyDevice(devName);
  JoyDevice::ErrorCode ret = joy->open();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joy->errText(ret), i18n("Device Error"));

    delete joy;
    restoreCurrDev();
    return;
  }

  showDeviceProps(joy);
}

//--------------------------------------------------------------

void JoyWidget::showDeviceProps(JoyDevice *joy)
{
  joydev = joy;

  buttonTbl->setNumRows(joydev->numButtons());

  axesTbl->setNumRows(joydev->numAxes());
  if ( joydev->numAxes() >= 2 )
  {
    axesTbl->verticalHeader()->setLabel(0, "1(x)");
    axesTbl->verticalHeader()->setLabel(1, "2(y)");
  }

  calibrate->setEnabled(true);
  idle->start(0);

  // make both tables use the same space for header; this looks nicer
  buttonTbl->setLeftMargin(TQMAX(buttonTbl->verticalHeader()->width(),
                                  axesTbl->verticalHeader()->width()));
  axesTbl->setLeftMargin(buttonTbl->verticalHeader()->width());
}

//--------------------------------------------------------------

void JoyWidget::checkDevice()
{
  if ( !joydev ) return;  // no open device yet

  JoyDevice::EventType type;
  int number, value;

  if ( !joydev->getEvent(type, number, value) )
    return;

  if ( type == JoyDevice::BUTTON )
  {
    if ( value == 0 )  // button release
      buttonTbl->setText(number, 0, "-");
    else
      buttonTbl->setText(number, 0, PRESSED);
  }

  if ( type == JoyDevice::AXIS )
  {
    if ( number == 0 ) // x-axis
      xyPos->changeX(value);

    if ( number == 1 ) // y-axis
      xyPos->changeY(value);

    axesTbl->setText(number, 0, TQString("%1").arg(int(value)));
  }
}

//--------------------------------------------------------------

void JoyWidget::calibrateDevice()
{
  if ( !joydev ) return;  // just to be save

  JoyDevice::ErrorCode ret = joydev->initCalibration();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
    return;
  }

  if ( KMessageBox::messageBox(this, KMessageBox::Information,
        i18n("<qt>Calibration is about to check the precision.<br><br>"
             "<b>Please move all axes to their center position and then "
             "do not touch the joystick anymore.</b><br><br>"
             "Click OK to start the calibration.</qt>"),
        i18n("Calibration"),
        KStdGuiItem::ok(), KStdGuiItem::cancel()) != KMessageBox::Ok )
    return;

  idle->stop();  // stop the joystick event getting; this must be done inside the calibrate dialog

  CalDialog dlg(this, joydev);
  dlg.calibrate();

  // user cancelled somewhere during calibration, therefore the device is in a bad state
  if ( dlg.result() == TQDialog::Rejected )
    joydev->restoreCorr();

  idle->start(0);  // continue with event getting
}

//--------------------------------------------------------------

void JoyWidget::resetCalibration()
{
  if ( !joydev ) return;  // just to be save

  JoyDevice::ErrorCode ret = joydev->restoreCorr();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
  }
  else
  {
    KMessageBox::information(this,
      i18n("Restored all calibration values for joystick device %1.").arg(joydev->device()),
      i18n("Calibration Success"));
  }
}

//--------------------------------------------------------------

#include "joywidget.moc"
