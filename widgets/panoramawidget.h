#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <QTimer>
#include <QDebug>

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void refresh();
public slots:
    void onImageChange(const QPixmap &p, const QRect &r);
    void onRectChange(const QRect &r);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent * event);
private:
    QSize preferSize_;
    QPixmap image_;
    QPixmap full_img_;
    QPixmap sized_img_;
    QTimer timer_;
    QRect viewport_;
    QPixmap drawViewport();
    void thumbnail();
};

#endif // PANORAMAWIDGET_H
