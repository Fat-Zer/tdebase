/***************************************************************************
  Copyright:
  (C) 2002 by George Russell <george.russell@clara.net>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <tdehtml_part.h> // this plugin applies to a tdehtml part
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <dom/dom_string.h>
#include <kdebug.h>
#include "tdehtmlkttsd.h"
#include <tdeaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <tqmessagebox.h>
#include <tdelocale.h>
#include <tqstring.h>
#include <tqtimer.h>
#include <kspeech.h>
#include <tqbuffer.h>

#include <tdeapplication.h>
#include <dcopclient.h>
#include <ktrader.h>

TDEHTMLPluginKTTSD::TDEHTMLPluginKTTSD( TQObject* parent, const char* name, const TQStringList& )
    : Plugin( parent, name )
{
    // If KTTSD is not installed, hide action.
    TDETrader::OfferList offers = TDETrader::self()->query("DCOP/Text-to-Speech", "Name == 'KTTSD'");
    if (offers.count() > 0)
    {
        (void) new TDEAction( i18n("&Speak Text"),
            "kttsd", 0,
            this, TQT_SLOT(slotReadOut()),
            actionCollection(), "tools_kttsd" );
    }
    else
        kdDebug() << "TDEHTMLPLuginKTTSD::TDEHTMLPluginKTTSD: TDETrader did not find KTTSD." << endl;
}

TDEHTMLPluginKTTSD::~TDEHTMLPluginKTTSD()
{
}

void TDEHTMLPluginKTTSD::slotReadOut()
{
    // The parent is assumed to be a TDEHTMLPart
    if ( !parent()->inherits("TDEHTMLPart") )
       TQMessageBox::warning( 0, i18n( "Cannot Read source" ),
                                i18n( "You cannot read anything except web pages with\n"
                                      "this plugin, sorry." ));
    else
    {
        // If KTTSD not running, start it.
        DCOPClient *client = kapp->dcopClient();
        if (!client->isApplicationRegistered("kttsd"))
        {
            TQString error;
            if (kapp->startServiceByDesktopName("kttsd", TQStringList(), &error))
                TQMessageBox::warning(0, i18n( "Starting KTTSD Failed"), error );
        }

        // Find out if KTTSD supports xhtml (rich speak).
        TQByteArray  data;
        TQBuffer     dataBuf(data);
        TQDataStream arg;
        dataBuf.open(IO_WriteOnly);
        arg.setDevice(&dataBuf);
        arg << "" << KSpeech::mtHtml;
        TQCString    replyType;
        TQByteArray  replyData;
        bool supportsXhtml = false;
        if ( !client->call("kttsd", "KSpeech", "supportsMarkup(TQString,uint)",
            data, replyType, replyData, true) )
            TQMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                     i18n( "The DCOP call supportsMarkup failed." ));
        else
        {
            TQDataStream reply(replyData, IO_ReadOnly);
            reply >> supportsXhtml;
        }

        TDEHTMLPart *part = (TDEHTMLPart *) parent();

        TQString query;
        if (supportsXhtml)
        {
            kdDebug() << "KTTS claims to support rich speak (XHTML to SSML)." << endl;
            if (part->hasSelection())
                query = part->selectedTextAsHTML();
            else
            {
                // TODO: Fooling around with the selection probably has unwanted
                // side effects, but until a method is supplied to get valid xhtml
                // from entire document..
                // query = part->document().toString().string();
                part->selectAll();
                query = part->selectedTextAsHTML();
                // Restore no selection.
                part->setSelection(part->document().createRange());
            }
        } else {
            if (part->hasSelection())
                query = part->selectedText();
            else
                query = part->htmlDocument().body().innerText().string();
        }
        // kdDebug() << "TDEHTMLPluginKTTSD::slotReadOut: query = " << query << endl;

        dataBuf.at(0);  // reset data
        arg << query << "";
        if ( !client->call("kttsd", "KSpeech", "setText(TQString,TQString)",
            data, replyType, replyData, true) )
            TQMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                     i18n( "The DCOP call setText failed." ));
        dataBuf.at(0);
        arg << 0;
        if ( !client->call("kttsd", "KSpeech", "startText(uint)",
            data, replyType, replyData, true) )
            TQMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                     i18n( "The DCOP call startText failed." ));
    }
}

K_EXPORT_COMPONENT_FACTORY( libtdehtmlkttsdplugin, KGenericFactory<TDEHTMLPluginKTTSD>("tdehtmlkttsd") )

#include "tdehtmlkttsd.moc"
