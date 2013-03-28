#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QWidget>
#include <QTimer>

class PanoramaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaWidget(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void refresh();
    void moveTo(const QPointF &p);
    void viewportChange(const QRectF &r);
public slots:
    void onImageChange(const QPixmap &p, const QRect &r);
    void onRectChange(const QRect &r);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
private:
    QSize preferSize_;
    QPixmap image_;
    QPixmap full_img_;
    QPixmap sized_img_;
    QTimer timer_;
    QRect viewport_;
    QPixmap drawViewport();
    void thumbnail();
    void navigateTo(const QPoint &p);
};

#endif // PANORAMAWIDGET_H
