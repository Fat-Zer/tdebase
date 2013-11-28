/*

clock module for tdm

Copyright (C) 2000 Espen Sand, espen@kde.org
  Based on work by NN(yet to be determined)
flicker free code by Remi Guyomarch <rguyom@mail.dotcom.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "tdmclock.h"

//#include <tdeapplication.h>
//#include <tdeconfig.h>

#include <tqdatetime.h>
#include <tqpixmap.h>
#include <tqpainter.h>
#include <tqtimer.h>

KdmClock::KdmClock( TQWidget *parent, const char *name )
	: inherited( parent, name )
{
	// start timer
	TQTimer *timer = new TQTimer( this );
	connect( timer, TQT_SIGNAL(timeout()), TQT_SLOT(timeout()) );
	timer->start( 1000 );

	// reading rc file
	//TDEConfig *config = kapp->config();

	//config->setGroup( "Option" );
	mDate = false;//config->readNumEntry( "date", FALSE );
	mSecond = true;//config->readNumEntry( "second", TRUE );
	mDigital = false;//config->readNumEntry( "digital", FALSE );
	mBorder = false;//config->readNumEntry( "border", FALSE );

	//config->setGroup( "Font" );
	mFont.setFamily( TQString::fromLatin1("Utopia")/*config->readEntry( "Family", "Utopia")*/ );
	mFont.setPointSize( 51/*config->readNumEntry( "Point Size", 51)*/ );
	mFont.setWeight( 75/*config->readNumEntry( "Weight", 75)*/ );
	mFont.setItalic( TRUE/*config->readNumEntry( "Italic",TRUE )*/ );
	mFont.setBold( TRUE/*config->readNumEntry( "Bold",TRUE )*/ );

	setFixedSize( 100, 100 );

	if (mBorder) {
		setLineWidth( 1 );
		setFrameStyle( Box|Plain );
		//setFrameStyle( WinPanel|Sunken );
	}

/*
	if (!mDigital) {
		if (height() < width())
			resize( height(), height() );
		else
			resize( width() ,width() );
	}
*/

	//setBackgroundOrigin( WindowOrigin );
	mBackgroundBrush = backgroundBrush();
	setBackgroundMode( NoBackground );
	repaint();
}


void KdmClock::showEvent( TQShowEvent * )
{
	repaint();
}


void KdmClock::timeout()
{
	repaint();
}

void KdmClock::paintEvent( TQPaintEvent * )
{
	if (!isVisible())
		return;

	TQPainter p( this );
	drawFrame( &p );

	TQPixmap pm( contentsRect().size() );
	TQPainter paint;
	paint.begin( &pm );
	paint.fillRect( contentsRect(), mBackgroundBrush );

	// get current time
	TQTime time = TQTime::currentTime();

/*
	if (mDigital) {
		TQString buf;
		if (mSecond)
			buf.sprintf( "%02d:%02d:%02d", time.hour(), time.minute(),
			             time.second() );
		else
			buf.sprintf( "%02d:%02d", time.hour(), time.minute() );
		mFont.setPointSize( TQMIN( (int)(width()/buf.length()*1.5),height() ) );
		paint.setFont( mFont );
		paint.setPen( backgroundColor() );
		paint.drawText( contentsRect(),AlignHCenter|AlignVCenter, buf,-1,0,0 );
	} else {
*/
		TQPointArray pts;
		TQPoint cp = contentsRect().center() - TQPoint( 2,2 );
		int d = TQMIN( contentsRect().width()-15,contentsRect().height()-15 );
		paint.setPen( foregroundColor() );
		paint.setBrush( foregroundColor() );

		TQWMatrix matrix;
		matrix.translate( cp.x(), cp.y() );
		matrix.scale( d/1000.0F, d/1000.0F );

		// Hour
		float h_angle = 30*(time.hour()%12-3) + time.minute()/2;
		matrix.rotate( h_angle );
		paint.setWorldMatrix( matrix );
		pts.setPoints( 4, -20,0,  0,-20, 300,0, 0,20 );
		paint.drawPolygon( pts );
		matrix.rotate( -h_angle );

		// Minute
		float m_angle = (time.minute()-15)*6;
		matrix.rotate( m_angle );
		paint.setWorldMatrix( matrix );
		pts.setPoints( 4, -10,0, 0,-10, 400,0, 0,10 );
		paint.drawPolygon( pts );
		matrix.rotate( -m_angle );

		// Second
		float s_angle = (time.second()-15)*6;
		matrix.rotate( s_angle );
		paint.setWorldMatrix( matrix );
		pts.setPoints( 4,0,0,0,0,400,0,0,0 );
		if (mSecond)
			paint.drawPolygon( pts );
		matrix.rotate( -s_angle );

		// quadrante
		for (int i=0 ; i < 60 ; i++) {
			paint.setWorldMatrix( matrix );
			if ((i % 5) == 0)
				paint.drawLine( 450,0, 500,0 ); // draw hour lines
			else
				paint.drawPoint( 480,0 ); // draw second lines
			matrix.rotate( 6 );
		}

//	} // if (mDigital)
	paint.end();

	// flicker free code by Remi Guyomarch <rguyom@mail.dotcom.fr>
	bitBlt( this, contentsRect().topLeft(), &pm );
}

#include "tdmclock.moc"
