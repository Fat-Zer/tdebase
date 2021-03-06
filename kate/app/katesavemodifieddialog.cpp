/* This file is part of the KDE project
   Copyright (C) 2004 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "katesavemodifieddialog.h"
#include "katesavemodifieddialog.moc"

#include <tdelocale.h>
#include <tqlistview.h>
#include <tdelistview.h>
#include <kguiitem.h>
#include <kactivelabel.h>
#include <kstdguiitem.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <tdetexteditor/encodinginterface.h>

class AbstractKateSaveModifiedDialogCheckListItem:public TQCheckListItem {
public:
        AbstractKateSaveModifiedDialogCheckListItem(TQListViewItem *parent,const TQString& title, const TQString& url):TQCheckListItem(parent,title,TQCheckListItem::CheckBox) {
                setText(1,url);
                setOn(true);
		setState(InitialState);
        }
        virtual ~AbstractKateSaveModifiedDialogCheckListItem() {
        }
        virtual bool synchronousSave(TQWidget *dialogParent)=0;
	enum STATE{InitialState,SaveOKState,SaveFailedState};
	STATE state() const { return m_state;}
	void setState(enum STATE state) {
		m_state=state;
		TDEIconLoader *loader = TDEGlobal::instance()->iconLoader();
		switch (state) {
			case InitialState:
				setPixmap(0,TQPixmap());
				break;
			case SaveOKState:
				setPixmap(0,loader->loadIcon("ok",TDEIcon::NoGroup,height()));
				break;
			case SaveFailedState:
				setPixmap(0,loader->loadIcon("cancel",TDEIcon::NoGroup,height()));
				break;
		}
	}
private:
	STATE m_state;
};

class KateSaveModifiedDocumentCheckListItem:public AbstractKateSaveModifiedDialogCheckListItem {
public:
	KateSaveModifiedDocumentCheckListItem(TQListViewItem *parent,Kate::Document *document):AbstractKateSaveModifiedDialogCheckListItem(parent,document->docName(),document->url().prettyURL()){
		m_document=document;
	}
	virtual ~KateSaveModifiedDocumentCheckListItem() {
	}
	virtual bool synchronousSave(TQWidget *dialogParent) {
		if (m_document->url().isEmpty() ) {
		        KEncodingFileDialog::Result r=KEncodingFileDialog::getSaveURLAndEncoding(
        	      	KTextEditor::encodingInterface(m_document)->encoding(),TQString::null,TQString::null,dialogParent,i18n("Save As (%1)").arg(m_document->docName()));

			m_document->setEncoding( r.encoding );
			if (!r.URLs.isEmpty()) {
				KURL tmp = r.URLs.first();
        	  		if ( !m_document->saveAs( tmp ) ) {
					setState(SaveFailedState);
					setText(1,m_document->url().prettyURL());
					return false;
				} else {
					bool sc=m_document->waitSaveComplete();
					setText(1,m_document->url().prettyURL());
					if (!sc) {
						setState(SaveFailedState);
						return false;
					} else {
						setState(SaveOKState);
						return true;
					}
				}
			} else {
				setState(SaveFailedState);
				return false;
			}
		} else { //document has an exising location
          		if ( !m_document->save() ) {
				setState(SaveFailedState);
				setText(1,m_document->url().prettyURL());
				return false;
			} else {
				bool sc=m_document->waitSaveComplete();
				setText(1,m_document->url().prettyURL());
				if (!sc) {
					setState(SaveFailedState);
					return false;
				} else {
					setState(SaveOKState);
					return true;
				}
			}

		}

		return false;

	}
private:
	Kate::Document *m_document;
};

KateSaveModifiedDialog::KateSaveModifiedDialog(TQWidget *parent, TQPtrList<Kate::Document> documents):
	KDialogBase( parent, "KateSaveModifiedDialog", true, i18n("Save Documents"), Yes | No | Cancel) {

	KGuiItem saveItem=KStdGuiItem::save();
	saveItem.setText(i18n("&Save Selected"));
	setButtonGuiItem(KDialogBase::Yes,saveItem);

	setButtonGuiItem(KDialogBase::No,KStdGuiItem::dontSave());

	KGuiItem cancelItem=KStdGuiItem::cancel();
	cancelItem.setText(i18n("&Abort Closing"));
	setButtonGuiItem(KDialogBase::Cancel,cancelItem);

	TQVBox *box=makeVBoxMainWidget();
	new KActiveLabel(i18n("<qt>The following documents have been modified. Do you want to save them before closing?</qt>"),box);
	m_list=new TDEListView(box);
	m_list->addColumn(i18n("Title"));
	m_list->addColumn(i18n("Location"));
	m_list->setRootIsDecorated(true);
	m_list->setResizeMode(TQListView::LastColumn);
	if (0) {
		m_projectRoot=new TQListViewItem(m_list,i18n("Projects"));
	} else m_projectRoot=0;
	if (documents.count()>0) {
		m_documentRoot=new TQListViewItem(m_list,i18n("Documents"));
		const uint docCnt=documents.count();
		for (uint i=0;i<docCnt;i++) {
			new KateSaveModifiedDocumentCheckListItem(m_documentRoot,documents.at(i));
		}
		m_documentRoot->setOpen(true);
	} else m_documentRoot=0;
	//FIXME - Is this the best way?
	connect(m_list, TQT_SIGNAL(clicked(TQListViewItem *)), TQT_SLOT(slotItemSelected()));
	connect(m_list, TQT_SIGNAL(doubleClicked(TQListViewItem *)), TQT_SLOT(slotItemSelected()));
	connect(m_list, TQT_SIGNAL(spacePressed(TQListViewItem *)), TQT_SLOT(slotItemSelected()));
	if(documents.count()>3) { //For 3 or less, it would be quicker just to tick or untick them yourself, so don't clutter the gui.
		connect(new TQPushButton(i18n("Se&lect All"),box),TQT_SIGNAL(clicked()),this,TQT_SLOT(slotSelectAll()));
	}
}

KateSaveModifiedDialog::~KateSaveModifiedDialog() {
}

void KateSaveModifiedDialog::slotItemSelected() {
	kdDebug(13001) << "slotItemSelected()" << endl;

	for(TQListViewItem *it=m_documentRoot->firstChild();it;it=it->nextSibling()) {
		if(((TQCheckListItem*)it)->isOn()) {
			enableButton(KDialogBase::Yes, true);
			return;
		}
	}
	enableButton(KDialogBase::Yes, false);
}

static void selectItems(TQListViewItem *root) {
	if (!root) return;
	for (TQListViewItem *it=root->firstChild();it;it=it->nextSibling()) {
		((TQCheckListItem*)it)->setOn(true);
	}
}

void KateSaveModifiedDialog::slotSelectAll() {
	selectItems(m_documentRoot);
	slotItemSelected();
}


void KateSaveModifiedDialog::slotUser2() {
	kdDebug(13001)<<"KateSaveModifiedDialog::slotYes()"<<endl;
	if (doSave(m_documentRoot)) done(TQDialog::Accepted);
}

void KateSaveModifiedDialog::slotUser1() {
	done(TQDialog::Accepted);
}

bool KateSaveModifiedDialog::doSave(TQListViewItem *root) {
	if (root) {
		for (TQListViewItem *it=root->firstChild();it;it=it->nextSibling()) {
			AbstractKateSaveModifiedDialogCheckListItem *cit= (AbstractKateSaveModifiedDialogCheckListItem*)it;
			if (cit->isOn() && (cit->state()!=AbstractKateSaveModifiedDialogCheckListItem::SaveOKState)) {
				if (!cit->synchronousSave(this /*perhaps that should be the kate mainwindow*/)) {
					KMessageBox::sorry( this, i18n("Data you requested to be saved could not be written. Please choose how you want to proceed."));
					return false;
				}
			} else if ((!cit->isOn()) && (cit->state()==AbstractKateSaveModifiedDialogCheckListItem::SaveFailedState)) {
				cit->setState(AbstractKateSaveModifiedDialogCheckListItem::InitialState);
			}

		}
	}
	return true;
}

bool KateSaveModifiedDialog::queryClose(TQWidget *parent,TQPtrList<Kate::Document> documents) {
	KateSaveModifiedDialog d(parent,documents);
	return (d.exec()!=TQDialog::Rejected);
}
