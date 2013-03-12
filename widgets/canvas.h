#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QStyleOption>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QPainter>
#include <QHash>
#include <QSharedPointer>
#include <QJsonDocument>


#include "../layermanager.h"
#include "../paintingTools/brush/sketchbrush.h"
#include "../paintingTools/brush/eraser.h"
#include "../paintingTools/brush/pencil.h"
#include "../paintingTools/colorpicker.h"

typedef QSharedPointer<Brush> BrushPointer;

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = 0);
    ~Canvas();
    QVariantMap brushInfo();
    quint64 userId();
    QString currentLayer();
    int count(){return layers.count();}
    int layerNum(){return layerNameCounter;}
    QPixmap currentCanvas();
    QPixmap allCanvas();
    void setHistorySize(const quint64 &size);

    virtual QSize sizeHint () const;
    virtual QSize minimumSizeHint () const;

public slots:
    void setBrushColor(const QColor &newColor);
    void setBrushWidth(int newWidth);
    void onNewData(const QByteArray & data);
    void addLayer(const QString &name);
    bool deleteLayer(const QString &name);
    void clearLayer(const QString &name);
    void clearAllLayer();
    void lockLayer(const QString &name);
    void unlockLayer(const QString &name);
    void hideLayer(const QString &name);
    void showLayer(const QString &name);
    void moveLayerUp(const QString &name);
    void moveLayerDown(const QString &name);
    void layerSelected(const QString &name);
    void changeBrush(const QString &name);
    void onColorPicker(bool in);

signals:
    void sendData(const QByteArray &);
    void pickColorComplete();
    void newBrushSettings(const QVariantMap &map);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void tabletEvent(QTabletEvent *ev);

    void remoteDrawPoint(const QPoint &point,
                         const QVariantMap &brushInfo,
                         const QString &layer,
                         const quint64 userid);
    void remoteDrawLine(const QPoint &start,
                        const QPoint &end,
                        const QVariantMap &brushInfo,
                        const QString &layer,
                        const quint64 userid);

private:

    void drawLineTo(const QPoint &endPoint);
    void drawPoint(const QPoint &point);
    void pickColor(const QPoint &point);
    void updateCursor(const int &width);

    void resizeImage(QPixmap *image, const QSize &newSize);
    void combineLayers(const QRect &rec);
    QPixmap combineLayers();

    void genUserid();

    BrushPointer brushFactory(const QString &name);

    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);

    bool inPicker;
    bool drawing;
    LayerManager layers;
    QSize canvasSize;
    QPixmap image;
    QPixmap *currentImage;
    QPoint lastPoint;
    qreal opacity;
    int layerNameCounter;
    BrushPointer brush_;
    quint64 userid_; // in room
    int historySize_;
    QHash<quint64, BrushPointer> remoteBrush;
    QHash<QString, BrushPointer> localBrush;
    QSharedPointer<ColorPicker> colorPicker;
};

#endif // CANVAS_H
