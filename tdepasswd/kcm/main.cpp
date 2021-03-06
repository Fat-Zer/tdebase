
/**
 *  Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
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
 *
 *
 *  Please see the README
 *
 */

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqbuttongroup.h>
#include <tqevent.h>
#include <tqpixmap.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <tqlayout.h>

#include <kpushbutton.h>
#include <kguiitem.h>
#include <tdeemailsettings.h>
#include <kpassdlg.h>
#include <kuser.h>
#include <kdialog.h>
#include <kimageio.h>
#include <kstandarddirs.h>
#include <tdeaboutdata.h>
#include <kgenericfactory.h>
#include <tdemessagebox.h>
#include <kprocess.h>
#include <tdeio/netaccess.h>
#include <kurl.h>
#include <kurldrag.h>

#include "settings.h"
#include "pass.h"
#include "chfnprocess.h"
#include "chfacedlg.h"
#include "main.h" 

typedef KGenericFactory<KCMUserAccount, TQWidget> Factory;
K_EXPORT_COMPONENT_FACTORY( kcm_useraccount, Factory("useraccount") )

KCMUserAccount::KCMUserAccount( TQWidget *parent, const char *name,
	const TQStringList &)
	: TDECModule( parent, name)
{
	TQVBoxLayout *topLayout = new TQVBoxLayout(this);
	_mw = new MainWidget(this);
	topLayout->addWidget( _mw );

	connect( _mw->btnChangeFace, TQT_SIGNAL(clicked()), TQT_SLOT(slotFaceButtonClicked()));
	connect( _mw->btnChangePassword, TQT_SIGNAL(clicked()), TQT_SLOT(slotChangePassword()));
	_mw->btnChangePassword->setGuiItem( KGuiItem( i18n("Change &Password..."), "password" ));
	
	connect( _mw->leRealname, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));
	connect( _mw->leOrganization, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));
	connect( _mw->leEmail, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));
	connect( _mw->leSMTP, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(changed()));

	_ku = new KUser();
	_kes = new KEMailSettings();

	_mw->lblUsername->setText( _ku->loginName() );
	_mw->lblUID->setText( TQString().number(_ku->uid()) );

	TDEAboutData *about = new TDEAboutData(I18N_NOOP("kcm_useraccount"), 
		I18N_NOOP("Password & User Information"), 0, 0,
		TDEAboutData::License_GPL,
		I18N_NOOP("(C) 2002, Braden MacDonald, "
			"(C) 2004 Ravikiran Rajagopal"));

	about->addAuthor("Frans Englich", I18N_NOOP("Maintainer"), "frans.englich@telia.com");
	about->addAuthor("Ravikiran Rajagopal", 0, "ravi@kde.org");
	about->addAuthor("Michael H\303\244ckel", "haeckel@kde.org" );

	about->addAuthor("Braden MacDonald", I18N_NOOP("Face editor"), "bradenm_k@shaw.ca");
	about->addAuthor("Geert Jansen", I18N_NOOP("Password changer"), "jansen@kde.org", 
			"http://www.stack.nl/~geertj/");
	about->addAuthor("Daniel Molkentin");
	about->addAuthor("Alex Zepeda");
	about->addAuthor("Hans Karlsson", I18N_NOOP("Icons"), "karlsson.h@home.se");
	about->addAuthor("Hermann Thomas", I18N_NOOP("Icons"), "h.thomas@gmx.de");
	setAboutData(about);

	setQuickHelp( i18n("<qt>Here you can change your personal information, which "
			"will be used in mail programs and word processors, for example. You can "
			"change your login password by clicking <em>Change Password</em>.</qt>") );

	addConfig( KCFGPassword::self(), this );
	load();
}

