// -*- c++ -*-


#ifndef KASLOADITEM_H
#define KASLOADITEM_H

#include "kasitem.h"

#include <kdemacros.h>

/**
 * An item that displays the system load.
 */
class KDE_EXPORT KasLoadItem : public KasItem
{
    Q_OBJECT

public:
    KasLoadItem( KasBar *parent );
    virtual ~KasLoadItem();

    void paint( TQPainter *p );

public slots:
    void updateDisplay();
    void showMenuAt( TQMouseEvent *ev );
    void showMenuAt( TQPoint p );

private:
    TQValueList<double> valuesOne;
    TQValueList<double> valuesFive;
    TQValueList<double> valuesFifteen;
};

#endif // KASLOADITEM_H

