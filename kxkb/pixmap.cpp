#include <tqimage.h>
//#include <tqbitmap.h>
#include <tqfont.h>
#include <tqpainter.h>
#include <tqregexp.h>
#include <tqdict.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include "pixmap.h"
#include "x11helper.h"
#include "kxkbconfig.h"


static const int FLAG_MAX_WIDTH = 21;
static const int FLAG_MAX_HEIGHT = 14;

const TQString LayoutIcon::flagTemplate("l10n/%1/flag.png");
const TQString& LayoutIcon::ERROR_CODE("error");
LayoutIcon* LayoutIcon::instance;


LayoutIcon& LayoutIcon::getInstance() {
	if( instance == NULL ) {
		instance = new LayoutIcon();
	}
	return *instance;
}

LayoutIcon::LayoutIcon():
		m_pixmapCache(80),
		m_labelFont("sans")
{
	m_labelFont.setPixelSize(10);
	m_labelFont.setWeight(TQFont::Bold);
}

const TQPixmap&
LayoutIcon::findPixmap(const TQString& code_, bool showFlag, const TQString& displayName_)
{
	TQPixmap* pm = NULL;

	if( code_ == ERROR_CODE ) {
		pm = m_pixmapCache[ERROR_CODE];
		if( pm == NULL ) {
			pm = createErrorPixmap();
			m_pixmapCache.insert(ERROR_CODE, pm);
		}
		return *pm;
	}

	TQString displayName(displayName_);
	
	if( displayName.isEmpty() ) {
		displayName = KxkbConfig::getDefaultDisplayName(code_);
	}
	if( displayName.length() > 3 )
		displayName = displayName.left(3);

	const TQString pixmapKey( showFlag ? code_ + "." + displayName : displayName );
	
	pm = m_pixmapCache[pixmapKey];
	if( pm )
		return *pm;

	TQString flag;
	if( showFlag ) {
		TQString countryCode = getCountryFromLayoutName( code_ );
		flag = locate("locale", flagTemplate.arg(countryCode));
	}

	if( flag.isEmpty() ) {
		pm = new TQPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
		pm->fill(Qt::gray);
	}
	else {
		pm = new TQPixmap(flag);
		dimPixmap( *pm );

#if 0		
		if( pm->height() < FLAG_MAX_HEIGHT ) {
			TQPixmap* pix = new TQPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
			pix->fill( Qt::lightGray );
//			pix->fill( TQColor(tqRgba(127,127,127,255)) );
//			TQBitmap tqmask;
//			tqmask.fill(1);
//			pix->setMask(tqmask);
			
			int dy = (pix->height() - pm->height()) / 2;
			copyBlt( pix, 0, dy, pm, 0, 0, -1, -1 );
//			TQPixmap* px = new TQPixmap(21, 14);
//			px->convertFromImage(img);*/
			delete pm;
			pm = pix;
		}
#endif
	}

	TQPainter p(pm);
	p.setFont(m_labelFont);

	p.setPen(Qt::black);
	p.drawText(1, 1, pm->width(), pm->height()-2, Qt::AlignCenter, displayName);
	p.setPen(Qt::white);
	p.drawText(0, 0, pm->width(), pm->height()-2, Qt::AlignCenter, displayName);

	m_pixmapCache.insert(pixmapKey, pm);

	return *pm;
}

