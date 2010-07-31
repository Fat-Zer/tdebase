/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


TQString ListViewSettingsWidget::title()
{
    return m_title->text();
}


TQColor ListViewSettingsWidget::gridColor()
{
    return m_gridColor->color();
}


TQColor ListViewSettingsWidget::backgroundColor()
{
    return m_backgroundColor->color();
}


TQColor ListViewSettingsWidget::textColor()
{
    return m_textColor->color();
}


void ListViewSettingsWidget::setTitle( const TQString &t )
{
    m_title->setText(t);
}


void ListViewSettingsWidget::setBackgroundColor( const TQColor &c )
{
    m_backgroundColor->setColor(c);
}


void ListViewSettingsWidget::setTextColor( const TQColor &c )
{
    m_textColor->setColor(c);
}


void ListViewSettingsWidget::setGridColor( const TQColor &c )
{
    m_gridColor->setColor(c);
}
