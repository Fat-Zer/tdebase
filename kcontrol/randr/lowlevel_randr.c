/*
 * Copyright © 2007 Intel Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "lowlevel_randr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Status crtc_disable (struct CrtcInfo *crtc);

char * get_output_name (struct ScreenInfo *screen_info, RROutput id)
{
	char *output_name = NULL;
	int i;
	
	for (i = 0; i < screen_info->n_output; i++) {
		if (id == screen_info->outputs[i]->id) {
			output_name = screen_info->outputs[i]->info->name;
		}
	}
	
	if (!output_name) {
		output_name = "Unknown";
	}
	
	return output_name;
}

XRRModeInfo * find_mode_by_xid (struct ScreenInfo *screen_info, RRMode mode_id)
{
	XRRModeInfo *mode_info = NULL;
	XRRScreenResources *res;
	int i;
	
	res = screen_info->res;
	for (i = 0; i < res->nmode; i++) {
		if (mode_id == res->modes[i].id) {
			mode_info = &res->modes[i];
			break;
		}
	}
	
	return mode_info;
}

static XRRCrtcInfo * find_crtc_by_xid (struct ScreenInfo *screen_info, RRCrtc crtc_id)
{
	XRRCrtcInfo *crtc_info;
	Display *dpy;
	XRRScreenResources *res;
	
	dpy = screen_info->dpy;
	res = screen_info->res;
	
	crtc_info = XRRGetCrtcInfo (dpy, res, crtc_id);
	
	return crtc_info;
}

int get_width_by_output_id (struct ScreenInfo *screen_info, RROutput output_id)
{
	struct OutputInfo *output_info;
	struct CrtcInfo *crtc_info;
	RRMode mode_id;
	XRRModeInfo *mode_info;
	int i;
	int width = -1;
	
	for (i = 0; i < screen_info->n_output; i++) {
		if (output_id == screen_info->outputs[i]->id) {
			crtc_info = screen_info->outputs[i]->cur_crtc;
			if (!crtc_info) {
				width = 0;
				break;
			}
			mode_id = crtc_info->cur_mode_id;
			mode_info = find_mode_by_xid (screen_info, mode_id);
			
			width = mode_width (mode_info, crtc_info->cur_rotation);
			
			break;
		}
	}
	
	return width;
}

int get_height_by_output_id (struct ScreenInfo *screen_info, RROutput output_id)
{
	struct OutputInfo *output_info;
	struct CrtcInfo *crtc_info;
	RRMode mode_id;
	XRRModeInfo *mode_info;
	int i;
	int height = -1;
	
	for (i = 0; i < screen_info->n_output; i++) {
		if (output_id == screen_info->outputs[i]->id) {
			crtc_info = screen_info->outputs[i]->cur_crtc;
			if (!crtc_info) {
				height = 0;
				break;
			}
			mode_id = crtc_info->cur_mode_id;
			mode_info = find_mode_by_xid (screen_info, mode_id);
			
			height = mode_height (mode_info, crtc_info->cur_rotation);
			
			break;
		}
	}
	
	return height;
}

int mode_height (XRRModeInfo *mode_info, Rotation rotation)
{
    switch (rotation & 0xf) {
    case RR_Rotate_0:
    case RR_Rotate_180:
        return mode_info->height;
    case RR_Rotate_90:
    case RR_Rotate_270:
        return mode_info->width;
    default:
        return 0;
    }
}

int mode_width (XRRModeInfo *mode_info, Rotation rotation)
{
    switch (rotation & 0xf) {
    case RR_Rotate_0:
    case RR_Rotate_180:
        return mode_info->width;
    case RR_Rotate_90:
    case RR_Rotate_270:
        return mode_info->height;
    default:
        return 0;
    }
}


static struct CrtcInfo * find_crtc (struct ScreenInfo *screen_info, XRROutputInfo *output)
{
	struct CrtcInfo *crtc_info = NULL;
	int i;
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		if (screen_info->crtcs[i]->id == output->crtc) {
			crtc_info = screen_info->crtcs[i];
			break;
		}
	}
	
	return crtc_info;
}

struct CrtcInfo * auto_find_crtc (struct ScreenInfo *screen_info, struct OutputInfo *output_info)
{
	struct CrtcInfo *crtc_info = NULL;
	int i;
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		if (0 == screen_info->crtcs[i]->cur_noutput) {
			crtc_info = screen_info->crtcs[i];
			break;
		}
	}
	
	if (NULL == crtc_info) {
		crtc_info = screen_info->crtcs[0];
	}
	
	return crtc_info;
}

int set_screen_size (struct ScreenInfo *screen_info)
{
	Display *dpy;
	int screen;
	struct CrtcInfo *crtc;
	XRRModeInfo *mode_info;
	int cur_x = 0, cur_y = 0;
	int w = 0, h = 0;
	int mmW, mmH;
	int max_width = 0, max_height = 0;
	int i;
	
	dpy = screen_info->dpy;
	screen = DefaultScreen (dpy);
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		crtc = screen_info->crtcs[i];
		if (!crtc->cur_mode_id) {
			continue;
		}
		mode_info = find_mode_by_xid (screen_info, crtc->cur_mode_id);
		cur_x = crtc->cur_x;
		cur_y = crtc->cur_y;
		
		w = mode_width (mode_info, crtc->cur_rotation);
		h = mode_height (mode_info, crtc->cur_rotation);
		
		if (cur_x + w > max_width) {
			max_width = cur_x + w;
		}
		if (cur_y + h > max_height) {
			max_height = cur_y + h;
		}
	}
	
		if (max_width > screen_info->max_width) {
	#if RANDR_GUI_DEBUG
			fprintf (stderr, "user set screen width %d, larger than max width %d, set to max width\n", 
						cur_x + w, screen_info->max_width);
	#endif
			return 0;
		} else if (max_width < screen_info->min_width) {
			screen_info->cur_width = screen_info->min_width;
		} else {
			screen_info->cur_width = max_width;
		} 
	
		if (max_height > screen_info->max_height) {
	#if RANDR_GUI_DEBUG
			fprintf (stderr, "user set screen height %d, larger than max height %d, set to max height\n", 
						cur_y + h, screen_info->max_height);
	#endif	
			return 0;
		} else if (max_height < screen_info->min_height) {
			screen_info->cur_height = screen_info->min_height;
		} else {
			screen_info->cur_height = max_height;
		}
	
	/* calculate mmWidth, mmHeight */
	if (screen_info->cur_width != DisplayWidth (dpy, screen) ||
		 screen_info->cur_height != DisplayHeight (dpy, screen) ) {
		double dpi; 
		
		dpi = (25.4 * DisplayHeight (dpy, screen)) / DisplayHeightMM(dpy, screen);
		mmW = (25.4 * screen_info->cur_width) / dpi;
		mmH = (25.4 * screen_info->cur_height) / dpi;
	} else {
		mmW = DisplayWidthMM (dpy, screen);
		mmH = DisplayHeightMM (dpy, screen);
	}

	screen_info->cur_mmWidth = mmW;
	screen_info->cur_mmHeight = mmH;
	
	return 1;
}

