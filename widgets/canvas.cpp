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
    canvasSize(3240,2160),
    image(canvasSize),
    layerNameCounter(0),
    historySize_(0)
{
    setAttribute(Qt::WA_StaticContents);
//    setBaseSize(canvasSize);
    inPicker = false;
    drawing = false;
    opacity = 1.0;
    brush_ = BrushPointer(new Brush(this));
    changeBrush("pencilButton");
    updateCursor(brush_->width());

    setMouseTracking(true);
    this->resize(canvasSize);
    genUserid();
}

/*!
    \fn Canvas::~Canvas()

    Destroys the canvas. The internal \l Brush will also be deleted.
*/

Canvas::~Canvas()
{
}

/*!
    \fn void Canvas::genUserid()

    Generates a pseudo-random number as an id to identical within a room.

    \warning You don't need to call this function manually since it will be called in constructor.
*/

void Canvas::genUserid()
{
    userid_ = quint64(&userid_); // treat as ramdom number
}

/*!
    \fn quint64 Canvas::userId()

    Returns the user id generated when constructed.
*/

quint64 Canvas::userId()
{
    return userid_;
}

void Canvas::setHistorySize(const quint64 &size)
{
    historySize_ = size;
}

QPixmap Canvas::currentCanvas()
{
    return combineLayers();
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
    return exp;
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
        updateCursor(newWidth);
//        qDebug()<<this->cursor().pixmap().size();
    }
}

BrushPointer Canvas::brushFactory(const QString &name)
{
    BrushPointer b;
    QString n_name = name.toLower();
    if(n_name.compare("brush") == 0){
        b = BrushPointer(new Brush(this));
    }else if(n_name.compare("pencil") == 0){
        b = BrushPointer(new Pencil(this));
    }else if(n_name.compare("sketch") == 0){
        b = BrushPointer(new SketchBrush(this));
    }else if(n_name.compare("eraser") == 0){
        b = BrushPointer(new Eraser(this));
    }else{
        qDebug()<<name<<"cannot identify";
        return BrushPointer(new Brush(this));
    }
    return b;
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
    QPixmap *sur = brush_->surface();
    QPointF lp = brush_->lastPoint();

    QString brushName = name.toLower();
    brushName.chop(6);
    if(localBrush.contains(brushName)){
        brush_ = localBrush[brushName];
        currentSettings = brush_->brushInfo();
    }else{
        brush_ = brushFactory(brushName);
        localBrush.insert(brushName, brush_);
        currentSettings = brush_->defaultInfo();
    }

    brush_->setLastPoint(lp);
    brush_->setSurface(sur);
    updateCursor(brush_->width());

    emit newBrushSettings(currentSettings);
}

