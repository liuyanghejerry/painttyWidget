#include "brushmanager.h"
#include "abstractbrush.h"
#include "brush.h"
#include <QDebug>

bool BrushManager::addBrush(BrushPointer brush)
{
    registeredBrushes_.insert(
                brush->name()
                .trimmed()
                .toLower(),
                brush);
    return true;
}

QList<BrushPointer> BrushManager::allBrushes()
{
    return registeredBrushes_.values();
}

BrushPointer BrushManager::getBrush(const QString &name)
{
    return registeredBrushes_.value(name
                                    .trimmed()
                                    .toLower());
}

BrushPointer BrushManager::makeBrush(const QString &name)
{
    BrushPointer ptr = registeredBrushes_.value(name
                                                .trimmed()
                                                .toLower());
    if(ptr.isNull()){
        qWarning()<<"Brush"<<name<<" cannot identify";

        // use Brush to fall back
        BrushPointer nptr = BrushPointer(new Brush);
        return nptr;
    }
    BrushPointer nptr = BrushPointer(ptr->createBrush());
    return nptr;
}
