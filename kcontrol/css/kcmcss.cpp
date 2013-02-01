
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqtextbrowser.h>

#include <kapplication.h>
#include <kcolorbutton.h>
#include <tdeconfig.h>
#include <kdialogbase.h>
#include <kfontdialog.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "cssconfig.h"
#include "csscustom.h"
#include "template.h"
#include "preview.h"

#include "kcmcss.h"

typedef KGenericFactory<CSSConfig, TQWidget> CSSFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_css, CSSFactory("kcmcss") )

CSSConfig::CSSConfig(TQWidget *parent, const char *name, const TQStringList &)
  : TDECModule(CSSFactory::instance(), parent, name)
{
  customDialogBase = new KDialogBase(this, "customCSSDialog", true, TQString::null, 
        KDialogBase::Close, KDialogBase::Close, true );
  customDialog = new CSSCustomDialog(customDialogBase);
  customDialogBase->setMainWidget(customDialog);
  configDialog = new CSSConfigDialog(this);

  setQuickHelp( i18n("<h1>Konqueror Stylesheets</h1> This module allows you to apply your own color"
              " and font settings to Konqueror by using"
              " stylesheets (CSS). You can either specify"
              " options or apply your own self-written"
              " stylesheet by pointing to its location.<br>"
              " Note that these settings will always have"
              " precedence before all other settings made"
              " by the site author. This can be useful to"
              " visually impaired people or for web pages"
              " that are unreadable due to bad design."));


  TQStringList fonts;
  TDEFontChooser::getFontList(fonts, 0);
  customDialog->fontFamily->insertStringList(fonts);

  connect(configDialog->useDefault, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(configDialog->useAccess, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(configDialog->useUser, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(configDialog->urlRequester, TQT_SIGNAL(textChanged(const TQString&)),
	  TQT_SLOT(changed()));
  connect(configDialog->customize, TQT_SIGNAL(clicked()),
          TQT_SLOT(slotCustomize()));
  connect(customDialog->basefontsize, TQT_SIGNAL(highlighted(int)),
	  TQT_SLOT(changed()));
  connect(customDialog->basefontsize, TQT_SIGNAL(textChanged(const TQString&)),
	  TQT_SLOT(changed()));
  connect(customDialog->dontScale, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->blackOnWhite, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->whiteOnBlack, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->customColor, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->foregroundColor, TQT_SIGNAL(changed(const TQColor &)),
	  TQT_SLOT(changed()));
  connect(customDialog->backgroundColor, TQT_SIGNAL(changed(const TQColor &)),
	  TQT_SLOT(changed()));
  connect(customDialog->fontFamily, TQT_SIGNAL(highlighted(int)),
	  TQT_SLOT(changed()));
  connect(customDialog->fontFamily, TQT_SIGNAL(textChanged(const TQString&)),
	  TQT_SLOT(changed()));
  connect(customDialog->sameFamily, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->preview, TQT_SIGNAL(clicked()),
          TQT_SLOT(slotPreview()));
  connect(customDialog->sameColor, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->hideImages, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));
  connect(customDialog->hideBackground, TQT_SIGNAL(clicked()),
	  TQT_SLOT(changed()));

  TQVBoxLayout *vbox = new TQVBoxLayout(this, 0, 0);
  vbox->addWidget(configDialog);

  load();
}

void CSSConfig::load()
{
   load( false );
}

void CSSConfig::load( bool useDefaults )
{
  TDEConfig *c = new TDEConfig("kcmcssrc", false, false);
  c->setReadDefaults( useDefaults );

  c->setGroup("Stylesheet");
  TQString u = c->readEntry("Use", "default");
  configDialog->useDefault->setChecked(u == "default");
  configDialog->useUser->setChecked(u == "user");
  configDialog->useAccess->setChecked(u == "access");
  configDialog->urlRequester->setURL(c->readEntry("SheetName"));

  c->setGroup("Font");
  customDialog->basefontsize->setEditText(TQString::number(c->readNumEntry("BaseSize", 12)));
  customDialog->dontScale->setChecked(c->readBoolEntry("DontScale", false));

  TQString fname = c->readEntry("Family", "Arial");
  for (int i=0; i < customDialog->fontFamily->count(); ++i)
    if (customDialog->fontFamily->text(i) == fname)
      {
	customDialog->fontFamily->setCurrentItem(i);
	break;
      }

  customDialog->sameFamily->setChecked(c->readBoolEntry("SameFamily", false));

  c->setGroup("Colors");
  TQString m = c->readEntry("Mode", "black-on-white");
  customDialog->blackOnWhite->setChecked(m == "black-on-white");
  customDialog->whiteOnBlack->setChecked(m == "white-on-black");
  customDialog->customColor->setChecked(m == "custom");
  customDialog->backgroundColor->setColor(c->readColorEntry("BackColor", &TQt::white));
  customDialog->foregroundColor->setColor(c->readColorEntry("ForeColor", &TQt::black));
  customDialog->sameColor->setChecked(c->readBoolEntry("SameColor", false));

  // Images
  c->setGroup("Images");
  customDialog->hideImages->setChecked(c->readBoolEntry("Hide", false));
  customDialog->hideBackground->setChecked(c->readBoolEntry("HideBackground", true));

  delete c;

  emit changed( useDefaults );
}


void CSSConfig::save()
{
  // write to config file
  TDEConfig *c = new TDEConfig("kcmcssrc", false, false);

  c->setGroup("Stylesheet");
  if (configDialog->useDefault->isChecked())
    c->writeEntry("Use", "default");
  if (configDialog->useUser->isChecked())
    c->writeEntry("Use", "user");
  if (configDialog->useAccess->isChecked())
    c->writeEntry("Use", "access");
  c->writeEntry("SheetName", configDialog->urlRequester->url());

  c->setGroup("Font");
  c->writeEntry("BaseSize", customDialog->basefontsize->currentText());
  c->writeEntry("DontScale", customDialog->dontScale->isChecked());
  c->writeEntry("SameFamily", customDialog->sameFamily->isChecked());
  c->writeEntry("Family", customDialog->fontFamily->currentText());

  c->setGroup("Colors");
  if (customDialog->blackOnWhite->isChecked())
    c->writeEntry("Mode", "black-on-white");
  if (customDialog->whiteOnBlack->isChecked())
    c->writeEntry("Mode", "white-on-black");
  if (customDialog->customColor->isChecked())
    c->writeEntry("Mode", "custom");
  c->writeEntry("BackColor", customDialog->backgroundColor->color());
  c->writeEntry("ForeColor", customDialog->foregroundColor->color());
  c->writeEntry("SameColor", customDialog->sameColor->isChecked());

  c->setGroup("Images");
  c->writeEntry("Hide", customDialog->hideImages->isChecked());
  c->writeEntry("HideBackground", customDialog->hideBackground->isChecked());

  c->sync();
  delete c;

  // generate CSS template
  TQString templ = locate("data", "kcmcss/template.css");
  TQString dest;
  if (!templ.isEmpty())
    {
      CSSTemplate css(templ);

      dest = kapp->dirs()->saveLocation("data", "kcmcss");
      dest += "/override.css";

      css.expand(dest, cssDict());
    }

  // make konqueror use the right stylesheet
  c = new TDEConfig("konquerorrc", false, false);

  c->setGroup("HTML Settings");
  c->writeEntry("UserStyleSheetEnabled", !configDialog->useDefault->isChecked());

  if (configDialog->useUser->isChecked())
    c->writeEntry("UserStyleSheet", configDialog->urlRequester->url());
  if (configDialog->useAccess->isChecked())
    c->writeEntry("UserStyleSheet", dest);

  c->sync();
  delete c;
  emit changed(false);
}


void CSSConfig::defaults()
{
   load( true );
}


TQString px(int i, double scale)
{
  TQString px;
  px.setNum(static_cast<int>(i * scale));
  px += "px";
  return px;
}


TQMap<TQString,TQString> CSSConfig::cssDict()
{
  TQMap<TQString,TQString> dict;

  // Fontsizes ------------------------------------------------------

  int bfs = customDialog->basefontsize->currentText().toInt();
  dict.insert("fontsize-base", px(bfs, 1.0));

  if (customDialog->dontScale->isChecked())
    {
      dict.insert("fontsize-small-1", px(bfs, 1.0));
      dict.insert("fontsize-large-1", px(bfs, 1.0));
      dict.insert("fontsize-large-2", px(bfs, 1.0));
      dict.insert("fontsize-large-3", px(bfs, 1.0));
      dict.insert("fontsize-large-4", px(bfs, 1.0));
      dict.insert("fontsize-large-5", px(bfs, 1.0));
    }
  else
    {
      // TODO: use something harmonic here
      dict.insert("fontsize-small-1", px(bfs, 0.8));
      dict.insert("fontsize-large-1", px(bfs, 1.2));
      dict.insert("fontsize-large-2", px(bfs, 1.4));
      dict.insert("fontsize-large-3", px(bfs, 1.5));
      dict.insert("fontsize-large-4", px(bfs, 1.6));
      dict.insert("fontsize-large-5", px(bfs, 1.8));
    }

  // Colors --------------------------------------------------------

  if (customDialog->blackOnWhite->isChecked())
    {
      dict.insert("background-color", "White");
      dict.insert("foreground-color", "Black");
    }
  else if (customDialog->whiteOnBlack->isChecked())
    {
      dict.insert("background-color", "Black");
      dict.insert("foreground-color", "White");
    }
  else
    {
      dict.insert("background-color", customDialog->backgroundColor->color().name());
      dict.insert("foreground-color", customDialog->foregroundColor->color().name());
    }

  if (customDialog->sameColor->isChecked())
    dict.insert("force-color", "! important");
  else
    dict.insert("force-color", "");

  // Fonts -------------------------------------------------------------
  dict.insert("font-family", customDialog->fontFamily->currentText());
  if (customDialog->sameFamily->isChecked())
    dict.insert("force-font", "! important");
  else
    dict.insert("force-font", "");

  // Images

  if (customDialog->hideImages->isChecked())
    dict.insert("display-images", "background-image : none ! important");
  else
    dict.insert("display-images", "");
  if (customDialog->hideBackground->isChecked())
    dict.insert("display-background", "background-image : none ! important");
  else
    dict.insert("display-background", "");

  return dict;
}


void CSSConfig::slotCustomize()
{
  customDialogBase->exec();
}


void CSSConfig::slotPreview()
{

  TQStyleSheetItem *h1 = new TQStyleSheetItem(TQStyleSheet::defaultSheet(), "h1");
  TQStyleSheetItem *h2 = new TQStyleSheetItem(TQStyleSheet::defaultSheet(), "h2");
  TQStyleSheetItem *h3 = new TQStyleSheetItem(TQStyleSheet::defaultSheet(), "h3");
  TQStyleSheetItem *text = new TQStyleSheetItem(TQStyleSheet::defaultSheet(), "p");

  // Fontsize

  int bfs = customDialog->basefontsize->currentText().toInt();
  text->setFontSize(bfs);
  if (customDialog->dontScale->isChecked())
    {
      h1->setFontSize(bfs);
      h2->setFontSize(bfs);
      h3->setFontSize(bfs);
    }
  else
    {
      h1->setFontSize(static_cast<int>(bfs * 1.8));
      h2->setFontSize(static_cast<int>(bfs * 1.6));
      h3->setFontSize(static_cast<int>(bfs * 1.4));
    }

  // Colors

  TQColor back, fore;

  if (customDialog->blackOnWhite->isChecked())
    {
      back = Qt::white;
      fore = Qt::black;
    }
  else if (customDialog->whiteOnBlack->isChecked())
    {
      back = Qt::black;
      fore = Qt::white;
    }
  else
    {
      back = customDialog->backgroundColor->color();
      fore = customDialog->foregroundColor->color();
    }

  h1->setColor(fore);
  h2->setColor(fore);
  h3->setColor(fore);
  text->setColor(fore);

  // Fonts

  h1->setFontFamily(customDialog->fontFamily->currentText());
  h2->setFontFamily(customDialog->fontFamily->currentText());
  h3->setFontFamily(customDialog->fontFamily->currentText());
  text->setFontFamily(customDialog->fontFamily->currentText());

  // Show the preview
  PreviewDialog *dlg = new PreviewDialog(this, 0, true);
  dlg->preview->setPaper(back);
  dlg->preview->viewport()->setFont(TQFont(TDEGlobalSettings::generalFont().family(), bfs));

  dlg->exec();

  delete dlg;
}




#include "kcmcss.moc"