void screen_apply (struct ScreenInfo *screen_info)
{
	int width, height;
	int mmWidth, mmHeight;
	Display *dpy, *cur_dpy;
	Window window;
	int screen;
	static int first = 1;
	
	width = screen_info->cur_width;
	height = screen_info->cur_height;
	mmWidth = screen_info->cur_mmWidth;
	mmHeight = screen_info->cur_mmHeight;
	dpy = screen_info->dpy;
	window = screen_info->window;
	screen = DefaultScreen (dpy);
	
	cur_dpy = XOpenDisplay (NULL);

	if (width == DisplayWidth (cur_dpy, screen) &&
			height == DisplayHeight (cur_dpy, screen) &&
			mmWidth == DisplayWidthMM (cur_dpy, screen) &&
			mmHeight == DisplayHeightMM (cur_dpy, screen) ) {
		return;
	} else {
		XRRSetScreenSize (dpy, window, width, height, mmWidth, mmHeight);
	}
}

Status crtc_apply (struct CrtcInfo *crtc_info)
{
	struct ScreenInfo *screen_info;
	XRRCrtcInfo *rr_crtc_info;
	Display *dpy;
	XRRScreenResources *res;
	RRCrtc crtc_id;
	int x, y;
	RRMode mode_id;
	Rotation rotation;
	RROutput *outputs;
	int noutput;
	Status s;
	int i;
	
	/*if (!crtc_info->changed) {
		return RRSetConfigSuccess;
	}*/
	
	screen_info = crtc_info->screen_info;
	dpy = screen_info->dpy;
	res = screen_info->res;
	crtc_id = crtc_info->id;
	x = crtc_info->cur_x;
	y = crtc_info->cur_y;
	
	mode_id = crtc_info->cur_mode_id;
	rotation = crtc_info->cur_rotation;

	noutput = crtc_info->cur_noutput;
	
	if (0 == noutput) {
		return crtc_disable (crtc_info);
	}
	
	outputs = malloc (sizeof (RROutput) * noutput);
	noutput = 0;
	for (i = 0; i < screen_info->n_output; i++) {
		struct OutputInfo *output_info = screen_info->outputs[i];
		
		if (output_info->cur_crtc && crtc_id == output_info->cur_crtc->id) {
			outputs[noutput++] = output_info->id;
		}
	}

	
	s = XRRSetCrtcConfig (dpy, res, crtc_id, CurrentTime,
                              x, y, mode_id, rotation,
                              outputs, noutput);

	if (RRSetConfigSuccess == s) {
		crtc_info->changed = 0;
	} 
	
	free (outputs);
	
	return s;
}

