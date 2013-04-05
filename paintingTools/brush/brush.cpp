#include "brush.h"

#include <QRadialGradient>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QEasingCurve>

/*!
     \class Brush

     \brief Brush class for painting on \l{Canvas}

     Brush is base class for all painting brush. However, it also can be directly used.

     Painting for a brush can be treated as several steps:
     \list
     \o loadStencil(). This will load a stencil file in 8bit gray format
     \o setDirectDraw(). Set true if we don't need more effect after drawing. This will make brush direct draw on a surface provided by user, usually from a layer via Canvas.
        If you need more steps after drawing, set to false. Then a QPixmap will be buffer.
     \o start(). In default, this function directly calls drawPoint() to draw a single point. For some special brushs, such as SketchBrush, this step do nothing.
     \o lineTo(). In default, this function directly calls drawLine().
     \o end(). In default, this function will calls clear() to clear QPixmap buffer if not direct draw on surface.
     \endlist
     Function setWidth() and setColor() should be used when change width and color. However, since change width and color depends on a stencil, you must load stencil before change these settings.

     In default, effect() has no effect. If you need some special effect after drawing, using effect() is a good way when sub-class.

     Most functions are virtual functions, this may help a lot when wirting new brush.

    \sa Canvas
*/

/*!
    \fn Brush::Brush(QObject *parent)

    Constructs a brush.

*/

Brush::Brush()
    :stencil(10, 10),
      width_(0)
{
    name_ = "Brush";
    displayName_ = QObject::tr("Brush");
    shortcut_ = Qt::Key_A;
    QVariantMap colorMap = this->defaultInfo()["color"].toMap();
    QColor c(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());
    this->setColor(c);
    this->setWidth(this->defaultInfo()["width"].toInt());
    icon_.addFile("iconset/ui/brush-1.png",
                  QSize(16, 16), QIcon::Disabled);
    icon_.addFile("iconset/ui/brush-2.png",
                  QSize(16, 16), QIcon::Active);
    icon_.addFile("iconset/ui/brush-3.png",
                  QSize(16, 16), QIcon::Selected);
    icon_.addFile("iconset/ui/brush-3.png",
                  QSize(16, 16), QIcon::Normal, QIcon::On);
    icon_.addFile("iconset/ui/brush-4.png",
                  QSize(16, 16), QIcon::Normal);
}

/*!
    \fn Brush::~Brush()

    Destructs brush.

*/

Brush::~Brush()
{
}

/*!
    \fn bool Brush::loadStencil(const QString &fileName)

    Loads \a fileName as stencil used by brush. Returns true if succeed.
    \note The \a fileName must be a 8bit 100*100 grayscale raw image file.

    \sa setSurface(), setDirectDraw()
*/

void Brush::makeStencil()
{
    stencil = QPixmap(width_, width_);
    stencil.fill(Qt::transparent);
    QEasingCurve easing(QEasingCurve::OutQuart);

    const QPoint center(width_/2, width_/2);
    QPainter painter;
    if(!painter.begin(&stencil)){
        return;
    }
    QRadialGradient gradient(center, width_/2);
    QColor color(mainColor);
    for(int i=0;i<100;++i){
        qreal value = i/100.0;
        color.setAlphaF(1-easing.valueForProgress(value));
        gradient.setColorAt(value, color);
    }
    gradient.setColorAt(1, Qt::transparent);
    gradient.setCenterRadius(width_/2);
    gradient.setFocalRadius(width_/2*hardness_/100.0 -1);
    QBrush brush(gradient);
    QPen pen(brush, width_);
    painter.setPen(pen);
    painter.drawPoint(width_/2, width_/2);
    painter.end();
}

/*!
    \fn int Brush::width()

    Returns width of current brush

    \sa setWidth()
*/

int Brush::width()
{
    return width_;
}

/*!
    \fn void Brush::setWidth(int w)

    Sets the width of brush to \a w.

    \sa width()
*/

