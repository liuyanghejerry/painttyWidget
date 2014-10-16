#ifndef PANORAMAVIEW_H
#define PANORAMAVIEW_H

#include <QWidget>
#include <QTimer>

class PanoramaView : public QWidget
{
    Q_OBJECT
public:
    explicit PanoramaView(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void refresh();
    void moveTo(const QPoint &p);
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
    QPixmap image_; //the image finally drawn on widget(with rect on it)
    QPixmap full_img_; //the full-size image
    QPixmap sized_img_; //the image resized to adjust the widget
    QTimer timer_;
    QRect viewport_;
    QPixmap drawViewport();
    void thumbnail();
    void navigateTo(const QPoint &p);
};

#endif // PANORAMAVIEW_H
