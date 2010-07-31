
#include "saverlist.h"

class SaverConfig;
class QPtrCollection;

int SaverList::compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2)
{
    SaverConfig *s1 = (SaverConfig *)item1;
    SaverConfig *s2 = (SaverConfig *)item2;

    return s1->name().localeAwareCompare(s2->name());
}
