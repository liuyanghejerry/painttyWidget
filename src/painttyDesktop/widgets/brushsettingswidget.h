#ifndef BRUSHSETTINGSWIDGET_H
#define BRUSHSETTINGSWIDGET_H

#include <QWidget>

class QBoxLayout;
class QLabel;
class QSlider;
class QSpinBox;
class QFrame;

class BrushSettingsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit BrushSettingsWidget(QWidget *parent = 0);
    ~BrushSettingsWidget();
    int width();
    int hardness();
public slots:
    void setWidth(int width);
    void setHardness(int hardness);
    void widthUp();
    void widthDown();
    void hardnessUp();
    void hardnessDown();
    void setOrientation(Qt::Orientation ori);
signals:
    void widthChanged(int width);
    void hardnessChanged(int hardness);
    
private:
    QLabel *widthLabel;
    QSlider *widthSlider;
    QSpinBox *widthSpinBox;
    QFrame *separator;
    QLabel *hardnessLabel;
    QSlider *hardnessSlider;
    QSpinBox *hardnessSpinBox;
    QBoxLayout *layout_;
};

#endif // BRUSHSETTINGSWIDGET_H
