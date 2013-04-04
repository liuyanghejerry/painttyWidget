#include "brushmanager.h"
#include "abstractbrush.h"

QMap<QString, BrushPointer> BrushManager::registeredBrushes_;

bool BrushManager::addBrush(BrushPointer brush)
{
    registeredBrushes_.insert(
                brush->brushInfo().value("name", "").toString(),
                brush);
    return true;
}

QList<BrushPointer> BrushManager::allBrushes()
{
    return registeredBrushes_.values();
}

BrushPointer BrushManager::getBrush(const QString &name)
{
    return registeredBrushes_.value(name);
}

BrushPointer BrushManager::makeBrush(const QString &name)
{
    BrushPointer ptr = registeredBrushes_.value(name);
    if(ptr.isNull()){
        return ptr;
    }
    BrushPointer nptr = BrushPointer(ptr->createBrush());
    return nptr;
}
