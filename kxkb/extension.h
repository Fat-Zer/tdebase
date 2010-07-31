#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <X11/Xlib.h>


class XKBExtension
{
public:
	XKBExtension(Display *display=NULL);
	~XKBExtension();
	bool init();
	void reset();

	static bool setXkbOptions(const TQString& options, bool resetOldOptions);
	bool setLayout(const TQString& model,
					const TQString& layout, const TQString& variant,
					const TQString& includeGroup, bool useCompiledLayouts=true);
	bool setGroup(unsigned int group);
	unsigned int getGroup() const;

private:
    Display *m_dpy;
	TQString m_tempDir;
	static TQMap<TQString, FILE*> fileCache;
	
	bool setLayoutInternal(const TQString& model,
				   const TQString& layout, const TQString& variant,
				   const TQString& includeGroup);
	bool compileCurrentLayout(const TQString& layoutKey);
	bool setCompiledLayout(const TQString& layoutKey);
	
	TQString getPrecompiledLayoutFilename(const TQString& layoutKey);
//	void deletePrecompiledLayouts();
};

#endif
