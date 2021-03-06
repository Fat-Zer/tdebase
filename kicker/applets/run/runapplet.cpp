/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqlabel.h>
#include <tqfont.h>
#include <tqstringlist.h>
#include <tqpushbutton.h>
#include <tqhbox.h>

#include <tdeapplication.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kcombobox.h>
#include <kurifilter.h>
#include <kdialog.h>
#include <krun.h>
#include <tdemessagebox.h>

#include "runapplet.h"
#include "runapplet.moc"

extern "C"
{
  KDE_EXPORT KPanelApplet* init(TQWidget *parent, const TQString& configFile)
  {
    TDEGlobal::locale()->insertCatalogue("krunapplet");
    return new RunApplet(configFile, KPanelApplet::Stretch, 0, parent, "krunapplet");
  }
}

RunApplet::RunApplet(const TQString& configFile, Type type, int actions,
                             TQWidget *parent, const char *name)
  : KPanelApplet(configFile, type, actions, parent, name)
{
  //  setBackgroundMode(X11ParentRelative);
    setBackgroundOrigin( AncestorOrigin );
    // setup label
    _label = new TQLabel(i18n("Run command:"), this);
    TQFont f(_label->font());
    f.setPixelSize(12);
//    _label->setBackgroundMode(X11ParentRelative);
    _label->setBackgroundOrigin( AncestorOrigin );
    _label->setFixedHeight(14);
    _label->setFont(f);

    // setup popup button
    _btn = new TQPushButton(this);
    f = _btn->font();
    f.setPixelSize(12);
    _btn->setFont(f);
    connect(_btn, TQT_SIGNAL(clicked()), TQT_SLOT(popup_combo()));

    // setup history combo
    _input = new KHistoryCombo(this);
    _input->setFocus();
    _input->clearEdit();
    watchForFocus(_input->lineEdit());
    connect(_input, TQT_SIGNAL(activated(const TQString&)),
	    TQT_SLOT(run_command(const TQString&)));

    TDEConfig *c = config();
    c->setGroup("General");

    // restore history and completion list
    TQStringList list = c->readListEntry("Completion list");
    _input->completionObject()->setItems(list);
    list = c->readListEntry("History list");
    _input->setHistoryItems(list);
    int mode = c->readNumEntry( "CompletionMode", TDEGlobalSettings::completionMode() );
    _input->setCompletionMode( (TDEGlobalSettings::Completion) mode );

    _filterData = new KURIFilterData();

    _hbox = new TQHBox( 0, 0, (WFlags)(WStyle_Customize | WType_Popup) );
    _hbox->setFixedSize(120, 22);
}

RunApplet::~RunApplet()
{
    TDEConfig *c = config();
    c->setGroup("General");

    // save history and completion list
    TQStringList list = _input->completionObject()->items();
    c->writeEntry("Completion list", list);
    list = _input->historyItems();
    c->writeEntry("History list", list);
    c->writeEntry( "CompletionMode", (int) _input->completionMode() );
    c->sync();

    delete _filterData;
    TDEGlobal::locale()->removeCatalogue("krunapplet");
}

void RunApplet::resizeEvent(TQResizeEvent*)
{
    if(orientation() == Qt::Horizontal)
	{
	    _btn->hide();
	    _input->reparent(this, TQPoint(0,0), true);
	    _label->setGeometry(0,0, width(), _label->height());

	    if(height() >= _input->sizeHint().height() + _label->height())
		{
                    int inputVOffset = height() - _input->sizeHint().height() - 2;
                    int labelHeight = _label->sizeHint().height();
		    _label->setGeometry(0, inputVOffset - labelHeight,
                                        width(), labelHeight);
		    _input->setGeometry(0, inputVOffset,
					width(), _input->sizeHint().height());
		    _label->show();
		}
	    else
		{
		    _label->hide();

                    // make it as high as the combobox naturally wants to be
                    // but no taller than the panel is!
                    // don't forget to center it vertically either.
                    int newHeight = _input->sizeHint().height();
                    if (newHeight > height())
                        newHeight = height();
		    _input->setGeometry(0, (height() - newHeight) / 2,
                                        width(), newHeight);
		}
	}
    else
	{
	    _btn->show();
	    _btn->setFixedSize(width(), 22);
	    _input->reparent( _hbox, TQPoint(0, 0), false);
	    _label->hide();
	}
    setButtonText();
}

