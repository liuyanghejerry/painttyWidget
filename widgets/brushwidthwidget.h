#ifndef BRUSHWIDTHWIDGET_H
#define BRUSHWIDTHWIDGET_H

#include <QWidget>

class QBoxLayout;
class QLabel;
class QSlider;
class QSpinBox;

class BrushWidthWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit BrushWidthWidget(QWidget *parent = 0);
    ~BrushWidthWidget();
    int value();
public slots:
    void setValue(int v);
    void setStep(int v);
    void up();
    void down();
    void setOrientation(Qt::Orientation ori);
signals:
    void valueChanged(int v);
    
private:
    QLabel *widthLabel;
    QSlider *widthSlider;
    QSpinBox *widthSpinBox;
    QLabel *unitLabel;
    QBoxLayout *layout_;
};

#endif // BRUSHWIDTHWIDGET_H
