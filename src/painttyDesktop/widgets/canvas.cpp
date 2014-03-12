#include <QPainter>
#include <QHash>
#include <QSharedPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStyleOption>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QSettings>
#include <QApplication>
#include <QTimer>
#include <QDir>
#include <QStaticText>
#include <QDateTime>
#include <QtCore/qmath.h>

#include "../../common/common.h"
#include "../../common/network/clientsocket.h"
#include "../paintingTools/brush/brushmanager.h"
#include "../paintingTools/brush/basicbrush.h"
#include "../paintingTools/brush/sketchbrush.h"
#include "../paintingTools/brush/basiceraser.h"
#include "../paintingTools/brush/binarybrush.h"
#include "../paintingTools/brush/waterbased.h"
#include "../paintingTools/brush/maskbased.h"
#include "../misc/platformextend.h"
#include "../misc/singleton.h"
#include "../misc/archivefile.h"

#include "canvas.h"


/*!
     \class Canvas

     \brief The widget that used for rendering painting results.

     Canvas uses several \l{QPixmap} to render contents. Each layer
     opaque one \l{QPixmap} and painted together when paintEvent happens.

     Canvas can be used for mouse painting, tablet painting and remote
     painting via json messages from network.

     \l{Brush} is used for painting. There're 4 kind of brush
     currently and will be more. When changing color of
     a \l{Brush} using on Canvas now,
     you can use either setBrushColor()
     or \l{Brush::} {setColor()} .

     \sa Brush
*/

/*!
    \fn Canvas::Canvas(QWidget *parent)

    Construct a canvas with width of 720px, height of 480px and a \a parent.
*/

Canvas::Canvas(QWidget *parent) :
    QWidget(parent),
    canvasSize(Singleton<ClientSocket>::instance().canvasSize()),
    layers(canvasSize),
    image(canvasSize, QImage::Format_ARGB32_Premultiplied),
    layerNameCounter(0),
    shareColor_(true),
    jitterCorrection_(true),
    jitterCorrectionLevel_(10),
    backend_(new CanvasBackend(0)),
    worker_(new QThread(this))
{
    setAttribute(Qt::WA_StaticContents);
    inPicker = false;
    drawing = false;
    brush_ = BrushPointer(new BasicBrush);
    updateCursor();

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus); // necessary for IME control
    resize(canvasSize);

    auto& brush_manager = Singleton<BrushManager>::instance();
    BrushPointer p1(new BasicBrush);
    p1->setSettings(p1->defaultSettings());
    BrushPointer p2(new BinaryBrush);
    p2->setSettings(p1->defaultSettings());
    BrushPointer p3(new SketchBrush);
    p3->setSettings(p1->defaultSettings());
    BrushPointer p4(new BasicEraser);
    p4->setSettings(p1->defaultSettings());
    // TODO
//    BrushPointer p5(new WaterBased);
//    p5->setSettings(p1->defaultSettings());
    BrushPointer p6(new MaskBased);
    p6->setSettings(p1->defaultSettings());
    brush_manager.addBrush(p1);
    brush_manager.addBrush(p2);
    brush_manager.addBrush(p3);
    brush_manager.addBrush(p4);
//    brush_manager.addBrush(p5);
    brush_manager.addBrush(p6);
    setJitterCorrectionLevel(5);

    auto& client_socket = Singleton<ClientSocket>::instance();

    worker_->start();
    backend_->moveToThread(worker_);
    connect(backend_, &CanvasBackend::remoteDrawLine,
            this, &Canvas::remoteDrawLine);
    connect(backend_, &CanvasBackend::remoteDrawPoint,
            this, &Canvas::remoteDrawPoint);
    connect(backend_, &CanvasBackend::repaintHint,
            this, static_cast<void (Canvas::*)()>(&Canvas::update));
