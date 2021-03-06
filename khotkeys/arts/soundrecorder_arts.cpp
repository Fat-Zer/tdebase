/*
    Copyright (C) 2005 by Olivier Goffart
    ogoffart@kde.org

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ARTS

#include "soundrecorder_arts.h"

#include <arts/kaudiorecordstream.h>
#include <arts/kartsserver.h>
#include <arts/kartsdispatcher.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tqtimer.h>


#define FS 11025
#define BITS 16

#define ABS(X) ( ((X)>0) ? (X) : -(X) )

extern "C"
KDE_EXPORT
KHotKeys::SoundRecorder* khotkeys_soundrecorder_create( TQObject* parent, const char* name )
{
    return new KHotKeys::SoundRecorderArts( parent, name );
}

namespace KHotKeys
{


SoundRecorderArts::SoundRecorderArts(TQObject *parent, const char *name)
	: SoundRecorder(parent, name) ,
	  m_dis(new KArtsDispatcher( this) ),
	  m_server( new KArtsServer( this ) ) ,
	  m_recStream( new KAudioRecordStream( m_server, i18n("khotkeys"), m_server ) )
{
        create_ptr check = khotkeys_soundrecorder_create; // check the type matches
        ( void ) check;

	m_recStream->usePolling( false );
	connect( m_recStream, TQT_SIGNAL(data (TQByteArray &)), this, TQT_SLOT(slotDataReceived(TQByteArray& )));
}

SoundRecorderArts::~SoundRecorderArts()
{
	delete m_recStream;
	delete m_server;
	delete m_dis;
}

void SoundRecorderArts::start()
{
	m_recStream->start(FS,BITS,2);
	m_data.resize(0);
}

void SoundRecorderArts::stop()
{
	m_recStream->stop();
	TQTimer::singleShot(400,this,TQT_SLOT(slotEmitSignal()));
}

void SoundRecorderArts::abort()
{
	m_recStream->stop();
	m_data.resize(0);
}


Sound SoundRecorderArts::sound()
{
	Sound s;
	uint BytePS=BITS/8;
	uint length=m_data.size()/BytePS;
	TQMemArray<TQ_INT32> da(length);
	s.max=0;
	s._fs=FS;
	for(uint f=0;f<length; f++)
	{
#if BITS==8
		int nb=(unsigned char)(m_data[f])  -128;
#elif BITS==16
		int nb=(m_data[2*f] &0x000000FF )  |  ( (m_data[2*f+1] &0x000000FF ) << 8 )    ;
		if(nb & (1<< 15))
			nb = nb-(1<<16);
#else
	#error  BITS is not 16 or 8
#endif
		if(s.max < (uint)ABS(nb))
		{
			s.max= (uint)ABS(nb);
		}
		da[f]=nb;
	}
	s.data=da;
	return s;
}

void SoundRecorderArts::slotDataReceived(TQByteArray & data)
{
	uint pos=m_data.size();
	m_data.resize(pos + data.size());
	for(uint f=0;f<data.size(); f++)
	{
		m_data[pos+f]=data[f];
	}
}

void SoundRecorderArts::slotEmitSignal()
{
	emit recorded( sound() );
}

}

#include "soundrecorder_arts.moc"

#endif
