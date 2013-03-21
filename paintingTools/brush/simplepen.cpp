#include "simplepen.h"
#include <QIcon>
#include <QKeySequence>
#include <QCursor>
#include <QVariantMap>

SimplePen::SimplePen(QObject *parent) :
    BasicBrush(parent), _brushWidth(0), _surface(0)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
}

QString SimplePen::brushIdentifer() const
{
    //brush identifer for communicating with each client
    return "mrpaint.simplepen";
}

QString SimplePen::brushName() const
{
    //brush name for displaying on the toolbar
    return tr("Simple Pen");
}

QIcon SimplePen::brushIcon() const
{
    //icon for this brush
    return QIcon();
}

QKeySequence SimplePen::brushShotcut() const
{
    //shortcut sequence for this brush
    //response to this sequence is not the brush's job
    return QKeySequence("P");
}

QCursor SimplePen::brushCursor() const
{
    //cursor for this brush when drawing
    return Qt::ArrowCursor;
}

int SimplePen::brushWidth() const
{
    return _brushWidth;
}

void SimplePen::setBrushWidth(int width)
{
    _brushWidth = width;
    QPen pen = painter.pen();
    pen.setWidth(_brushWidth);
    painter.setPen(pen);

    //suppose we have a member function to construct a new cursor and emit it.
    [=] {
        emit newBrushCursor(Qt::ArrowCursor);
    };
}
 QColor SimplePen::brushColor() const
 {
     return _brushColor;
 }

 void SimplePen::setBrushColor(const QColor &color)
 {
     _brushColor = color;
     QPen pen = painter.pen();
     pen.setColor(_brushColor);
     painter.setPen(pen);

     //if cursor changes with color, also emit this signal
//     [=] {
//         emit newBrushCursor(Qt::ArrowCursor);
//     };
 }

 int SimplePen::brushSolidity() const
 {
     return 100;
 }

 void SimplePen::setBrushSolidity(int solidity)
 {
 }

 QVariantMap SimplePen::brushInfo() const
 {
     //generate a information map for communicating
     //you can insert whatever you want, as long as setBrushInfo() needs them
     QVariantMap map;
     map.insert("identifer", brushIdentifer());
     map.insert("width", _brushWidth);
     map.insert("color", _brushColor);
     map.insert("solidity", brushSolidity());
     return map;
 }

 void SimplePen::setBrushInfo(const QVariantMap &brushInfo)
 {
     //set brush's properties using information map
     //chech identifer first
     //only recognise what we need
     if (brushInfo.value("identifer").toString() != brushIdentifer())
         return;
     setBrushWidth(brushInfo.value("width", 0).toInt());
     setBrushColor(brushInfo.value("color").value<QColor>());
     setBrushSolidity(brushInfo.value("solidity", 100).toInt());
 }

 void SimplePen::setSurface(QPixmap *surface)
 {
     _surface = surface;
     if (painter.isActive())
         painter.end();
     painter.begin(_surface);
 }

 QPixmap* SimplePen::surface() const
 {
     return _surface;
 }

 void SimplePen::start(const QPointF &startPoint, qreal pressure)
 {
     if (!painter.isActive())
         return;
     _lastPoint = startPoint;
     painter.drawPoint(_lastPoint);
 }

 void SimplePen::moveTo(const QPointF &newPoint, qreal pressure)
 {
     if (!painter.isActive())
         return;
     if (!_lastPoint.isNull())
        painter.drawLine(_lastPoint, newPoint);
     _lastPoint = newPoint;
 }

 void SimplePen::end()
 {
     _lastPoint = QPointF();
 }
