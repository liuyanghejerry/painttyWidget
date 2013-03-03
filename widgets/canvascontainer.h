#ifndef CANVASCONTAINER_H
#define CANVASCONTAINER_H

#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsProxyWidget;

const qreal MAX_SCALE_FACTOR = 5.0;
const qreal MIN_SCALE_FACTOR = 0.125;

class CanvasContainer : public QGraphicsView
{
    Q_OBJECT
public:
    CanvasContainer(QWidget *parent = 0);
    void setCanvas(QWidget *canvas);
    qreal currentScaleFactor() const;
    QRectF visualRect() const;
    
signals:
    void scaled(qreal factor);
public slots:

private:
    QGraphicsScene *scene;
    QGraphicsProxyWidget *proxy;
    QPoint moveStartPoint;
    int horizontalScrollValue;
    int verticalScrollValue;
protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // CANVASCONTAINER_H
