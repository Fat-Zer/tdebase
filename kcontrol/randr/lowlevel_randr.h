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

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

struct ScreenInfo;

struct CrtcInfo {
	RRCrtc id;
	XRRCrtcInfo *info;
	int cur_x;
	int cur_y;
	RRMode cur_mode_id;
	Rotation cur_rotation;
	Rotation rotations;
	int cur_noutput;
	
	int changed;
	
	struct ScreenInfo *screen_info;
};

struct OutputInfo {
	RROutput id;
	XRROutputInfo *info;
	struct CrtcInfo *cur_crtc;
	
	int auto_set;
	int off_set;
};

struct ScreenInfo {
	Display *dpy;
	Window window;
	XRRScreenResources *res;
	int min_width, min_height;
	int max_width, max_height;
	int cur_width;
	int cur_height;
	int cur_mmWidth;
	int cur_mmHeight;
	
  	int n_output;
  	int n_crtc;
  	struct OutputInfo **outputs;
  	struct CrtcInfo **crtcs;
  	
  	int clone;
  	struct CrtcInfo *primary_crtc;
  	
  	struct CrtcInfo *cur_crtc;
  	struct OutputInfo *cur_output;
};

extern struct ScreenInfo *screen_info;
extern const uint big_pixbuf[], small_pixbuf[];

#ifdef __cplusplus
extern "C" {
#endif
void free_screen_info (struct ScreenInfo *screen_info);

struct ScreenInfo* read_screen_info (Display *);

int set_screen_size (struct ScreenInfo *screen_info);
void output_auto (struct ScreenInfo *screen_info, struct OutputInfo *output_info);
void output_off (struct ScreenInfo *screen_info, struct OutputInfo *output);
struct CrtcInfo* auto_find_crtc (struct ScreenInfo *screen_info, struct OutputInfo *output_info);

XRRModeInfo *find_mode_by_xid (struct ScreenInfo *screen_info, RRMode mode_id);
int mode_height (XRRModeInfo *mode_info, Rotation rotation);
int mode_width (XRRModeInfo *mode_info, Rotation rotation);
int get_width_by_output_id (struct ScreenInfo *screen_info, RROutput output_id);
int get_height_by_output_id (struct ScreenInfo *screen_info, RROutput output_id);
char *get_output_name (struct ScreenInfo *screen_info, RROutput id);
Status crtc_apply (struct CrtcInfo *crtc_info);
Status crtc_disable (struct CrtcInfo *crtc);
int main_low_apply (struct ScreenInfo *screen_info);

#ifdef __cplusplus
}
#endif