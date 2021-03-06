/*
 * logitechmouse.cpp
 *
 * Copyright (C) 2004 Brad Hards <bradh@frogmouth.net>
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


#include <tqdialog.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <tqwidget.h>
#include <tqlayout.h>
#include <tqprogressbar.h>
#include <tqtimer.h>

#include <kdebug.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#include <config.h>

#ifdef HAVE_LIBUSB
#include <usb.h>

#include "logitechmouse.h"

LogitechMouse::LogitechMouse( struct usb_device *usbDev, int mouseCapabilityFlags, TQWidget* parent, const char* name )
    : LogitechMouseBase( parent, name, 0 )
{
    if ( !name )
        setName( "LogitechMouse" );

    cordlessNameLabel->setText( i18n("Mouse type: %1").arg( this->name() ) );

    m_mouseCapabilityFlags = mouseCapabilityFlags;

    m_usbDeviceHandle = usb_open( usbDev );

    if ( 0 == m_usbDeviceHandle ) {
        kdWarning() << "Error opening usbfs file: " << usb_strerror() << endl;
        return;
    }

    if ( mouseCapabilityFlags & USE_CH2 ) {
       m_useSecondChannel = 0x0100;
    } else {
        m_useSecondChannel = 0x0000;
    }

    permissionProblemText->hide();

    if ( mouseCapabilityFlags & HAS_RES ) {
        updateResolution();
        resolutionSelector->setEnabled( TRUE );

        connect( button400cpi, TQT_SIGNAL( clicked() ), parent, TQT_SLOT( changed() ) );
        connect( button800cpi, TQT_SIGNAL( clicked() ), parent, TQT_SLOT( changed() ) );

        if ( 4 == resolution() ) {
            button800cpi->setChecked( TRUE );
        } else if ( 3 == resolution() ) {
            button400cpi->setChecked( TRUE );
        } else {
            // it must have failed, try to help out
            resolutionSelector->setEnabled(FALSE);
            permissionProblemText->show();
        }
    }

    if ( mouseCapabilityFlags & HAS_CSR ) {

        initCordlessStatusReporting();

        // Do a name
        cordlessNameLabel->setText( i18n("Mouse type: %1").arg( cordlessName() ) );
        cordlessNameLabel->setEnabled( TRUE );

        // Display the battery power level - the level gets updated in updateGUI()
        batteryBox->setEnabled( TRUE );

        // Channel
        channelSelector->setEnabled( TRUE );
        // if the channel is changed, we need to turn off the timer, otherwise it
        // just resets the button to reflect the current status. The timer is
        // started again when we applyChanges()
        connect( channel1, TQT_SIGNAL( clicked() ), this, TQT_SLOT( stopTimerForNow() ) );
        connect( channel1, TQT_SIGNAL( clicked() ), parent, TQT_SLOT( changed() ) );
        if ( isDualChannelCapable() ) {
            channel2->setEnabled( TRUE );
            connect( channel2, TQT_SIGNAL( clicked() ), this, TQT_SLOT( stopTimerForNow() ) );
            connect( channel2, TQT_SIGNAL( clicked() ), parent, TQT_SLOT( changed() ) );
        }

        updateGUI();
    }

}

LogitechMouse::~LogitechMouse()
{
    if (m_usbDeviceHandle != 0) {
        usb_close( m_usbDeviceHandle );
    }
}

void LogitechMouse::initCordlessStatusReporting()
{
    updateCordlessStatus();
    doUpdate = new TQTimer( this ); // will be automatically deleted
    connect( doUpdate, TQT_SIGNAL( timeout() ), this, TQT_SLOT( updateGUI() ) );
    doUpdate->start( 20000 );
}

void LogitechMouse::updateCordlessStatus()
{
    TQByteArray status(8);

    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg(  m_usbDeviceHandle,
                                    USB_TYPE_VENDOR | USB_ENDPOINT_IN,0x09,
                                    (0x0003 | m_useSecondChannel),
                                    (0x0000 | m_useSecondChannel),
                                    status.data(),
                                    0x0008,
                                    1000)
                  : -1);

    if (0 > result) {
        // We probably have a permission problem
        channelSelector->setEnabled( FALSE );
        batteryBox->setEnabled( FALSE );
        cordlessNameLabel->hide();
        permissionProblemText->show();
    } else {
        // kdDebug() << "P6 (connect status): " << (status[0] & 0xFF) << endl;
        if ( status[0] & 0x20 ) { // mouse is talking
            m_connectStatus = ( status[0] & 0x80 );
            m_mousePowerup = ( status[0] & 0x40 );
            m_receiverUnlock = ( status[0] & 0x10 );
            m_waitLock = ( status[0] & 0x08 );
        }

        // kdDebug() << "P0 (receiver type): " << (status[1] & 0xFF) << endl;
        /*
         0x38 = pid C501
         0x39 = pid C502
         0x3B = pid C504
         0x3C = pid C508
         0x3D = pid C506
         0x3E = pid C505
        */

        m_cordlessNameIndex = (status[2] & 0xFF);

        m_batteryLevel = (status[3] & 0x07 );
        if ( status[3] & 0x08 ) {
            m_channel = 2;
        } else {
            m_channel = 1;
        }

        m_cordlessSecurity = ( ( status[4] ) & ( status[5] << 8 ) );

        m_caseShape = ( status[6] & 0x7F );

        // kdDebug() << "PB1 (device Capabilities): " << (status[7] & 0xFF) << endl;
        m_numberOfButtons = 2 + ( status[7] & 0x07 ); // 9 means something more than 8
        m_twoChannelCapable = ( status[7] & 0x40 );
        m_verticalRoller = ( status[7] & 0x08 );
        m_horizontalRoller = ( status[7] & 0x10 );
        m_has800cpi = ( status[7] & 0x20 );
    }

}

