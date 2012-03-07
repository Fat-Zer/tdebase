/**
 * \file man2html.h
 *
 * \note Despite that this file is installed publically, it should not be included
 * \todo ### KDE4: make this file private
 *
 */

#include <tqcstring.h>

/** call this with the buffer you have */
void scan_man_page(const char *man_page);

/**
 * Set the paths to TDE resources
 *
 * \param htmlPath Path to the TDE resources, encoded for HTML
 * \param cssPath Path to the TDE resources, encoded for CSS
 * \since 3.5
 *
 */
extern void setResourcePath(const TQCString& _htmlPath, const TQCString& _cssPath);

/** implement this somewhere. It will be called
   with HTML contents
*/
extern void output_real(const char *insert);

/**
 * called for requested man pages. filename can be a
 * relative path! Return NULL on errors. The returned
 * char array is freed by man2html
 */
extern char *read_man_page(const char *filename);
