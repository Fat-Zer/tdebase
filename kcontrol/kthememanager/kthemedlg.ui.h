/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <krun.h>

void KThemeDlg::startKonqui( const TQString & url )
{
    (void) new KRun(url);
}


void KThemeDlg::startBackground()
{
    KRun::runCommand("tdecmshell tde-background");
}


void KThemeDlg::startColors()
{
    KRun::runCommand("tdecmshell tde-colors");
}


void KThemeDlg::startStyle()
{
    KRun::runCommand("tdecmshell tde-style");
}


void KThemeDlg::startIcons()
{
    KRun::runCommand("tdecmshell tde-icons");
}

void KThemeDlg::startFonts()
{
   KRun::runCommand("tdecmshell tde-fonts");
}


void KThemeDlg::startSaver()
{
    KRun::runCommand("tdecmshell tde-screensaver");
}