//    connect(this, &Canvas::destroyed,
//            backend_, &CanvasBackend::deleteLater);
//    connect(this, &Canvas::destroyed,
//            worker_, &QThread::terminate);
    connect(worker_, &QThread::finished,
            backend_, &CanvasBackend::deleteLater);
    connect(this, &Canvas::newPaintAction,
            backend_, &CanvasBackend::onDataBlock);
    connect(this, &Canvas::parsePaused,
            backend_, &CanvasBackend::pauseParse);

    // TODO: proper use of signal requestSortedMembers,
    // instead of using QTimer
    QTimer *t = new QTimer(this);

    qRegisterMetaType<MSI>("CanvasBackend::MemberSectionIndex");
    qRegisterMetaType< QList<MS> >("QList<MemberSection>");

    connect(backend_, &CanvasBackend::membersSorted,
            this, &Canvas::onMembersSorted);
    connect(this, &Canvas::requestSortedMembers,
            backend_, &CanvasBackend::requestMembers);
    connect(this, &Canvas::requestClearMembers,
            backend_, &CanvasBackend::clearMembers);
    connect(backend_, &CanvasBackend::archiveParsed,
            [this](){
        setEnabled(true);
    });
    connect(t, &QTimer::timeout,
            [this](){
        emit requestSortedMembers(MSI::Count);
    });
    t->start(500);

    if(client_socket.schedualDataLength()){
        this->setEnabled(false);
    }

}

/*!
    \fn Canvas::~Canvas()

    Destroys the canvas. The internal \l Brush will also be deleted.
*/

Canvas::~Canvas()
{
    pause();
    if(worker_){
        worker_->quit();
        worker_->wait();
    }
    this->disconnect();
}

QImage Canvas::currentCanvas()
{
    QImage pmp = image;
    layers.combineLayers(&pmp);
    return appendAuthorSignature(pmp);
}

QImage Canvas::allCanvas()
{
    QImage exp(canvasSize, QImage::Format_ARGB32_Premultiplied);
    exp.fill(Qt::white);
    QPainter painter(&exp);
    int count = layers.count();
    QImage * im = 0;
    for(int i=0;i<count;++i){
        LayerPointer l = layers.layerFrom(i);
        im = l->imagePtr();
        painter.drawImage(0, 0, *im);
    }
    return appendAuthorSignature(exp);
}

QImage Canvas::appendAuthorSignature(QImage target)
{

    QHash<QString, MS> list;
    for(const MS &elem: author_list_){
        QString author = std::get<MSI::Name>(elem);
        if(list.contains(author)){
            std::get<MSI::Count>(list[author]) += std::get<MSI::Count>(elem);
        }else{
            list.insert(author, elem);
        }
    }

    // TODO: make all numbers configurable
    int textSize = qMin(target.size().height(), target.width());
    textSize = qBound(10, int(textSize * 0.02), 100);

    QPainter painter(&target);
    QPen textPen;
    textPen.setColor(Qt::black);
    painter.setOpacity(0.4);
    painter.setPen(textPen);

    QStaticText text;
    QString authors;
    text.setTextFormat(Qt::RichText);
    QTextOption textOp;
    textOp.setAlignment(Qt::AlignRight);
    text.setTextOption(textOp);

    for(auto& item: list.values()){
        QString name = std::get<MSI::Name>(item);
        authors += QString("BY ") + name + "<br/>";
    }

    text.setText(authors);

    QFont textFont(QGuiApplication::font());
    textFont.setPointSize(textSize);
    painter.setFont(textFont);
    text.prepare(QTransform(), textFont);

    QPoint rightCorner = target.rect().bottomRight();
    QPoint padding(20, 20);
    rightCorner -= QPoint(text.size().width(),
                          text.size().height());
    rightCorner -= padding;

    painter.drawStaticText(rightCorner, text);
    return target;
}

int Canvas::jitterCorrectionLevel() const
{
    return jitterCorrectionLevel_;
}

bool Canvas::isJitterCorrectionEnabled() const
{
    return jitterCorrection_;
}

void Canvas::setJitterCorrectionEnabled(bool correct)
{
    jitterCorrection_ = correct;
}

void Canvas::setJitterCorrectionLevel(int value)
{
    jitterCorrectionLevel_ = qBound(0, value, 10);
    jitterCorrectionLevel_internal_ = jitterCorrectionLevel_ * 0.5;
}