void LogitechMouse::updateGUI()
{
    updateCordlessStatus();

    batteryBar->setProgress( batteryLevel() );

    if ( isDualChannelCapable() ) {
        if ( 2 == channel() ) {
            channel2->setChecked( TRUE );
        } else if ( 1 == channel() ) {
            channel1->setChecked( TRUE );
        } // else it might have failed - we don't do anything
    }
}

void LogitechMouse::stopTimerForNow()
{
    doUpdate->stop();
}

void LogitechMouse::applyChanges()
{
    if ( m_mouseCapabilityFlags & HAS_RES ) {
        if ( ( resolution() == 4 ) && ( button400cpi->isChecked() ) ) {
            // then we are in 800cpi mode, but want 400cpi
            setLogitechTo400();
        } else if ( ( resolution() == 3 ) && (button800cpi->isChecked() ) ) {
            // then we are in 400 cpi mode, but want 800 cpi
            setLogitechTo800();
        }
    }

    if ( isDualChannelCapable() ) {
        if ( ( channel() == 2 ) && ( channel1->isChecked() ) ) {
           // we are on channel 2, but want channel 1
           setChannel1();
           KMessageBox::information(this, i18n("RF channel 1 has been set. Please press Connect button on mouse to re-establish link"), i18n("Press Connect Button") );
        } else if ( ( channel() == 1 ) && ( channel2->isChecked() ) ) {
            // we are on channel 1, but want channel 2
            setChannel2();
            KMessageBox::information(this, i18n("RF channel 2 has been set. Please press Connect button on mouse to re-establish link"), i18n("Press Connect Button") );
        }

        initCordlessStatusReporting();
    }
}

void LogitechMouse::save(TDEConfig * /*config*/)
{
    kdDebug() << "Logitech mouse settings not saved - not implemented yet" << endl;
}

TQ_UINT8 LogitechMouse::resolution()
{
    // kdDebug() << "resolution: " << m_resolution << endl;
    if ( 0 == m_resolution ) {
        updateResolution();
    }
    return m_resolution;
}

