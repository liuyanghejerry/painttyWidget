#ifndef WETFINGURE_H
#define WETFINGURE_H

#include "abstractbrush.h"

class WetFingure : public AbstractBrush
{
public:
    WetFingure();
    int width();
    void setWidth(int w);
    QVariantMap defaultInfo();
    QVariantMap brushInfo();
    void start(const QPointF &st, qreal pressure=1.0);
    void lineTo(const QPointF &st, qreal pressure=1.0) override;

private:
    int width_;
    QImage lastPointImage;
};

#endif // WETFINGURE_H
