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
#include <QStaticText>
#include <QtCore/qmath.h>


#include "../../common/common.h"
#include "../../common/network/clientsocket.h"
#include "../paintingTools/brush/brushmanager.h"
#include "../paintingTools/brush/brush.h"
#include "../paintingTools/brush/sketchbrush.h"
#include "../paintingTools/brush/eraser.h"
#include "../paintingTools/brush/pencil.h"
#include "../misc/platformextend.h"
#include "../misc/singleton.h"

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
    image(canvasSize),
    layerNameCounter(0),
    shareColor_(true),
    jitterCorrection_(true),
    jitterCorrectionLevel_(3),
    backend_(new CanvasBackend(0)),
    worker_(new QThread(this))
{
    setAttribute(Qt::WA_StaticContents);
    inPicker = false;
    drawing = false;
    brush_ = BrushPointer(new Brush);
    updateCursor();

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus); // necessary for IME control
    qDebug()<<"Canvas Size is "<<canvasSize;
    resize(canvasSize);

    BrushPointer p1(new Brush);
    BrushPointer p2(new Pencil);
    BrushPointer p3(new SketchBrush);
    BrushPointer p4(new Eraser);
    Singleton<BrushManager>::instance().addBrush(p1);
    Singleton<BrushManager>::instance().addBrush(p2);
    Singleton<BrushManager>::instance().addBrush(p3);
    Singleton<BrushManager>::instance().addBrush(p4);
    setJitterCorrectionLevel(5);

    worker_->start();
    backend_->moveToThread(worker_);
    connect(backend_, &CanvasBackend::remoteDrawLine,
            this, &Canvas::remoteDrawLine);
    connect(backend_, &CanvasBackend::remoteDrawPoint,
            this, &Canvas::remoteDrawPoint);
    connect(this, &Canvas::destroyed,
            backend_, &CanvasBackend::deleteLater);
    connect(this, &Canvas::destroyed,
            worker_, &QThread::terminate);
    connect(this, &Canvas::newPaintAction,
            backend_, &CanvasBackend::onDataBlock);
    connect(this, &Canvas::paintActionComplete,
            backend_, &CanvasBackend::commit);

    // TODO: proper use of signal requestSortedMembers,
    // instead of using QTimer
    QTimer *t = new QTimer(this);

    typedef CanvasBackend::MemberSectionIndex CM;

    qRegisterMetaType<CM>("CanvasBackend::MemberSectionIndex");
    qRegisterMetaType< QList<CanvasBackend::MemberSection> >("QList<MemberSection>");

    connect(backend_, &CanvasBackend::membersSorted,
            this, &Canvas::onMembersSorted);
    connect(this, &Canvas::requestSortedMembers,
            backend_, &CanvasBackend::requestMembers);
    connect(this, &Canvas::requestClearMembers,
            backend_, &CanvasBackend::clearMembers);
    connect(t, &QTimer::timeout,
            [this](){
        emit requestSortedMembers(CM::Count, true);
    });
    t->start(5000);
}

/*!
    \fn Canvas::~Canvas()

    Destroys the canvas. The internal \l Brush will also be deleted.
*/

Canvas::~Canvas()
{
}

QPixmap Canvas::currentCanvas()
{
    QPixmap pmp = image;
    layers.combineLayers(&pmp);
    return appendAuthorSignature(pmp);
}

QPixmap Canvas::allCanvas()
{
    QPixmap exp(canvasSize);
    exp.fill();
    QPainter painter(&exp);
    int count = layers.count();
    QPixmap * im = 0;
    for(int i=0;i<count;++i){
        LayerPointer l = layers.layerFrom(i);
        im = l->imagePtr();
        painter.drawPixmap(0, 0, *im);
    }
    return appendAuthorSignature(exp);
}

QPixmap Canvas::appendAuthorSignature(QPixmap target)
{
    // TODO: make all numbers configurable
    using CBMSI = CanvasBackend::MemberSectionIndex;
    int textSize = qMin(target.size().height(), target.width());
    textSize = qBound(10, int(textSize * 0.02), 100);

    QPainter painter(&target);
    QPen textPen;
    // TODO: use contrast color to render text
    textPen.setColor(Qt::black);
    painter.setOpacity(0.4);
    painter.setPen(textPen);

    QStaticText text;
    QString authors;
    text.setTextFormat(Qt::RichText);
    QTextOption textOp;
    textOp.setAlignment(Qt::AlignRight);
    text.setTextOption(textOp);

    for(auto& item: author_list_){
        QString name = std::get<CBMSI::Name>(item);
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
    jitterCorrectionLevel_internal_ =
            qBound(0.0,
                   jitterCorrectionLevel_ / 10.0 * 0.99 + 0.85,
                   0.99);
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
        // l3^2 = l1^2 + l2^2 + 2*l1*l2*cos(a), where cos(a) is our goal
        qreal len3 = l3.dx()*l3.dx() + l3.dy()*l3.dy();
        len3 = qSqrt(len3);
        qreal len2 = l2.dx()*l2.dx() + l2.dy()*l2.dy();
        len2 = qSqrt(len2);
        qreal len1 = l1.dx()*l1.dx() + l1.dy()*l1.dy();
        len1 = qSqrt(len1);

        qreal cosa = (len3*len3 - len2*len2 - len1*len1) / (2*len1*len2);

        if(cosa < jitterCorrectionLevel_internal_ ){
            redudent--;
            return true;
        }else{
            return false;
        }

    };

    for(int i=0;i<stackPoints.length()-3;++i){
        if(should_correct(stackPoints[i],
                          stackPoints[i+1],
                          stackPoints[i+2])){
            stackPoints.removeAt(i+1);
        }
    }
    // you can see the correction rate.
    qDebug()<<"correction rate: "<<qreal(amount-redudent)/amount *100<<"%";
}

