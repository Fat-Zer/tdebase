/* This file is part of the KDE project
   Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation version 2.0

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KONQ_INFOLISTVIEWWIDGET_H__
#define __KONQ_INFOLISTVIEWWIDGET_H__

#include "konq_listviewwidget.h"

#include <kurl.h>
#include <tqmap.h>
#include <tqpair.h>

namespace TDEIO {class MetaInfoJob;}
class KonqListView;
class TDESelectAction;

/**
 * The info list view
 */
class KonqInfoListViewWidget : public KonqBaseListViewWidget
{
//   friend class KonqTextViewItem;
   Q_OBJECT
   public:
      KonqInfoListViewWidget( KonqListView *parent, TQWidget *parentWidget );
      ~KonqInfoListViewWidget();
      
     const TQStringList columnKeys() {return m_columnKeys;}
      
      virtual bool openURL( const KURL &url );

   protected slots:
      // slots connected to the directory lister
//      virtual void setComplete();
      virtual void slotNewItems( const KFileItemList & );
      virtual void slotRefreshItems( const KFileItemList & );
      virtual void slotDeleteItem( KFileItem * );
      virtual void slotClear();
      virtual void slotSelectMimeType();
      
      void slotMetaInfo(const KFileItem*);
      void slotMetaInfoResult();
      
   protected:
       void determineCounts(const KFileItemList& list);
       void rebuildView();
   
      virtual void createColumns();
      void createFavoriteColumns();
      
      /**
       * @internal
       */
      struct KonqILVMimeType
      {
          KonqILVMimeType() : mimetype(0), count(0), hasPlugin(false) {};

          KMimeType::Ptr  mimetype;
          int             count;
          bool            hasPlugin;
      };

      // all the mimetypes
      TQMap<TQString, KonqILVMimeType > m_counts; 
      TQStringList                     m_columnKeys;

      KonqILVMimeType                 m_favorite;
      
      TDESelectAction*                  m_mtSelector;
      TDEIO::MetaInfoJob*               m_metaInfoJob;
      KFileItemList                   m_metaInfoTodo;
};

#endif
