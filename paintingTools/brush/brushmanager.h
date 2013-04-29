#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <QSharedPointer>
#include <QMap>

class AbstractBrush;

typedef QSharedPointer<AbstractBrush> BrushPointer;

class BrushManager
{
public:
    bool addBrush(BrushPointer brush);
    QList<BrushPointer> allBrushes();
    BrushPointer getBrush(const QString &name);
    BrushPointer makeBrush(const QString &name);
private:
    QMap<QString, BrushPointer> registeredBrushes_;
};

#endif // BRUSHMANAGER_H
