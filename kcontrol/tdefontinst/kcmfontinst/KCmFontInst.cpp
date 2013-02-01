////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKCmFontInst
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 26/04/2003
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
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "KCmFontInst.h"
#include "KfiConstants.h"
#include "PrintDialog.h"
#include "SettingsDialog.h"
#ifdef HAVE_XFT
#include "KfiPrint.h"
#include "FcEngine.h"
#endif
#include <tqapplication.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>
#include <tqsettings.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kdiroperator.h>
#include <kprinter.h>
#include "Misc.h"
#include "KFileFontIconView.h"
#include "KFileFontView.h"
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kstdaccel.h>
#include <tdefiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <tdeio/job.h>
#include <tdeio/netaccess.h>
#include <kdirlister.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <tqsplitter.h>

#define CFG_GROUP          "Main Settings"
#define CFG_LISTVIEW       "ListView"
#define CFG_PATH           "Path"
#define CFG_SPLITTER_SIZES "SplitterSizes"
#define CFG_SHOW_BITMAP    "ShowBitmap"
#define CFG_FONT_SIZE      "FontSize"

typedef KGenericFactory<KFI::CKCmFontInst, TQWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_fontinst, FontInstallFactory("kcmfontinst"))

namespace KFI
{

CKCmFontInst::CKCmFontInst(TQWidget *parent, const char *, const TQStringList&)
            : TDECModule(parent, "kfontinst"),
#ifdef HAVE_XFT
              itsPreview(NULL),
#endif
              itsConfig(KFI_UI_CFG_FILE)
{
    TDEGlobal::locale()->insertCatalogue(KFI_CATALOGUE);

    TDEAboutData* about = new TDEAboutData("kcmfontinst",
         I18N_NOOP("TDE Font Installer"),
         0, 0,
         TDEAboutData::License_GPL,
         I18N_NOOP("GUI front end to the fonts:/ ioslave.\n"
         "(c) Craig Drummond, 2000 - 2004"));
    about->addAuthor("Craig Drummond", I18N_NOOP("Developer and maintainer"), "craig@kde.org");
    setAboutData(about);

    const char *appName=TDECmdLineArgs::appName();

    itsEmbeddedAdmin=Misc::root() && (NULL==appName || strcmp("kcontrol", appName) &&
                     TDECmdLineArgs::parsedArgs()->isSet("embed"));

    itsStatusLabel = new TQLabel(this);
    itsStatusLabel->setFrameShape(TQFrame::Panel);
    itsStatusLabel->setFrameShadow(TQFrame::Sunken);
    itsStatusLabel->setLineWidth(1);

    itsConfig.setGroup(CFG_GROUP);

    TQFrame      *fontsFrame;
#ifdef HAVE_XFT
    KLibFactory *factory=KLibLoader::self()->factory("libkfontviewpart");

    if(factory)
    {
        itsSplitter=new TQSplitter(this);
        fontsFrame=new TQFrame(itsSplitter),
        itsPreview=(KParts::ReadOnlyPart *)factory->create(TQT_TQOBJECT(itsSplitter), "kcmfontinst", "KParts::ReadOnlyPart");
        itsSplitter->setSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding);

        TQValueList<int> sizes(itsConfig.readIntListEntry(CFG_SPLITTER_SIZES));

        if(2!=sizes.count())
        {
            sizes.clear();
            sizes+=250;
            sizes+=150;
        }
        itsSplitter->setSizes(sizes);
    }
    else
    {
#endif
        fontsFrame=new TQFrame(this);
        fontsFrame->setSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding);
#ifdef HAVE_XFT
    }