void Canvas::onColorPicker(bool in)
{
    if(in){
        inPicker = true;
        QPixmap icon = QPixmap("iconset/color-picker.png");
        setCursor(QCursor(icon, 0, icon.height()));
    }else{
        inPicker = false;
        //        setCursor(Qt::CrossCursor);
        updateCursor(brush_->width());
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

void Canvas::drawLineTo(const QPoint &endPoint)
{
    LayerPointer l = layers.selectedLayer();
    if(l.isNull() || l->isLocked() || l->isHided()){
        setCursor(Qt::ForbiddenCursor);
        return;
    }
    //    setCursor(Qt::CrossCursor);
    updateCursor(brush_->width());
    brush_->setSurface(l->imagePtr());
    brush_->lineTo(endPoint);

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
    map.insert("layer", QVariant(currentLayer()));
    map.insert("userid", QVariant(userId()));

    QVariantMap bigMap;
    bigMap.insert("info", map);
    bigMap.insert("action", "drawline");

    QByteArray tmp = toJson(QVariant(bigMap));
    emit sendData(tmp);
}

/*!
    \fn void Canvas::drawPoint(const QPoint &point)

    Draws a point at \a endPoint.
    After drawing, it emits pointPainted()
    \sa drawPoint() , pointPainted()
*/

void Canvas::drawPoint(const QPoint &point)
{
    LayerPointer l = layers.selectedLayer();
    if(l.isNull() || l->isLocked() || l->isHided()){
        setCursor(Qt::ForbiddenCursor);
        return;
    }
    //    setCursor(Qt::CrossCursor);
    updateCursor(brush_->width());
    brush_->setSurface(l->imagePtr());
    brush_->start(point);

    int rad = (brush_->width() / 2) + 2;
    update(QRect(lastPoint, point).normalized()
           .adjusted(-rad, -rad, +rad, +rad));

    QVariantMap point_j;
    point_j.insert("x", point.x());
    point_j.insert("y", point.y());

    QVariantMap map;
    map.insert("brush", QVariant(brushInfo()));
    map.insert("layer", QVariant(currentLayer()));
    map.insert("point", QVariant(point_j));
    map.insert("userid", QVariant(userId()));

    QVariantMap bigMap;
    bigMap.insert("info", map);
    bigMap.insert("action", "drawpoint");

    QByteArray tmp = toJson(QVariant(bigMap));
    emit sendData(tmp);
}

void Canvas::pickColor(const QPoint &point)
{
    brush_->setColor(image.toImage().pixel(point));
    newBrushSettings(brush_->brushInfo());
}

void Canvas::updateCursor(const int &width)
{
    int r_width = width+10;
    QPixmap img(r_width, r_width);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.drawEllipse(0, 0, r_width/2, r_width/2);
    this->setCursor(QCursor(img, r_width/4, r_width/4));
}

/*!
    \fn void Canvas::remoteDrawPoint(const QPoint &point, const QVariantMap &brushInfo,
                             const QString &layer,
                             const quint64 userid)

    Draws a remote point at \a point at \a layer with \a brushInfo.
    To identical user, \a userid must provided.
    \sa Canvas::remoteDrawLine()
*/

void Canvas::remoteDrawPoint(const QPoint &point, const QVariantMap &brushInfo,
                             const QString &layer,
                             const quint64 userid)
{
    if(!layers.exists(layer)) return;
    LayerPointer l = layers.layerFrom(layer);

    QString brushName = brushInfo["name"].toString();
    int width = brushInfo["width"].toInt();
    QVariantMap colorMap = brushInfo["color"].toMap();
    QColor color(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt(),
            colorMap["alpha"].toInt());
//    QColor color = brushInfo["color"].value<QColor>();

    if(remoteBrush.contains(userid)){
        BrushPointer t = remoteBrush[userid];
        if(brushInfo != t->brushInfo()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l->imagePtr());
            newOne->setWidth(width);
            newOne->setColor(color);
            newOne->start(point);
            remoteBrush[userid] = newOne;
            t.clear();
        }else{
            BrushPointer original = remoteBrush[userid];
            original->setSurface(l->imagePtr());
            original->setWidth(width);
            original->setColor(color);
            original->start(point);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l->imagePtr());
        newOne->setWidth(width);
        newOne->setColor(color);
        newOne->start(point);
        remoteBrush[userid] = newOne;
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

void Canvas::remoteDrawLine(const QPoint &start, const QPoint &end,
                            const QVariantMap &brushInfo,
                            const QString &layer,
                            const quint64 userid)
{
    if(!layers.exists(layer)){
        return;
    }
    LayerPointer l = layers.layerFrom(layer);

    QString brushName = brushInfo["name"].toString();
    int width = brushInfo["width"].toInt();
    QVariantMap colorMap = brushInfo["color"].toMap();
    QColor color(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt(),
            colorMap["alpha"].toInt());

    if(remoteBrush.contains(userid)){
        BrushPointer t = remoteBrush[userid];
        if(brushName != t->brushInfo()["name"].toString()){
            BrushPointer newOne = brushFactory(brushName);
            newOne->setSurface(l->imagePtr());
            newOne->setWidth(width);
            newOne->setColor(color);
            newOne->lineTo(end);
            remoteBrush[userid] = newOne;
            t.clear();
        }else{
            BrushPointer original = remoteBrush[userid];
            original->setSurface(l->imagePtr());
            original->setWidth(width);
            original->setColor(color);
            original->lineTo(end);
        }
    }else{
        BrushPointer newOne = brushFactory(brushName);
        newOne->setSurface(l->imagePtr());
        newOne->setWidth(width);
        newOne->setColor(color);
        newOne->lineTo(end);
        remoteBrush[userid] = newOne;
    }

    update();
}

void Canvas::onNewData(const QByteArray & array)
{
//    static quint64 h_size = 0;
//    if(historySize_) {
//        h_size += array.size();
//        qDebug()<<h_size<<historySize_;
//        if(h_size < quint64(historySize_)){
//            this->setDisabled(true);
//        }else{
//            historySize_ = 0;
//            h_size = 0;
//            this->setEnabled(true);
//            qDebug()<<"enable!";
//        }
//    }
    QVariantMap m = fromJson(array).toMap();
    QString action = m["action"].toString().toLower();

    if(action == "drawpoint"){
        QPoint point;
        QString layerName;

        QVariantMap map = m["info"].toMap();
        QVariantMap point_j = map["point"].toMap();
        point.setX(point_j["x"].toInt());
        point.setY(point_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        quint64 userid = map["userid"].toLongLong();

        remoteDrawPoint(point, brushInfo,
                        layerName, userid);
    }else if(action == "drawline"){
        QPoint start;
        QPoint end;
        QString layerName;

        QVariantMap map = m["info"].toMap();
        QVariantMap start_j = map["start"].toMap();
        start.setX(start_j["x"].toInt());
        start.setY(start_j["y"].toInt());
        QVariantMap end_j = map["end"].toMap();
        end.setX(end_j["x"].toInt());
        end.setY(end_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        quint64 userid = map["userid"].toLongLong();

        remoteDrawLine(start, end,
                       brushInfo, layerName,
                       userid);

    }
}

QByteArray Canvas::toJson(const QVariant &m)
{
    return QJsonDocument::fromVariant(m).toJson();
//    static QJson::Serializer serializer;
//    serializer.setIndentMode(QJson::IndentCompact);
//    return serializer.serialize(m);

}

QVariant Canvas::fromJson(const QByteArray &d)
{
//    static QJson::Parser parser;
//    return parser.parse(d);
    return QJsonDocument::fromJson(d).toVariant();

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
    LayerPointer layer(new Layer(canvasSize));
    layer->rename(name);
    layers.appendLayer(layer, name);
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

/*!
    \fn void Canvas::combineLayers()

    Combines all layers together.

    This function is used when render on the screen.
*/

void Canvas::combineLayers(const QRect &rec)
{
    QPainter painter(&image);
    image.fill(Qt::white);
    int count = layers.count();
    QPixmap * im = 0;
    for(int i=0;i<count;++i){
        LayerPointer l = layers.layerFrom(i);
        if(l->isHided()){
            continue;
        }
        im = l->imagePtr();
        painter.drawPixmap(QRectF(rec), *im, QRectF(rec));
    }
}

QPixmap Canvas::combineLayers()
{
    QPainter painter(&image);
    image.fill(Qt::white);
    int count = layers.count();
    QPixmap * im = 0;
    for(int i=0;i<count;++i){
        LayerPointer l = layers.layerFrom(i);
        if(l->isHided()){
            continue;
        }
        im = l->imagePtr();
        painter.drawPixmap(0, 0, *im);
    }
    return image;
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

void Canvas::tabletEvent(QTabletEvent *ev)
{
    //TODO: fully support tablet
    qreal pressure = ev->pressure();
    if(pressure < 0.000001){
        drawing = false;
        ev->accept();
        return;
    }
    //    pen->setWidthF(pressure*10);
    if(!drawing){
        lastPoint = ev->pos();
        drawing = true;
        drawPoint(lastPoint);
    }else{
        drawLineTo(ev->pos());
        lastPoint = ev->pos();
    }
    ev->accept();
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        if(inPicker){
        }else{
            drawing = true;
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
                updateCursor(brush_->width());
            }
        }
    }
}

void Canvas::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    QRect dirtyRect = event->rect();
    if(dirtyRect.isEmpty())return;
    //    painter.setOpacity(opacity);
    combineLayers(dirtyRect);

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
    QSize newSize = event->size();
    canvasSize = newSize;
    layers.resizeLayers(newSize);
    resizeImage(&image, newSize);
    update();
    QWidget::resizeEvent(event);
}

/*!
    \fn void Canvas::resizeImage(QPixmap *image, const QSize &newSize)

    Resizes one QPixmap to \a newSize. It also fills all images into white.
*/

void Canvas::resizeImage(QPixmap *image, const QSize &newSize)
{
    QPixmap newImage(newSize);
    newImage.fill(Qt::white);
    QPainter painter(&newImage);
    painter.drawPixmap(QPoint(0, 0), *image);
    *image = newImage;
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
