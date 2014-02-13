#ifndef PANORAMAROTATOR_H
#define PANORAMAROTATOR_H

#include <QWidget>

class QLineEdit;
class QSlider;

class PanoramaRotator : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaRotator(QWidget *parent = 0);

signals:
    void rotated(int deg);

public slots:
    void setRotation(int deg);

private:
    QLineEdit* lineEdit_;
    QSlider* slider_;

};

#endif // PANORAMAROTATOR_H