#endif

    TQGridLayout *fontsLayout=new TQGridLayout(fontsFrame, 1, 1, 0, 1);
    TQVBoxLayout *layout=new TQVBoxLayout(this, 0, KDialog::spacingHint());
    TDEToolBar    *toolbar=new TDEToolBar(this);
    bool        showBitmap(itsConfig.readBoolEntry(CFG_SHOW_BITMAP, false));

    fontsFrame->setLineWidth(0);
    toolbar->setSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::Minimum);
    toolbar->setMovingEnabled(false);

    TQString previousPath=itsConfig.readEntry(CFG_PATH);

    itsDirOp = new KDirOperator(Misc::root() ? TQString("fonts:/") : TQString("fonts:/")+i18n(KFI_KIO_FONTS_USER),
                                fontsFrame);
    itsDirOp->setViewConfig(&itsConfig, "ListView Settings");
    itsDirOp->setMinimumSize(TQSize(96, 64));
    setMimeTypes(showBitmap);
    itsDirOp->dirLister()->setMainWindow(this);
    itsDirOp->setSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding);
    fontsLayout->addMultiCellWidget(itsDirOp, 0, 0, 0, 1);

    KPushButton *button=new KPushButton(KGuiItem(i18n("Add Fonts..."), "newfont"), fontsFrame);
    connect(button, TQT_SIGNAL(clicked()), TQT_SLOT(addFonts()));
    button->setSizePolicy(TQSizePolicy::Minimum, TQSizePolicy::Minimum);
    fontsLayout->addWidget(button, 1, 0);
    TQT_TQLAYOUT(fontsLayout)->addItem(new TQSpacerItem(4, 4, TQSizePolicy::Expanding, TQSizePolicy::Minimum));

    layout->addWidget(toolbar);
#ifdef HAVE_XFT
    layout->addWidget(itsPreview ? itsSplitter : fontsFrame);
#else
    layout->addWidget(fontsFrame);
#endif
    layout->addWidget(itsStatusLabel);

    setButtons(0);
    setRootOnlyMsg(i18n("<b>The fonts shown are your personal fonts.</b><br>To see (and install) "
                        "system-wide fonts, click on the \"Administrator Mode\" button below."));
    setUseRootOnlyMsg(true);
    itsDirOp->setMode(KFile::Files);

    //
    // Now for the hack!
    TDEAction     *act;
    TDEActionMenu *topMnu=dynamic_cast<TDEActionMenu *>(itsDirOp->actionCollection()->action("popupMenu"));

    itsViewMenuAct=dynamic_cast<TDEActionMenu *>(itsDirOp->actionCollection()->action("view menu"));
    topMnu->popupMenu()->clear();
    connect(topMnu->popupMenu(), TQT_SIGNAL(aboutToShow()), TQT_SLOT(setupMenu()));
    if((act=itsDirOp->actionCollection()->action("up")))
        act->disconnect(TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(cdUp()));
    if((act=itsDirOp->actionCollection()->action("home")))
        act->disconnect(TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(home()));
    if((act=itsDirOp->actionCollection()->action("back")))
        act->disconnect(TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(back()));
    if((act=itsDirOp->actionCollection()->action("forward")))
        act->disconnect(TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(forward()));

    if((act=itsDirOp->actionCollection()->action("reload")))
        act->plug(toolbar);

    topMnu->insert(itsViewMenuAct);

    if((itsIconAct=dynamic_cast<TDERadioAction *>(itsDirOp->actionCollection()->action("short view"))))
    {
        disconnect(itsIconAct, TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(slotSimpleView()));
        connect(itsIconAct, TQT_SIGNAL(activated()), TQT_SLOT(iconView()));
        itsIconAct->plug(toolbar);
    }

    if((itsListAct=dynamic_cast<TDERadioAction *>(itsDirOp->actionCollection()->action("detailed view"))))
    {
        disconnect(itsListAct, TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(slotDetailedView()));
        connect(itsListAct, TQT_SIGNAL(activated()), TQT_SLOT(listView()));
        itsListAct->plug(toolbar);
    }

    itsShowBitmapAct=new TDEToggleAction(i18n("Show Bitmap Fonts"), "font_bitmap", 0, TQT_TQOBJECT(this), TQT_SLOT(filterFonts()), 
                                       itsDirOp->actionCollection(), "showbitmap");
    itsShowBitmapAct->setChecked(showBitmap);
    itsShowBitmapAct->plug(toolbar);

    toolbar->insertLineSeparator();

    act=new TDEAction(i18n("Add Fonts..."), "newfont", 0, TQT_TQOBJECT(this), TQT_SLOT(addFonts()), itsDirOp->actionCollection(), "addfonts");
    act->plug(toolbar);
    topMnu->insert(act);

    if((itsDeleteAct=itsDirOp->actionCollection()->action("delete")))
    {
        itsDeleteAct->plug(toolbar);
        itsDeleteAct->setEnabled(false);
        topMnu->insert(itsDeleteAct);
        disconnect(itsDeleteAct, TQT_SIGNAL(activated()), itsDirOp, TQT_SLOT(deleteSelected()));
        connect(itsDeleteAct, TQT_SIGNAL(activated()), this, TQT_SLOT(removeFonts()));
    }

    toolbar->insertLineSeparator();
    act=new TDEAction(i18n("Configure..."), "configure", 0, TQT_TQOBJECT(this), TQT_SLOT(configure()), itsDirOp->actionCollection(), "configure");
    act->plug(toolbar);
