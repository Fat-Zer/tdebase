#include "config.h"

// Whether to enable PCRE regular expression support in blacklists, enabled
// by default
#cmakedefine CONFIG_REGEX_PCRE 1
// Whether to enable JIT support of libpcre. This may cause problems on PaX
// kernels.
#cmakedefine CONFIG_REGEX_PCRE_JIT 1
// Whether to enable parsing of configuration files using libconfig.
#cmakedefine CONFIG_LIBCONFIG 1
// Whether we are using a legacy version of libconfig (1.3.x).
#cmakedefine CONFIG_LIBCONFIG_LEGACY 1
// Whether to enable DRM VSync support
#cmakedefine CONFIG_VSYNC_DRM 1
// Whether to enable OpenGL support
#cmakedefine CONFIG_VSYNC_OPENGL 1
// Whether to enable GLX GLSL support
#cmakedefine CONFIG_VSYNC_OPENGL_GLSL 1
// Whether to enable GLX FBO support
#cmakedefine CONFIG_VSYNC_OPENGL_FBO 1
// Whether to enable DBus support with libdbus.
#define CONFIG_DBUS 1
// Whether to enable condition support.
#define CONFIG_C2 1
// Whether to enable X Sync support.
#define CONFIG_XSYNC 1
// Whether to enable GLX Sync support.
#cmakedefine CONFIG_GLX_XSYNC 1