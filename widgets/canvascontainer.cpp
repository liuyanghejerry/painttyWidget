#include <QGraphicsScene>
#include "canvascontainer.h"
#include <QGraphicsProxyWidget>
#include <QScrollBar>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <qmath.h>
#include <QDebug>

CanvasContainer::CanvasContainer(QWidget *parent) :
    QGraphicsView(parent), proxy(0), scaleSliderWidget(0),
    smoothScaleFlag(true)
{
    setCacheMode(QGraphicsView::CacheBackground);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
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
    factor = qBound(MIN_SCALE_FACTOR, factor, MAX_SCALE_FACTOR);
    if (proxy)
    {
        if (smoothScaleFlag)
        {
            if (factor < 1)
                setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            else
            {
                setRenderHint(QPainter::Antialiasing, false);
                setRenderHint(QPainter::SmoothPixmapTransform, false);
            }
        }
        proxy->setScale(factor);
        setSceneRect(scene->itemsBoundingRect());
        emit scaled(factor);
        emit rectChanged(visualRect().toRect());
    }
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

QWidget* CanvasContainer::scaleSlider()
{
    if (!scaleSliderWidget)
    {
        scaleSliderWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(scaleSliderWidget);
        QSlider *slider = new QSlider(Qt::Horizontal, scaleSliderWidget);
        QLabel *label = new QLabel("100%", scaleSliderWidget);
        scaleSliderWidget->setLayout(layout);
        layout->addWidget(slider);
        layout->addWidget(label);
        layout->setContentsMargins(0, 0, 0, 0);

        qreal internalFactor = -100 * qLn(MIN_SCALE_FACTOR) / MIN_SCALE_FACTOR / qLn(2);
        slider->setRange(-qCeil(MIN_SCALE_FACTOR * internalFactor),
                         qCeil(MAX_SCALE_FACTOR - 1) * 100);
        slider->setPageStep(100);
        slider->setTickPosition(QSlider::TicksBelow);

        auto setScaleOrigin = [=]()
        {
            if (proxy)
            {
                QPointF position = proxy->mapFromScene(mapToScene(viewport()->rect().center()));
                if (proxy->rect().contains(position))
                    proxy->setTransformOriginPoint(position);
            }
        };

        auto calculateScale = [=](int sliderValue)
        {
            if (sliderValue >= 0)
            {
                qreal newScale = 1.0 + 0.01 * sliderValue;
                if (!qFuzzyCompare(newScale, currentScaleFactor()))
                {
                    setScaleOrigin();
                    setScaleFactor(1.0 + 0.01 * sliderValue);
                    label->setText(QString("%1%").arg(100 + sliderValue));
                }
              }
            else
            {
                qreal newScale = qPow(MIN_SCALE_FACTOR, -sliderValue / internalFactor / MIN_SCALE_FACTOR);
                if (!qFuzzyCompare(newScale, currentScaleFactor()))
                {
                    setScaleOrigin();
                    setScaleFactor(newScale);
                    label->setText(QString("%1%").arg(newScale * 100, 0, 'f', 1));
                }
            }
        };
        auto calculateValue = [=](qreal scaleFactor)
        {
            slider->blockSignals(true);
            if (scaleFactor >= 1.0)
            {
                int newValue = 100 * scaleFactor - 100;
                if (newValue != slider->value())
                {
                    slider->setValue(newValue);
                    label->setText(QString("%1%").arg(qFloor(100 * scaleFactor)));
                }
            }
            else
            {
                int newValue = -internalFactor * MIN_SCALE_FACTOR / qLn(MIN_SCALE_FACTOR) * qLn(scaleFactor);
                if (newValue != slider->value())
                {
                    slider->setValue(newValue);
                    label->setText(QString("%1%").arg(100 * scaleFactor, 0, 'f', 1));
                }
            }
            slider->blockSignals(false);
        };

        connect(slider, &QSlider::valueChanged, calculateScale);
        connect(this, &CanvasContainer::scaled, calculateValue);
    }
    return scaleSliderWidget;
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

void CanvasContainer::wheelEvent(QWheelEvent *event)
{
    if (!event->modifiers().testFlag(Qt::ControlModifier)
            || !proxy)
    {
        QGraphicsView::wheelEvent(event);
        return;
    }
    QPointF position = proxy->mapFromScene(mapToScene(event->pos()));
    if (!proxy->rect().contains(position))
        return;
    proxy->setTransformOriginPoint(position);
    setScaleFactor(calculateFactor(proxy->scale(), event->delta() > 0));
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
