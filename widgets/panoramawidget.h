#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QDebug>

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
    QSize sizeHint() const;
signals:
    void refresh();
public slots:
    void onImageChange(QPixmap p, const QRegion &r);
protected:
    void paintEvent(QPaintEvent *);
private:
    QSize preferSize_;
    QPixmap image_;
    QTimer timer_;
};

#endif // PANORAMAWIDGET_H
