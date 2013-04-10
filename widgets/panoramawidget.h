#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>
#include "panoramaview.h"

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
signals:
    void refresh();
    void moveTo(const QPointF &p);
    void viewportChange(const QRectF &r);
public slots:
    void onImageChange(const QPixmap &p, const QRect &r);
    void onRectChange(const QRect &r);
private:
    PanoramaView view;
    
};

#endif // PANORAMAWIDGET_H
