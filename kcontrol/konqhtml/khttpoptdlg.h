// khttpoptdlg.h - extra HTTP configuration by Jacek Konieczy <jajcus@zeus.polsl.gliwice.pl>
#ifndef __KHTTPOPTDLG_H
#define __KHTTPOPTDLG_H

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqstring.h>

#include <kcmodule.h>
#include <kconfig.h>


/**
*  Dialog for configuring HTTP Options like charset and language negotiation
*  and assuming that file got from HTTP is HTML if no Content-Type is given
*/
class KHTTPOptions : public KCModule
{
Q_OBJECT
  public:
    KHTTPOptions(KConfig *config, TQString group, TQWidget *parent = 0L, const char *name = 0L);

    virtual void load();
    virtual void load( bool useDefaults );
    virtual void save();
    virtual void defaults();

  private:

    KConfig *m_pConfig;
    TQString m_groupname;

    // Acceptable languages "LANG" - locale selected languages
    TQLabel *lb_languages;	
    TQLineEdit *le_languages;	

    // Acceptable charsets "CHARSET" - locale selected charset
    TQLabel *lb_charsets;	
    TQLineEdit *le_charsets;	

    TQString defaultCharsets;

private slots:
    void slotChanged();

};

#endif // __KHTTPOPTDLG_H
