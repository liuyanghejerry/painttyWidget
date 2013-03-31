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
    void start(const QPointF &st);
    void lineTo(const QPointF &st) override;

private:
    int width_;
    QImage lastPointImage;
};

#endif // WETFINGURE_H