void KCMUserAccount::slotChangePassword()
{
	TDEProcess *proc = new TDEProcess;
	TQString bin = TDEGlobal::dirs()->findExe("tdepasswd");
	if ( !bin )
	{
		kdDebug() << "kcm_useraccount: tdepasswd was not found." << endl;
		KMessageBox::sorry ( this, i18n( "A program error occurred: the internal "
			"program 'tdepasswd' could not be found. You will "
			"not be able to change your password."));

		_mw->btnChangePassword->setEnabled(false);
		delete proc;
		return;
	}

	*proc << bin << _ku->loginName() ;
	proc->start(TDEProcess::DontCare);

	delete proc;

}


KCMUserAccount::~KCMUserAccount()
{
	delete _ku;
	delete _kes;
}

void KCMUserAccount::load()
{
	_mw->lblUsername->setText(_ku->loginName());

	_kes->setProfile(_kes->defaultProfileName());

	_mw->leRealname->setText( _kes->getSetting( KEMailSettings::RealName ));
	_mw->leEmail->setText( _kes->getSetting( KEMailSettings::EmailAddress ));
	_mw->leOrganization->setText( _kes->getSetting( KEMailSettings::Organization ));
	_mw->leSMTP->setText( _kes->getSetting( KEMailSettings::OutServer ));

	TQString _userPicsDir = KCFGUserAccount::faceDir() +  
		TDEGlobal::dirs()->resourceDirs("data").last() + "tdm/faces/";

	TQString fs = KCFGUserAccount::faceSource();
	if (fs == TQString::fromLatin1("UserOnly"))
		_facePerm = userOnly;
	else if (fs == TQString::fromLatin1("PreferUser"))
		_facePerm = userFirst;
	else if (fs == TQString::fromLatin1("PreferAdmin"))
		_facePerm = adminFirst;
	else
		_facePerm = adminOnly; // Admin Only

	if ( _facePerm == adminFirst )
	{ 	// If the administrator's choice takes preference
		_facePixmap = TQPixmap( _userPicsDir + _ku->loginName() + ".face.icon" );

		if ( _facePixmap.isNull() )
			_facePerm = userFirst;
		else
			_mw->btnChangeFace->setPixmap( _facePixmap );
	}

	if ( _facePerm == userFirst )
	{
		// If the user's choice takes preference
		_facePixmap = TQPixmap( KCFGUserAccount::faceFile() );

		// The user has no face, should we check for the admin's setting?
		if ( _facePixmap.isNull() )
			_facePixmap = TQPixmap( _userPicsDir + _ku->loginName() + ".face.icon" );

		if ( _facePixmap.isNull() )
			_facePixmap = TQPixmap( _userPicsDir + KCFGUserAccount::defaultFile() );

		_mw->btnChangeFace->setPixmap( _facePixmap );
	}
	else if ( _facePerm == adminOnly )
	{
		// Admin only
		_facePixmap = TQPixmap( _userPicsDir + _ku->loginName() + ".face.icon" );
		if ( _facePixmap.isNull() )
			_facePixmap = TQPixmap( _userPicsDir + KCFGUserAccount::defaultFile() );
		_mw->btnChangeFace->setPixmap( _facePixmap );
	}

	TDECModule::load(); /* TDEConfigXT */

}