void Canvas::tryJitterCorrection()
{
    if(stackPoints.length() < qBound(3, jitterCorrectionLevel_, 10) )
        return;

    int amount = stackPoints.length();
    int redudent = amount;

    auto should_correct = [this, &redudent](const QPoint& p1,
            const QPoint& p2,
            const QPoint& p3) -> bool
    {
        QLine l1(p1, p2);
        QLine l2(p2, p3);
        QLine l3(p3, p1);

        qreal A = 1.0;
        if(l3.dx() != l1.dx()){
            A = (l1.dy() - l3.dy()) / (l3.dx() - l1.dx());
        }
        qreal B = 1.0;
        qreal C = l1.dx() * (-A) - l1.dy();

        qreal distance_up = A*l2.dx() + B*l2.dy() + C;
        qreal distance_down = qSqrt(A*A+B*B);
        if(qFuzzyCompare(distance_down, 0.0)){
            return false;
        }

        qreal distance = qAbs(distance_up / distance_down);
        //        qDebug()<<"distance"<<distance;

        if(distance <= jitterCorrectionLevel_internal_ ){
            return true;
        }else{
            return false;
        }

    };

    int basePos = 0;
    while((stackPoints.length() >= 3) && (basePos < stackPoints.length() - 3)){
        if(should_correct(stackPoints[basePos],
                          stackPoints[basePos+1],
                          stackPoints[basePos+2])){
            stackPoints.removeAt(basePos+1);
            redudent--;
        }else{
            basePos++;
        }
    }
    // you can see the correction rate.
    //    qDebug()<<"correction rate: "<<qreal(amount-redudent)/amount *100<<"%";
}

/*!
    \fn QVariantMap Canvas::brushInfo()

    Returns the brush info painting now.
    \sa Brush::brushInfo()
*/

QVariantMap Canvas::brushSettings() const
{
    auto m = brush_->settings();
    m.insert("name", brush_->name().toLower());
    return m;
}

BrushFeature Canvas::brushFeatures() const
{
    return brush_->features();
}

void Canvas::setShareColor(bool b)
{
    shareColor_ = b;
}

/*!
    \fn void Canvas::setBrushColor(const QColor &newColor)

    Sets a \a newColor to current brush.
    \sa Brush::setColor()
*/

void Canvas::setBrushColor(const QColor &newColor)
{
    if(brush_->color() == newColor) return;
    brush_->setColor(newColor);
}

/*!
    \fn void Canvas::setBrushWidth(int newWidth)

    Sets a \a newWidth to current brush.
    \sa Brush::setWidth()
*/

void Canvas::setBrushWidth(int newWidth)
{
    setBrushFeature("width", newWidth);
    updateCursor();
}

void Canvas::setBrushHardness(int h)
{
    setBrushFeature("hardness", h);
}

void Canvas::setBrushThickness(int t)
{
    setBrushFeature("thickness", t);
}

void Canvas::setBrushWater(int w)
{
    setBrushFeature("water", w);
}

void Canvas::setBrushExtend(int e)
{
    setBrushFeature("extend", e);
}

void Canvas::setBrushMixin(int e)
{
    setBrushFeature("mixin", e);
}

void Canvas::setBrushSettings(const QVariantMap &settings)
{
    brush_->setSettings(settings);
}

BrushPointer Canvas::brushFactory(const QString &name)
{
    return Singleton<BrushManager>::instance().makeBrush(name);
}

void Canvas::setBrushFeature(const QString &key, const QVariant &value)
{
    auto&& settings = brushSettings();
    settings.insert(key, value);
    setBrushSettings(settings);
}

