#ifndef __MODSYSTEMCLIENT_H
#define __MODSYSTEMCLIENT_H

#include <tqbitmap.h>
#include <kpixmap.h>
#include <kcommondecoration.h>
#include <kdecorationfactory.h>

class TQLabel;
class TQSpacerItem;

namespace ModernSystem {

class ModernSys;

class ModernButton : public KCommonDecorationButton
{
public:
    ModernButton(ButtonType type, ModernSys *parent, const char *name);
    void setBitmap(const unsigned char *bitmap);
    virtual void reset(unsigned long changed);
protected:

    virtual void drawButton(TQPainter *p);
    void drawButtonLabel(TQPainter *){;}
    TQBitmap deco;
};

class ModernSys : public KCommonDecoration
{
public:
    ModernSys( KDecorationBridge* b, KDecorationFactory* f );
    ~ModernSys(){;}

    virtual TQString visibleName() const;
    virtual TQString defaultButtonsLeft() const;
    virtual TQString defaultButtonsRight() const;
    virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
    virtual int tqlayoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
    virtual KCommonDecorationButton *createButton(ButtonType type);

    virtual void updateWindowShape();
    virtual void updateCaption();

    void init();
protected:
    void drawRoundFrame(TQPainter &p, int x, int y, int w, int h);
    void paintEvent( TQPaintEvent* );
    void recalcTitleBuffer();
    void reset( unsigned long );
private:
    TQPixmap titleBuffer;
    TQString oldTitle;
    bool reverse;
};

class ModernSysFactory : public TQObject, public KDecorationFactory
{
public:
    ModernSysFactory();
    virtual ~ModernSysFactory();
    virtual KDecoration* createDecoration( KDecorationBridge* );
    virtual bool reset( unsigned long changed );
    virtual bool supports( Ability ability );
    TQValueList< BorderSize > borderSizes() const;
private:
    void read_config();
};

}

#endif
