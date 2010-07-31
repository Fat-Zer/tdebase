/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


TQString SensorLoggerSettingsWidget::title()
{
    return m_title->text();
}


TQColor SensorLoggerSettingsWidget::foregroundColor()
{
    return m_foregroundColor->color();
}


TQColor SensorLoggerSettingsWidget::backgroundColor()
{
    return m_backgroundColor->color();
}


TQColor SensorLoggerSettingsWidget::alarmColor()
{
    return m_alarmColor->color();
}


void SensorLoggerSettingsWidget::setTitle( const TQString &t )
{
    m_title->setText(t);
}


void SensorLoggerSettingsWidget::setForegroundColor( const TQColor &c )
{
    m_foregroundColor->setColor(c);
}


void SensorLoggerSettingsWidget::setBackgroundColor( const TQColor &c )
{
    m_backgroundColor->setColor(c);
}


void SensorLoggerSettingsWidget::setAlarmColor( const TQColor &c )
{
    m_alarmColor->setColor(c);
}
