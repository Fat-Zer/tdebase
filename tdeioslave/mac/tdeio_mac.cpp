/***************************************************************************
                          tdeio_mac.cpp
                             -------------------
    copyright            : (C) 2002 Jonathan Riddell
    email                : jr@jriddell.org
    version              : 1.0.1
    release date         : 19 July 2002
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define PARTITION "/dev/hda11"

#include <kinstance.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tqstring.h>
#include <tqregexp.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "tdeio_mac.moc"

using namespace TDEIO;

extern "C" {
    int KDE_EXPORT kdemain(int, char **argv) {
        TDEInstance instance("tdeio_mac");
        MacProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}

MacProtocol::MacProtocol(const TQCString &pool, const TQCString &app)
                                             : TQObject(), SlaveBase("mac", pool, app) {
/*  logFile = new TQFile("/home/jr/logfile");
    logFile->open(IO_ReadWrite | IO_Append);
    logStream = new TQTextStream(logFile);
    *logStream << "Start Macprotocol()" << endl;
    */
}

MacProtocol::~MacProtocol() {
/*    *logStream << "destructor ~MacProtocol()" << endl;
    logFile->close();
    delete logFile;
    logFile = 0;
    delete logStream;
    logStream = 0;
*/
    delete myTDEProcess;
    myTDEProcess = 0L;
}

//get() called when a file is to be read
void MacProtocol::get(const KURL& url) {
    TQString path = prepareHP(url);  //mount and change to correct directory - return the filename
    TQString query = url.query();
    TQString mode("-");
    TQString mime;
    processedBytes = 0;

    //Find out the size and if it's a text file
    UDSEntry entry = doStat(url);
    UDSEntry::Iterator it;
    for(it = entry.begin(); it != entry.end(); ++it) {
        if ((*it).m_uds == TDEIO::UDS_MIME_TYPE) {
            mime = (*it).m_str;
        }
        if ((*it).m_uds == TDEIO::UDS_SIZE) {
            totalSize((*it).m_long);
        }
    }

    //find out if a mode has been specified in the query e.g. ?mode=t
    //or if it's a text file then set the mode to text
    int modepos = query.find("mode=");
    int textpos = mime.find("text");
    if (modepos != -1) {
        mode += query.mid(modepos + 5, 1);
        if (mode != "-r" && mode != "-b" && mode != "-m" && mode != "-t" && mode != "-a") {
            error(ERR_SLAVE_DEFINED, i18n("Unknown mode"));
        }
    } else if (textpos != -1) {
        mode += "t";
    } else {
        mode += "r";
    }

    //now we can read the file
    myTDEProcess = new TDEProcess();

    *myTDEProcess << "hpcopy" << mode << path << "-";

    //data is now sent directly from the slot
    connect(myTDEProcess, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
            this, TQT_SLOT(slotSetDataStdOutput(TDEProcess *, char *, int)));

    myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

    if (!myTDEProcess->normalExit() || !(myTDEProcess->exitStatus() == 0)) {
        error(ERR_SLAVE_DEFINED,
              i18n("There was an error with hpcopy - please ensure it is installed"));
        return;
    }

    //clean up
    delete myTDEProcess; myTDEProcess = 0;
    //finish
    data(TQByteArray());
    finished();
}

