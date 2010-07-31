/***************************************************************************
                          mac.cpp
                             -------------------
    copyright            : (C) 2002 Jonathan Riddell
    email                : jr@jriddell.org
    version              : 1.0
    release date         : 10 Feburary 2002
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kio/slavebase.h>
#include <kio/global.h>
#include <kurl.h>
#include <kprocess.h>

#include <tqstring.h>
#include <tqcstring.h>
#include <tqfile.h>
#include <tqtextstream.h>

class MacProtocol : public TQObject, public KIO::SlaveBase
{
    Q_OBJECT
public:
    MacProtocol(const TQCString &pool, const TQCString &app);
    ~MacProtocol();
    virtual void get(const KURL& url );
    virtual void listDir(const KURL& url);
    virtual void stat(const KURL& url);
protected slots:
    void slotGetStdOutput(KProcess*, char*, int);
    void slotSetDataStdOutput(KProcess*, char *s, int len);
protected:
    TQString prepareHP(const KURL& _url);
    TQValueList<KIO::UDSAtom> makeUDS(const TQString& _line);
    int makeTime(TQString mday, TQString mon, TQString third);
    TQString getMimetype(TQString type, TQString app);
    TQValueList<KIO::UDSAtom> doStat(const KURL& url);

    KIO::filesize_t processedBytes;
    TQString standardOutputStream;
    KProcess* myKProcess;

    //for debugging
    //TQFile* logFile;
    //TQTextStream* logStream;
};
