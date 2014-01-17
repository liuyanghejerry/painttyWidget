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
    int thickness();
    int water();
    int extend();
    int mixin();
public slots:
    void setWidth(int width);
    void setHardness(int hardness);
    void setThickness(int thickness);
    void setWater(int water);
    void setExtend(int extend);
    void setMixin(int mixin);
    void widthUp();
    void widthDown();
    void hardnessUp();
    void hardnessDown();
    void thicknessUp();
    void thicknessDown();
    void waterUp();
    void waterDown();
    void extendUp();
    void extendDown();
    void mixinUp();
    void mixinDown();
    void setHardnessEnabled(bool on);
    void setThicknessEnabled(bool on);
    void setWaterEnabled(bool on);
    void setExtendEnabled(bool on);
    void setMixinEnabled(bool on);
    void setOrientation(Qt::Orientation ori);
signals:
    void widthChanged(int width);
    void hardnessChanged(int hardness);
    void thicknessChanged(int thickness);
    void waterChanged(int water);
    void extendChanged(int extend);
    void mixinChanged(int mixin);
    
private:
    QLabel *widthLabel;
    QSlider *widthSlider;
    QSpinBox *widthSpinBox;
    QFrame *separator;
    QLabel *hardnessLabel;
    QSlider *hardnessSlider;
    QSpinBox *hardnessSpinBox;
    QLabel *thicknessLabel;
    QSlider *thicknessSlider;
    QSpinBox *thicknessSpinBox;
    QLabel *waterLabel;
    QSlider *waterSlider;
    QSpinBox *waterSpinBox;
    QLabel *extendLabel;
    QSlider *extendSlider;
    QSpinBox *extendSpinBox;
    QLabel *mixinLabel;
    QSlider *mixinSlider;
    QSpinBox *mixinSpinBox;
    QBoxLayout *layout_;
};

#endif // BRUSHSETTINGSWIDGET_H