void Canvas::loadLayers()
{
    // TODO: merge this feature into ArchiveFile, maybe?
    QString dir_name = Singleton<ArchiveFile>::instance().dirName();
    for(int i=layers.count()-1;i>=0;--i){
        QString img_name = QString("%1/%2.png").arg(dir_name).arg(i);
        QImage img(img_name);
        if(img.isNull()){
            continue;
        }
        QPainter painter(layers.layerFrom(i)->imagePtr());
        painter.drawImage(0, 0, img);
    }
//    connect(&Singleton<ArchiveFile>::instance(), &ArchiveFile::newSignature,
//            [this](){
//        this->clearAllLayer();
//    });
    connect(&Singleton<ArchiveFile>::instance(), &ArchiveFile::newSignature,
           this, &Canvas::clearAllLayer);
}

void Canvas::saveLayers()
{
    // TODO: merge this feature into ArchiveFile, maybe?
    QString dir_name = Singleton<ArchiveFile>::instance().dirName();
    QDir::current().mkpath(dir_name);
    for(int i=layers.count()-1;i>=0;--i){
        if(!layers.layerFrom(i)->isTouched()){
            continue;
        }
        QString img_name = QString("%1/%2.png").arg(dir_name).arg(i);
        layers.layerFrom(i)->imageConstPtr()->save(img_name);
    }
}

void Canvas::pause()
{
    emit parsePaused();
}

/*!
    \fn void Canvas::changeBrush(const QString &name)

    Changes current brush to \a name .
    At this time, we have only brushButton for Brush, pencilButton for Pencil, sketchButton for Sketch, and eraserButton for Eraser.
    \sa Brush, SketchBrush, Pencil, Eraser
*/

void Canvas::changeBrush(const QString &name)
{
    QVariantMap currentSettings;
    LayerPointer sur = brush_->surface();
    QVariantMap colorMap = brush_->settings()
            .value("color").toMap();

    QString brushName = name;
    if(localBrush.contains(brushName)){
        brush_ = localBrush[brushName];
        currentSettings = brush_->settings();
    }else{
        brush_ = brushFactory(brushName);
        localBrush.insert(brushName, brush_);
        currentSettings = brush_->settings();
    }
    // share same color between brushes
    if(shareColor_){
        currentSettings["color"] = colorMap;
    }

//    brush_->setLastPoint(lp);
    brush_->setSurface(sur);
    updateCursor();

    emit newBrushSettings(currentSettings);
}

void Canvas::onColorPicker(bool in)
{
    if(in){
        inPicker = true;
        QPixmap icon = QPixmap(":/iconset/ui/picker-cursor.png");
        setCursor(QCursor(icon, 11, 20));
    }else{
        inPicker = false;
        updateCursor();
        emit pickColorComplete();
    }
}

/*!
    \fn void Canvas::drawLineTo(const QPoint &endPoint)

    Draws a line from last point to \a endPoint .
    The last point can be either determined by drawPoint() or by function itself.
    After drawing, it emits linePainted()

    \sa drawPoint() , linePainted()
*/

void Canvas::drawLineTo(const QPoint &endPoint, qreal pressure)
{
    LayerPointer l = layers.selectedLayer();
    if(l.isNull() || l->isLocked() || l->isHided()){
        setCursor(Qt::ForbiddenCursor);
        return;
    }
    updateCursor();
    brush_->setSurface(l);
    brush_->drawLineTo(endPoint, pressure);

    update();

    QVariantMap point;
    point.insert("x", endPoint.x());
    point.insert("y", endPoint.y());
    // guarantee that pressure is double
    point.insert("pressure", double(pressure));

    storeAction(point);
}

/*!
    \fn void Canvas::drawPoint(const QPoint &point)

    Draws a point at \a endPoint.
    After drawing, it emits pointPainted()
    \sa drawPoint() , pointPainted()
*/

void Canvas::drawPoint(const QPoint &point, qreal pressure)
{
    LayerPointer l = layers.selectedLayer();
    if(l.isNull() || l->isLocked() || l->isHided()){
        setCursor(Qt::ForbiddenCursor);
        return;
    }
    updateCursor();
    brush_->setSurface(l);
    brush_->drawPoint(point, pressure);

    int rad = (brush_->width() / 2) + 2;
    update(QRect(lastPoint, point).normalized()
           .adjusted(-rad, -rad, +rad, +rad));

    QVariantMap point_j;
    point_j.insert("x", point.x());
    point_j.insert("y", point.y());
    // guarantee that pressure is double
    point_j.insert("pressure", double(pressure));

    storeAction(point_j);
}

