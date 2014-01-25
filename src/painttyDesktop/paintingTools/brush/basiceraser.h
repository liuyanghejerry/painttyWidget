#ifndef BASICERASER_H
#define BASICERASER_H

#include "abstractbrush.h"
#include <QBrush>
#include <QPainter>
#include <QPen>

class BasicEraser : public AbstractBrush
{
public:
    BasicEraser();

    void drawPoint(const QPoint& p, qreal pressure=1) Q_DECL_OVERRIDE;
    void drawLineTo(const QPoint& end, qreal pressure=1) Q_DECL_OVERRIDE;
    AbstractBrush* createBrush() Q_DECL_OVERRIDE;
protected:
    QBrush brush_;
    QPen pen_;
    QPainter painter_;
};

#endif // BASICERASER_H