void RunApplet::positionChange(KPanelApplet::Position)
{
    setButtonText();
}

void RunApplet::setButtonText()
{
    TQString t;

    if (position() == pRight)
	{
	    if (width() >= 42)
		t = i18n("< Run");
	    else
		t = "<";
	}
    else
	{
	    if(width() >= 42)
		t = i18n("Run >");
	    else
		t = ">";
	}

    _btn->setText(t);
}

int RunApplet::widthForHeight(int ) const
{
    return _label->sizeHint().width();
}

int RunApplet::heightForWidth(int ) const
{
    return 22;
}

void RunApplet::popup_combo()
{
    TQPoint p;
    if (position() == pRight)
	p = mapToGlobal(TQPoint(-_input->width()-1, 0));
    else
	p = mapToGlobal(TQPoint(width()+1, 0));
    _hbox->move(p);
    _hbox->show();
    _input->setFocus();
}

void RunApplet::run_command(const TQString& command)
{
    TQString exec;
    bool focusNeeded = false;

    kapp->propagateSessionManager();

    _filterData->setData( _input->currentText().stripWhiteSpace() );
    TQStringList filters;
    filters << "kurisearchfilter" << "tdeshorturifilter";
    KURIFilter::self()->filterURI( *(_filterData), filters );

    _input->addToHistory(command);
    _input->clearEdit();

    TQString cmd = (_filterData->uri().isLocalFile() ? _filterData->uri().path():_filterData->uri().url());

    // Nothing interesting. Quit!
    if ( cmd.isEmpty() ){
	KMessageBox::sorry(0L, i18n("You have to enter a command to execute "
				    "or a URL to be opened first."));
        focusNeeded = true;
	goto hide;
    }
    else if (cmd == "logout")
	{
	    bool shutdown = kapp->requestShutDown();
	    if( !shutdown )
            {
                // This i18n string is in kdesktop/desktop.cc as well. Maybe we should DCOP to kdesktop instead ?
		KMessageBox::error( 0, i18n("Unable to log out properly.\nThe session manager cannot "
					    "be contacted. You can try to force a shutdown by pressing "
					    "Ctrl+Alt+Backspace. Note, however, that your current "
                                            "session will not be saved with a forced shutdown." ) );
                focusNeeded = true;
            }
	    goto hide;
	}
    else
	{
	    switch( _filterData->uriType() )
		{
		case KURIFilterData::LOCAL_FILE:
		case KURIFilterData::LOCAL_DIR:
		case KURIFilterData::NET_PROTOCOL:
		case KURIFilterData::HELP:
		    {
		        (void) new KRun( _filterData->uri() );
			goto hide;
		    }
		case KURIFilterData::EXECUTABLE:
		case KURIFilterData::SHELL:
		    {
		        exec = cmd;
			if( _filterData->hasArgsAndOptions() )
			  cmd += _filterData->argsAndOptions();
			break;
		    }
		case KURIFilterData::UNKNOWN:
		case KURIFilterData::ERROR:
		default:
		    KMessageBox::sorry( 0, i18n("<qt>The program name or command <b>%1</b>\n"
						"cannot be found. Please correct the command\n"
						"or URL and try again</qt>").arg( cmd ) );
		    _input->removeFromHistory( _input->currentText() );
                    focusNeeded = true;
		    goto hide;
		}
	}
    if (KRun::runCommand( cmd, exec, "" ))
	goto hide;
    else
	{
	    KMessageBox::sorry( 0, i18n("<qt>Could not run <b>%1</b>.\nPlease correct"
					" the command or URL and try again.</qt>").arg( cmd ) );
	    _input->removeFromHistory( _input->currentText() );
            focusNeeded = true;
	    goto hide;
	}

    needsFocus(focusNeeded);
    return;

 hide:
    if (orientation() == Qt::Vertical)
	_hbox->hide();
    needsFocus(focusNeeded);
}
