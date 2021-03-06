/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * toplevel.cpp
 *
 * Copyright (C) 2000-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************/

#ifndef TOPLEVEL_H
#define TOPLEVEL_H

class TDEAboutData;
class KrashConfig;
class DrKBugReport;
class BugDescription;

#include <kdialogbase.h>

#include <tdeio/job.h>
#include <tdeio/netaccess.h>

class Toplevel : public KDialogBase
{
  Q_OBJECT

public:
  Toplevel(KrashConfig *krash, TQWidget *parent = 0, const char * name = 0);
  ~Toplevel();

private:
  // helper methods
  TQString generateText() const;
  int postCrashDataToServer(TQCString data);
  int saveOfflineCrashReport(TQCString data);

protected slots:
  void slotUser1();
  void slotUser2();
  void slotNewDebuggingApp(const TQString& launchName);
  void slotUser3();

protected slots:
  void slotBacktraceSomeError();
  void slotBacktraceDone(const TQString &);

  void slotSendReportBacktraceSomeError();
  void slotSendReportBacktraceDone(const TQString &);

  void postCrashDataToServerData(TDEIO::Job *, const TQByteArray &);
  void postCrashDataToServerResult(TDEIO::Job *job);
  void postCrashDataToServerDataRedirection(TDEIO::Job * /*job*/, const KURL& url);

private:
  KrashConfig *m_krashconf;
  DrKBugReport *m_bugreport;
  BugDescription* m_bugdescription;
  TQCString m_serverResponse;
  TQCString m_backtraceSubmissionData;
};

#endif
