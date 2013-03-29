#ifndef BRUSHWIDTHWIDGET_H
#define BRUSHWIDTHWIDGET_H

#include <QWidget>

class QBoxLayout;

namespace Ui {
class BrushWidthWidget;
}

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
    Ui::BrushWidthWidget *ui;
    QBoxLayout *layout_;
    void makeLayout();
};

#endif // BRUSHWIDTHWIDGET_H
