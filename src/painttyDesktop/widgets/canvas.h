#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "../paintingTools/brush/abstractbrush.h"
#include "../misc/layermanager.h"

typedef QSharedPointer<AbstractBrush> BrushPointer;

class CanvasBackend : public QObject
{
    Q_OBJECT
public:
    // id, name and count, where count can be use to sort
    typedef std::tuple<QString, QString, quint64> MemberSection;
    enum MemberSectionIndex {
        Id = 0,
        Name,
        Count
    };

    enum BlockLevel {
        NONE = 0,
        LOW = 5,
        MEDIUM = 10,
        HIGH = 20
    };

    CanvasBackend(QObject *parent = nullptr);
    void commit();
    BlockLevel blockLevel() const;
public slots:
    void onDataBlock(const QVariantMap& d);
    void onIncomingData(const QJsonObject &d);
    void requestMembers(MemberSectionIndex index,
                        bool mergeSameName = false);
    void clearMembers();
    void setBlockLevel(const BlockLevel le);
signals:
    void newDataGroup(const QByteArray& d);
    void remoteDrawPoint(const QPoint &point,
                         const QVariantMap &brushInfo,
                         const QString &layer,
                         const QString clientid,
                         const qreal pressure=1.0);
    void remoteDrawLine(const QPoint &start,
                        const QPoint &end,
                        const QVariantMap &brushInfo,
                        const QString &layer,
                        const QString clientid,
                        const qreal pressure=1.0);
    void membersSorted(QList<MemberSection> list);
private:
    BlockLevel blocklevel_;
    QVariantList tempStore;
    // Warning, access memberHistory_ across thread
    // via member functions is not thread-safe
    QHash<QString, MemberSection> memberHistory_;
    void upsertMember(const QString& id, const QString& name);
    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);
};

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
    int jitterCorrectionLevel() const;
    bool isJitterCorrectionEnabled() const;

    virtual QSize sizeHint () const;
    virtual QSize minimumSizeHint () const;

public slots:
    void setJitterCorrectionEnabled(bool correct);
    void setJitterCorrectionLevel(int value);
    void setShareColor(bool b);
    void setBrushColor(const QColor &newColor);
    void setBrushWidth(int newWidth);
    void setBrushHardness(int h);
    void onNewData(const QJsonObject &data);
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
    void ready();
    void sendData(const QByteArray &);
    void pickColorComplete();
    void newBrushSettings(const QVariantMap &map);
    void historyComplete();
    void newPaintAction(const QVariantMap &m);
    void paintActionComplete();
    void newInternalData(const QJsonObject &);
    void requestSortedMembers(CanvasBackend::MemberSectionIndex index
                               = CanvasBackend::MemberSectionIndex::Count,
                              bool mergeSameName = true);
    void requestClearMembers();
    void canvasExported(const QPixmap& pic);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
//    void tabletEvent(QTabletEvent *ev);
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);

private slots:
    void remoteDrawPoint(const QPoint &point,
                         const QVariantMap &brushInfo,
                         const QString &layer,
                         const QString clientid,
                         const qreal pressure=1.0);
    void remoteDrawLine(const QPoint &start,
                        const QPoint &end,
                        const QVariantMap &brushInfo,
                        const QString &layer,
                        const QString clientid,
                        const qreal pressure=1.0);
    void onMembersSorted(const QList<CanvasBackend::MemberSection> &list);

private:
    void drawLineTo(const QPoint &endPoint, qreal pressure=1.0);
    void drawPoint(const QPoint &point, qreal pressure=1.0);
    void pickColor(const QPoint &point);
    void updateCursor();
    void tryJitterCorrection();
    QPixmap appendAuthorSignature(QPixmap target);

    BrushPointer brushFactory(const QString &name);

    bool inPicker;
    bool drawing;
    bool disableMouse_;
    QSize canvasSize;
    LayerManager layers;
    QPixmap image;
    QPixmap *currentImage;
    QPoint lastPoint;
    QList<QPoint> stackPoints;
    int layerNameCounter;
    BrushPointer brush_;
    quint64 historySize_;
    bool shareColor_;
    bool jitterCorrection_;
    int jitterCorrectionLevel_;
    qreal jitterCorrectionLevel_internal_;
    QHash<QString, BrushPointer> remoteBrush;
    QHash<QString, BrushPointer> localBrush;
    CanvasBackend* backend_;
    QThread *worker_;
    QList<CanvasBackend::MemberSection> author_list_;
};

#endif // CANVAS_H
