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
    typedef std::tuple<QString, QString, quint64, QPoint, qint64> MemberSection;
    enum MemberSectionIndex {
        Id = 0,
        Name,
        Count,
        Footprint,
        LastActiveStamp
    };

    enum BlockLevel {
        NONE = 0,
        LOW = 5,
        MEDIUM = 10,
        HIGH = 20
    };

    CanvasBackend(QObject *parent = nullptr);
    ~CanvasBackend();
    void commit();
    BlockLevel blockLevel() const;
public slots:
    void onDataBlock(const QVariantMap d);
    void onIncomingData(const QJsonObject &d);
    void requestMembers(MemberSectionIndex index);
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
    void archiveParsed();
protected:
    void timerEvent(QTimerEvent * event);
private:
    BlockLevel blocklevel_;
    QVariantList tempStore;
    QList<QJsonObject> incoming_store_;
    // Warning, access memberHistory_ across thread
    // via member functions is not thread-safe
    QHash<QString, MemberSection> memberHistory_;
    int send_timer_id_;
    int parse_timer_id_;
    bool archive_loaded_;
    bool is_parsed_signal_sent;
    void upsertFootprint(const QString& id, const QString& name, const QPoint &point);
    void upsertFootprint(const QString& id, const QString& name);
    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);
    void parseIncoming();
    void onArchiveLoaded();
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
    QImage currentCanvas();
    QImage allCanvas();
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
    void loadLayers();
    void saveLayers();

signals:
    void pickColorComplete();
    void newBrushSettings(const QVariantMap &map);
    void historyComplete();
    void newPaintAction(const QVariantMap m);
    void paintActionComplete();
    void requestSortedMembers(CanvasBackend::MemberSectionIndex index
                               = CanvasBackend::MemberSectionIndex::Count);
    void requestClearMembers();
    void canvasExported(const QPixmap& pic);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
#ifndef PAINTTY_NO_TABLET
    void tabletEvent(QTabletEvent *ev);
#endif
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
    QImage appendAuthorSignature(QImage target);
    BrushPointer brushFactory(const QString &name);


    bool inPicker;
    bool drawing;
    QSize canvasSize;
    LayerManager layers;
    QImage image;
    QImage *currentImage;
    QPoint lastPoint;
    QList<QPoint> stackPoints;
    int layerNameCounter;
    BrushPointer brush_;
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
