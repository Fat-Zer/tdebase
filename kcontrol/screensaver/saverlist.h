#ifndef SAVERLIST_H
#define SAVERLIST_H

#include <tqptrlist.h>

#include "saverconfig.h"

class SaverList : public TQPtrList<SaverConfig>
{
protected:
    virtual int compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2);
};

#endif
