#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <QSharedPointer>
#include <QMap>

class AbstractBrush;

typedef QSharedPointer<AbstractBrush> BrushPointer;

class BrushManager
{
public:
    static bool addBrush(BrushPointer brush);
    static QList<BrushPointer> allBrushes();
private:
    static QMap<QString, BrushPointer> registeredBrushes_;
    BrushManager();
};

#endif // BRUSHMANAGER_H