/**
@brief Try to get country code from tqlayout name in xkb before xorg 6.9.0
*/
TQString LayoutIcon::getCountryFromLayoutName(const TQString& tqlayoutName)
{
	TQString flag;
	
	if( X11Helper::areLayoutsClean() ) { // >= Xorg 6.9.0
		if( tqlayoutName == "mkd" )
			flag = "mk";
		else
		if( tqlayoutName == "srp" ) {
			TQString csFlagFile = locate("locale", flagTemplate.arg("cs"));
			flag = csFlagFile.isEmpty() ? "yu" : "cs";
		}
		else
			if( tqlayoutName.endsWith("/jp") )
				flag = "jp";
        else
            if( tqlayoutName == "trq" || tqlayoutName == "trf" || tqlayoutName == "tralt" )
                flag = "tr";
		else
			if( tqlayoutName.length() > 2 )
				flag = "";
		else
				flag = tqlayoutName;
	}
	else {
		if( tqlayoutName == "ar" )	// Arabic - not argentina
			;
		else
			if( tqlayoutName == "sr" || tqlayoutName == "cs")	// Serbian language - Yugoslavia
				flag = "yu";
		else
			if( tqlayoutName == "bs" )	// Bosnian language - Bosnia
				flag = "ba";
		else
			if( tqlayoutName == "la" )	// Latin America
				;
		else
			if( tqlayoutName == "lo" )	// Lao
				flag = "la";
		else
			if( tqlayoutName == "pl2" )	// Poland
				flag = "pl";
		else
			if( tqlayoutName == "iu" )	// Inuktitut - Canada
				flag = "ca";
		else
			if( tqlayoutName == "syr" )	// Syriac
				flag = "sy";
		else
			if( tqlayoutName == "dz" )	// Dzongka/Tibetian - Buthan
				flag = "bt";
		else
			if( tqlayoutName == "ogham" )	// Ogham - Ireland
				flag = "ie";
		else
			if( tqlayoutName == "ge_la" || tqlayoutName == "ge_ru" )
				flag = "ge";
		else
			if( tqlayoutName == "el" )
				flag = "gr";
		else
			if( tqlayoutName.endsWith("/jp") )
				flag = "jp";
		else
			if( tqlayoutName == "ml" || tqlayoutName == "dev" || tqlayoutName == "gur" 
						 || tqlayoutName == "guj" || tqlayoutName == "kan" || tqlayoutName == "ori" 
						 || tqlayoutName == "tel" || tqlayoutName == "tml" || tqlayoutName == "ben" ) // some Indian languages
				flag = "in";
		else {
			int sepPos = tqlayoutName.find(TQRegExp("[-_]"));
			TQString leftCode = tqlayoutName.mid(0, sepPos);
			TQString rightCode;
			if( sepPos != -1 )
				rightCode = tqlayoutName.mid(sepPos+1);
//			kdDebug() << "tqlayout name breakup: " << leftCode << ":" << rightCode << endl;
	
			if( rightCode.length() == 2 
					&& TQRegExp("[A-Z][A-Z]").exactMatch(rightCode) ) {
				flag = rightCode.lower();
			}
			else {
				flag = leftCode.length() == 2 ? leftCode : "";
			}
		}
	}
	
    return flag;
}


void LayoutIcon::dimPixmap(TQPixmap& pm)
{
	TQImage image = pm.convertToImage();
	for (int y=0; y<image.height(); y++)
		for(int x=0; x<image.width(); x++)
	{
		QRgb rgb = image.pixel(x,y);
		QRgb dimRgb(tqRgb(tqRed(rgb)*3/4, tqGreen(rgb)*3/4, tqBlue(rgb)*3/4));
		image.setPixel(x, y, dimRgb);
	}
	pm.convertFromImage(image);
}

static const char* ERROR_LABEL = "err";

//private
TQPixmap* LayoutIcon::createErrorPixmap()
{
	TQPixmap* pm = new TQPixmap(21, 14);
	pm->fill(Qt::white);

	TQPainter p(pm);

	p.setFont(m_labelFont);
	p.setPen(Qt::red);
	p.drawText(1, 1, pm->width(), pm->height()-2, Qt::AlignCenter, ERROR_LABEL);
	p.setPen(Qt::blue);
	p.drawText(0, 0, pm->width(), pm->height()-2, Qt::AlignCenter, ERROR_LABEL);
	m_pixmapCache.insert(ERROR_CODE, pm);

	return pm;
}


