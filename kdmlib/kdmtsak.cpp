/*
   This file is part of the TDE project
   Copyright (C) 2011 Timothy Pearson <kb9vqf@pearsoncomputing.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

#include <tqstring.h>

#include "config.h"

#define FIFO_FILE "/tmp/ksocket-global/tsak"

// #define DEBUG

int main (int argc, char *argv[])
{
	int mPipe_fd;
	char readbuf[128];
	int numread;

	pid_t parentproc = getppid();
#ifdef DEBUG
	printf("Parent pid is: %d\n\r", parentproc);
#endif

	char parentexecutable[8192];
	TQString procparent = TQString("/proc/%1/exe").arg(parentproc);
	int chars = readlink(procparent.ascii(), parentexecutable, sizeof(parentexecutable));
	parentexecutable[chars] = 0;
	parentexecutable[8191] = 0;
	procparent = parentexecutable;
#ifdef DEBUG
	printf("Parent executable name and full path is: %s\n\r", procparent.ascii());
#endif

	TQString tdeBinaryPath = TQString(KDE_BINDIR "/");
#ifdef DEBUG
	printf("The TDE binary path is: %s\n\r", tdeBinaryPath.ascii());
#endif

	if (!procparent.startsWith(tdeBinaryPath)) {
		printf("Unauthorized path detected in calling process\n\r");
		return 2;
	}
	else {
		procparent = procparent.mid(tdeBinaryPath.length());
#ifdef DEBUG
		printf("Parent executable name is: %s\n\r", procparent.ascii());
#endif
		if ((procparent == "ksmserver") || (procparent == "kdesktop_lock") || (procparent == "kdm")) {
			// OK, the calling process is authorized to retrieve SAK data
			// First, flush the buffer
			mPipe_fd = open(FIFO_FILE, O_RDWR | O_NONBLOCK);
			numread = 1;
			while (numread > 0) {
				numread = read(mPipe_fd, readbuf, 128);
			}
			// Now wait for SAK press
			mPipe_fd = open(FIFO_FILE, O_RDWR);
			if (mPipe_fd > -1) {
				numread = read(mPipe_fd, readbuf, 128);
				readbuf[numread] = 0;
				readbuf[127] = 0;
				close(mPipe_fd);
				if (strcmp(readbuf, "SAK\n\r") == 0) {
					return 0;
				}
				else {
					return 1;
				}
			}
		}
		else {
			printf("Unauthorized calling process detected\n\r");
			return 3;
		}
	}

	return 4;
}