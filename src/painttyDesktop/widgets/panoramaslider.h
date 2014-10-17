#ifndef PANORAMASLIDER_H
#define PANORAMASLIDER_H

#include <QWidget>
#include <QRegExp>

class QSlider;
class QLineEdit;

class PanoramaSlider : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaSlider(QWidget *parent = 0);
signals:
    void scaled(int scaleFactor);
public slots:
    void setScale(int scaleFactor);
private:
    QSlider *slider;
    QLineEdit *input;
    qreal internalFactor;
    QRegExp inputReg;
private slots:
    void calculateScale(int sliderValue);
    void inputScaleConfirmed();
};

#endif // PANORAMASLIDER_H
