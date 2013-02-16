/* vi: ts=8 sts=4 sw=4

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <tqcheckbox.h>
#include <tqevent.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>

#include <tdefiledialog.h>
#include <kimageio.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kurldrag.h>

#include "bgsettings.h"
#include "bgwallpaper.h"
#include "bgwallpaper_ui.h"

/**** BGMultiWallpaperList ****/

BGMultiWallpaperList::BGMultiWallpaperList(TQWidget *parent, const char *name)
	: TQListBox(parent, name)
{
   setAcceptDrops(true);
   setSelectionMode(TQListBox::Extended);
}


void BGMultiWallpaperList::dragEnterEvent(TQDragEnterEvent *ev)
{
   ev->accept(KURLDrag::canDecode(ev));
}


void BGMultiWallpaperList::dropEvent(TQDropEvent *ev)
{
   TQStringList files;
   KURL::List urls;
   KURLDrag::decode(ev, urls);
   for(KURL::List::ConstIterator it = urls.begin();
       it != urls.end(); ++it)
   {
      // TODO: Download remote files
      if ((*it).isLocalFile())
          files.append((*it).path());
   }
   insertStringList(files);
}

bool BGMultiWallpaperList::hasSelection()
{
    for ( unsigned i = 0; i < count(); i++)
    {
        if ( item( i ) && item( i )->isSelected() )
            return true;
    }
    return false;
}

void BGMultiWallpaperList::ensureSelectionVisible()
{
    for ( int i = topItem(); i < topItem() + numItemsVisible() - 1; i++)
        if ( item( i ) && item( i )->isSelected() )
            return;

    for ( unsigned i = 0; i < count(); i++)
        if ( item( i ) && item( i )->isSelected() )
        {
            setTopItem( i );
            return;
        }
}

/**** BGMultiWallpaperDialog ****/

BGMultiWallpaperDialog::BGMultiWallpaperDialog(KBackgroundSettings *settings,
	TQWidget *parent, const char *name)
	: KDialogBase(parent, name, true, i18n("Setup Slide Show"),
	Ok | Cancel, Ok, true), m_pSettings(settings)
{
   dlg = new BGMultiWallpaperBase(this);
   setMainWidget(dlg);

   dlg->m_spinInterval->setRange(1, 99999);
   dlg->m_spinInterval->setSteps(1, 15);
   dlg->m_spinInterval->setSuffix(i18n(" min"));

   // Load
   dlg->m_spinInterval->setValue(QMAX(1,m_pSettings->wallpaperChangeInterval()));

   dlg->m_listImages->insertStringList(m_pSettings->wallpaperList());

   if (m_pSettings->multiWallpaperMode() == KBackgroundSettings::Random)
      dlg->m_cbRandom->setChecked(true);

   connect(dlg->m_buttonAdd, TQT_SIGNAL(clicked()), TQT_SLOT(slotAdd()));
   connect(dlg->m_buttonRemove, TQT_SIGNAL(clicked()), TQT_SLOT(slotRemove()));
   connect(dlg->m_buttonMoveUp, TQT_SIGNAL(clicked()), TQT_SLOT(slotMoveUp()));
   connect(dlg->m_buttonMoveDown, TQT_SIGNAL(clicked()), TQT_SLOT(slotMoveDown()));
   connect(dlg->m_listImages,  TQT_SIGNAL(clicked ( TQListBoxItem * )), TQT_SLOT(slotItemSelected( TQListBoxItem *)));
   dlg->m_buttonRemove->setEnabled( false );
   dlg->m_buttonMoveUp->setEnabled( false );
   dlg->m_buttonMoveDown->setEnabled( false );

}

void BGMultiWallpaperDialog::slotItemSelected( TQListBoxItem * )
{
    dlg->m_buttonRemove->setEnabled( dlg->m_listImages->hasSelection() );
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::setEnabledMoveButtons()
{
    bool hasSelection = dlg->m_listImages->hasSelection();
    TQListBoxItem * item;

    item = dlg->m_listImages->firstItem();
    dlg->m_buttonMoveUp->setEnabled( hasSelection && item && !item->isSelected() );
    item  = dlg->m_listImages->item( dlg->m_listImages->count() - 1 );
    dlg->m_buttonMoveDown->setEnabled( hasSelection && item && !item->isSelected() );
}

void BGMultiWallpaperDialog::slotAdd()
{
    TQStringList mimeTypes = KImageIO::mimeTypes( KImageIO::Reading );
#ifdef HAVE_LIBART
    mimeTypes += "image/svg+xml";
#endif

    KFileDialog fileDialog(TDEGlobal::dirs()->findDirs("wallpaper", "").first(),
			   mimeTypes.join( " " ), this,
			   0L, true);

    fileDialog.setCaption(i18n("Select Image"));
    KFile::Mode mode = static_cast<KFile::Mode> (KFile::Files |
                                                 KFile::Directory |
                                                 KFile::ExistingOnly |
                                                 KFile::LocalOnly);
    fileDialog.setMode(mode);
    fileDialog.exec();
    TQStringList files = fileDialog.selectedFiles();
    if (files.isEmpty())
	return;

    dlg->m_listImages->insertStringList(files);
}

void BGMultiWallpaperDialog::slotRemove()
{
    int current = -1;
    for ( unsigned i = 0; i < dlg->m_listImages->count();)
    {
        TQListBoxItem * item = dlg->m_listImages->item( i );
        if ( item && item->isSelected())
        {
            dlg->m_listImages->removeItem(i);
            if (current == -1)
               current = i;
        }
        else
            i++;
    }
    if ((current != -1) && (current < (signed)dlg->m_listImages->count()))
       dlg->m_listImages->setSelected(current, true);

    dlg->m_buttonRemove->setEnabled(dlg->m_listImages->hasSelection());

    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotMoveUp()
{
    for ( unsigned i = 1; i < dlg->m_listImages->count(); i++)
    {
        TQListBoxItem * item = dlg->m_listImages->item( i );
        if ( item && item->isSelected() )
        {
            dlg->m_listImages->takeItem( item );
            dlg->m_listImages->insertItem( item, i - 1 );
        }
    }
    dlg->m_listImages->ensureSelectionVisible();
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotMoveDown()
{
    for ( unsigned i = dlg->m_listImages->count() - 1; i > 0; i--)
    {
        TQListBoxItem * item = dlg->m_listImages->item( i - 1 );
        if ( item && item->isSelected())
        {
            dlg->m_listImages->takeItem( item );
            dlg->m_listImages->insertItem( item, i );
        }
    }
    dlg->m_listImages->ensureSelectionVisible();
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotOk()
{
    TQStringList lst;
    for (unsigned i=0; i < dlg->m_listImages->count(); i++)
	lst.append(dlg->m_listImages->text(i));
    m_pSettings->setWallpaperList(lst);
    m_pSettings->setWallpaperChangeInterval(dlg->m_spinInterval->value());
    if (dlg->m_cbRandom->isChecked())
       m_pSettings->setMultiWallpaperMode(KBackgroundSettings::Random);
    else
       m_pSettings->setMultiWallpaperMode(KBackgroundSettings::InOrder);
    accept();
}


#include "bgwallpaper.moc"