Status crtc_disable (struct CrtcInfo *crtc)
{
	struct ScreenInfo *screen_info;
	
	screen_info = crtc->screen_info;
	
	return XRRSetCrtcConfig (screen_info->dpy, screen_info->res, crtc->id, CurrentTime,
                             0, 0, None, RR_Rotate_0, NULL, 0);
}

struct ScreenInfo* read_screen_info (Display *display)
{
	struct ScreenInfo *screen_info;
	int screen_num;
	Window root_window;
	XRRScreenResources *sr;
	int i;
	
	screen_num = DefaultScreen (display);
	root_window = RootWindow (display, screen_num);
	
	sr = XRRGetScreenResources (display, root_window);
	
	screen_info = malloc (sizeof (struct ScreenInfo));
	screen_info->dpy = display;
	screen_info->window = root_window;
	screen_info->res = sr;
	screen_info->cur_width = DisplayWidth (display, screen_num);
	screen_info->cur_height = DisplayHeight (display, screen_num);
	screen_info->cur_mmWidth = DisplayWidthMM (display, screen_num);
	screen_info->cur_mmHeight = DisplayHeightMM (display, screen_num);
	screen_info->n_output = sr->noutput;
	screen_info->n_crtc = sr->ncrtc;
	screen_info->outputs = malloc (sizeof (struct OutputInfo *) * sr->noutput);
	screen_info->crtcs = malloc (sizeof (struct CrtcInfo *) * sr->ncrtc);
	screen_info->clone = 0;
	
	XRRGetScreenSizeRange (display, root_window, &screen_info->min_width, &screen_info->min_height, &screen_info->max_width, &screen_info->max_height);
	