void LogitechMouse::updateResolution()
{
    char resolution;

    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR | USB_ENDPOINT_IN,
                                   0x01,
                                   0x000E,
                                   0x0000,
                                   &resolution,
                                   0x0001,
                                   100)
                  : -1);

    // kdDebug() << "resolution is: " << resolution << endl;
    if (0 > result) {
        kdWarning() << "Error getting resolution from device : " << usb_strerror() << endl;
        m_resolution = 0;
    } else {
        m_resolution = resolution;
    }
}

void LogitechMouse::setLogitechTo800()
{
    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg( m_usbDeviceHandle,
                                  USB_TYPE_VENDOR,
                                  0x02,
                                  0x000E,
                                  4,
                                  NULL,
                                  0x0000,
                                  100)
                  : -1);
    if (0 > result) {
        kdWarning() << "Error setting resolution on device: " << usb_strerror() << endl;
    }
}

void LogitechMouse::setLogitechTo400()
{
    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg( m_usbDeviceHandle,
                                  USB_TYPE_VENDOR,
                                  0x02,
                                  0x000E,
                                  3,
                                  NULL,
                                  0x0000,
                                  100)
                  : -1);
    if (0 > result) {
        kdWarning() << "Error setting resolution on device: " << usb_strerror() << endl;
    }
}

TQ_UINT8 LogitechMouse::batteryLevel()
{
    return m_batteryLevel;
}


TQ_UINT8 LogitechMouse::channel()
{
    return m_channel;
}

bool LogitechMouse::isDualChannelCapable()
{
    return m_twoChannelCapable;
}

void LogitechMouse::setChannel1()
{
    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR,
                                   0x02,
                                   (0x0008 | m_useSecondChannel),
                                   (0x0000 | m_useSecondChannel),
                                   NULL,
                                   0x0000,
                                   1000)
                  : -1);

    if (0 > result) {
        kdWarning() << "Error setting mouse to channel 1 : " << usb_strerror() << endl;
    }

}

void LogitechMouse::setChannel2()
{
    int result = (m_usbDeviceHandle != 0
                  ? usb_control_msg( m_usbDeviceHandle,
                                   USB_TYPE_VENDOR,
                                   0x02,
                                   (0x0008 | m_useSecondChannel),
                                   (0x0001 | m_useSecondChannel),
                                   NULL,
                                   0x0000,
                                   1000)
                  : -1);

    if (0 > result) {
        kdWarning() << "Error setting mouse to channel 2 : " << usb_strerror() << endl;
    }

}

TQString LogitechMouse::cordlessName()
{
    switch ( m_cordlessNameIndex ) {
    case 0x00:
        return i18n( "none" );
        break;
    case 0x04:
        return i18n( "Cordless Mouse" );
        break;
    case 0x05:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x06:
        return i18n( "Cordless MouseMan Wheel" );
        break;
    case 0x07:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x08:
        return i18n( "Cordless Wheel Mouse" );
        break;
    case 0x09:
        return i18n( "Cordless TrackMan Wheel" );
        break;
    case 0x0A:
        return i18n( "TrackMan Live" );
        break;
    case 0x0C:
        return i18n( "Cordless TrackMan FX" );
        break;
    case 0x0D:
        return i18n( "Cordless MouseMan Optical" );
        break;
    case 0x0E:
        return i18n( "Cordless Optical Mouse" );
        break;
    case 0x0F:
        return i18n( "Cordless Mouse" );
        break;
    case 0x12:
        return i18n( "Cordless MouseMan Optical (2ch)" );
        break;
    case 0x13:
        return i18n( "Cordless Optical Mouse (2ch)" );
        break;
    case 0x14:
        return i18n( "Cordless Mouse (2ch)" );
        break;
    case 0x82:
        return i18n( "Cordless Optical TrackMan" );
        break;
    case 0x8A:
        return i18n( "MX700 Cordless Optical Mouse" );
        break;
    case 0x8B:
        return i18n( "MX700 Cordless Optical Mouse (2ch)" );
        break;
    default:
        return i18n( "Unknown mouse");
    }
}

#include "logitechmouse.moc"

#endif

