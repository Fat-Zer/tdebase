#include "shutdowndlg.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>

int
main(int argc, char *argv[])
{
   KAboutData about("kapptest", "kapptest", "version");
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
