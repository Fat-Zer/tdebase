
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kapplication.h>

#include "htmlsearch.h"

static KCmdLineOptions options[] =
{
  { "lang <lang>", I18N_NOOP("The language to index"), "en" },
   KCmdLineLastOption // End of options.
};


int main(int argc, char *argv[])
{
  TDEAboutData aboutData( "tdehtmlindex", I18N_NOOP("KHtmlIndex"),
	"",
	I18N_NOOP("TDE Index generator for help files."));

  TDECmdLineArgs::init(argc, argv, &aboutData);
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEGlobal::locale()->setMainCatalogue("htmlsearch");
  TDEApplication app;
  HTMLSearch search;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
  search.generateIndex(args->getOption("lang"));
}
