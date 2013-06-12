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
#include "../misc/platformextend.h"

using GlobalDef::MAX_SCALE_FACTOR;
using GlobalDef::MIN_SCALE_FACTOR;

CanvasContainer::CanvasContainer(QWidget *parent) :
    QGraphicsView(parent), proxy(0),
    smoothScaleFlag(true),
    tbl_spt(new TabletSupport(this))
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
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    enable_tablet = settings.value("canvas/enable_tablet", false)
            .toBool();

    if(enable_tablet && tbl_spt->hasDevice()){
        emit tabletDetected();
        tbl_spt->start();
    }
}
CanvasContainer::~CanvasContainer()
{
    if(enable_tablet)
        tbl_spt->stop();
    delete tbl_spt;
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

void CanvasContainer::tabletEvent(QTabletEvent *event)
{
    if(!enable_tablet){
        return;
    }
    emit tabletDetected();
    if(!proxy->widget() || !enable_tablet)
        return;
    // TODO: simplify
    QPointF global_pos = event->globalPosF();
    // exclude the scroll bar
    int bottom = 0;
    int right = 0;
    if(horizontalScrollBar()->value()){ // we have horizontal scrollbar
        bottom = horizontalScrollBar()->height();
    }
    if(verticalScrollBar()->value()){ // we have vertical scrollbar
        right = verticalScrollBar()->width();
    }
    QRect re = this->visibleRegion().boundingRect();
    re.setRight(re.right() - right);
    re.setBottom(re.bottom() - bottom);
    if(!re.contains(event->globalPos() - this->pos())){
        return;
    }

    QPointF view_pos = this->mapFromGlobal(global_pos.toPoint());
    QPointF canvas_pos;

    if(this->currentScaleFactor() < 1.0){
        canvas_pos = view_pos;

        // caculate offsets
        QPointF plus;
        if(horizontalScrollBar()->value()){
            int v = horizontalScrollBar()->value()
                    - horizontalScrollBar()->minimum();
            plus.setX( v );
        }
        if(verticalScrollBar()->value()){
            int v = verticalScrollBar()->value()
                    - verticalScrollBar()->minimum();
            plus.setY( v );
        }
        canvas_pos += plus;
        canvas_pos /= this->currentScaleFactor();
    }else{
        canvas_pos = proxy->mapFromParent(view_pos);
        canvas_pos += QPointF(horizontalScrollBar()->value(),
                              verticalScrollBar()->value())
                /this->currentScaleFactor();
    }

    QTabletEvent event2(event->type(),
                        canvas_pos,
                        event->globalPosF(),
                        event->device(),
                        event->pointerType(),
                        event->pressure(),
                        event->xTilt(),
                        event->yTilt(),
                        event->tangentialPressure(),
                        event->rotation(),
                        event->z(),
                        event->modifiers(),
                        event->uniqueId()
                        );
    event->accept();
    qApp->sendEvent(proxy->widget(), &event2);
}

bool CanvasContainer::eventFilter(QObject *object, QEvent *event)
{
    if (object == proxy->widget()
            && event->type() == QEvent::CursorChange)
        proxy->setCursor(proxy->widget()->cursor());
    return false;
}
