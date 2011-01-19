/*
  'Web' kwin client

  Copyright (C) 2005 Sandro Giessl <sandro@giessl.com>
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KWIN_WEB_H
#define KWIN_WEB_H

#include "../../lib/kcommondecoration.h"
#include "../../lib/kdecorationfactory.h"

class TQLabel;
class TQSpacerItem;
class TQBoxLayout;

namespace Web
{

  class WebButton;

  class WebClient : public KCommonDecoration
  {
    public:

      WebClient(KDecorationBridge* bridge, KDecorationFactory* factory);
      ~WebClient();

      virtual TQString visibleName() const;
      virtual TQString defaultButtonsLeft() const;
      virtual TQString defaultButtonsRight() const;
      virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
      virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
      virtual KCommonDecorationButton *createButton(ButtonType type);

      virtual void updateWindowShape();

      virtual void init();

    protected:
      virtual void reset( unsigned long changed );

      virtual void paintEvent(TQPaintEvent *);

    private:

      int titleHeight_, borderSize_;

      bool tqshape_;

      TQBitmap       _buttonBitmap(ButtonType t) const;
  };

  class WebFactory : public TQObject, public KDecorationFactory
  {
    Q_OBJECT

    public:

      WebFactory() {};
      virtual ~WebFactory() {};
      virtual KDecoration* createDecoration( KDecorationBridge* );
      virtual bool reset( unsigned long changed );
      virtual bool supports( Ability ability );
      virtual TQValueList< BorderSize > borderSizes() const;
  };
}

#endif
// vim:ts=2:sw=2:tw=78:set et:
// kate: indent-width 2; replace-tabs on; tab-width 2; space-indent on;