	/* get crtc */
	for (i = 0; i < sr->ncrtc; i++) {
		struct CrtcInfo *crtc_info;
		screen_info->crtcs[i] = malloc (sizeof (struct CrtcInfo));
		crtc_info = screen_info->crtcs[i];
		XRRCrtcInfo *xrr_crtc_info = XRRGetCrtcInfo (display, sr, sr->crtcs[i]);
		
		crtc_info->id = sr->crtcs[i];
		crtc_info->info = xrr_crtc_info;
		crtc_info->cur_x = xrr_crtc_info->x;
		crtc_info->cur_y = xrr_crtc_info->y;
		crtc_info->cur_mode_id = xrr_crtc_info->mode;
		crtc_info->cur_rotation = xrr_crtc_info->rotation;
		crtc_info->rotations = xrr_crtc_info->rotations;
		crtc_info->cur_noutput = xrr_crtc_info->noutput;
	
		crtc_info->changed = 0;
		crtc_info->screen_info = screen_info;
	}
	
	
	/* get output */
	for (i = 0; i < sr->noutput; i++) {
		struct OutputInfo *output;
		screen_info->outputs[i] = malloc (sizeof (struct OutputInfo));
		output = screen_info->outputs[i];
		
		output->id = sr->outputs[i];
		output->info = XRRGetOutputInfo (display, sr, sr->outputs[i]);
		output->cur_crtc = find_crtc (screen_info, output->info);
		output->auto_set = 0;
		if (output->cur_crtc) {
			output->off_set = 0;
		} else {
			output->off_set = 1;
		}
		
	}
	
	/* set current crtc */
	screen_info->cur_crtc = screen_info->outputs[0]->cur_crtc;
	screen_info->primary_crtc = screen_info->cur_crtc;
	screen_info->cur_output = screen_info->outputs[0];
	
	return screen_info;	
}

void free_screen_info (struct ScreenInfo *screen_info)
{
	free (screen_info->outputs);
	free (screen_info->crtcs);
	free (screen_info);
}



/*check if other outputs that connected to the same crtc support this mode*/
static int check_mode (struct ScreenInfo *screen_info, struct OutputInfo *output, RRMode mode_id)
{
	XRRCrtcInfo *crtc_info;
	/* XRR */
	int i, j;
	int mode_ok = 1;
	
	if (!output->cur_crtc) {
		return 1;
	}
	
	crtc_info = output->cur_crtc->info;
	for (i = 0; i < crtc_info->noutput; i++) {
		XRROutputInfo *output_info;
		int nmode;
		
		if (output->id == crtc_info->outputs[i]) {
			continue;
		}
		
		mode_ok = 0;
		output_info = XRRGetOutputInfo (screen_info->dpy, screen_info->res, crtc_info->outputs[i]);
		nmode = output_info->nmode;
		for (j = 0; j < nmode; j++) {
			if (mode_id == output_info->modes[j]) {
				mode_ok = 1;
				break;
			}
		}
		if (!mode_ok) {
			break;
		}
	}
	
	return mode_ok;
}

static RRCrtc get_crtc_id_by_output_id (struct ScreenInfo *screen_info, RROutput output_id)
{
	int i;
	RRCrtc crtc_id = -1;
	
	for (i = 0; i < screen_info->n_output; i++) {
		if (output_id == screen_info->outputs[i]->id) {
			if (screen_info->outputs[i]->cur_crtc) {
				crtc_id = screen_info->outputs[i]->cur_crtc->id;
			} else {
				crtc_id = 0;	/* this output is off */
			}
			break;
		}
	}
	
	return crtc_id;
}

static struct CrtcInfo *
get_crtc_info_by_xid (struct ScreenInfo *screen_info, RRCrtc crtc_id)
{
	struct CrtcInfo *crtc_info = NULL;
	int i;
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		if (crtc_id == screen_info->crtcs[i]->id) {
			crtc_info = screen_info->crtcs[i];
			break;
		}
	}
	
	return crtc_info;
}

