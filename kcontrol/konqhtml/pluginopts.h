//-----------------------------------------------------------------------------
//
// Plugin Options
//
// (c) 2002 Leo Savernik, per-domain settings
// (c) 2001, Daniel Naber, based on javaopts.h
// (c) 2000, Stefan Schimanski <1Stein@gmx.de>, Netscape parts
//
//-----------------------------------------------------------------------------

#ifndef __PLUGINOPTS_H__
#define __PLUGINOPTS_H__

#include <tqwidget.h>

#include "domainlistview.h"
#include "policies.h"

class KConfig;
class TQCheckBox;

#include <kcmodule.h>
#include "nsconfigwidget.h"

class TQBoxLayout;
class TQLabel;
class TQProgressDialog;
class TQSlider;
class KDialogBase;
class KPluginOptions;
class KProcIO;

/** policies with plugin-specific constructor
  */
class PluginPolicies : public Policies {
public:
  /**
   * constructor
   * @param config configuration to initialize this instance from
   * @param group config group to use if this instance tqcontains the global
   *	policies (global == true)
   * @param global true if this instance tqcontains the global policy settings,
   *	false if this instance tqcontains policies specific for a domain.
   * @param domain name of the domain this instance is used to configure the
   *	policies for (case insensitive, ignored if global == true)
   */
  PluginPolicies(KConfig* config, const TQString &group, bool global,
  		const TQString &domain = TQString::null);

  virtual ~PluginPolicies();
};

/** Plugin-specific enhancements to the domain list view
  */
class PluginDomainListView : public DomainListView {
  Q_OBJECT
public:
  PluginDomainListView(KConfig *config,const TQString &group,KPluginOptions *opt,
  		TQWidget *parent,const char *name = 0);
  virtual ~PluginDomainListView();

protected:
  virtual PluginPolicies *createPolicies();
  virtual PluginPolicies *copyPolicies(Policies *pol);
  virtual void setupPolicyDlg(PushButton trigger,PolicyDialog &pDlg,
  		Policies *copy);

private:
  TQString group;
  KPluginOptions *options;
};

/**
 * dialog for embedding a PluginDomainListView widget
 */
class PluginDomainDialog : public TQWidget {
  Q_OBJECT
public:

  PluginDomainDialog(TQWidget *parent);
  virtual ~PluginDomainDialog();

  void setMainWidget(TQWidget *widget);

private slots:
  virtual void slotClose();

private:
  PluginDomainListView *domainSpecific;
  TQBoxLayout *thisLayout;
};

class KPluginOptions : public KCModule
{
    Q_OBJECT

public:
    KPluginOptions( KConfig* config, TQString group, TQWidget* parent = 0, const char* name = 0 );
	~KPluginOptions();

    virtual void load();
    virtual void load( bool useDefaults );
    virtual void save();
    virtual void defaults();
    TQString quickHelp() const;

private slots:
    void slotChanged();
    void slotTogglePluginsEnabled();
    void slotShowDomainDlg();

private:

    KConfig* m_pConfig;
    TQString  m_groupname;

    TQCheckBox *enablePluginsGloballyCB, *enableHTTPOnly, *enableUserDemand;


 protected slots:
  void progress(KProcIO *);
  void updatePLabel(int);
  void change() { change( true ); };
  void change( bool c ) { emit changed(c); m_changed = c; };

  void scan();
  void scanDone();

 private:
  NSConfigWidget *m_widget;
  bool m_changed;
  TQProgressDialog *m_progress;
  KProcIO* m_nspluginscan;
  TQSlider *priority;
  TQLabel *priorityLabel;
  PluginPolicies global_policies;
  PluginDomainListView *domainSpecific;
  KDialogBase *domainSpecificDlg;

/******************************************************************************/
 protected:
  void dirInit();
  void dirLoad( KConfig *config, bool useDefault = false );
  void dirSave( KConfig *config );

 protected slots:
  void dirNew();
  void dirRemove();
  void dirUp();
  void dirDown();
  void dirEdited(const TQString &);
  void dirSelect( TQListBoxItem * );

/******************************************************************************/
 protected:
  void pluginInit();
  void pluginLoad( KConfig *config );
  void pluginSave( KConfig *config );

  friend class PluginDomainListView;
};

#endif		// __PLUGINOPTS_H__