void KCMUserAccount::save()
{
	TDECModule::save(); /* TDEConfigXT */

	/* Save KDE's homebrewn settings */
	_kes->setSetting( KEMailSettings::RealName, _mw->leRealname->text() );
	_kes->setSetting( KEMailSettings::EmailAddress, _mw->leEmail->text() );
	_kes->setSetting( KEMailSettings::Organization, _mw->leOrganization->text() );
	_kes->setSetting( KEMailSettings::OutServer, _mw->leSMTP->text() );

	/* Save realname to /etc/passwd */
	if ( _mw->leRealname->isModified() )
	{
		TQCString password;
		int ret = KPasswordDialog::getPassword( password, i18n("Please enter "
			"your password in order to save your settings:"));

		if ( !ret ) 
		{
			KMessageBox::sorry( this, i18n("You must enter "
				"your password in order to change your information."));
			return;
		}

		ChfnProcess *proc = new ChfnProcess();
		ret = proc->exec(password, _mw->leRealname->text().ascii() );
		if ( ret )
			{
			if ( ret == ChfnProcess::PasswordError )
				KMessageBox::sorry( this, i18n("You must enter a correct password."));

			else 
			{
				KMessageBox::sorry( this, i18n("An error occurred and your password has "
							"probably not been changed. The error "
							"message was:\n%1").arg(static_cast<const char *>(proc->error())));
				kdDebug() << "ChfnProcess->exec() failed. Error code: " << ret 
					<< "\nOutput:" << proc->error() << endl;
			}	
		}	

		delete proc;
	}	

	/* Save the image */
	if( !_facePixmap.save( KCFGUserAccount::faceFile(), "PNG" ))
		KMessageBox::error( this, i18n("There was an error saving the image: %1" ).arg(
			KCFGUserAccount::faceFile()) );

}

void KCMUserAccount::changeFace(const TQPixmap &pix)
{
  if ( _facePerm != userFirst )
    return; // If the user isn't allowed to change their face, don't!

  if ( pix.isNull() ) {
    KMessageBox::sorry( this, i18n("There was an error loading the image.") );
    return;
  }

  _facePixmap = pix;
  _mw->btnChangeFace->setPixmap( _facePixmap );
  emit changed( true );
}

void KCMUserAccount::slotFaceButtonClicked()
{
  if ( _facePerm != userFirst )
  {
    KMessageBox::sorry( this, i18n("Your administrator has disallowed changing your image.") );
    return;
  }

  ChFaceDlg* pDlg = new ChFaceDlg( TDEGlobal::dirs()->resourceDirs("data").last() +
	"/tdm/pics/users/" );

  if ( pDlg->exec() == TQDialog::Accepted && !pDlg->getFaceImage().isNull() )
      changeFace( pDlg->getFaceImage() );

  delete pDlg;
}

/**
 * I merged faceButtonDropEvent into this /Frans
 * The function was called after checking event type and 
 * the code is now below that if statement
 */
bool KCMUserAccount::eventFilter(TQObject *, TQEvent *e)
{
	if (e->type() == TQEvent::DragEnter)
		{
		TQDragEnterEvent *ee = (TQDragEnterEvent *) e;
		ee->accept( KURLDrag::canDecode(ee) );
		return true;
	}

	if (e->type() == TQEvent::Drop)
	{
		if ( _facePerm != userFirst )
		{
			KMessageBox::sorry( this, i18n("Your administrator has disallowed changing your image.") );
			return true;
		}

		KURL *url = decodeImgDrop( (TQDropEvent *) e, this);
		if (url)
		{
			TQString pixPath;
			TDEIO::NetAccess::download(*url, pixPath, this);
			changeFace( TQPixmap( pixPath ) );
			TDEIO::NetAccess::removeTempFile(pixPath);
			delete url;
		}
		return true;
	}
	return false;
}

inline KURL *KCMUserAccount::decodeImgDrop(TQDropEvent *e, TQWidget *wdg)
{
  KURL::List uris;

  if (KURLDrag::decode(e, uris) && (uris.count() > 0))
  {
    KURL *url = new KURL(uris.first());

    KImageIO::registerFormats();
    if( KImageIO::canRead(KImageIO::type(url->fileName())) )
      return url;

    TQStringList qs = TQStringList::split('\n', KImageIO::pattern());
    qs.remove(qs.begin());

    TQString msg = i18n( "%1 does not appear to be an image file.\n"
			  "Please use files with these extensions:\n"
			  "%2").arg(url->fileName()).arg(qs.join("\n"));
    KMessageBox::sorry( wdg, msg);
    delete url;
  }
  return 0;
}

#include "main.moc"