static XRRModeInfo *
preferred_mode (struct ScreenInfo *screen_info, struct OutputInfo *output)
{
    XRROutputInfo   *output_info = output->info;
    Display *dpy;
    int screen;
    int             m;
    XRRModeInfo     *best;
    int             bestDist;

	 dpy = screen_info->dpy;
	 screen = DefaultScreen (dpy);
    best = NULL;
    bestDist = 0;
    for (m = 0; m < output_info->nmode; m++) {
        XRRModeInfo *mode_info = find_mode_by_xid (screen_info, output_info->modes[m]);
        int         dist;

        if (m < output_info->npreferred)
            dist = 0;
        else if (output_info->mm_height)
            dist = (1000 * DisplayHeight(dpy, screen) / DisplayHeightMM(dpy, screen) -
                    1000 * mode_info->height / output_info->mm_height);
        else
            dist = DisplayHeight(dpy, screen) - mode_info->height;

        if (dist < 0) dist = -dist;
        if (!best || dist < bestDist) {
            best = mode_info;
            bestDist = dist;
        	   }
    	}
    return best;
}

int main_low_apply (struct ScreenInfo *screen_info)
{
	int i;
	struct CrtcInfo *crtc_info;

	/* set_positions (screen_info); */
	
	if (!set_screen_size (screen_info)) {
		printf("Screen Size FAILURE\n\r");
		return 0;
	}
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		int old_x, old_y, old_w, old_h;
		
		XRRCrtcInfo *crtc_info = XRRGetCrtcInfo (screen_info->dpy, screen_info->res, screen_info->crtcs[i]->id);
		XRRModeInfo *old_mode = find_mode_by_xid (screen_info, crtc_info->mode);
		
		if (crtc_info->mode == None) {
			continue;
		}
		
		old_x = crtc_info->x;
		old_y = crtc_info->y;
		old_w = mode_width (old_mode, crtc_info->rotation);
		old_h = mode_height (old_mode, crtc_info->rotation);

		if (old_x + old_w <= screen_info->cur_width &&
			 old_y + old_h <= screen_info->cur_height ) {
			continue;	
		} else {
			crtc_disable (screen_info->crtcs[i]);
		}
	}
	
	screen_apply (screen_info);
	
	for (i = 0; i < screen_info->n_crtc; i++) {
		Status s;
		crtc_info = screen_info->crtcs[i];
		
			s = crtc_apply (crtc_info);
			if (RRSetConfigSuccess != s) {
				fprintf (stderr, "crtc apply error\n");
			}
	}
	
	return 1;
}

void output_auto (struct ScreenInfo *screen_info, struct OutputInfo *output_info)
{
	XRRModeInfo *mode_info;
	RRMode mode_id;
	struct CrtcInfo *crtc_info;
	XRROutputInfo *probe_output_info;
	
	if (RR_Disconnected == output_info->info->connection) {
		XRRScreenResources *cur_res;
		
		cur_res = XRRGetScreenResources (screen_info->dpy, screen_info->window);
		probe_output_info = XRRGetOutputInfo (screen_info->dpy, cur_res, output_info->id);
		if (RR_Disconnected != probe_output_info->connection) {
			output_info->info = probe_output_info;
			output_info->cur_crtc = auto_find_crtc (screen_info, output_info);
		}
	}
	
	mode_info = preferred_mode (screen_info, output_info);
	if (!mode_info) {
		return;
	}
	mode_id = mode_info->id;
	
	crtc_info = output_info->cur_crtc;
	if (crtc_info) {
		crtc_info->cur_mode_id = mode_id;
	} else {
		crtc_info = auto_find_crtc (screen_info, output_info);
		if (!crtc_info) {
#if RANDR_GUI_DEBUG
			fprintf (stderr, "Can not find usable CRTC\n");
#endif
			return;
		} else {
			screen_info->cur_output->cur_crtc = crtc_info;
			screen_info->cur_crtc = crtc_info;
			screen_info->cur_crtc->cur_noutput++;
			fprintf (stderr, "n output: %d\n", screen_info->cur_crtc->cur_noutput);
			screen_info->cur_crtc->cur_mode_id = mode_id;
			screen_info->cur_crtc->changed = 1;
		}
	}
	
}

void output_off (struct ScreenInfo *screen_info, struct OutputInfo *output)
{
	if (output->cur_crtc) {
		output->cur_crtc->cur_noutput--;
	}
	output->cur_crtc = NULL;
	screen_info->cur_crtc = NULL;
	output->off_set = 1;
}
