////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 03/08/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "FontViewPart.h"
#include "FontPreview.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "KfiPrint.h"
#include <tdelocale.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqframe.h>
#include <tqfile.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>
#include <tqvalidator.h>
#include <tqregexp.h>
#include <tqsettings.h>
#include <tqstringlist.h>
#include <tqtimer.h>
#include <tdeio/netaccess.h>
#include <kinstance.h>
#include <tdemessagebox.h>
#include <knuminput.h>
#include <kstdaction.h>
#include <tdeaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kprinter.h>
#include <fontconfig/fontconfig.h>

static KURL getDest(const KURL &url, bool system)
{
    return KURL(KFI::Misc::root()
                  ? TQString("fonts:/") + url.fileName()
                  : TQString("fonts:/") + TQString(system ? i18n(KFI_TDEIO_FONTS_SYS) : i18n(KFI_TDEIO_FONTS_USER))
                                       + TQChar('/') + url.fileName());
}

namespace KFI
{

CFontViewPart::CFontViewPart(TQWidget *parent, const char *name)
{
    bool kcm=0==strcmp(name, "kcmfontinst");

    itsFrame=new TQFrame(parent, "frame");

    TQFrame *previewFrame=new TQFrame(itsFrame);

    itsToolsFrame=new TQFrame(itsFrame);

    TQVBoxLayout *layout=new TQVBoxLayout(itsFrame, kcm ? 0 : KDialog::marginHint(), kcm ? 0 : KDialog::spacingHint());
    TQGridLayout *previewLayout=new TQGridLayout(previewFrame, 1, 1, 1, 1);
    TQHBoxLayout *toolsLayout=new TQHBoxLayout(itsToolsFrame, 0, KDialog::spacingHint());

    itsFrame->setFrameShape(TQFrame::NoFrame);
    itsFrame->setFocusPolicy(TQ_ClickFocus);
    itsToolsFrame->setFrameShape(TQFrame::NoFrame);
    previewFrame->setFrameShadow(kcm ? TQFrame::Sunken : TQFrame::Raised);
    previewFrame->setFrameShape(TQFrame::Panel);
    setInstance(new TDEInstance("tdefontview"));

    itsPreview=new CFontPreview(previewFrame, "FontViewPart::Preview");
    itsPreview->setSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding);
    itsFaceLabel=new TQLabel(i18n("Face:"), itsToolsFrame);
    itsFaceSelector=new KIntNumInput(1, itsToolsFrame);
    itsInstallButton=new TQPushButton(i18n("Install..."), itsToolsFrame, "button");
    itsInstallButton->hide();
    previewLayout->addWidget(itsPreview, 0, 0);
    layout->addWidget(previewFrame);
    layout->addWidget(itsToolsFrame);
    toolsLayout->addWidget(itsFaceLabel);
    toolsLayout->addWidget(itsFaceSelector);
    itsFaceLabel->hide();
    itsFaceSelector->hide();
    toolsLayout->addItem(new TQSpacerItem(5, 5, TQSizePolicy::MinimumExpanding, TQSizePolicy::Minimum));
    toolsLayout->addWidget(itsInstallButton);
    itsToolsFrame->hide();
    connect(itsPreview, TQT_SIGNAL(status(bool)), TQT_SLOT(previewStatus(bool)));
    connect(itsInstallButton, TQT_SIGNAL(clicked()), TQT_SLOT(install()));
    connect(itsFaceSelector, TQT_SIGNAL(valueChanged(int)), itsPreview, TQT_SLOT(showFace(int)));

    itsChangeTextAction=new TDEAction(i18n("Change Text..."), "text", TDEShortcut(),
                                    this, TQT_SLOT(changeText()), actionCollection(), "changeText");
    itsChangeTextAction->setEnabled(false);
    itsPrintAction=KStdAction::print(this, TQT_SLOT(print()), actionCollection(), "print");
    itsPrintAction->setEnabled(false);

    setXMLFile("tdefontviewpart.rc");
    setWidget(itsFrame);
}

bool CFontViewPart::openURL(const KURL &url)
{
    if (!url.isValid() || !closeURL())
        return false;

    if(KFI_TDEIO_FONTS_PROTOCOL==url.protocol() || url.isLocalFile())
    {
        m_url=url;
        emit started(0);
        m_file = m_url.path();
        bool ret=openFile();
        if (ret)
        {
            emit completed();
            emit setWindowCaption(m_url.prettyURL());
        }
        return ret;
    }
    else
        return ReadOnlyPart::openURL(url);
}

bool CFontViewPart::openFile()
{
    // NOTE: Cant do the real open here, as dont seem to be able to use TDEIO::NetAccess functions during initial start-up.
    // Bug report 111535 indicates that calling "konqueror <font>" crashes.
    TQTimer::singleShot(0, this, TQT_SLOT(timeout()));
    return true;
}