void Brush::setWidth(int w)
{
    width_ = w;
    makeStencil();

    updateCursor(w);
}

/*!
    \fn QColor Brush::color()

    Returns current color of brush.

    \sa setColor()
*/

QColor Brush::color()
{
    return mainColor;
}

/*!
    \fn void Brush::setColor(const QColor &color)

    Sets the color of brush.

    \sa color()
*/

void Brush::setColor(const QColor &color)
{
    mainColor = color;
    makeStencil();
}

void Brush::setHardness(int h)
{
    hardness_ = h;
    makeStencil();
}

int Brush::hardness()
{
    return hardness_;
}

/*!
    \fn void Brush::drawPoint(const QPointF &st)

    Draws a point at \a st.
    \sa start(), drawLine()
*/

void Brush::drawPoint(const QPointF &st)
{
    QPainter painter;
    if(!painter.begin(surface_->imagePtr())){
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(st.x() - stencil.width()/2.0,
                       st.y() - stencil.width()/2.0,
                       stencil);
    painter.end();
}

/*!
    \fn void Brush::drawLine(const QPointF &st, const QPointF &end, qreal &left)

    Draws a line from \a st to \a end and record distance that left over after drawing to \a left.
    \sa drawPoint(), lineTo()
*/

void Brush::drawLine(const QPointF &st, const QPointF &end, qreal &left)
{
    QPainter painter;
    if(!painter.begin(surface_->imagePtr())){
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);
    qreal spacing = width_*0.1;

    qreal deltaX = end.x() - st.x();
    qreal deltaY = end.y() - st.y();

    qreal distance = sqrt( deltaX * deltaX + deltaY * deltaY );
    qreal stepX = 0.0;
    qreal stepY = 0.0;
    if ( distance > 0.0 ) {
        qreal invertDistance = 1.0 / distance;
        stepX = deltaX * invertDistance;
        stepY = deltaY * invertDistance;
    }

    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    qreal totalDistance = left + distance;

    while ( totalDistance >= spacing ) {
        if ( left > 0.0 ) {
            offsetX += stepX * (spacing - left);
            offsetY += stepY * (spacing - left);
            painter.drawPixmap(st.x() + offsetX - stencil.width()/2.0,
                               st.y() + offsetY - stencil.width()/2.0,
                               stencil);
            left -= spacing;
        } else {
            offsetX += stepX * spacing;
            offsetY += stepY * spacing;
            painter.drawPixmap(st.x() + offsetX - stencil.width()/2.0,
                               st.y() + offsetY - stencil.width()/2.0,
                               stencil);
        }
        totalDistance -= spacing;
    }
    left = totalDistance;
    painter.end();
}

/*!
    \fn QVariantMap Brush::brushInfo()
    Returns the basic info of current brush. These infos will be transfered to network.
    \sa defaultInfo()
*/

QVariantMap Brush::brushInfo()
{
    QVariantMap map;
    QVariantMap colorMap;
    colorMap.insert("red", this->color().red());
    colorMap.insert("green", this->color().green());
    colorMap.insert("blue", this->color().blue());
    map.insert("width", QVariant(this->width()));
    map.insert("hardness", QVariant(this->hardness()));
    map.insert("color", colorMap);
    map.insert("name", QVariant("Brush"));
    return map;
}

/*!
    \fn QVariantMap Brush::defaultInfo()
    Return default info of brush. After load a new brush, Canvas will set its brush settings to these default info.
    \sa brushInfo()
*/

QVariantMap Brush::defaultInfo()
{
    QVariantMap map;
    map.insert("width", QVariant(10));
    map.insert("hardness", QVariant(50));
    QVariantMap colorMap;
    colorMap.insert("red", 254);
    colorMap.insert("green", 127);
    colorMap.insert("blue", 127);

    map.insert("color", colorMap);
    return map;
}

AbstractBrush* Brush::createBrush()
{
    return new Brush;
}
