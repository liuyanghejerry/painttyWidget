#ifndef PENCIL_H
#define PENCIL_H

#include <QPen>
#include <QPainter>
#include <QVariant>
#include "brush.h"

class Pencil : public Brush
{
    Q_OBJECT
public:
    explicit Pencil(QObject *parent = 0);
    bool loadStencil(const QString &);
    void setColor(const QColor &color);
    int width();
    void setWidth(int w);
    QVariantMap brushInfo();
    QVariantMap defaultInfo();
signals:
    
public slots:
protected:
    void drawPoint(const QPointF &st);
    void drawLine(const QPointF &st,
                  const QPointF &end,
                  qreal &left);

protected:
    QPen pencil;
    
};

#endif // PENCIL_H
