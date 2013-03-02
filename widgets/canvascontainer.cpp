#include <QGraphicsScene>
#include "canvascontainer.h"
#include <QGraphicsProxyWidget>
#include <QScrollBar>

CanvasContainer::CanvasContainer(QWidget *parent) :
    QGraphicsView(parent), proxy(0)
{
    setCacheMode(QGraphicsView::CacheBackground);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    scene = new QGraphicsScene(this);
    setScene(scene);
}

void CanvasContainer::setCanvas(QWidget *canvas)
{
    if (canvas->parent())
    {
        qDebug("Canvas has parent, try set it to 0");
        canvas->setParent(0);
    }
    proxy = scene->addWidget(canvas);
}

qreal CanvasContainer::currentScaleFactor() const
{
    if (proxy)
        return proxy->scale();
}

void CanvasContainer::wheelEvent(QWheelEvent *event)
{
    if (!event->modifiers().testFlag(Qt::ControlModifier) || !proxy)
    {
        QGraphicsView::wheelEvent(event);
        return;
    }
    QPointF position = mapToScene(event->pos());
    position = proxy->mapFromScene(position);
    if (!proxy->rect().contains(position))
        return;
    proxy->setTransformOriginPoint(position);
    qreal currentScale = proxy->scale();
    if (event->delta() > 0)
    {
        if (currentScale >= 1 && currentScale < MAX_SCALE_FACTOR)
            currentScale++;
        else if (currentScale < 1)
            currentScale *= 2;
    }
    else
    {
        if (currentScale > 1)
            currentScale--;
        else if (currentScale <= 1 && currentScale > MIN_SCALE_FACTOR)
            currentScale *= 0.5;
    }
    proxy->setScale(currentScale);
    setSceneRect(scene->itemsBoundingRect());
    emit scaled(currentScale);
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
