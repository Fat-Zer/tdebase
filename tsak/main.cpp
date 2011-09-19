/*
Copyright 2010 Adam Marchetti
Copyright 2011 Timothy Pearson <kb9vqf@pearsoncomputing.net>

This file is part of tsak.

tsak is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

tsak is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with tsak. If not, see http://www.gnu.org/licenses/.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

#define FIFO_DIR "/tmp/ksocket-global"
#define FIFO_FILE_OUT "/tmp/ksocket-global/tsak"

#define TestBit(bit, array) (array[(bit) / 8] & (1 << ((bit) % 8)))

typedef unsigned char byte;

bool        mPipeOpen_out = false;
int         mPipe_fd_out = -1;

struct sigaction usr_action;
sigset_t block_mask;

const char *keycode[256] =
{
	"", "<esc>", "1", "2", "3", "4", "5", "6", "7", "8",
	"9", "0", "−", "=", "<backspace>", "<tab>", "q", "w", "e", "r",
	"t", "y", "u", "i", "o", "p", "[", "]", "\n", "<control>",
	"a", "s", "d", "f", "g", "h", "j", "k", "l", ";",
	"'", "", "<shift>", "\\", "z", "x", "c", "v", "b", "n",
	"m", ",", ".", "/", "<shift>", "", "<alt>", " ", "<capslock>",
	"<f1>", "<f2>", "<f3>", "<f4>", "<f5>", "<f6>", "<f7>", "<f8>", "<f9>", "<f10>",
	"<numlock>", "<scrolllock>", "", "", "", "", "", "", "", "",
	"", "", "\\", "f11", "f12", "", "", "", "", "",
	"", "", "", "<control>", "", "<sysrq>", "", "", "<control>", "", "",
	"<alt>", "", "", "", "", "", "", "", "", "",
	"", "<del>", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", ""
};

/* returns 1 if bit number i is set, otherwise returns 0 */
int bit_set(size_t i, const byte* a)
{
  return a[i/CHAR_BIT] & (1 << i%CHAR_BIT);
}

/* Assign features (supported axes and keys) of the physical input device (devin)
 * to the virtual input device (devout) */
static void copy_features(int devin, int devout)
{
	byte evtypes[EV_MAX/CHAR_BIT + 1] = {0};
	byte codes[KEY_MAX/CHAR_BIT + 1];
	unsigned i,code;
	int op;
	if (ioctl(devin, EVIOCGBIT(0, sizeof(evtypes)), evtypes) < 0) return;
	for(i=0;i<EV_MAX;++i) {
		if (bit_set(i, evtypes)) {
			switch(i) {
			case EV_KEY: op = UI_SET_KEYBIT; break;
			case EV_REL: op = UI_SET_RELBIT; break;
			case EV_ABS: op = UI_SET_ABSBIT; break;
			case EV_MSC: op = UI_SET_MSCBIT; break;
			case EV_LED: op = UI_SET_LEDBIT; break;
			case EV_SND: op = UI_SET_SNDBIT; break;
			case EV_SW: op = UI_SET_SWBIT; break;
			default: op = -1;
		}
	}
	if (op == -1) continue;
	ioctl(devout, UI_SET_EVBIT, i);
	memset(codes,0,sizeof(codes));
	if (ioctl(devin, EVIOCGBIT(i, sizeof(codes)), codes) < 0) return;
	for(code=0;code<KEY_MAX;code++) {
		if (bit_set(code, codes)) ioctl(devout, op, code);
		}
	}
}

int find_keyboard() {
	int i, j;
	int fd;
	char filename[32];
	char key_bitmask[(KEY_MAX + 7) / 8];
	
	for (i=0; i<32; i++) {
		snprintf(filename,sizeof(filename), "/dev/input/event%d", i);
	
		fd = open(filename, O_RDWR|O_SYNC);
		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	
		/* We assume that anything that has an alphabetic key in the
			QWERTYUIOP range in it is the main keyboard. */
		for (j = KEY_Q; j <= KEY_P; j++) {
			if (TestBit(j, key_bitmask))
				return fd;
		}
	
		close (fd);
	}
	return 0;
}

void tearDownPipe()
{
	if (mPipeOpen_out == true) {
		mPipeOpen_out = false;
		close(mPipe_fd_out);
		unlink(FIFO_FILE_OUT);
	}
}

bool setFileLock(int fd, bool close_on_failure)
{
	struct flock fl;
	
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;

	// Set the exclusive file lock
	if (fcntl(mPipe_fd_out, F_SETLK, &fl) == -1) {
		close(mPipe_fd_out);
		return false;
	}

	return true;
}

bool checkFileLock()
{
	struct flock fl;

	fl.l_type    = F_WRLCK;   /* Test for any lock on any part of file. */
	fl.l_start   = 0;
	fl.l_whence  = SEEK_SET;
	fl.l_len     = 0;

	int fd = open(FIFO_FILE_OUT, O_RDWR | O_NONBLOCK);
	fcntl(fd, F_GETLK, &fl);  /* Overwrites lock structure with preventors. */

	if (fd > -1) {
		if (fl.l_type == F_WRLCK) {
			return false;
		}
		return true;
	}

	return true;
}