// Note: this seems stupid, but allows for translations
#if 0
   I18N_NOOP("Belgian");
   I18N_NOOP("Bulgarian");
   I18N_NOOP("Brazilian");
   I18N_NOOP("Canadian");
   I18N_NOOP("Czech");
   I18N_NOOP("Czech (qwerty)");
   I18N_NOOP("Danish");
   I18N_NOOP("Estonian");
   I18N_NOOP("Finnish");
   I18N_NOOP("French");
   I18N_NOOP("German");
   I18N_NOOP("Hungarian");
   I18N_NOOP("Hungarian (qwerty)");
   I18N_NOOP("Italian");
   I18N_NOOP("Japanese");
   I18N_NOOP("Lithuanian");
   I18N_NOOP("Norwegian");
   I18N_NOOP("PC-98xx Series");
   I18N_NOOP("Polish");
   I18N_NOOP("Portuguese");
   I18N_NOOP("Romanian");
   I18N_NOOP("Russian");
   I18N_NOOP("Slovak");
   I18N_NOOP("Slovak (qwerty)");
   I18N_NOOP("Spanish");
   I18N_NOOP("Swedish");
   I18N_NOOP("Swiss German");
   I18N_NOOP("Swiss French");
   I18N_NOOP("Thai");
   I18N_NOOP("United Kingdom");
   I18N_NOOP("U.S. English");
   I18N_NOOP("U.S. English w/ deadkeys");
   I18N_NOOP("U.S. English w/ISO9995-3");

  //lukas: these seem to be new in XF 4.0.2
   I18N_NOOP("Armenian");
   I18N_NOOP("Azerbaijani");
   I18N_NOOP("Icelandic");
   I18N_NOOP("Israeli");
   I18N_NOOP("Lithuanian azerty standard");
   I18N_NOOP("Lithuanian querty \"numeric\"");	     //for bw compatibility
   I18N_NOOP("Lithuanian querty \"programmer's\"");
   I18N_NOOP("Macedonian");
   I18N_NOOP("Serbian");
   I18N_NOOP("Slovenian");
   I18N_NOOP("Vietnamese");

  //these seem to be new in XFree86 4.1.0
   I18N_NOOP("Arabic");
   I18N_NOOP("Belarusian");
   I18N_NOOP("Bengali");
   I18N_NOOP("Croatian");
   I18N_NOOP("Greek");
   I18N_NOOP("Latvian");
   I18N_NOOP("Lithuanian qwerty \"numeric\"");
   I18N_NOOP("Lithuanian qwerty \"programmer's\"");
   I18N_NOOP("Turkish");
   I18N_NOOP("Ukrainian");

  //these seem to be new in XFree86 4.2.0
   I18N_NOOP("Albanian");
   I18N_NOOP("Burmese");
   I18N_NOOP("Dutch");
   I18N_NOOP("Georgian (latin)");
   I18N_NOOP("Georgian (russian)");
   I18N_NOOP("Gujarati");
   I18N_NOOP("Gurmukhi");
   I18N_NOOP("Hindi");
   I18N_NOOP("Inuktitut");
   I18N_NOOP("Iranian");
//   I18N_NOOP("Iranian"); // should be not Iranian but Farsi
   I18N_NOOP("Latin America");
   I18N_NOOP("Maltese");
   I18N_NOOP("Maltese (US tqlayout)");
   I18N_NOOP("Northern Saami (Finland)");
   I18N_NOOP("Northern Saami (Norway)");
   I18N_NOOP("Northern Saami (Sweden)");
   I18N_NOOP("Polish (qwertz)");
   I18N_NOOP("Russian (cyrillic phonetic)");
   I18N_NOOP("Tajik");
   I18N_NOOP("Turkish (F)");
   I18N_NOOP("U.S. English w/ ISO9995-3");
   I18N_NOOP("Yugoslavian");

  //these seem to be new in XFree86 4.3.0
   I18N_NOOP("Bosnian");
   I18N_NOOP("Croatian (US)");
   I18N_NOOP("Dvorak");
   I18N_NOOP("French (alternative)");
   I18N_NOOP("French Canadian");
   I18N_NOOP("Kannada");
   I18N_NOOP("Lao");
   I18N_NOOP("Malayalam");
   I18N_NOOP("Mongolian");
   I18N_NOOP("Ogham");
   I18N_NOOP("Oriya");
   I18N_NOOP("Syriac");
   I18N_NOOP("Telugu");
   I18N_NOOP("Thai (Kedmanee)");
   I18N_NOOP("Thai (Pattachote)");
   I18N_NOOP("Thai (TIS-820.2538)");

  //these seem to be new in XFree86 4.4.0
   I18N_NOOP("Uzbek");
   I18N_NOOP("Faroese");

  //these seem to be new in XOrg 6.8.2
   I18N_NOOP("Dzongkha / Tibetan");
   I18N_NOOP("Hungarian (US)");
   I18N_NOOP("Irish");
   I18N_NOOP("Israeli (phonetic)");
   I18N_NOOP("Serbian (Cyrillic)");
   I18N_NOOP("Serbian (Latin)");
   I18N_NOOP("Swiss");
#endif