/*!
    \fn QVariantMap Canvas::brushInfo()

    Returns the brush info painting now.
    \sa Brush::brushInfo()
*/

QVariantMap Canvas::brushInfo()
{
    return brush_->brushInfo();
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
    if(brush_->width() != newWidth){
        brush_->setWidth(newWidth);
        updateCursor();
    }
}

void Canvas::setBrushHardness(int h)
{
    if(brush_->hardness() != h){
        brush_->setHardness(h);
    }
}

BrushPointer Canvas::brushFactory(const QString &name)
{
    return Singleton<BrushManager>::instance().makeBrush(name);
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
    QPointF lp = brush_->lastPoint();
    QVariantMap colorMap = brush_->brushInfo()
            .value("color").toMap();

    QString brushName = name;
    if(localBrush.contains(brushName)){
        brush_ = localBrush[brushName];
        currentSettings = brush_->brushInfo();
    }else{
        brush_ = brushFactory(brushName);
        localBrush.insert(brushName, brush_);
        currentSettings = brush_->defaultInfo();
    }
    // share same color between brushes
    if(shareColor_){
        currentSettings["color"] = colorMap;
    }

    brush_->setLastPoint(lp);
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
    brush_->lineTo(endPoint, pressure);

    update();

    QVariantMap start_j;
    start_j.insert("x", this->lastPoint.x());
    start_j.insert("y", this->lastPoint.y());
    QVariantMap end_j;
    end_j.insert("x", endPoint.x());
    end_j.insert("y", endPoint.y());

    QVariantMap map;
    map.insert("brush", QVariant(brushInfo()));
    map.insert("start", QVariant(start_j));
    map.insert("end", QVariant(end_j));
    // guarantee that pressure is double
    map.insert("pressure", QVariant(double(pressure)));
    map.insert("layer", QVariant(currentLayer()));
    map.insert("clientid",
               Singleton<ClientSocket>::instance().clientId());
    map.insert("name",
               Singleton<ClientSocket>::instance().userName());

    QVariantMap bigMap;
    bigMap.insert("info", map);
    bigMap.insert("action", "drawline");
    bigMap.insert("type", "data");

    emit newPaintAction(bigMap);
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
    brush_->start(point, pressure);

    int rad = (brush_->width() / 2) + 2;
    update(QRect(lastPoint, point).normalized()
           .adjusted(-rad, -rad, +rad, +rad));

    QVariantMap point_j;
    point_j.insert("x", point.x());
    point_j.insert("y", point.y());

    QVariantMap map;
    map.insert("brush", QVariant(brushInfo()));
    // guarantee that pressure is double
    map.insert("pressure", QVariant(double(pressure)));
    map.insert("layer", QVariant(currentLayer()));
    map.insert("point", QVariant(point_j));
    map.insert("clientid",
               QVariant(Singleton<ClientSocket>::instance().clientId()));
    map.insert("name",
               Singleton<ClientSocket>::instance().userName());

    QVariantMap bigMap;
    bigMap.insert("info", map);
    bigMap.insert("action", "drawpoint");
    bigMap.insert("type", "data");

    emit newPaintAction(bigMap);
}

