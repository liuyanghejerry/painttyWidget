#ifndef PANORAMASLIDER_H
#define PANORAMASLIDER_H

#include <QWidget>

class PanoramaSlider : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaSlider(QWidget *parent = 0);
    qreal currentScaleFactor();
signals:
    void scaled(qreal scaleFactor);
public slots:
    void setScale(qreal scaleFactor);
private:
    void makeScaleSlider();
    qreal currentScaleFactor_;
};

#endif // PANORAMASLIDER_H
