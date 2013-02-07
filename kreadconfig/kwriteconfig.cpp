/* Write TDEConfig() entries - for use in shell scripts.
 * (c) 2001 Red Hat, Inc. & Lu�s Pedro Coelho
 * Programmed by Lu�s Pedro Coelho <luis_pedro@netcabo.pt>
 *  based on kreadconfig by Bernhard Rosenkraenzer <bero@redhat.com>
 *
 * License: GPL
 *
 */
#include <tdeconfig.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdio.h>

static TDECmdLineOptions options[] =
{
	{ "file <file>", I18N_NOOP("Use <file> instead of global config"), 0 },
	{ "group <group>", I18N_NOOP("Group to look in"), "TDE" },
        { "key <key>", I18N_NOOP("Key to look for"), 0 },
	{ "type <type>", I18N_NOOP("Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string"), 0 },
	{ "+value", I18N_NOOP( "The value to write. Mandatory, on a shell use '' for empty" ), 0 },
        TDECmdLineLastOption
};
int main(int argc, char **argv)
{
	TDEAboutData aboutData("kwriteconfig", I18N_NOOP("KWriteConfig"),
		"1.0.0",
		I18N_NOOP("Write TDEConfig entries - for use in shell scripts"),
		TDEAboutData::License_GPL,
		"(c) 2001 Red Hat, Inc. & Lu�s Pedro Coelho");
	aboutData.addAuthor("Lu�s Pedro Coelho", 0, "luis_pedro@netcabo.pt");
	aboutData.addAuthor("Bernhard Rosenkraenzer", "Wrote kreadconfig on which this is based", "bero@redhat.com");
	TDECmdLineArgs::init(argc, argv, &aboutData);
	TDECmdLineArgs::addCmdLineOptions(options);
	TDECmdLineArgs *args=TDECmdLineArgs::parsedArgs();

	TQString group=TQString::fromLocal8Bit(args->getOption("group"));
	TQString key=TQString::fromLocal8Bit(args->getOption("key"));
	TQString file=TQString::fromLocal8Bit(args->getOption("file"));
	TQCString type=args->getOption("type").lower();


	if (key.isNull() || !args->count()) {
		TDECmdLineArgs::usage();
		return 1;
	}
	TQCString value = args->arg( 0 );

	TDEInstance inst(&aboutData);

	TDEConfig *konfig;
	if (file.isEmpty())
	   konfig = new TDEConfig(TQString::fromLatin1("kdeglobals"), false, false);
	else
	   konfig = new TDEConfig(file, false, false);

	konfig->setGroup(group);
	if ( konfig->getConfigState() != TDEConfig::ReadWrite || konfig->entryIsImmutable( key ) ) return 2;

	if(type=="bool") {
		// For symmetry with kreadconfig we accept a wider range of values as true than Qt
		bool boolvalue=(value=="true" || value=="on" || value=="yes" || value=="1");
		konfig->writeEntry( key, boolvalue );
	} else if (type=="path") {
		konfig->writePathEntry( key, TQString::fromLocal8Bit( value ) );
	} else {
		konfig->writeEntry( key, TQString(TQString::fromLocal8Bit( value )) );
	}
	konfig->sync();
        delete konfig;
	return 0;
}