void Canvas::storeAction(const QVariantMap &map)
{
    action_buffer_.push_back(map);
}

void Canvas::sendAction()
{
    QVariantMap store;
    store.insert("layer", currentLayer());
    store.insert("clientid",
               Singleton<ClientSocket>::instance().clientId());
    store.insert("name",
               Singleton<ClientSocket>::instance().userName());
    store.insert("type", "data");
    store.insert("brush", brushSettings());
    store.insert("action", "block");
    store.insert("block", action_buffer_);

    emit newPaintAction(store);
    action_buffer_.clear();
}

void Canvas::pickColor(const QPoint &point)
{
    brush_->setColor(image.pixel(point));
    newBrushSettings(brush_->settings());
}

void Canvas::updateCursor()
{
    this->setCursor(brush_->cursor());
}

/*!
    \fn void Canvas::remoteDrawPoint(const QPoint &point, const QVariantMap &brushInfo,
                             const QString &layer,
                             const quint64 userid)

    Draws a remote point at \a point at \a layer with \a brushInfo.
    To identical user, \a userid must provided.
    \sa Canvas::remoteDrawLine()
*/

void Canvas::remoteDrawPoint(const QPoint &point,
                             const QVariantMap &brushInfo,
                             const QString &layer,
                             const QString clientid,
                             const qreal pressure)
{
    if(!layers.exists(layer)) return;
    LayerPointer l = layers.layerFrom(layer);

    QVariantMap cpd_brushInfo = brushInfo;
    QString brushName = cpd_brushInfo["name"].toString().toLower();

    cpd_brushInfo.remove("name"); // remove useless info

    if(remoteBrush.contains(clientid)){
        BrushPointer t = remoteBrush[clientid];
        if(brushName != t->name().toLower()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l);
            newOne->setSettings(cpd_brushInfo);
            newOne->drawPoint(point, pressure);
            remoteBrush[clientid] = newOne;
//            t.clear();
        }else{
            BrushPointer original = remoteBrush[clientid];
            original->setSurface(l);
            original->setSettings(cpd_brushInfo);
            original->drawPoint(point, pressure);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l);
        newOne->setSettings(cpd_brushInfo);
        newOne->drawPoint(point, pressure);
        remoteBrush[clientid] = newOne;
    }

//    update();
}

/*!
    \fn void Canvas::remoteDrawLine(const QPoint &start, const QPoint &end,
                            const QVariantMap &brushInfo,
                            const QString &layer,
                            const quint64 userid)

    Draws a remote line from \a start to \a end at \a layer with \a brushInfo .
    To identical user, \a userid must provided.
    \sa Canvas::remoteDrawLine()
*/

void Canvas::remoteDrawLine(const QPoint &, const QPoint &end,
                            const QVariantMap &brushInfo,
                            const QString &layer,
                            const QString clientid,
                            const qreal pressure)
{
    if(!layers.exists(layer)){
        return;
    }
    LayerPointer l = layers.layerFrom(layer);

    QVariantMap cpd_brushInfo = brushInfo;
    QString brushName = cpd_brushInfo["name"].toString().toLower();

    if(remoteBrush.contains(clientid)){
        BrushPointer t = remoteBrush[clientid];
        if(brushName != t->name().toLower()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l);
            newOne->setSettings(cpd_brushInfo);
            newOne->drawLineTo(end, pressure);
            remoteBrush[clientid] = newOne;
//            t.clear();
        }else{
            BrushPointer original = remoteBrush[clientid];
            original->setSurface(l);
            original->setSettings(cpd_brushInfo);
            original->drawLineTo(end, pressure);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l);
        newOne->setSettings(cpd_brushInfo);
        qDebug()<<"warning, remote drawing starts with line drawing";
        newOne->drawLineTo(end, pressure);
        remoteBrush[clientid] = newOne;
    }
