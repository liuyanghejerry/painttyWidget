#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>

class PanoramaSlider;
class PanoramaView;

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
signals:
    void refresh();
    void moveTo(const QPointF &p);
    void viewportChange(const QRectF &r);
    void scaled(qreal);
public slots:
    void setScaled(qreal);
    void onImageChange(const QPixmap &p, const QRect &r);
    void onRectChange(const QRect &r);
private:
    PanoramaSlider *slider;
    PanoramaView *view;
};

#endif // PANORAMAWIDGET_H
