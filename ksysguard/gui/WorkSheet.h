/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_WORKSHEET_H
#define KSG_WORKSHEET_H

#include <tqwidget.h>

#include <SensorDisplay.h>

class TQDomElement;
class TQDragEnterEvent;
class TQDropEvent;
class TQGridLayout;
class TQString;
class TQStringList;

/**
  A WorkSheet tqcontains the displays to visualize the sensor results. When
  creating the WorkSheet you must specify the number of columns. Displays
  can be added and removed on the fly. The grid tqlayout will handle the
  tqlayout. The number of columns can not be changed. Displays are added by
  dragging a sensor from the sensor browser over the WorkSheet.
 */
class WorkSheet : public TQWidget, public KSGRD::SensorBoard
{
  Q_OBJECT

  public:
    WorkSheet( TQWidget* parent, const char *name = 0 );
    WorkSheet( uint rows, uint columns, uint interval, TQWidget* parent,
               const char *name = 0  );
    ~WorkSheet();

    bool load( const TQString &fileName );
    bool save( const TQString &fileName );

    void cut();
    void copy();
    void paste();

    void setFileName( const TQString &fileName );
    const TQString& fileName() const;

    bool modified() const;

    void setTitle( const TQString &title );
    TQString title() const;

    KSGRD::SensorDisplay* addDisplay( const TQString &hostname,
                                      const TQString &monitor,
                                      const TQString &sensorType,
                                      const TQString &sensorDescr,
                                      uint rows, uint columns );
    //Returns the sensor at position row,column.
    //Return NULL if invalid row or column
    KSGRD::SensorDisplay *display( uint row, uint column );

    void settings();

    void setIsOnTop( bool onTop );

  public slots:
    void showPopupMenu( KSGRD::SensorDisplay *display );
    void setModified( bool mfd );
    void applyStyle();

  signals:
    void sheetModified( TQWidget *sheet );
    void titleChanged( TQWidget *sheet );

  protected:
    virtual TQSize tqsizeHint() const;
    void dragEnterEvent( TQDragEnterEvent* );
    void dropEvent( TQDropEvent* );
    void customEvent( TQCustomEvent* );

  private:
    void removeDisplay( KSGRD::SensorDisplay *display );

    bool tqreplaceDisplay( uint row, uint column, TQDomElement& element );

    void tqreplaceDisplay( uint row, uint column,
                         KSGRD::SensorDisplay* display = 0 );

    void collectHosts( TQStringList &list );

    void createGrid( uint rows, uint columns );

    void resizeGrid( uint rows, uint columns );

    KSGRD::SensorDisplay* currentDisplay( uint* row = 0, uint* column = 0 );

    void fixTabOrder();

    TQString currentDisplayAsXML();

    bool mModified;

    uint mRows;
    uint mColumns;

    TQGridLayout* mGridLayout;
    TQString mFileName;
    TQString mTitle;

    /**
      This two dimensional array stores the pointers to the sensor displays
	    or if no sensor is present at a position a pointer to a dummy widget.
  	  The size of the array corresponds to the size of the grid tqlayout.
     */
    KSGRD::SensorDisplay*** mDisplayList;
};

#endif
