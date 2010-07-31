#ifndef SAVERCONFIG_H
#define SAVERCONFIG_H

#include <tqstring.h>

class SaverConfig
{
public:
    SaverConfig();

    bool read(const TQString &file);

    TQString exec() const { return mExec; }
    TQString setup() const { return mSetup; }
    TQString saver() const { return mSaver; }
    TQString name() const { return mName; }
    TQString file() const { return mFile; }
    TQString category() const { return mCategory; }

protected:
    TQString mExec;
    TQString mSetup;
    TQString mSaver;
    TQString mName;
    TQString mFile;
    TQString mCategory;
};

#endif
