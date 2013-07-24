/*
Copyright 2011-2013 Timothy Pearson <kb9vqf@pearsoncomputing.net>

This file is part of tdekbdledsync, the TDE Keyboard LED Synchronization Daemon

tdekbdledsync is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

tdekbdledsync is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with tdekbdledsync. If not, see http://www.gnu.org/licenses/.

*/

// The idea here is to periodically read the Xorg core keyboard state, and then forcibly set the physical LED states on all attached keyboards to match (via the event interface)
// Once every half second should work well enough on most systems

#include <stdio.h>
#include <stdlib.h>
#include <exception>
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
extern "C" {
#include <libudev.h>
}
#include <libgen.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

using namespace std;

// WARNING
// MAX_KEYBOARDS must be greater than or equal to MAX_INPUT_NODE
#define MAX_KEYBOARDS 128
#define MAX_INPUT_NODE 128

#define TestBit(bit, array) (array[(bit) / 8] & (1 << ((bit) % 8)))

typedef unsigned char byte;

char filename[32];
char key_bitmask[(KEY_MAX + 7) / 8];

int keyboard_fd_num;
int keyboard_fds[MAX_KEYBOARDS];

Display* display = NULL;

int find_keyboards() {
	int i, j;
	int fd;

	keyboard_fd_num = 0;
	for (i=0; i<MAX_KEYBOARDS; i++) {
		keyboard_fds[i] = 0;
	}

	for (i=0; i<MAX_INPUT_NODE; i++) {
		snprintf(filename, sizeof(filename), "/dev/input/event%d", i);

		fd = open(filename, O_RDWR|O_SYNC);
		if (fd >= 0) {
			ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	
			struct input_id input_info;
			ioctl (fd, EVIOCGID, &input_info);
			if ((input_info.vendor != 0) && (input_info.product != 0)) {
				/* We assume that anything that has an alphabetic key in the
					QWERTYUIOP range in it is the main keyboard. */
				for (j = KEY_Q; j <= KEY_P; j++) {
					if (TestBit(j, key_bitmask)) {
						keyboard_fds[keyboard_fd_num] = fd;
					}
				}
			}
	
			if (keyboard_fds[keyboard_fd_num] == 0) {
				close(fd);
			}
			else {
				keyboard_fd_num++;
			}
		}
	}
	return 0;
}

int main() {
	int current_keyboard;
	char name[256] = "Unknown";
	unsigned int states;
	struct input_event ev;

	bool num_lock_set = false;
	bool caps_lock_set = false;
	bool scroll_lock_set = false;

	// Open X11 display
	display = XOpenDisplay(NULL);
	if (!display) {
		printf ("[tdekbdledsync] Unable to open X11 display!\n");
		return -1;
	}

	// Find keyboards
	find_keyboards();
	if (keyboard_fd_num == 0) {
		printf ("[tdekbdledsync] Could not find any usable keyboard(s)!\n");
		return -2;
	}
	else {
		fprintf(stderr, "[tdekbdledsync] Found %d keyboard(s)\n", keyboard_fd_num);

		for (current_keyboard=0;current_keyboard<keyboard_fd_num;current_keyboard++) {
			// Print device name
			ioctl(keyboard_fds[current_keyboard], EVIOCGNAME (sizeof (name)), name);
			fprintf(stderr, "[tdekbdledsync] Syncing keyboard: (%s)\n", name);
		}

		while (1) {
			// Get Virtual Core keyboard status
			if (XkbGetIndicatorState(display, XkbUseCoreKbd, &states) != Success) {
				fprintf(stderr, "[tdekbdledsync] Unable to query X11 Virtual Core keyboard!\n");
				return -3;
			}
	
			// From "xset -q"
			caps_lock_set = (states & 0x01);
			num_lock_set = (states & 0x02);
			scroll_lock_set = (states & 0x04);
	
			for (current_keyboard=0;current_keyboard<keyboard_fd_num;current_keyboard++) {
				// Set LEDs
				ev.type = EV_LED;
				ev.code = LED_CAPSL;
				ev.value = caps_lock_set;
				if (write(keyboard_fds[current_keyboard], &ev, sizeof(ev)) < 0) {
					fprintf(stderr, "[tdekbdledsync] Unable to set LED state\n");
				}
	
				ev.type = EV_LED;
				ev.code = LED_NUML;
				ev.value = num_lock_set;
				if (write(keyboard_fds[current_keyboard], &ev, sizeof(ev)) < 0) {
					fprintf(stderr, "[tdekbdledsync] Unable to set LED state\n");
				}
	
				ev.type = EV_LED;
				ev.code = LED_SCROLLL;
				ev.value = scroll_lock_set;
				if (write(keyboard_fds[current_keyboard], &ev, sizeof(ev)) < 0) {
					fprintf(stderr, "[tdekbdledsync] Unable to set LED state\n");
				}
			}

			usleep(500*1000);
		}
	}

	return 0;
}