bool setupPipe()
{
	/* Create the FIFOs if they do not exist */
	umask(0);
	mkdir(FIFO_DIR,0644);
	
	mknod(FIFO_FILE_OUT, S_IFIFO|0600, 0);
	chmod(FIFO_FILE_OUT, 0600);
	
	mPipe_fd_out = open(FIFO_FILE_OUT, O_RDWR | O_NONBLOCK);
	if (mPipe_fd_out > -1) {
		mPipeOpen_out = true;
	}

	// Set the exclusive file lock
	return setFileLock(mPipe_fd_out, true);
}

class PipeHandler
{
public:
	PipeHandler();
	~PipeHandler();
};

PipeHandler::PipeHandler()
{
}

PipeHandler::~PipeHandler()
{
	tearDownPipe();
}

int main (int argc, char *argv[])
{
	struct input_event ev[64];
	struct input_event event;
	struct uinput_user_dev devinfo={0};
	int fd, devout, rd, value, size = sizeof (struct input_event);
	char name[256] = "Unknown";
	bool ctrl_down = false;
	bool alt_down = false;
	bool hide_event = false;
	bool established = false;
	bool testrun = false;

	if (argc == 2) {
		if (strcmp(argv[1], "checkactive") == 0) {
			testrun = true;
		}
	}

	// Check for existing file locks
	if (!checkFileLock()) {
		fprintf(stderr, "Another instance of this program is already running\n");
		return 8;
	}

	// Create the output pipe
	PipeHandler controlpipe;
	if (!setupPipe()) {
		fprintf(stderr, "Another instance of this program is already running\n");
		return 8;
	}

	while (1) {
		if ((getuid ()) != 0) {
			printf ("You are not root! This WILL NOT WORK!\nDO NOT attempt to bypass security restrictions, e.g. by changing keyboard permissions or owner, if you want the SAK system to remain secure...\n");
			return 5;
		}
	
		// Open Device
		fd = find_keyboard();
		if (fd == -1) {
			printf ("Could not find your keyboard!\n");
			if (established)
				sleep(1);
			else
				return 4;
		}
		else {
			// Print Device Name
			ioctl (fd, EVIOCGNAME (sizeof (name)), name);
			fprintf(stderr, "Reading From : (%s)\n", name);
		
			// Create filtered virtual output device
			devout=open("/dev/misc/uinput",O_WRONLY|O_NONBLOCK);
			if (devout<0) {
				devout=open("/dev/uinput",O_WRONLY|O_NONBLOCK);
			}
			if (devout<0) {
				fprintf(stderr,"Unable to open /dev/uinput or /dev/misc/uinput (char device 10:223).\nPossible causes: Device node inexistent or kernel not compiled with evdev user level driver support or permission denied.\n");
				perror("open(\"/dev/misc/uinput\")");
				if (established)
					sleep(1);
				else
					return 3;
			}
			else {
				if(ioctl(fd, EVIOCGRAB, 2) < 0) {
					close(fd);
					fprintf(stderr, "Failed to grab exclusive input device lock");
					if (established)
						sleep(1);
					else
						return 1;
				}
				else {
					ioctl(fd, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), devinfo.name);
					strncat(devinfo.name, "+tsak", UINPUT_MAX_NAME_SIZE-1);
					fprintf(stderr, "%s\n", devinfo.name);
					ioctl(fd, EVIOCGID, &devinfo.id);
					
					copy_features(fd, devout);
					write(devout,&devinfo,sizeof(devinfo));
					if (ioctl(devout,UI_DEV_CREATE)<0) {
						fprintf(stderr,"Unable to create input device with UI_DEV_CREATE\n");
						if (established)
							sleep(1);
						else
							return 2;
					}
					else {
						fprintf(stderr,"Device created.\n");

						if (established == false) {
							tearDownPipe();
							int i=fork();
							if (i<0) return 9; // fork failed
							if (i>0) {
								// close parent process
								close(mPipe_fd_out);
								return 0;
							}
							setupPipe();
						}

						established = true;

						if (testrun == true) {
							return 0;
						}
				
						while (1) {
							if ((rd = read (fd, ev, size * 2)) < size) {
								fprintf(stderr,"Read failed.\n");
								break;
							}
							
							value = ev[0].value;
							
							if (value != ' ' && ev[1].value == 0 && ev[1].type == 1){ // Read the key release event
								if (keycode[(ev[1].code)]) {
									if (strcmp(keycode[(ev[1].code)], "<control>") == 0) ctrl_down = false;
									if (strcmp(keycode[(ev[1].code)], "<alt>") == 0) alt_down = false;
								}
							}
							if (value != ' ' && ev[1].value == 1 && ev[1].type == 1){ // Read the key press event
								if (keycode[(ev[1].code)]) {
									if (strcmp(keycode[(ev[1].code)], "<control>") == 0) ctrl_down = true;
									if (strcmp(keycode[(ev[1].code)], "<alt>") == 0) alt_down = true;
								}
							}
					
							hide_event = false;
							if (keycode[(ev[1].code)]) {
								if (alt_down && ctrl_down && (strcmp(keycode[(ev[1].code)], "<del>") == 0)) {
									hide_event = true;
								}
							}
					
							if (hide_event == false) {
								// Pass the event on...
								event = ev[0];
								write(devout, &event, sizeof event);
								event = ev[1];
								write(devout, &event, sizeof event);
							}
							if (hide_event == true) {
								// Let anyone listening to our interface know that an SAK keypress was received
								// I highly doubt there are more than 255 VTs active at once...
								int i;
								for (i=0;i<255;i++) {
									write(mPipe_fd_out, "SAK\n\r", 6);
								}
							}
						}
					}
				}
			}
		}
	}

	return 6;
}
