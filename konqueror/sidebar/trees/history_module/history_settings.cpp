
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <dcopclient.h>

#include "history_settings.h"

KonqSidebarHistorySettings::KonqSidebarHistorySettings( TQObject *parent, const char *name )
    : TQObject( parent, name ),
      DCOPObject( "KonqSidebarHistorySettings" )
{
    m_fontOlderThan.setItalic( true ); // default
}

KonqSidebarHistorySettings::KonqSidebarHistorySettings()
    : TQObject(),
      DCOPObject( "KonqSidebarHistorySettings" )
{
    m_fontOlderThan.setItalic( true ); // default
}

KonqSidebarHistorySettings::KonqSidebarHistorySettings( const KonqSidebarHistorySettings& s )
    : TQObject(),
      DCOPObject( "KonqSidebarHistorySettings" )
{
    m_valueYoungerThan = s.m_valueYoungerThan;
    m_valueOlderThan = s.m_valueOlderThan;

    m_metricYoungerThan = s.m_metricYoungerThan;
    m_metricOlderThan = s.m_metricOlderThan;

    m_detailedTips = s.m_detailedTips;

    m_fontYoungerThan = s.m_fontYoungerThan;
    m_fontOlderThan = s.m_fontOlderThan;
}

KonqSidebarHistorySettings::~KonqSidebarHistorySettings()
{
}

void KonqSidebarHistorySettings::readSettings(bool global)
{
    TDEConfig *config;
    TQString oldgroup;

    if (global) {
      config = TDEGlobal::config();
      oldgroup= config->group();
    }
    else
      config = new TDEConfig("konquerorrc");

    config->setGroup("HistorySettings");
    m_valueYoungerThan = config->readNumEntry("Value youngerThan", 1 );
    m_valueOlderThan = config->readNumEntry("Value olderThan", 2 );

    TQString minutes = TQString::fromLatin1("minutes");
    TQString days = TQString::fromLatin1("days");
    TQString metric = config->readEntry("Metric youngerThan", days );
    m_metricYoungerThan = (metric == days) ? DAYS : MINUTES;
    metric = config->readEntry("Metric olderThan", days );
    m_metricOlderThan = (metric == days) ? DAYS : MINUTES;

    m_detailedTips = config->readBoolEntry("Detailed Tooltips", true);

    m_fontYoungerThan = config->readFontEntry( "Font youngerThan",
					       &m_fontYoungerThan );
    m_fontOlderThan   = config->readFontEntry( "Font olderThan",
					       &m_fontOlderThan );
    if (global)
      config->setGroup( oldgroup );
    else
      delete config;
}

void KonqSidebarHistorySettings::applySettings()
{
    TDEConfig *config = new TDEConfig("konquerorrc");
    config->setGroup("HistorySettings");

    config->writeEntry("Value youngerThan", m_valueYoungerThan );
    config->writeEntry("Value olderThan", m_valueOlderThan );

    TQString minutes = TQString::fromLatin1("minutes");
    TQString days = TQString::fromLatin1("days");
    config->writeEntry("Metric youngerThan", m_metricYoungerThan == DAYS ?
			  days : minutes );
    config->writeEntry("Metric olderThan", m_metricOlderThan == DAYS ?
 	 	 	   days : minutes );

    config->writeEntry("Detailed Tooltips", m_detailedTips);

    config->writeEntry("Font youngerThan", m_fontYoungerThan );
    config->writeEntry("Font olderThan", m_fontOlderThan );

    delete config;

    // notify konqueror instances about the new configuration
    kapp->dcopClient()->send( "konqueror*", "KonqSidebarHistorySettings",
			      "notifySettingsChanged()", TQByteArray() );
}

void KonqSidebarHistorySettings::notifySettingsChanged()
{
    readSettings(false);
    emit settingsChanged();
}

#include "history_settings.moc"
