#include "brush.h"

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
{
    shortcut_ = Qt::Key_A;
    brushData = new uchar[1];
    loadStencil("iconset/brush/brush.raw");
    QVariantMap colorMap = this->defaultInfo()["color"].toMap();
    QColor c(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());
    this->setColor(c);
    this->setWidth(this->defaultInfo()["width"].toInt());
}

/*!
    \fn Brush::~Brush()

    Destructs brush.

*/

Brush::~Brush()
{
    delete [] brushData;
    stencilFile.close();
}

/*!
    \fn bool Brush::loadStencil(const QString &fileName)

    Loads \a fileName as stencil used by brush. Returns true if succeed.
    \note The \a fileName must be a 8bit 100*100 grayscale raw image file.

    \sa setSurface(), setDirectDraw()
*/


bool Brush::loadStencil(const QString &fileName)
{
    char *brushPrototype; // raw brush prototype

    stencilFile.setFileName(fileName);
    // TODO: more native way to due with the error
    bool ret = stencilFile.open(QIODevice::ReadOnly);
    Q_ASSERT(ret);
    if(!ret) return false;
    QDataStream in;
    in.setDevice(&stencilFile);

    int size = stencilFile.size();  // set size to the length of the raw file
    if(!size){
        return false;
    }
    brushPrototype = new char[size];  // create the brush prototype array
    in.readRawData(brushPrototype, size);  // read the file into the prototype
    stencilFile.close();
    brushData = new uchar[size];  // create the uchar array you need to construct QImage

    for (int i = 0; i < size; ++i)
        brushData[i] = (uchar)brushPrototype[i];  // copy the char to the uchar array

    QImage img1(brushData, 100, 100, QImage::Format_Indexed8); // create QImage from the brush data array

    QImage img2(100, 100, QImage::Format_ARGB32);


    QVector<QRgb> vectorColors(256);  // create a color table for the image
    for (int c = 0; c < 256; c++)
        vectorColors[c] = qRgb(c, c, c);

    img1.setColorTable(vectorColors);  // set the color table to the image

    for (int iX = 0; iX < 100; ++iX)
    {
        for (int iY = 0; iY < 100; ++iY)
        {
            img2.setPixel(iX, iY, qRgba(255, 0, 0 , (255-qGray(img1.pixel(iX, iY)))*0.1));
        }
    }

    originalStencil = img1;
    stencil = QPixmap::fromImage(img2);
    delete [] brushPrototype;
    return ret;
}

/*!
    \fn int Brush::width()

    Returns width of current brush

    \sa setWidth()
*/

int Brush::width()
{
    return stencil.width();
}

/*!
    \fn void Brush::setWidth(int w)

    Sets the width of brush to \a w.

    \sa width()
*/

void Brush::setWidth(int w)
{
    if(stencil.isNull()) return;
    stencil = stencil.scaled(w,w);
    setColor(color());
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
    QImage img(stencil.size(), QImage::Format_ARGB32);
    QImage originalStencil_sized =
            originalStencil.scaled(stencil.size());


    for(int iX = 0; iX < img.width(); ++iX)
    {
        for(int iY = 0; iY < img.width(); ++iY)
        {
            int r = color.red();
            int g = color.green();
            int b = color.blue();
            int a = (255 -
                     qGray(originalStencil_sized.pixel(iX, iY)))
                    *color.alphaF();
            img.setPixel(iX, iY, qRgba(r, g, b, a));
        }
    }
    mainColor = color;

    stencil = QPixmap::fromImage(img);
}

void Brush::setHardness(int h)
{
    hardness_ = h;
}

int Brush::hardness()
{
    return hardness_;
}

QIcon Brush::icon()
{
    return icon_;
}

QCursor Brush::cursor()
{
    return cursor_;
}

QKeySequence Brush::shortcut()
{
    return shortcut_;
}

/*!
    \fn void Brush::start(const QPointF &st)

    Starts a new drawing action at \a st.
    Internally, this function calls drawPoint().

    \sa lineTo(), end()
*/

void Brush::start(const QPointF &st)
{
    leftOverDistance = 0;
    drawPoint(st);
    lastPoint_ = st;
}

/*!
    \fn void Brush::lineTo(const QPointF &st)

    Draws a line from last point to \a st. And \a st will be the last point at next time.
    This function usually used with start().
    Internally, this function calls drawLine().
    \sa start(), end()
*/

void Brush::lineTo(const QPointF &st)
{    
    if(lastPoint_.isNull()){
        start(st);
        return;
    }
    this->drawLine(lastPoint_, st, leftOverDistance);
    lastPoint_ = st;
}

/*!
    \fn void Brush::drawPoint(const QPointF &st)

    Draws a point at \a st.
    \sa start(), drawLine()
*/

void Brush::drawPoint(const QPointF &st)
{
    QPainter painter;
    painter.begin(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(st.x() - stencil.width()/2.0,st.y() - stencil.width()/2.0, stencil);
}

/*!
    \fn void Brush::drawLine(const QPointF &st, const QPointF &end, qreal &left)

    Draws a line from \a st to \a end and record distance that left over after drawing to \a left.
    \sa drawPoint(), lineTo()
*/

void Brush::drawLine(const QPointF &st, const QPointF &end, qreal &left)
{
    QPainter painter;
    painter.begin(surface_->imagePtr());
    painter.setRenderHint(QPainter::Antialiasing);
    qreal spacing = stencil.width()*0.1;

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
    QVariantMap colorMap;
    colorMap.insert("red", 254);
    colorMap.insert("green", 127);
    colorMap.insert("blue", 127);

    map.insert("color", colorMap);
    return map;
}