void Canvas::pickColor(const QPoint &point)
{
    brush_->setColor(image.toImage().pixel(point));
    newBrushSettings(brush_->brushInfo());
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

    QString brushName = brushInfo["name"].toString();
    int width = brushInfo["width"].toInt();
    int hardness = brushInfo["hardness"].toInt();
    QVariantMap colorMap = brushInfo["color"].toMap();
    QColor color(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());

    if(remoteBrush.contains(clientid)){
        BrushPointer t = remoteBrush[clientid];
        if(brushInfo != t->brushInfo()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l);
            newOne->setWidth(width);
            newOne->setHardness(hardness);
            newOne->setColor(color);
            newOne->start(point, pressure);
            remoteBrush[clientid] = newOne;
            t.clear();
        }else{
            BrushPointer original = remoteBrush[clientid];
            original->setSurface(l);
            original->setWidth(width);
            original->setHardness(hardness);
            original->setColor(color);
            original->start(point, pressure);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l);
        newOne->setWidth(width);
        newOne->setHardness(hardness);
        newOne->setColor(color);
        newOne->start(point, pressure);
        remoteBrush[clientid] = newOne;
    }

    update();
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

    QString brushName = brushInfo["name"].toString();
    int width = brushInfo["width"].toInt();
    int hardness = brushInfo["hardness"].toInt();
    QVariantMap colorMap = brushInfo["color"].toMap();
    QColor color(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());

    if(remoteBrush.contains(clientid)){
        BrushPointer t = remoteBrush[clientid];
        if(brushName != t->brushInfo()["name"].toString()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l);
            newOne->setWidth(width);
            newOne->setHardness(hardness);
            newOne->setColor(color);
            newOne->lineTo(end, pressure);
            remoteBrush[clientid] = newOne;
            t.clear();
        }else{
            BrushPointer original = remoteBrush[clientid];
            original->setSurface(l);
            original->setWidth(width);
            original->setHardness(hardness);
            original->setColor(color);
            original->lineTo(end, pressure);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l);
        newOne->setWidth(width);
        newOne->setHardness(hardness);
        newOne->setColor(color);
        newOne->lineTo(end, pressure);
        remoteBrush[clientid] = newOne;
    }

    update();
}

void Canvas::onMembersSorted(const QList<CanvasBackend::MemberSection>& list)
{
    author_list_ = list;
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
// FIXME: uncomment below when Qt support tablet
//void Canvas::tabletEvent(QTabletEvent *ev)
//{
//    if(!enable_tablet) {
//        return;
//    }
//    //TODO: fully support tablet
//    qreal pressure = ev->pressure();
//    QPoint pos = ev->pos();

//    switch(ev->type()){
//    case QEvent::TabletPress:
//        if(!drawing){
//            lastPoint = pos;
//            drawPoint(pos, pressure);
//            drawing = true;
//        }
//        break;
//    case QEvent::TabletMove:
//        if(drawing && lastPoint != pos){
//            drawLineTo(pos, pressure);
//            lastPoint = pos;
//        }
//        break;
//    case QEvent::TabletRelease:
//        if(drawing){
//            drawing = false;
//            updateCursor();
//        }
//        break;
//    default:
//        break;
//    }
//    ev->accept();
//}

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
            if(drawing){
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
                emit paintActionComplete();
            }
        }
    }
}

void Canvas::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    QRect dirtyRect = event->rect();
    if(dirtyRect.isEmpty())return;
    layers.combineLayers(&image, dirtyRect);

    painter.drawPixmap(dirtyRect, image, dirtyRect);

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
    QPixmap newImage(newSize);
    newImage.fill(Qt::transparent);
    QPainter painter(&newImage);
    painter.drawPixmap(QPoint(0, 0), image);
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

CanvasBackend::CanvasBackend(QObject *parent)
    :QObject(parent),
      blocklevel_(LOW)
{
    QTimer *sendTimer = new QTimer(this);
    sendTimer->setInterval(1000*10);
    connect(sendTimer, &QTimer::timeout,
            this, &CanvasBackend::commit);
    auto& client_socket = Singleton<ClientSocket>::instance();

    connect(&client_socket, &ClientSocket::dataPack,
            this, &CanvasBackend::onIncomingData);
    connect(this, &CanvasBackend::newDataGroup,
            &client_socket, &ClientSocket::sendDataPack);
}

CanvasBackend::~CanvasBackend()
{
    auto& client_socket = Singleton<ClientSocket>::instance();

    disconnect(&client_socket, &ClientSocket::dataPack,
            this, &CanvasBackend::onIncomingData);
    disconnect(this, &CanvasBackend::newDataGroup,
            &client_socket, &ClientSocket::sendDataPack);
}

void CanvasBackend::commit()
{
    if(blocklevel_ == NONE){
        while(!tempStore.isEmpty()){
            QVariant element = tempStore.takeFirst();
            auto data = toJson(element);
            emit newDataGroup(data);
        }
    }else{
        if(!tempStore.isEmpty()){
            QVariantMap doc;
            doc.insert("action", "block");
            doc.insert("type", "data");
            doc.insert("block", tempStore);
            auto data = toJson(QVariant(doc));
            emit newDataGroup(data);
            tempStore.clear();
        }
    }
}

void CanvasBackend::setBlockLevel(const BlockLevel le)
{
    blocklevel_ = le;
}

CanvasBackend::BlockLevel CanvasBackend::blockLevel() const
{
    return blocklevel_;
}

