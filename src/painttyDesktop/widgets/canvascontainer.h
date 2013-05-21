#ifndef CANVASCONTAINER_H
#define CANVASCONTAINER_H

#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsProxyWidget;
class TabletSupport;

class CanvasContainer : public QGraphicsView
{
    Q_OBJECT
public:
    CanvasContainer(QWidget *parent = 0);
    ~CanvasContainer();
    void setCanvas(QWidget *canvas);
    qreal currentScaleFactor() const;
    bool smoothScale() const;
    QRectF visualRect() const;
    void centerOn(const QPointF &pos);
    void centerOn(qreal x, qreal y);
    
signals:
    void scaled(qreal factor);
    void rectChanged(const QRect&);

public slots:
    void setScaleFactor(qreal factor);
    void setSmoothScale(bool smooth);

private:
    QGraphicsScene *scene;
    QGraphicsProxyWidget *proxy;
    QPoint moveStartPoint;
    int horizontalScrollValue;
    int verticalScrollValue;
    bool smoothScaleFlag;
    qreal calculateFactor(qreal current, bool zoomIn);
    void setScaleFactorInternal(qreal factor, const QPoint scaleCenter = QPoint());
    TabletSupport* tbl_spt;

protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void tabletEvent(QTabletEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // CANVASCONTAINER_H
