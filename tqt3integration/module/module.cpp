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

#include "module.h"

#include <assert.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <krecentdocument.h>
#include <kwin.h>
#include <qtimer.h>
#include <stdlib.h>
#include <unistd.h>
#include <kmessagebox.h>

#include <X11/Xutil.h>

extern "C"
{ 
    KDE_EXPORT KDEDModule *create_kdeintegration( const QCString& obj )
        {
            return new KDEIntegration::Module( obj );
        }
};

namespace KDEIntegration
{

static void prepareDialog( QWidget* w, long parent, const QCString& wmclass1, const QCString& wmclass2 )
    {
    XClassHint hints;
    hints.res_name = ( char* ) ( const char* ) wmclass1;
    hints.res_class = ( char* ) ( const char* ) wmclass2;
    XSetClassHint( qt_xdisplay(), w->winId(), &hints );
    KWin::setMainWindow( w, parent );
    KWin::setState( w->winId(), NET::Modal );
    KWin::WindowInfo info = KWin::windowInfo( parent, (unsigned long)NET::WMGeometry );
    if( info.valid())
        w->move( info.geometry().x() + ( info.geometry().width() - w->width())/2,
            info.geometry().y() + ( info.geometry().height()- w->height())/2 );
    }
    
// duped in qtkde
static QString getHostname()
    {
    char hostname[ 256 ];
    if( gethostname( hostname, 255 ) == 0 )
        {
        hostname[ 255 ] = '\0';
        return hostname;
        }
    return "";
    }

bool Module::initializeIntegration( const QString& hostname )
    {
    if( hostname != getHostname())
        return false;
    // multihead support in KDE is just a hack, it wouldn't work very well anyway
    if( KGlobalSettings::isMultiHead())
        return false;
    return true;
    }

void* Module::getOpenFileNames( const QString& filter, QString workingDirectory, long parent,
    const QCString& name, const QString& caption, QString /*selectedFilter*/, bool multiple,
    const QCString& wmclass1, const QCString& wmclass2 )
    {
    KFileDialog* dlg = new KFileDialog( workingDirectory, filter, 0, name.isEmpty() ? "filedialog" : name, false);
    prepareDialog( dlg, parent, wmclass1, wmclass2 );
    dlg->setOperationMode( KFileDialog::Opening );
    dlg->setMode(( multiple ? KFile::Files : KFile::File ) | KFile::LocalOnly );
    dlg->setPlainCaption( caption.isNull() ? i18n("Open") : caption );
// TODO    dlg->ops->clearHistory();
    connect( dlg, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dlg->show();
    return dlg;
    }

void* Module::getSaveFileName( const QString& initialSelection, const QString& filter,
    QString workingDirectory, long parent, const QCString& name, const QString& caption, QString /*selectedFilter*/,
    const QCString& wmclass1, const QCString& wmclass2 )
    {
    QString initial = workingDirectory;
    if( !initialSelection.isEmpty())
        {
        if( initial.right( 1 ) != QChar( '/' ))
            initial += '/';
        initial += initialSelection;
        }
    bool specialDir = initial.at(0) == ':';
    KFileDialog* dlg = new KFileDialog( specialDir ? initial : QString::null, filter, 0,
        name.isEmpty() ? "filedialog" : name, false);
    if ( !specialDir )
        dlg->setSelection( initial ); // may also be a filename
    prepareDialog( dlg, parent, wmclass1, wmclass2 );
    dlg->setOperationMode( KFileDialog::Saving );
    dlg->setPlainCaption( caption.isNull() ? i18n("Save As") : caption );
    connect( dlg, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dlg->show();
    return dlg;
    }


void* Module::getExistingDirectory( const QString& initialDirectory, long parent,
    const QCString& name, const QString& caption, const QCString& wmclass1, const QCString& wmclass2 )
    {
    KDirSelectDialog* dlg = new KDirSelectDialog( initialDirectory, true, 0,
        name.isEmpty() ? name : "kdirselect dialog", false );
    prepareDialog( dlg, parent, wmclass1, wmclass2 );
    dlg->setPlainCaption( caption.isNull() ? i18n( "Select Folder" ) : caption );
    connect( dlg, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dlg->show();
    return dlg;
    }

void* Module::getColor( const QColor& color, long parent, const QCString& name,
    const QCString& wmclass1, const QCString& wmclass2 )
    {
    KColorDialog* dlg = new KColorDialog( NULL, name.isEmpty() ? name : "colordialog", true );
    dlg->setModal( false ); // KColorDialog creates its buttons depending on modality :(
    if( color.isValid())
        dlg->setColor( color );
    prepareDialog( dlg, parent, wmclass1, wmclass2 );
    dlg->setPlainCaption( i18n( "Select Color" ));
    connect( dlg, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dlg->show();
    return dlg;
    }

void* Module::getFont( bool /*ok*/, const QFont& def, long parent, const QCString& name,
    const QCString& wmclass1, const QCString& wmclass2 )
    {
    KFontDialog* dlg = new KFontDialog( NULL, name.isEmpty() ? name : "Font Selector", false, false );
    dlg->setFont( def, false );
    prepareDialog( dlg, parent, wmclass1, wmclass2 );
    dlg->setPlainCaption( i18n( "Select Font" ));
    connect( dlg, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dlg->show();
    return dlg;
    }

namespace
{
struct btns
    {
    int buttons[ 3 ];
    };
}
static QMap< KDialogBase*, btns > msgbox1_buttons;

void* Module::messageBox1( int type, long parent, const QString& caption, const QString& text,
    int button0, int button1, int button2, const QCString& wmclass1, const QCString& wmclass2 )
    {
    static const char* const caps[ 4 ]
        = { I18N_NOOP( "Information" ), I18N_NOOP( "Question" ), I18N_NOOP( "Warning" ), I18N_NOOP( "Error" )};
    int buttons[ 3 ] = { button0 & QMessageBox::ButtonMask,
        button1 & QMessageBox::ButtonMask, button2 & QMessageBox::ButtonMask };
    KGuiItem buttonItems[ 3 ];
    for( int i = 0;
         i < 3;
         ++i )
        switch( buttons[ i ] )
            {
            case QMessageBox::Ok:
                buttonItems[ i ] = KStdGuiItem::ok();
              break;
            case QMessageBox::Cancel:
                buttonItems[ i ] = KStdGuiItem::cancel();
              break;
            case QMessageBox::Yes:
                buttonItems[ i ] = KStdGuiItem::yes();
              break;
            case QMessageBox::No:
                buttonItems[ i ] = KStdGuiItem::no();
              break;
            case QMessageBox::Abort:
                buttonItems[ i ] = KGuiItem( i18n( "&Abort" ));
              break;
            case QMessageBox::Retry:
                buttonItems[ i ] = KGuiItem( "&Retry" );
              break;
            case QMessageBox::Ignore:
                buttonItems[ i ] = KGuiItem( "&Ignore" );
              break;
            case QMessageBox::YesAll:
                buttonItems[ i ] = KStdGuiItem::yes();
                buttonItems[ i ].setText( i18n( "Yes to &All" ));
              break;
            case QMessageBox::NoAll:
                buttonItems[ i ] = KStdGuiItem::no();
                buttonItems[ i ].setText( i18n( "N&o to All" ));
              break;
            default:
              break;
            };
    KDialogBase::ButtonCode defaultButton = KDialogBase::NoDefault;
    if( button0 & QMessageBox::Default )
        defaultButton = KDialogBase::Yes;
    else if( button1 & QMessageBox::Default )
        defaultButton = KDialogBase::No;
    else if( button2 & QMessageBox::Default )
        defaultButton = KDialogBase::Cancel;
    else // TODO KDialogBase's handling of NoDefault has strange focus effects
        defaultButton = KDialogBase::Yes;
    KDialogBase::ButtonCode escapeButton = KDialogBase::Cancel;
    if( button0 & QMessageBox::Escape )
        escapeButton = KDialogBase::Yes;
    else if( button1 & QMessageBox::Escape )
        escapeButton = KDialogBase::No;
    else if( button2 & QMessageBox::Escape )
        escapeButton = KDialogBase::Cancel;
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n( caps[ type ] ) : caption,
                       KDialogBase::Yes
                       | ( buttons[ 1 ] == QMessageBox::NoButton ? 0 : int( KDialogBase::No ))
                       | ( buttons[ 2 ] == QMessageBox::NoButton ? 0 : int( KDialogBase::Cancel )),
                       defaultButton, escapeButton,
                       NULL, "messageBox2", true, true,
                       buttonItems[ 0 ], buttonItems[ 1 ],buttonItems[ 2 ] );
    bool checkboxResult = false;
    KMessageBox::createKMessageBox(dialog, static_cast< QMessageBox::Icon >( type ), text, QStringList(),
                       QString::null,
                       &checkboxResult, KMessageBox::Notify | KMessageBox::NoExec);
    prepareDialog( dialog, parent, wmclass1, wmclass2 );
    dialog->setPlainCaption( caption );
    connect( dialog, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    btns b;
    b.buttons[ 0 ] = buttons[ 0 ];
    b.buttons[ 1 ] = buttons[ 1 ];
    b.buttons[ 2 ] = buttons[ 2 ];
    msgbox1_buttons[ dialog ] = b;
    dialog->show();
    return dialog;
    }

void* Module::messageBox2( int type, long parent, const QString& caption, const QString& text, const QString& button0Text,
    const QString& button1Text, const QString& button2Text, int defaultButton, int escapeButton,
    const QCString& wmclass1, const QCString& wmclass2 )
    {
    static KDialogBase::ButtonCode map[ 4 ]
        = { KDialogBase::NoDefault, KDialogBase::Yes, KDialogBase::No, KDialogBase::Cancel };
    static const char* const caps[ 4 ]
        = { I18N_NOOP( "Information" ), I18N_NOOP( "Question" ), I18N_NOOP( "Warning" ), I18N_NOOP( "Error" )};
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n( caps[ type ] ) : caption,
                       KDialogBase::Yes
                       | ( button1Text.isEmpty() ? 0 : int( KDialogBase::No ))
                       | ( button2Text.isEmpty() ? 0 : int( KDialogBase::Cancel )),
                       map[ defaultButton + 1 ], map[ escapeButton + 1 ],
                       NULL, "messageBox2", true, true,
                       button0Text.isEmpty() ? KStdGuiItem::ok() : KGuiItem( button0Text ), button1Text,button2Text);
    bool checkboxResult = false;
    KMessageBox::createKMessageBox(dialog, static_cast< QMessageBox::Icon >( type ), text, QStringList(),
                       QString::null,
                       &checkboxResult, KMessageBox::Notify | KMessageBox::NoExec);
    prepareDialog( dialog, parent, wmclass1, wmclass2 );
    dialog->setPlainCaption( caption );
    connect( dialog, SIGNAL( dialogDone( int )), SLOT( dialogDone( int )));
    dialog->show();
    return dialog;
    }

void Module::dialogDone( int result )
    {
    void* handle = (void*)sender(); // TODO?
    JobData job = jobs[ handle ];
    switch( job.type )
        {
        case JobData::GetOpenFileNames:
            {
            KFileDialog* dlg = static_cast< KFileDialog* >( handle );
            post_getOpenFileNames( dlg, result == QDialog::Accepted ? dlg->selectedFiles() : QStringList(),
                dlg->baseURL().path(), dlg->currentFilter());
            dlg->deleteLater();
          break;
            }
        case JobData::GetSaveFileName:
            {
            KFileDialog* dlg = static_cast< KFileDialog* >( handle );
            QString filename = result == QDialog::Accepted ? dlg->selectedFile() : QString();
            if (!filename.isEmpty())
                KRecentDocument::add(filename);
            post_getSaveFileName( dlg, filename, dlg->baseURL().path(), dlg->currentFilter());
            dlg->deleteLater();
          break;
            }
        case JobData::GetExistingDirectory:
            {
            KDirSelectDialog* dlg = static_cast< KDirSelectDialog* >( handle );
            post_getExistingDirectory( dlg, result == QDialog::Accepted ? dlg->url().path() : QString());
            dlg->deleteLater();
          break;
            }
        case JobData::GetColor:
            {
            KColorDialog* dlg = static_cast< KColorDialog* >( handle );
            post_getColor( dlg, result == QDialog::Accepted ? dlg->color() : QColor());
            dlg->deleteLater();
          break;
            }
        case JobData::GetFont:
            {
            KFontDialog* dlg = static_cast< KFontDialog* >( handle );
            post_getFont( dlg, result == QDialog::Accepted ? dlg->font() : QFont(), result == QDialog::Accepted );
            dlg->deleteLater();
          break;
            }
        case JobData::MessageBox1:
            {
            KDialogBase* dlg = static_cast< KDialogBase* >( handle );
            btns b = msgbox1_buttons[ dlg ];
            int res;
            if( result == KDialogBase::Cancel )
                res = b.buttons[ 2 ];
            else if( result == KDialogBase::Yes )
                res = b.buttons[ 0 ];
            else
                res = b.buttons[ 1 ];
            msgbox1_buttons.remove( dlg );
            post_messageBox1( dlg, res );
//            if (checkboxResult)
//                saveDontShowAgainYesNo(dontAskAgainName, res);
            dlg->deleteLater();
          break;
            }
        case JobData::MessageBox2:
            {
            KDialogBase* dlg = static_cast< KDialogBase* >( handle );
            int res;
            if( result == KDialogBase::Cancel )
                res = 2;
            else if( result == KDialogBase::Yes )
                res = 0;
            else if( result == KDialogBase::No )
                res = 1;
            else
                res = -1;
            post_messageBox2( dlg, res );
//            if (checkboxResult)
//                saveDontShowAgainYesNo(dontAskAgainName, res);
            dlg->deleteLater();
          break;
            }
        }
    }

Module::Module( const QCString& obj )
    : KDEDModule( obj )
    {
    }

#include "module_functions.cpp"

} // namespace

#include "module.moc"
