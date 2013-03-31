#include "wetfingure.h"

WetFingure::WetFingure()
{
    name_ = "WetFingure";
    displayName_ = QObject::tr("Wet Fingure");
    shortcut_ = Qt::Key_F;
    setWidth(5);
}

int WetFingure::width()
{
    return width_;
}

void WetFingure::setWidth(int w)
{
    width_ = w;
    lastPointImage = QImage(width_, width_, QImage::Format_ARGB32);
    lastPointImage.fill(Qt::white);
}

QVariantMap WetFingure::defaultInfo()
{
    QVariantMap map;
    map.insert("width", 5);
    QVariantMap colorMap;
    colorMap.insert("red", 255);
    colorMap.insert("green", 255);
    colorMap.insert("blue", 255);
    map.insert("color", colorMap);
    return map;
}

QVariantMap WetFingure::brushInfo()
{
    QVariantMap map;
    map.insert("width", width_);
    QVariantMap colorMap;
    colorMap.insert("red", 255);
    colorMap.insert("green", 255);
    colorMap.insert("blue", 255);
    map.insert("color", colorMap);
    return map;
}

void WetFingure::start(const QPointF &st)
{
    QRectF rect(0, 0, width_, width_);
    rect.moveCenter(st);
    lastPointImage = surface_->imagePtr()->copy(rect.toRect()).toImage();
    lastPoint_ = st;
}

void WetFingure::lineTo(const QPointF &end)
{
    QPainter painter(surface_->imagePtr());
    painter.setOpacity(0.1);
    QPointF moveBy = (end - lastPoint_)/4;
    QRectF rect(0, 0, width_, width_);
    lastPoint_ += moveBy;
    painter.drawImage(lastPoint_-QPointF(width_/2.0,width_/2.0), lastPointImage);
    rect.moveCenter(lastPoint_);
    lastPointImage = surface_->imagePtr()->copy(rect.toRect()).toImage();
    lastPoint_ += moveBy;
    painter.drawImage(lastPoint_-QPointF(width_/2.0,width_/2.0), lastPointImage);
    rect.moveCenter(lastPoint_);
    lastPointImage = surface_->imagePtr()->copy(rect.toRect()).toImage();
    lastPoint_ += moveBy;
    painter.drawImage(lastPoint_-QPointF(width_/2.0,width_/2.0), lastPointImage);
    rect.moveCenter(lastPoint_);
    lastPointImage = surface_->imagePtr()->copy(rect.toRect()).toImage();
    lastPoint_ += moveBy;
    painter.drawImage(lastPoint_-QPointF(width_/2.0,width_/2.0), lastPointImage);
    rect.moveCenter(lastPoint_);
    lastPointImage = surface_->imagePtr()->copy(rect.toRect()).toImage();
    lastPoint_ = end;
}
