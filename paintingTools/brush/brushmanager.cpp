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