//    update();
}

void Canvas::onMembersSorted(const QList<MS>& list)
{
    author_list_ = list;
    update();
}

/* Layer */

/*!
    \fn QString Canvas::currentLayer()

    Returns current drawing layer.
    In detail, it always returns the layer selected by user, even it is hided or locked.
*/

QString Canvas::currentLayer()
{
    return layers.selectedLayer()->name();
}

/*!
    \fn void Canvas::addLayer(const QString &name)

    Adds a new layer named \a name. The layer will append on the top.
    \sa deleteLayer()
*/

void Canvas::addLayer(const QString &name)
{
    layers.appendLayer(name);
    layerNameCounter++;
}

/*!
    \fn bool Canvas::deleteLayer(const QString &name)

    Deletes the layer named \a name. Returns false if failed.
    A locked layer cannot be deleted, while a hided one can.
    \sa addLayer()
*/

bool Canvas::deleteLayer(const QString &name)
{
    if(layers.layerFrom(name)->isLocked())
        return false;

    layers.removeLayer(name);
    update();
    return true;
}

void Canvas::clearLayer(const QString &name)
{
    layers.clearLayer(name);
    update();
}

void Canvas::clearAllLayer()
{
    emit requestClearMembers();
    layers.clearAllLayer();
    update();
}

/*!
    \fn void Canvas::lockLayer(const QString &name)

    Locks one layer named \a name.
    \sa unlockLayer()
*/

void Canvas::lockLayer(const QString &name)
{
    layers.layerFrom(name)->lock();
}

/*!
    \fn void Canvas::unlockLayer(const QString &name)

    Unlocks one layer named \a name.
    \sa lockLayer()
*/

void Canvas::unlockLayer(const QString &name)
{
    layers.layerFrom(name)->unlock();
}

/*!
    \fn void Canvas::hideLayer(const QString &name)

    Hides one layer named \a name.
    \sa showLayer()
*/

void Canvas::hideLayer(const QString &name)
{
    layers.layerFrom(name)->hide();
    update();
}

/*!
    \fn void Canvas::showLayer(const QString &name)

    Show one layer named \a name.
    \sa hideLayer()
*/

void Canvas::showLayer(const QString &name)
{
    layers.layerFrom(name)->show();
    update();
}

void Canvas::moveLayerUp(const QString &name)
{
    layers.moveUp(name);
    update();
}

void Canvas::moveLayerDown(const QString &name)
{
    layers.moveDown(name);
    update();
}

/*!
    \fn void Canvas::layerSelected(const QString &name)

    Selects the layer named \a name.

    This function is used when render on the screen.
*/

void Canvas::layerSelected(const QString &name)
{
    layers.select(name);
}

/* Event control */

#ifndef PAINTTY_NO_TABLET

void Canvas::tabletEvent(QTabletEvent *ev)
{
    //TODO: fully support tablet
    qreal pressure = ev->pressure();
    QPoint pos = ev->pos();
    qDebug()<<"pressure:"<<pressure<<"at"<<pos;
    switch(ev->type()){
    case QEvent::TabletPress:
        if(!drawing){
            lastPoint = pos;
            drawPoint(pos, pressure);
            drawing = true;
        }
        break;
    case QEvent::TabletMove:
        if(drawing && lastPoint != pos){
            drawLineTo(pos, pressure);
            lastPoint = pos;
        }
        break;
    case QEvent::TabletRelease:
        if(drawing){
            drawing = false;
            updateCursor();
        }
        break;
    default:
        break;
    }
    ev->accept();
}

#endif

void Canvas::focusInEvent(QFocusEvent *)
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool disable_ime = settings.value("canvas/auto_disable_ime").toBool();
    if(disable_ime)
        PlatformExtend::setIMEState(this, false);
}

