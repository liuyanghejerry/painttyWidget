#include "eraser.h"

Eraser::Eraser() :
    width_(10)
{
    name_ = "Eraser";
    displayName_ = QObject::tr("Eraser");
    shortcut_ = Qt::Key_E;
    mainColor = Qt::transparent;

    icon_.addFile("iconset/ui/eraser-1.png",
                  QSize(16, 16), QIcon::Disabled);
    icon_.addFile("iconset/ui/eraser-2.png",
                  QSize(16, 16), QIcon::Active);
    icon_.addFile("iconset/ui/eraser-3.png",
                  QSize(16, 16), QIcon::Selected);
    icon_.addFile("iconset/ui/eraser-3.png",
                  QSize(16, 16), QIcon::Normal, QIcon::On);
    icon_.addFile("iconset/ui/eraser-4.png",
                  QSize(16, 16), QIcon::Normal);
    updateCursor(this->width());
}

Eraser::~Eraser()
{
    //
}

void Eraser::setColor(const QColor &c)
{
    mainColor = c;
    return;
}

int Eraser::width()
{
    return width_;
}

void Eraser::setWidth(int w)
{
    width_ = w;
    updateCursor(w);
}

void Eraser::drawPoint(const QPointF &st)
{
    QPainter painter(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen eraser(Qt::transparent);
    eraser.setWidth(width_);
    eraser.setCapStyle(Qt::RoundCap);
    eraser.setJoinStyle(Qt::RoundJoin);
    painter.setPen(eraser);
    painter.setBrush(Qt::transparent);
    painter.drawPoint(st.x(), st.y());
}

void Eraser::drawLine(const QPointF &st,
                      const QPointF &end,
                      qreal &)
{
    QPainter painter(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen eraser(Qt::transparent);
    eraser.setWidth(width_);
    eraser.setCapStyle(Qt::RoundCap);
    eraser.setJoinStyle(Qt::RoundJoin);
    painter.setPen(eraser);
    painter.setBrush(Qt::transparent);
    painter.drawLine(st, end);
}

QVariantMap Eraser::brushInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", this->color().red());
    colorMap.insert("green", this->color().green());
    colorMap.insert("blue", this->color().blue());
    map.insert("width", this->width());
    map.insert("hardness", this->hardness());
    map.insert("color", colorMap);
    map.insert("name", this->name());
    return map;
}

QVariantMap Eraser::defaultInfo()
{
    QVariantMap map;
    map.insert("width", 10);
    map.insert("hardness", 100);
    QVariantMap colorMap;
    colorMap.insert("red", 255);
    colorMap.insert("green", 255);
    colorMap.insert("blue", 255);
    map.insert("color", colorMap);
    return map;
}

AbstractBrush* Eraser::createBrush()
{
    return new Eraser;
}
