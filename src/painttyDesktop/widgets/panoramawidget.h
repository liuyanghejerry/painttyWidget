#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>

class PanoramaSlider;
class PanoramaRotator;
class PanoramaView;

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
signals:
    void refresh();
    void moveTo(const QPoint &p);
    void viewportChange(const QRectF &r);
    void scaled(qreal);
    void rotated(int);
public slots:
    void setScaled(qreal);
    void setRotation(int);
    void onImageChange(const QPixmap &p, const QRect &r);
    void onRectChange(const QRect &r);
private:
    PanoramaSlider *slider;
    PanoramaRotator *rotator;
    PanoramaView *view;
};

#endif // PANORAMAWIDGET_H
