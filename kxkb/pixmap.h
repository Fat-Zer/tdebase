#ifndef __PIXMAP_H__
#define __PIXMAP_H__


#include <tqpixmap.h>
#include <tqdict.h>
#include <tqstring.h>


class LayoutIcon {

private:
	static LayoutIcon* instance;
	static const TQString flagTemplate;
	
	TQDict<TQPixmap> m_pixmapCache;
	TQFont m_labelFont;

	LayoutIcon();
	TQPixmap* createErrorPixmap();
	void dimPixmap(TQPixmap& pixmap);
	TQString getCountryFromLayoutName(const TQString& tqlayoutName);
	
  public:
	static const TQString& ERROR_CODE;
	
	static LayoutIcon& getInstance();
	const TQPixmap& findPixmap(const TQString& code, bool showFlag, const TQString& displayName="");
};

#endif
