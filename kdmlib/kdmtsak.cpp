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

#include "kdmtsak.h"

#define FIFO_FILE "/tmp/ksocket-global/tsak"

int main (int argc, char *argv[])
{
	int mPipe_fd;
	char readbuf[128];
	int numread;

	int verifier_result = tde_sak_verify_calling_process();

	if (verifier_result == 0) {
			// OK, the calling process is authorized to retrieve SAK data
			// First, flush the buffer
			mPipe_fd = open(FIFO_FILE, O_RDONLY | O_NONBLOCK);
			numread = 1;
			while (numread > 0) {
				numread = read(mPipe_fd, readbuf, 128);
			}
			// Now wait for SAK press
			mPipe_fd = open(FIFO_FILE, O_RDONLY);
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
			return 6;
	}
	else {
		return verifier_result;
	}
}