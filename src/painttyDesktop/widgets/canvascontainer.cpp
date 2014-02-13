#include <QGraphicsScene>
#include "canvascontainer.h"
#include <QGraphicsProxyWidget>
#include <QApplication>
#include <QScrollBar>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QSettings>
#include <qmath.h>
#include <QDebug>

#include "../../common/common.h"

using GlobalDef::MAX_SCALE_FACTOR;
using GlobalDef::MIN_SCALE_FACTOR;

CanvasContainer::CanvasContainer(QWidget *parent) :
    QGraphicsView(parent), proxy(0),
    smoothScaleFlag(true)
{
    setCacheMode(QGraphicsView::CacheBackground);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scene = new QGraphicsScene(this);
    setScene(scene);

    auto rc = [&](){
        emit rectChanged(visualRect().toRect());
    };
    connect(horizontalScrollBar(), &QScrollBar::valueChanged,
            rc);
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            rc);
}
CanvasContainer::~CanvasContainer()
{
}

void CanvasContainer::setCanvas(QWidget *canvas)
{
    if (canvas->parent())
    {
        canvas->setParent(0);
        canvas->setWindowFlags(canvas->windowFlags() | Qt::Window);
    }
    proxy = scene->addWidget(canvas);
    canvas->installEventFilter(this);
}

void CanvasContainer::setScaleFactor(qreal factor)
{
    //new signal and slot syntax has some trouble
    //dealing with slots having default parameter
    setScaleFactorInternal(factor);
}

qreal CanvasContainer::currentScaleFactor() const
{
    if (proxy)
        return proxy->scale();
    return 0;
}

bool CanvasContainer::smoothScale() const
{
    return smoothScaleFlag;
}

void CanvasContainer::setSmoothScale(bool smooth)
{
    smoothScaleFlag = smooth;
    if (!smoothScaleFlag)
    {
        setRenderHint(QPainter::Antialiasing, false);
        setRenderHint(QPainter::SmoothPixmapTransform, false);
    }
}

void CanvasContainer::scaleBy(qreal factor)
{
    factor *= currentScaleFactor();
    setScaleFactorInternal(factor);
}
void CanvasContainer::setRotation(int degree)
{
    degree ^= 360;
    degree += 360;
    QPointF position = proxy->mapFromScene(
                mapToScene(viewport()->rect().center()));
    if(proxy->rect().contains(position))
        proxy->setTransformOriginPoint(position);
    proxy->setRotation(-degree);
    setSceneRect(scene->itemsBoundingRect());
}

QRectF CanvasContainer::visualRect() const
{
    return proxy->mapFromScene(
                mapToScene(viewport()->rect()))
            .boundingRect().intersected(proxy->rect());
}

void CanvasContainer::centerOn(const QPointF &pos)
{
    QPointF point = proxy->mapToScene(pos);
    QGraphicsView::centerOn(point);
}

void CanvasContainer::centerOn(qreal x, qreal y)
{
    centerOn(QPoint(x, y));
}

qreal CanvasContainer::calculateFactor(qreal current, bool zoomIn)
{
    if (current <= MIN_SCALE_FACTOR && !zoomIn)
        return MIN_SCALE_FACTOR;
    if (current >= MAX_SCALE_FACTOR && zoomIn)
        return MAX_SCALE_FACTOR;
    if (current < 1)
    {
        qreal power = qLn(current) / qLn(0.5);
        int newPower = qFloor(power);
        if (zoomIn)
        {
            if (qFuzzyCompare(qreal(newPower), power))
                newPower--;
            return qBound(MIN_SCALE_FACTOR, qPow(0.5, newPower), MAX_SCALE_FACTOR);
        }
        else
        {
            return qBound(MIN_SCALE_FACTOR, qPow(0.5, newPower + 1), MAX_SCALE_FACTOR);
        }
    }
    else
    {
        qreal newScale = qFloor(current);
        if (zoomIn)
        {
            return qBound(MIN_SCALE_FACTOR, newScale + 1, MAX_SCALE_FACTOR);
        }
        else
        {
            if (qFuzzyCompare(newScale, current))
                newScale--;
            if (qFuzzyCompare(newScale, 0.0))
                newScale = 0.5;
            return qBound(MIN_SCALE_FACTOR, newScale, MAX_SCALE_FACTOR);
        }

    }
}

void CanvasContainer::setScaleFactorInternal(qreal factor, const QPoint scaleCenter)
{
    factor = qBound(MIN_SCALE_FACTOR, factor, MAX_SCALE_FACTOR);
    if(proxy){
        if (qFuzzyCompare(factor, proxy->scale()))
            return;
        if(smoothScaleFlag){
            if(factor < 1)
                setRenderHints(QPainter::Antialiasing
                               | QPainter::SmoothPixmapTransform);
            else{
                setRenderHint(QPainter::Antialiasing, false);
                setRenderHint(QPainter::SmoothPixmapTransform,
                              false);
            }
        }

        QPointF position = proxy->mapFromScene(
                    mapToScene(scaleCenter.isNull()? viewport()->rect().center() : scaleCenter));
        if(proxy->rect().contains(position))
            proxy->setTransformOriginPoint(position);
        proxy->setScale(factor);
        setSceneRect(scene->itemsBoundingRect());

        emit scaled(factor);
        emit rectChanged(visualRect().toRect());
    }
}

void CanvasContainer::wheelEvent(QWheelEvent *event)
{
    if (!event->modifiers().testFlag(Qt::ControlModifier)
            || !proxy)
    {
        QGraphicsView::wheelEvent(event);
        return;
    }
    //QPointF position = proxy->mapFromScene(mapToScene(event->pos()));
    //if (!proxy->rect().contains(position))
    //return;
    //proxy->setTransformOriginPoint(position);
    setScaleFactorInternal(calculateFactor(proxy->scale(), event->delta() > 0), event->pos());
}

void CanvasContainer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        horizontalScrollValue = horizontalScrollBar()->value();
        verticalScrollValue = verticalScrollBar()->value();
        moveStartPoint = event->pos();
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasContainer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton)
    {
        horizontalScrollValue += (moveStartPoint - event->pos()).x();
        verticalScrollValue += (moveStartPoint - event->pos()).y();
        horizontalScrollBar()->setValue(horizontalScrollValue);
        verticalScrollBar()->setValue(verticalScrollValue);
        moveStartPoint = event->pos();
    }
    QGraphicsView::mouseMoveEvent(event);
}

bool CanvasContainer::eventFilter(QObject *object, QEvent *event)
{
    if (object == proxy->widget()
            && event->type() == QEvent::CursorChange)
        proxy->setCursor(proxy->widget()->cursor());
    return false;
}
