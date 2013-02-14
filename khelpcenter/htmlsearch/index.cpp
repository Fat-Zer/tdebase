
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <kglobal.h>
#include <tdeapplication.h>

#include "htmlsearch.h"

static TDECmdLineOptions options[] =
{
  { "lang <lang>", I18N_NOOP("The language to index"), "en" },
   TDECmdLineLastOption // End of options.
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