//listDir() called when the user is looking at a directory
void MacProtocol::listDir(const KURL& url) {
    TQString filename = prepareHP(url);

    if (filename.isNull()) {
        error(ERR_CANNOT_LAUNCH_PROCESS, i18n("No filename was found"));
    } else {
        myTDEProcess = new TDEProcess();
        *myTDEProcess << "hpls" << "-la" << filename;

        standardOutputStream = TQString::null;
        connect(myTDEProcess, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
                this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

        myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

        if ((!myTDEProcess->normalExit()) || (!myTDEProcess->exitStatus() == 0)) {
            error(ERR_SLAVE_DEFINED,
                  i18n("There was an error with hpls - please ensure it is installed"));
        }

        //clean up
        delete myTDEProcess; myTDEProcess = 0;
        disconnect(myTDEProcess, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
                this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

        UDSEntry entry;
        if (!standardOutputStream.isEmpty()) {
            TQTextStream in(&standardOutputStream, IO_ReadOnly);
            TQString line = in.readLine(); //throw away top file which shows current directory
            line = in.readLine();

            while (line != NULL) {
                //1.0.4 puts this funny line in sometimes, we don't want it
                if (line.contains("Thread               ") == 0) {
                    entry = makeUDS(line);
                    listEntry(entry, false);
                }
                line = in.readLine();
            }
        }//if standardOutputStream != null

        listEntry(entry, true);
        finished();

    }//if filename == null
}

//stat() called to see if it's a file or directory, called before listDir() or get()
void MacProtocol::stat(const KURL& url) {
    statEntry(doStat(url));
    finished();
}

//doStat(), does all the work that stat() needs
//it's been separated out so it can be called from get() which
//also need information
TQValueList<TDEIO::UDSAtom> MacProtocol::doStat(const KURL& url) {
    TQString filename = prepareHP(url);

    if (filename.isNull()) {
        error(ERR_SLAVE_DEFINED, i18n("No filename was found in the URL"));
    } else if (! filename.isEmpty()) {
        myTDEProcess = new KShellProcess();

        *myTDEProcess << "hpls" << "-ld" << filename;

        standardOutputStream = TQString::null;
        connect(myTDEProcess, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
                this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

        myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

        if ((!myTDEProcess->normalExit()) || (!myTDEProcess->exitStatus() == 0)) {
            error(ERR_SLAVE_DEFINED,
                  i18n("hpls did not exit normally - please ensure you have installed the hfsplus tools"));
        }

        //clean up
        delete myTDEProcess; myTDEProcess = 0;
        disconnect(myTDEProcess, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
                this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

        if (standardOutputStream.isEmpty()) {
            filename.replace("\\ ", " "); //get rid of escapes
            filename.replace("\\&", "&"); //mm, slashes...
            filename.replace("\\!", "!");
            filename.replace("\\(", "(");
            filename.replace("\\)", ")");
            error(ERR_DOES_NOT_EXIST, filename);
        } else {
            //remove trailing \n
            TQString line = standardOutputStream.left(standardOutputStream.length()-1);
            UDSEntry entry = makeUDS(line);
            return entry;
        }
    } else {     //filename is empty means we're looking at root dir
                 //we don't have a listing for the root directory so here's a dummy one
            UDSEntry entry = makeUDS("d         0 item               Jan 01  2000 /");
            return entry;
    }//if filename == null

    return TQValueList<TDEIO::UDSAtom>();
}

//prepareHP() called from get() listDir() and stat()
//(re)mounts the partition and changes to the appropriate directory
TQString MacProtocol::prepareHP(const KURL& url) {
    TQString path = url.path(-1);
    if (path.left(1) == "/") {
        path = path.mid(1); // strip leading slash
    }

    //find out if a device has been specified in the query e.g. ?dev=/dev/fd0
    //or in the config file (query device entries are saved to config file)
    TQString device;
    TDEConfig* config = new TDEConfig("macrc");

    TQString query = url.query();
    int modepos = query.find("dev=");
    if (modepos == -1) {
        //no device specified, read from config or go with #define PARTITION
        device = config->readEntry("device",PARTITION);
    } else {
        //TODO this means dev=foo must be the last argument in the query
        device = query.mid(modepos + 4);
        config->writeEntry("device",device);
    }
    delete config; config = 0;

    //first we run just hpmount and check the output to see if it's version 1.0.2 or 1.0.4
    myTDEProcess = new TDEProcess();
    *myTDEProcess << "hpmount";
    standardOutputStream = TQString::null;
    connect(myTDEProcess, TQT_SIGNAL(receivedStderr(TDEProcess *, char *, int)),
            this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

    myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

    bool version102 = true;

    if (standardOutputStream.contains("options") != 0) {
        version102 = false;
    }

    delete myTDEProcess; myTDEProcess = 0;
    disconnect(myTDEProcess, TQT_SIGNAL(receivedStderr(TDEProcess *, char *, int)),
            this, TQT_SLOT(slotGetStdOutput(TDEProcess *, char *, int)));

    //now mount the drive
    myTDEProcess = new TDEProcess();
    if (version102) {
        *myTDEProcess << "hpmount" << device;
    } else {
        *myTDEProcess << "hpmount" << "-r" << device;
    }

    myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

    if ((!myTDEProcess->normalExit()) || (!myTDEProcess->exitStatus() == 0)) {
        //TODO this error interrupts the user when typing ?dev=foo on each letter of foo
        error(ERR_SLAVE_DEFINED,
              i18n("hpmount did not exit normally - please ensure that hfsplus utils are installed,\n"
                   "that you have permission to read the partition (ls -l /dev/hdaX)\n"
                   "and that you have specified the correct partition.\n"
                   "You can specify partitions by adding ?dev=/dev/hda2 to the URL."));
        return NULL;
    }

    //clean up
    delete myTDEProcess; myTDEProcess = 0;

    //escape any funny characters
    //TODO are there any more characters to escape?
    path.replace(" ", "\\ ");
    path.replace("&", "\\&");
    path.replace("!", "\\!");
    path.replace("(", "\\(");
    path.replace(")", "\\)");

    //then change to the right directory
    int s;  TQString dir;
    s = path.find('/');
    while (s != -1) {
        dir = path.left(s);
        path = path.mid(s+1);

        myTDEProcess = new TDEProcess();
        *myTDEProcess << "hpcd" << dir;

        myTDEProcess->start(TDEProcess::Block, TDEProcess::All);

        if ((!myTDEProcess->normalExit()) || (!myTDEProcess->exitStatus() == 0)) {
            error(ERR_SLAVE_DEFINED,
                  i18n("hpcd did not exit normally - please ensure it is installed"));
            return NULL;
        }

        //clean up
        delete myTDEProcess; myTDEProcess = 0;

        s = path.find('/');
    }

    return path;
}

//makeUDS()  takes a line of output from hpls -l and converts it into
// one of these UDSEntrys to return
//called from listDir() and stat()
TQValueList<TDEIO::UDSAtom> MacProtocol::makeUDS(const TQString& _line) {
    TQString line(_line);
    UDSEntry entry;

    //is it a file or a directory
    TQRegExp dirRE("^d. +([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +(.*)");
    TQRegExp fileRE("^([f|F]). +(....)/(....) +([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +(.*)");
    if (dirRE.exactMatch(line)) {
        UDSAtom atom;
        atom.m_uds = TDEIO::UDS_NAME;
        atom.m_str = dirRE.cap(6);
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_MODIFICATION_TIME;
        atom.m_long = makeTime(dirRE.cap(4), dirRE.cap(3), dirRE.cap(5));
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_FILE_TYPE;
        atom.m_long = S_IFDIR;
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_ACCESS;
        atom.m_long = 0755;
        entry.append(atom);

    } else if (fileRE.exactMatch(line)) {
        UDSAtom atom;
        atom.m_uds = TDEIO::UDS_NAME;
        atom.m_str = fileRE.cap(9);
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_SIZE;
        TQString theSize(fileRE.cap(4)); //TODO: this is data size, what about  resource size?
        atom.m_long = theSize.toLong();
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_MODIFICATION_TIME;
        atom.m_long = makeTime(fileRE.cap(7), fileRE.cap(6), fileRE.cap(8));
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_ACCESS;
        if (TQString(fileRE.cap(1)) == TQString("F")) { //if locked then read only
            atom.m_long = 0444;
        } else {
            atom.m_long = 0644;
        }
        entry.append(atom);

        atom.m_uds = TDEIO::UDS_MIME_TYPE;
        TQString mimetype = getMimetype(fileRE.cap(2),fileRE.cap(3));
        atom.m_str = mimetype.local8Bit();
        entry.append(atom);

        // Is it a file or a link/alias, just make aliases link to themselves
        if (TQString(fileRE.cap(2)) == TQString("adrp") ||
            TQString(fileRE.cap(2)) == TQString("fdrp")) {
            atom.m_uds = TDEIO::UDS_FILE_TYPE;
            atom.m_long = S_IFREG;
            entry.append(atom);

            atom.m_uds = TDEIO::UDS_LINK_DEST;
            atom.m_str = fileRE.cap(9); //I have a file called "Mozilla alias" the name
                                          // of which displays funny because of this.
                                          // No idea why.  Same for other tdeioslaves. A font thing?
            entry.append(atom);
            } else {
            atom.m_uds = TDEIO::UDS_FILE_TYPE;
            atom.m_long = S_IFREG;
            entry.append(atom);
        }
    } else {
        error(ERR_INTERNAL, i18n("hpls output was not matched"));
    } //if match dirRE or fileRE

    return entry;
}

//slotGetStdOutput() grabs output from the hp commands
// and adds it to the buffer
void MacProtocol::slotGetStdOutput(TDEProcess*, char *s, int len) {
  standardOutputStream += TQString::fromLocal8Bit(s, len);
}

//slotSetDataStdOutput() is used during hpcopy to give
//standard output to KDE
void MacProtocol::slotSetDataStdOutput(TDEProcess*, char *s, int len) {
    processedBytes += len;
    processedSize(processedBytes);
    TQByteArray array;
    array.setRawData(s, len);
    data(array);
    array.resetRawData(s, len);
}

//makeTime() takes in the date output from hpls -l
//and returns as good a timestamp as we're going to get
int MacProtocol::makeTime(TQString mday, TQString mon, TQString third) {
    int year; int month; int day;
    int hour; int minute;

    //find the month
    if (mon == "Jan") { month = 1; }
    else if (mon == "Feb") { month = 2; }
    else if (mon == "Mar") { month = 3; }
    else if (mon == "Apr") { month = 4; }
    else if (mon == "May") { month = 5; }
    else if (mon == "Jun") { month = 6; }
    else if (mon == "Jul") { month = 7; }
    else if (mon == "Aug") { month = 8; }
    else if (mon == "Sep") { month = 9; }
    else if (mon == "Oct") { month = 10; }
    else if (mon == "Nov") { month = 11; }
    else if (mon == "Dec") { month = 12; }
    else {
        error(ERR_INTERNAL, i18n("Month output from hpls -l not matched"));
        month = 13;
    }

    //if the file is recent (last 12 months) hpls gives us the time,
    // otherwise it only prints the year
    TQRegExp hourMin("(..):(..)");
    if (hourMin.exactMatch(third)) {
        TQDate currentDate(TQDate::currentDate());

        if (month > currentDate.month()) {
            year = currentDate.year() - 1;
        } else {
            year = currentDate.year();
        }
        TQString h(hourMin.cap(1));
        TQString m(hourMin.cap(2));
        hour = h.toInt();
        minute = m.toInt();
    } else {
        year = third.toInt();
        hour = 0;
        minute = 0;
    }// if hour:min or year

    day = mday.toInt();

    //check it's valid
    if ( (!TQDate::isValid(year, month, day)) || (!TQTime::isValid(hour, minute, 0) ) ) {
        error(ERR_INTERNAL, i18n("Could not parse a valid date from hpls"));
    }

    //put it together and work it out
    TQDate fileDate(year, month, day);
    TQTime fileTime(hour, minute);
    TQDateTime fileDateTime(fileDate, fileTime);

    return fileDateTime.toTime_t();
}

TQString MacProtocol::getMimetype(TQString type, TQString app) {
    if (type == TQString("TEXT") && app == TQString("ttxt")) {
        return TQString("text/plain");
    } else if (type == TQString("TEXT") && app == TQString("udog")) {
        return TQString("text/html");
    } else if (type == TQString("svgs")) {
        return TQString("text/xml");
    } else if (type == TQString("ZIP ")) {
        return TQString("application/zip");
    } else if (type == TQString("pZip")) {
        return TQString("application/zip");
    } else if (type == TQString("APPL")) {
        return TQString("application/x-executable");
    } else if (type == TQString("MooV")) {
        return TQString("video/quicktime");
    } else if (type == TQString("TEXT") && app == TQString("MSWD")) {
        return TQString("application/vnd.ms-word");
    } else if (type == TQString("PDF ")) {
        return TQString("application/pdf");
    } else if (app == TQString("CARO")) {
        return TQString("application/pdf");
    } else if (type == TQString("SIT5")) {
        return TQString("application/x-stuffit");
    } else if (type == TQString("SITD")) {
        return TQString("application/x-stuffit");
    } else if (type == TQString("SIT!")) {
        return TQString("application/x-stuffit");
    } else if (app == TQString("SIT!")) {
        return TQString("application/x-stuffit");
    } else if (type == TQString("RTFf")) {
        return TQString("text/rtf");
    } else if (type == TQString("GIFf")) {
        return TQString("image/gif");
    } else if (type == TQString("JPEG")) {
        return TQString("image/jpeg");
    } else if (type == TQString("PNGf")) {
        return TQString("image/png");
    } else if (type == TQString("XBMm")) {
        return TQString("image/x-xbm");
    } else if (type == TQString("EPSF")) {
        return TQString("image/x-epsf");
    } else if (type == TQString("TIFF")) {
        return TQString("image/tiff");
    } else if (type == TQString("PICT")) {
        return TQString("image/pict");
    } else if (type == TQString("TPIC")) {
        return TQString("image/x-targa");
    } else if (type == TQString("ULAW")) {
        return TQString("audio/basic");
    } else if (type == TQString("AIFF")) {
        return TQString("audio/x-aiff");
    } else if (type == TQString("WAVE")) {
        return TQString("audio/x-wav");
    } else if (type == TQString("FFIL") && app == TQString("DMOV")) {
        return TQString("application/x-font");
    } else if (type == TQString("XLS3")) {
        return TQString("application/vnd.ms-excel");
    } else if (type == TQString("XLS4")) {
        return TQString("application/vnd.ms-excel");
    } else if (type == TQString("XLS5")) {
        return TQString("application/vnd.ms-excel");
    } else if (app == TQString("MSWD")) {
        return TQString("application/vnd.ms-word");
    } else if (type == TQString("TEXT")) {
        return TQString("text/plain");
    } else if (app == TQString("ttxt")) {
        return TQString("text/plain");
    }
    return TQString("application/octet-stream");
}