#ifdef HAVE_XFT
    toolbar->insertLineSeparator();
    act=new TDEAction(i18n("Print..."), "fileprint", 0, TQT_TQOBJECT(this), TQT_SLOT(print()), itsDirOp->actionCollection(), "print");
    act->plug(toolbar);
#endif

    if( (itsSepDirsAct=itsDirOp->actionCollection()->action("separate dirs")) &&
        (itsShowHiddenAct=itsDirOp->actionCollection()->action("show hidden")))
    {
        //disconnect(itsViewMenuAct->popupMenu(), TQT_SIGNAL(aboutToShow()), itsDirOp, TQT_SLOT(insertViewDependentActions()));
        connect(itsViewMenuAct->popupMenu(), TQT_SIGNAL(aboutToShow()), TQT_SLOT(setupViewMenu()));
        setupViewMenu();
    }

#ifdef HAVE_XFT
    if(itsPreview)
    {
        TDEActionCollection *previewCol=itsPreview->actionCollection();

        if(previewCol && previewCol->count()>0 && (act=previewCol->action("changeText")))
            act->plug(toolbar);
    }
#endif

    //
    // Set view...
    if(itsConfig.readBoolEntry(CFG_LISTVIEW, true))
        listView();
    else
        iconView();

    itsDirOp->dirLister()->setShowingDotFiles(true);

    connect(itsDirOp, TQT_SIGNAL(fileHighlighted(const KFileItem *)), TQT_SLOT(fileHighlighted(const KFileItem *)));
    connect(itsDirOp, TQT_SIGNAL(finishedLoading()), TQT_SLOT(loadingFinished()));
    connect(itsDirOp, TQT_SIGNAL(dropped(const KFileItem *, TQDropEvent *, const KURL::List &)),
                      TQT_SLOT(dropped(const KFileItem *, TQDropEvent *, const KURL::List &)));
    connect(itsDirOp->dirLister(), TQT_SIGNAL(infoMessage(const TQString &)), TQT_SLOT(infoMessage(const TQString &)));
    connect(itsDirOp, TQT_SIGNAL(updateInformation(int, int)), TQT_SLOT(updateInformation(int, int)));
}

CKCmFontInst::~CKCmFontInst()
{
#ifdef HAVE_XFT
    if(itsPreview)
    {
        itsConfig.setGroup(CFG_GROUP);
        itsConfig.writeEntry(CFG_SPLITTER_SIZES, itsSplitter->sizes());
    }
#endif
    delete itsDirOp;
}

void CKCmFontInst::setMimeTypes(bool showBitmap)
{
    TQStringList mimeTypes;

    mimeTypes << "application/x-font-ttf"
              << "application/x-font-otf"
              << "application/x-font-ttc"
              << "application/x-font-type1";
    if(showBitmap)
        mimeTypes << "application/x-font-pcf"
                  << "application/x-font-bdf";

    itsDirOp->setMimeFilter(mimeTypes);
}

void CKCmFontInst::filterFonts()
{
    setMimeTypes(itsShowBitmapAct->isChecked());
    itsDirOp->rereadDir();
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_SHOW_BITMAP, itsShowBitmapAct->isChecked());
    if(itsEmbeddedAdmin)
        itsConfig.sync();
}

TQString CKCmFontInst::quickHelp() const
{
    return Misc::root()
               ? i18n("<h1>Font Installer</h1><p> This module allows you to"
                      //" install TrueType, Type1, Speedo, and Bitmap"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy one into the folder.</p>")
               : i18n("<h1>Font Installer</h1><p> This module allows you to"
                      //" install TrueType, Type1, Speedo, and Bitmap"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy it into the appropriate folder - "
                      " \"Personal\" for fonts available to just yourself, or "
                      " \"System\" for system-wide fonts (available to all).</p>"
                      "<p><b>NOTE:</b> As you are not logged in as \"root\", any"
                      " fonts installed will only be available to you. To install"
                      " fonts system-wide, use the \"Administrator Mode\""
                      " button to run this module as \"root\".</p>");
}

void CKCmFontInst::listView()
{
    CKFileFontView *newView=new CKFileFontView(itsDirOp, "detailed view");

    itsDirOp->setView(newView);
    itsListAct->setChecked(true);
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, true);
    if(itsEmbeddedAdmin)
        itsConfig.sync();
    itsDirOp->setAcceptDrops(true);
}

