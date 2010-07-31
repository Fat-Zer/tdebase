/*****************************************************************
This file is part of the KDE project.

Copyright (C) 1999, 2000    Daniel M. Duley <mosfet@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
******************************************************************/

#ifndef KDECORATION_PLUGINS_H
#define KDECORATION_PLUGINS_H

//
// This header file is internal. I mean it.
// 

#include <tqcstring.h>
#include <tqstring.h>
#include <tqwidget.h>

#include "kdecoration.h"

class KLibrary;
class KConfig;
class KDecoration;
class KDecorationBridge;
class KDecorationFactory;

class KWIN_EXPORT KDecorationPlugins
    : public KDecorationProvides
    {
    public:
        KDecorationPlugins( KConfig* cfg );
        virtual ~KDecorationPlugins();
        bool loadPlugin( TQString name );
        void destroyPreviousPlugin();
        KDecorationFactory* factory();
        KDecoration* createDecoration( KDecorationBridge* );
        TQString currentPlugin() { return pluginStr; }
        bool reset( unsigned long changed ); // returns true if decorations need to be recreated
    protected:
        virtual void error( const TQString& error_msg );
        TQCString defaultPlugin; // FRAME normalne protected?
    private:
        KDecorationFactory* (*create_ptr)();
        KLibrary *library;
        KDecorationFactory* fact;
        KLibrary *old_library;
        KDecorationFactory* old_fact;
        TQString pluginStr;
        KConfig* config;
    };

/*

 Plugins API:
    KDecorationFactory* create_factory(); - called once after loading

*/

#endif