void Canvas::focusOutEvent(QFocusEvent *)
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool disable_ime = settings.value("canvas/auto_disable_ime").toBool();
    if(disable_ime)
        PlatformExtend::setIMEState(this, true);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        if(inPicker){
            pickColor(event->pos());
        }else{
            drawing = true;
            stackPoints.push_back(lastPoint);
            drawPoint(lastPoint);
        }
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton)){
        if(inPicker){
            pickColor(event->pos());
        }else{
            if(!drawing){
                return;
            }
            if(jitterCorrection_){
                if(stackPoints.length() < qBound(3, jitterCorrectionLevel_, 10)){
                    stackPoints.push_back(event->pos());
                }else{
                    tryJitterCorrection();
                    for(auto &p: stackPoints){
                        drawLineTo(p);
                        lastPoint = p;
                    }
                    stackPoints.clear();
                }
            }else{
                drawLineTo(event->pos());
                lastPoint = event->pos();
            }
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(inPicker){
            pickColor(event->pos());
            onColorPicker(false);
        }else{
            if(drawing){
                drawing = false;
                stackPoints.clear();
                updateCursor();
                sendAction();
            }
        }
    }
}

static inline qint64 find_newest_active(const QList<CanvasBackend::MemberSection>& al)
{
    qint64 longest = 0;
    for(auto &item: al) {
        qint64 stamp = std::get<MSI::LastActiveStamp>(item);
        if(stamp > longest){
            longest = stamp;
        }
    }
    if(!longest){
        longest = QDateTime::currentMSecsSinceEpoch();
    }
    return longest;
}

void Canvas::drawAuthorTips(QPainter& painter,
                            const QPoint& pos,
                            const QString& name)
{
    QFontMetrics fm(this->font());
    int t_width = fm.width(name)+15;
    int t_height = fm.height()+15;
    QRect box_rect(pos.x(), pos.y(), t_width, t_height);

    painter.save();
    QPen pen(Qt::transparent);
    QBrush brush(Qt::black);
    painter.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing, true);
    painter.setOpacity(0.6);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawRoundedRect(box_rect, 10, 10);

    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawText(box_rect, Qt::AlignCenter, name);
    painter.restore();
}

void Canvas::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    QRect dirtyRect = event->rect();
    if(dirtyRect.isEmpty())return;
    layers.combineLayers(&image, dirtyRect);

    painter.drawImage(dirtyRect, image, dirtyRect);

    // filter outdated names.
    // Considering using another QImage instead of direct draw
    qint64 longest = find_newest_active(author_list_);
    for(auto& item: author_list_){
        QPoint point = std::get<MSI::Footprint>(item);
        QString name = std::get<MSI::Name>(item);
        qint64 stamp = std::get<MSI::LastActiveStamp>(item);
        if(name.isEmpty()){
            name = std::get<MSI::Id>(item);
        }
        if(longest - stamp > 1000*30){
            break;
        }
        if(point.isNull()){
            break;
        }

        drawAuthorTips(painter, point, name);
    }

    if(!isEnabled()){
        QBrush brush;
        brush.setStyle(Qt::BDiagPattern);
        brush.setColor(Qt::lightGray);
        painter.setBrush(brush);
        QRect rect = this->rect();
        rect.setWidth(rect.width());
        rect.setHeight(rect.height());
        painter.drawRect(rect);
    }

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget,
                           &opt, &painter, this);
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    // NOTE: only to stop unexpected resize
    if(event->size() != canvasSize)
        return;
    QSize newSize = event->size();
    canvasSize = newSize;
    layers.resizeLayers(newSize);
    QImage newImage(newSize, QImage::Format_ARGB32_Premultiplied);
    newImage.fill(Qt::transparent);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), image);
    image = newImage;

    update();
    QWidget::resizeEvent(event);
}

/*!
    \fn QSize Canvas::sizeHint() const

    Returns a pre-defined size for canvas.

    \sa QWidget::minimumSizeHint()
*/

QSize Canvas::sizeHint() const
{
    return canvasSize;
}

/*!
    \fn QSize Canvas::minimumSizeHint() const

    Returns a pre-defined minimal size for canvas.

    \sa sizeHint()
*/

QSize Canvas::minimumSizeHint() const
{
    return canvasSize;
}