void CKCmFontInst::iconView()
{
    CKFileFontIconView *newView=new CKFileFontIconView(itsDirOp, "simple view");

    itsDirOp->setView(newView);
    itsIconAct->setChecked(true);
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, false);
    if(itsEmbeddedAdmin)
        itsConfig.sync();
    itsDirOp->setAcceptDrops(true);
}

void CKCmFontInst::setupMenu()
{
    itsDirOp->setupMenu(KDirOperator::SortActions|/*KDirOperator::FileActions|*/KDirOperator::ViewActions);
}

void CKCmFontInst::setupViewMenu()
{
    itsViewMenuAct->remove(itsSepDirsAct);
    itsViewMenuAct->remove(itsShowHiddenAct);
}

void CKCmFontInst::fileHighlighted(const KFileItem *item)
{
    const KFileItemList *list=itsDirOp->selectedItems();

    itsDeleteAct->setEnabled(list && list->count());

#ifdef HAVE_XFT
    if(itsPreview)
    {
        //
        // Generate preview...
        const KFileItem *previewItem=item
                                       ? item
                                       : list && 1==list->count()
                                             ? list->getFirst()
                                             : NULL;

        if(previewItem && list && list->contains(previewItem))  // OK, check its been selected - not deselected!!!
            itsPreview->openURL(previewItem->url());
    }
#endif
}

void CKCmFontInst::loadingFinished()
{
    TQListView *lView=dynamic_cast<TQListView *>(itsDirOp->view());

    if(lView)
        lView->sort();
    else
    {
        TQIconView *iView=dynamic_cast<TQIconView *>(itsDirOp->view());

        if(iView)
            iView->sort();
    }
    fileHighlighted(NULL);
}

void CKCmFontInst::addFonts()
{
    KURL::List list=KFileDialog::getOpenURLs(TQString::null, "application/x-font-ttf application/x-font-otf "
                                                            "application/x-font-ttc application/x-font-type1 "
                                                            "application/x-font-pcf application/x-font-bdf",
                                                            //"application/x-font-snf application/x-font-speedo",
                                             this, i18n("Add Fonts"));

    if(list.count())
        addFonts(list, itsDirOp->url());
}

