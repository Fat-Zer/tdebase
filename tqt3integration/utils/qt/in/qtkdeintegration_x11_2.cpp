 /*
 *  This file is part of the Trinity Desktop Environment
 *
 *  Original file taken from the OpenSUSE kdebase builds
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

// ---

int QKDEIntegration::information( QWidget* parent, const QString& caption,
    const QString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        QMessageBox::Information, parentToWinId( parent ), caption, text, button0, button1, button2 );
    }

int QKDEIntegration::question( QWidget* parent, const QString& caption,
    const QString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        QMessageBox::Question, parentToWinId( parent ), caption, text, button0, button1, button2 );
    }

int QKDEIntegration::warning( QWidget* parent, const QString& caption,
    const QString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        QMessageBox::Warning, parentToWinId( parent ), caption, text, button0, button1, button2 );
    }

int QKDEIntegration::critical( QWidget* parent, const QString& caption,
    const QString& text, int button0, int button1, int button2 )
    {
    return qtkde_messageBox1(
        QMessageBox::Critical, parentToWinId( parent ), caption, text, button0, button1, button2 );
    }

int QKDEIntegration::information( QWidget* parent, const QString& caption,
    const QString& text, const QString& button0Text, const QString& button1Text, const QString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        QMessageBox::Information, parentToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int QKDEIntegration::question( QWidget* parent, const QString& caption,
    const QString& text, const QString& button0Text, const QString& button1Text, const QString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        QMessageBox::Question, parentToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int QKDEIntegration::warning( QWidget* parent, const QString& caption,
    const QString& text, const QString& button0Text, const QString& button1Text, const QString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        QMessageBox::Warning, parentToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }

int QKDEIntegration::critical( QWidget* parent, const QString& caption,
    const QString& text, const QString& button0Text, const QString& button1Text, const QString& button2Text,
    int defaultButton, int escapeButton )
    {
    return qtkde_messageBox2(
        QMessageBox::Critical, parentToWinId( parent ), caption, text, button0Text, button1Text, button2Text, defaultButton, escapeButton );
    }
