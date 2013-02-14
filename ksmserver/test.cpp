#include "shutdowndlg.h"
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kiconloader.h>

int
main(int argc, char *argv[])
{
   TDEAboutData about("kapptest", "kapptest", "version");
   TDECmdLineArgs::init(argc, argv, &about);

   TDEApplication a;
   a.iconLoader()->addAppDir("ksmserver");
   KSMShutdownFeedback::start();

   // ShutdownTypeNone == Logout == 0
   // ShutdownTypeReboot == 1
   // ShutdownTypeHalt == 2
   TDEApplication::ShutdownType sdtype = TDEApplication::ShutdownTypeNone;
   TQString bopt;
   KSMDelayedMessageBox::showTicker( sdtype );
   /*
   (void)KSMShutdownDlg::confirmShutdown( true,
                                          sdtype,
                                          bopt );*/
/*   (void)KSMShutdownDlg::confirmShutdown( false,
                                          sdtype,
                                          bopt ); */

   KSMShutdownFeedback::stop();
}