void CFontViewPart::timeout()
{
    bool showFs=false,
         isFonts=KFI_TDEIO_FONTS_PROTOCOL==m_url.protocol();

    itsShowInstallButton=false;

    if(isFonts)
        FcInitReinitialize();
    else
    {
        KURL destUrl;

        //
        // Not from fonts:/, so try to see if font is already installed...
        if(Misc::root())
        {
            destUrl=TQString("fonts:/")+itsPreview->engine().getName(m_url);
            itsShowInstallButton=!TDEIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget());
        }
        else
        {
            destUrl=TQString("fonts:/")+i18n(KFI_TDEIO_FONTS_SYS)+TQChar('/')+itsPreview->engine().getName(m_url);
            if(TDEIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget()))
                itsShowInstallButton=false;
            else
            {
                destUrl=TQString("fonts:/")+i18n(KFI_TDEIO_FONTS_USER)+TQChar('/')+itsPreview->engine().getName(m_url);
                itsShowInstallButton=!TDEIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget());
            }
        }
    }

    itsPreview->showFont(isFonts ? m_url : m_file);

    if(!isFonts && itsPreview->engine().getNumIndexes()>1)
    {
        showFs=true;
        itsFaceSelector->setRange(1, itsPreview->engine().getNumIndexes(), 1, false);
    }

    itsFaceLabel->setShown(showFs);
    itsFaceSelector->setShown(showFs);
    itsToolsFrame->hide();
}

void CFontViewPart::previewStatus(bool st)
{
    itsInstallButton->setShown(st && itsShowInstallButton);
    itsToolsFrame->setShown(itsInstallButton->isShown()||itsFaceSelector->isShown());
    itsChangeTextAction->setEnabled(st);
    itsPrintAction->setEnabled(st && KFI_TDEIO_FONTS_PROTOCOL==m_url.protocol());
}

void CFontViewPart::install()
{
    int resp=Misc::root() ? KMessageBox::Yes
                           : KMessageBox::questionYesNoCancel(itsFrame,
                                                              i18n("Where do you wish to install \"%1\" (%2)?\n"
                                                                   "\"%3\" - only accessible to you, or\n"
                                                                   "\"%4\" - accessible to all (requires administrator "
                                                                   "password)")
                                                                   .arg(itsPreview->engine().getName(m_url))
                                                                   .arg(m_url.fileName())
                                                                   .arg(i18n(KFI_TDEIO_FONTS_USER))
                                                                   .arg(i18n(KFI_TDEIO_FONTS_SYS)),
                                                              i18n("Install"), i18n(KFI_TDEIO_FONTS_USER),
                                                              i18n(KFI_TDEIO_FONTS_SYS));

    if(KMessageBox::Cancel!=resp)
    {
        KURL destUrl(getDest(m_url, KMessageBox::No==resp));

        if(TDEIO::NetAccess::copy(m_url, destUrl, itsFrame->parentWidget()))
        {
            //
            // OK file copied, now look for any AFM or PFM file...
            KURL::List urls;

            Misc::getAssociatedUrls(m_url, urls);

            if(urls.count())
            {
                KURL::List::Iterator it,
                                     end=urls.end();

                for(it=urls.begin(); it!=end; ++it)
                {
                    destUrl=getDest(*it, KMessageBox::No==resp);
                    TDEIO::NetAccess::copy(*it, destUrl, itsFrame->parentWidget());
                }
            }

            KMessageBox::information(itsFrame, i18n("%1:%2 successfully installed.").arg(m_url.protocol())
                                                   .arg(m_url.path()), i18n("Success"),
                                     "FontViewPart_DisplayInstallationSuccess");
            itsShowInstallButton=false;
            itsInstallButton->setShown(itsShowInstallButton);
        }
        else
            KMessageBox::error(itsFrame, i18n("Could not install %1:%2").arg(m_url.protocol()).arg(m_url.path()),
                               i18n("Error"));
    }
}

void CFontViewPart::changeText()
{
    bool             status;
    TQRegExpValidator validator(TQRegExp(".*"), 0L);
    TQString          oldStr(itsPreview->engine().getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview String"), i18n("Please enter new string:"),
                                                  oldStr, &status, itsFrame,
                                                  "preview string dialog", &validator));

    if(status && newStr!=oldStr)
    {
        itsPreview->engine().setPreviewString(newStr);
        itsPreview->showFont();
    }
}

void CFontViewPart::print()
{
    TQStringList items;

    items.append(itsPreview->engine().getName(m_url));

    Print::printItems(items, 0, itsFrame->parentWidget(), itsPreview->engine());
}

}

#include "FontViewPart.moc"
