/*
 *
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KWINDECORATION_PREVIEW_H
#define KWINDECORATION_PREVIEW_H

#include <tqwidget.h>
#include <kdecoration_p.h>
#include <kdecoration_plugins_p.h>

class TQLabel;

class KDecorationPreviewBridge;
class KDecorationPreviewOptions;

class KDecorationPreview
    : public TQWidget
    {
    Q_OBJECT
    public:
        // Note: Windows can't be added or removed without making changes to
        //       the code, since parts of it assume there's just an active
        //       and an inactive window.
        enum Windows { Inactive = 0, Active, NumWindows };

        KDecorationPreview( TQWidget* parent = NULL, const char* name = NULL );
        virtual ~KDecorationPreview();
        bool recreateDecoration( KDecorationPlugins* plugin );
        void enablePreview();
        void disablePreview();
        void setPreviewMask( const TQRegion&, int, bool );
        TQRegion unobscuredRegion( bool, const TQRegion& ) const;
        TQRect windowGeometry( bool ) const;
        void setTempBorderSize(KDecorationPlugins* plugin, KDecorationDefines::BorderSize size);
        void setTempButtons(KDecorationPlugins* plugin, bool customEnabled, const TQString &left, const TQString &right);
    protected:
        virtual void resizeEvent( TQResizeEvent* );
    private:
        void positionPreviews();
        KDecorationPreviewOptions* options;
        KDecorationPreviewBridge* bridge[NumWindows];
        KDecoration* deco[NumWindows];
        TQLabel* no_preview;
        TQRegion mask;
    };

class KDecorationPreviewBridge
    : public KDecorationBridge
    {
    public:
        KDecorationPreviewBridge( KDecorationPreview* preview, bool active );
    	virtual bool isActive() const;
	virtual bool isCloseable() const;
	virtual bool isMaximizable() const;
	virtual MaximizeMode maximizeMode() const;
	virtual bool isMinimizable() const;
        virtual bool providesContextHelp() const;
        virtual int desktop() const;
        virtual bool isModal() const;
        virtual bool isShadeable() const;
        virtual bool isShade() const;
        virtual bool isSetShade() const;
        virtual bool keepAbove() const;
        virtual bool keepBelow() const;
        virtual bool isMovable() const;
        virtual bool isResizable() const;
        virtual NET::WindowType windowType( unsigned long supported_types ) const;
	virtual TQIconSet icon() const;
	virtual TQString caption() const;
	virtual void processMousePressEvent( TQMouseEvent* );
	virtual void showWindowMenu( const TQRect &);
	virtual void showWindowMenu( TQPoint );
	virtual void performWindowOperation( WindowOperation );
        virtual void setMask( const TQRegion&, int );
        virtual bool isPreview() const;
        virtual TQRect geometry() const;
        virtual TQRect iconGeometry() const;
        virtual TQRegion unobscuredRegion( const TQRegion& r ) const;
        virtual TQWidget* workspaceWidget() const;
        virtual WId windowId() const;
	virtual void closeWindow();
	virtual void maximize( MaximizeMode mode );
	virtual void minimize();
        virtual void showContextHelp();
        virtual void setDesktop( int desktop );
        virtual void titlebarDblClickOperation();
        virtual void titlebarMouseWheelOperation( int delta );
        virtual void setShade( bool set );
        virtual void setKeepAbove( bool );
        virtual void setKeepBelow( bool );
        virtual int currentDesktop() const;
        virtual TQWidget* initialParentWidget() const;
        virtual Qt::WFlags initialWFlags() const;
        virtual void helperShowHide( bool show );
        virtual void grabXServer( bool grab );
    private:
        KDecorationPreview* preview;
        bool active;
    };

class KDecorationPreviewOptions
    : public KDecorationOptions
    {
    public:
        KDecorationPreviewOptions();
        virtual ~KDecorationPreviewOptions();
        virtual unsigned long updateSettings();

        void setCustomBorderSize(BorderSize size);
        void setCustomTitleButtonsEnabled(bool enabled);
        void setCustomTitleButtons(const TQString &left, const TQString &right);

    private:
        BorderSize customBorderSize;
        bool customButtonsChanged;
        bool customButtons;
        TQString customTitleButtonsLeft;
        TQString customTitleButtonsRight;
    };

class KDecorationPreviewPlugins
    : public KDecorationPlugins
    {
    public:
        KDecorationPreviewPlugins( KConfig* cfg );
        virtual bool provides( Requirement );
    };

inline KDecorationPreviewPlugins::KDecorationPreviewPlugins( KConfig* cfg )
    : KDecorationPlugins( cfg )
    {
    }

#endif