void CanvasBackend::onDataBlock(const QVariantMap& d)
{
    tempStore.append(d);

    QVariantMap info = d["info"].toMap();
    QString author = info["name"].toString();
    QString clientid = info["clientid"].toString();
    upsertMember(clientid, author);

    if(blocklevel_ == NONE){
        commit();
    }else{
        if(tempStore.length() >= blocklevel_ ){
            commit();
        }
    }
}

void CanvasBackend::onIncomingData(const QJsonObject& obj)
{
    QString action = obj.value("action").toString().toLower();

    auto drawPoint = [this](const QVariantMap& m){
        QPoint point;
        QString layerName;

        QVariantMap map = m["info"].toMap();
        QString clientid = map["clientid"].toString();
        // don't draw your own move from remote
        if(clientid == Singleton<ClientSocket>::instance().clientId()){
            return;
        }

        QVariantMap point_j = map["point"].toMap();
        point.setX(point_j["x"].toInt());
        point.setY(point_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        qreal pressure = 1.0;
        if(map.contains("pressure")){
            pressure = map["pressure"].toDouble();
        }

        if(map.contains("name")){
            QString author = map["name"].toString();
            upsertMember(clientid, author);
        }

        emit remoteDrawPoint(point, brushInfo,
                             layerName, clientid,
                             pressure);
    };

    auto drawLine = [this](const QVariantMap& m){
        QPoint start;
        QPoint end;
        QString layerName;

        QVariantMap map = m["info"].toMap();

        QString clientid = map["clientid"].toString();
        // don't draw your own move from remote
        if(clientid == Singleton<ClientSocket>::instance().clientId()){
            return;
        }

        QVariantMap start_j = map["start"].toMap();
        start.setX(start_j["x"].toInt());
        start.setY(start_j["y"].toInt());
        QVariantMap end_j = map["end"].toMap();
        end.setX(end_j["x"].toInt());
        end.setY(end_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        qreal pressure = 1.0;
        if(map.contains("pressure")){
            pressure = map["pressure"].toDouble();
        }

        if(map.contains("name")){
            QString author = map["name"].toString();
            upsertMember(clientid, author);
        }

        emit remoteDrawLine(start, end,
                            brushInfo, layerName,
                            clientid, pressure);
    };

    auto dataBlock = [&drawPoint,
            &drawLine](const QVariantMap& m){
        QVariantList list = m["block"].toList();
        for(auto &item: list){
            QVariantMap singleData = item.toMap();
            QString action = singleData["action"].toString().toLower();
            if(action == "drawpoint"){
                drawPoint(singleData);
            }else if(action == "drawline"){
                drawLine(singleData);
            }
        }
    };

    if(action == "drawpoint"){
        drawPoint(obj.toVariantMap());
    }else if(action == "drawline"){
        drawLine(obj.toVariantMap());
    }else if(action == "block"){
        dataBlock(obj.toVariantMap());
    }
}

void CanvasBackend::requestMembers(MemberSectionIndex index,
                                   bool mergeSameName)
{
    qDebug()<<"Members requested!";
//    using MSI = MemberSectionIndex;
    using MS = MemberSection;
    using MSL = QList<MS>;

    MSL list = memberHistory_.values();
    qSort(list.begin(), list.end(), [index](const MS &e1,
          const MS &e2) {
        // Note, std::get<> never receive dynamic index.
        // And that's why we have to use switch :(
        switch (index){
        case Name:
            return std::get<Name>(e1) < std::get<Name>(e2);
            break;
        default: // fall-through
        case Count:
            return std::get<Count>(e1) < std::get<Count>(e2);
            break;
        }
    });

    if(mergeSameName){
        QHash<QString, MS> list2;
        for(const MS &elem: list){
            QString author = std::get<Name>(elem);
            if(list2.contains(author)){
                std::get<Count>(list2[author]) += std::get<Count>(elem);
            }else{
                list2.insert(author, elem);
            }
        }
        list = list2.values();
    }
    emit membersSorted(list);
}

void CanvasBackend::clearMembers()
{
    memberHistory_.clear();
}

void CanvasBackend::upsertMember(const QString& id, const QString& name)
{
    using MSI = MemberSectionIndex;
    if( memberHistory_.contains(id) ) {
        std::get<MSI::Count>( memberHistory_[id] )++;
    }else{
        memberHistory_[id] = MemberSection(id, name, 1);
    }
}

QByteArray CanvasBackend::toJson(const QVariant &m)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return QJsonDocument::fromVariant(m).toJson(QJsonDocument::Compact);
#else
    return QJsonDocument::fromVariant(m).toJson();
#endif
}

QVariant CanvasBackend::fromJson(const QByteArray &d)
{
    return QJsonDocument::fromJson(d).toVariant();
}
