#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "../paintingTools/brush/abstractbrush.h"
#include "../misc/layermanager.h"

typedef QSharedPointer<AbstractBrush> BrushPointer;

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = 0);
    ~Canvas();
    QVariantMap brushInfo();
    QString currentLayer();
    int count(){return layers.count();}
    int layerNum(){return layerNameCounter;}
    QPixmap currentCanvas();
    QPixmap allCanvas();
    void setHistorySize(quint64 s);

    virtual QSize sizeHint () const;
    virtual QSize minimumSizeHint () const;

public slots:
    void setShareColor(bool b);
    void setBrushColor(const QColor &newColor);
    void setBrushWidth(int newWidth);
    void setBrushHardness(int h);
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
    void historyComplete();
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
                         const QString clientid);
    void remoteDrawLine(const QPoint &start,
                        const QPoint &end,
                        const QVariantMap &brushInfo,
                        const QString &layer,
                        const QString clientid);

private:

    void drawLineTo(const QPoint &endPoint);
    void drawPoint(const QPoint &point);
    void pickColor(const QPoint &point);
    void updateCursor();

    BrushPointer brushFactory(const QString &name);

    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);

    bool inPicker;
    bool drawing;
    QSize canvasSize;
    LayerManager layers;
    QPixmap image;
    QPixmap *currentImage;
    QPoint lastPoint;
    qreal opacity;
    int layerNameCounter;
    BrushPointer brush_;
    quint64 historySize_;
    bool shareColor_;
    QHash<QString, BrushPointer> remoteBrush;
    QHash<QString, BrushPointer> localBrush;
//    QSharedPointer<ColorPicker> colorPicker;
};

#endif // CANVAS_H
