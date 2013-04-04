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
    static BrushPointer getBrush(const QString &name);
    static BrushPointer makeBrush(const QString &name);
private:
    static QMap<QString, BrushPointer> registeredBrushes_;
    BrushManager();
};

#endif // BRUSHMANAGER_H