void CKCmFontInst::removeFonts()
{
    if(itsDirOp->selectedItems()->isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to delete."), i18n("Nothing to Delete"));
    else
    {
        KURL::List            urls;
        TQStringList           files;
        KFileItemListIterator it(*(itsDirOp->selectedItems()));

        for(; it.current(); ++it)
        {
            KURL url((*it)->url());

            url.setQuery(KFI_KIO_NO_CLEAR);
            files.append((*it)->text());
            urls.append(url);
        }

        bool doIt=false;

        switch(files.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancel(this,
                           i18n("<qt>Do you really want to delete\n <b>'%1'</b>?</qt>").arg(files.first()),
			   i18n("Delete Font"), KStdGuiItem::del());
            break;
            default:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancelList(this,
                           i18n("Do you really want to delete this font?", "Do you really want to delete these %n fonts?",
                                files.count()),
			   files, i18n("Delete Fonts"), KStdGuiItem::del());
        }

        if(doIt)
        {
            TDEIO::DeleteJob *job = TDEIO::del(urls, false, true);
            connect(job, TQT_SIGNAL(result(TDEIO::Job *)), this, TQT_SLOT(delResult(TDEIO::Job *)));
            job->setWindow(this);
            job->setAutoErrorHandlingEnabled(true, this);
        }
    }
}

void CKCmFontInst::configure()
{
    CSettingsDialog(this).exec();
}

void CKCmFontInst::print()
{
#ifdef HAVE_XFT
    KFileItemList list;
    bool          ok=false;

    for (KFileItem *item=itsDirOp->view()->firstFileItem(); item && !ok; item=itsDirOp->view()->nextItem(item))
        if(Print::printable(item->mimetype()))
            ok=true;

    if(ok)
    {
        const KFileItemList *list=itsDirOp->selectedItems();
        bool                select=false;

        if(list)
        {
            KFileItemList::Iterator it(list->begin()),
                                    end(list->end());

            for(; it!=end && !select; ++it)
                if(Print::printable((*it)->mimetype()))
                    select=true;
        }

        CPrintDialog dlg(this);

        itsConfig.setGroup(CFG_GROUP);
        if(dlg.exec(select, itsConfig.readNumEntry(CFG_FONT_SIZE, 1)))
        {
            static const int constSizes[]={0, 12, 18, 24, 36, 48};
    
            TQStringList       items;
            TQValueVector<int> sizes;
            CFcEngine         engine;

            if(dlg.outputAll())
            {
                for (KFileItem *item=itsDirOp->view()->firstFileItem(); item; item=itsDirOp->view()->nextItem(item))
                    items.append(item->name());
            }
            else
            {
                KFileItemList::Iterator it(list->begin()),
                                        end(list->end());

                for(; it!=end; ++it)
                    items.append((*it)->name());
            }
            Print::printItems(items, constSizes[dlg.chosenSize()], this, engine);
            itsConfig.writeEntry(CFG_FONT_SIZE, dlg.chosenSize());
            if(itsEmbeddedAdmin)
                itsConfig.sync();
        }
    }
    else
        KMessageBox::information(this, i18n("There are no printable fonts.\nYou can only print non-bitmap fonts."),
                                 i18n("Cannot Print"));
#endif
}

void CKCmFontInst::dropped(const KFileItem *i, TQDropEvent *, const KURL::List &urls)
{
    if(urls.count())
        addFonts(urls, i && i->isDir() ?  i->url() : itsDirOp->url());
}

void CKCmFontInst::infoMessage(const TQString &msg)
{
    itsStatusLabel->setText(msg);
}

static TQString family(const TQString &name)
{
    int commaPos=name.find(',');

    return -1==commaPos ? name : name.left(commaPos);
}

void CKCmFontInst::updateInformation(int, int fonts)
{
    TDEIO::filesize_t size=0;
    TQString         text(i18n("One Font", "%n Fonts", fonts));
    TQStringList     families;

    if(fonts>0)
    {
        KFileItem *item=NULL;

        for (item=itsDirOp->view()->firstFileItem(); item; item=itsDirOp->view()->nextItem(item))
        {
            TQString fam(family(item->text()));

            size+=item->size();
            if(-1==families.findIndex(fam))
                families+=fam;
        }
    }

    if(fonts>0)
    {
        text+=" ";
        text+=i18n("(%1 Total)").arg(TDEIO::convertSize(size));
    }
    text+=" - ";
    text+=i18n("One Family", "%n Families", families.count());
    itsStatusLabel->setText(text);
}

void CKCmFontInst::delResult(TDEIO::Job *job)
{
    //
    // To speed up font deletion, we dont rescan font list each time - so after this has completed, we need
    // to refresh font list before updating the directory listing...
    TQByteArray  packedArgs;
    TQDataStream stream(packedArgs, IO_WriteOnly);

    stream << KFI::SPECIAL_RESCAN;

    TDEIO::NetAccess::synchronousRun(TDEIO::special(KFI_KIO_FONTS_PROTOCOL ":/", packedArgs), this);
    jobResult(job);
}

void CKCmFontInst::jobResult(TDEIO::Job *job)
{
    //
    // Force an update of the view. For some reason the view is not automatically updated when
    // run in embedded mode - e.g. from the "Admin" mode button on KControl.
    itsDirOp->dirLister()->updateDirectory(itsDirOp->url());
    if(job && 0==job->error())
        KMessageBox::information(this,
#ifdef HAVE_XFT
                                 i18n("<p>Please note that any open applications will need to be restarted in order "
                                      "for any changes to be noticed.<p><p>(You will also have to restart this application "
                                      "in order to use its print function on any newly installed fonts.)</p>"),
#else
                                 i18n("Please note that any open applications will need to be restarted in order "
                                      "for any changes to be noticed."),
#endif
                                 i18n("Success"), "TDEFontinst_WarnAboutFontChangesAndOpenApps");
}

void CKCmFontInst::addFonts(const KURL::List &src, const KURL &dest)
{
    if(src.count())
    {
        KURL::List                copy(src);
        KURL::List::ConstIterator it;

        //
        // Check if font has any associated AFM or PFM file...
        for(it=src.begin(); it!=src.end(); ++it)
        {
            KURL::List associatedUrls;

            Misc::getAssociatedUrls(*it, associatedUrls, false, this);
            copy+=associatedUrls;
        }

        TDEIO::CopyJob *job=TDEIO::copy(copy, dest, true);
        connect(job, TQT_SIGNAL(result(TDEIO::Job *)), this, TQT_SLOT(jobResult(TDEIO::Job *)));
        job->setWindow(this);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

}

#include "KCmFontInst.moc"
