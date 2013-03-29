#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QToolButton>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QShortcut>
#include <QFile>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QCryptographicHash>
#include <QHostAddress>
#include <QVariantList>
#include <QFileDialog>
#include <QMessageBox>

#include "../misc/singleshortcut.h"
#include "layerwidget.h"
#include "layeritem.h"
#include "colorgrid.h"
#include "aboutdialog.h"
#include "brushwidthwidget.h"
#include "../network/commandsocket.h"
#include "../paintingTools/brush/brushmanager.h"
#include "../common.h"

MainWindow::MainWindow(const QSize& canvasSize, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    msgSocket(this),
    dataSocket(this),
    historySize_(0),
    canvasSize_(canvasSize),
    lastBrushAction(nullptr),
    widthControl_(nullptr),
    toolbar_(nullptr),
    brushActionGroup_(nullptr),
    colorPickerButton_(nullptr)
{
    ui->setupUi(this);
    ui->canvas->resize(canvasSize_);
    defaultView = saveState();
}

MainWindow::~MainWindow()
{
    msgSocket.close();
    dataSocket.close();
    CommandSocket::cmdSocket()->close();
    delete ui;
}

void MainWindow::stylize()
{
    QFile stylesheet("./iconset/style.qss",this);
    stylesheet.open(QIODevice::ReadOnly);
    QTextStream stream(&stylesheet);
    QString string;
    string = stream.readAll();
    this->setStyleSheet(string);
    stylesheet.close();
}

void MainWindow::init()
{
    ui->centralWidget->setBackgroundRole(QPalette::Dark);
    ui->centralWidget->setCanvas(ui->canvas);
    ui->panoramaLayout->addWidget(ui->centralWidget->scaleSlider());
    ui->panoramaLayout->setContentsMargins(8, 8, 8, 8);
    ui->panoramaLayout->setSpacing(2);
    ui->canvas->setDisabled(true);
    ui->layerWidget->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    ui->pushButton->setDisabled(true);


    connect(ui->lineEdit,&QLineEdit::returnPressed,
            this,&MainWindow::onSendPressed);
    connect(ui->pushButton,&QPushButton::clicked,
            this,&MainWindow::onSendPressed);

    connect(&msgSocket,&MessageSocket::connected,
            this,&MainWindow::onServerConnected);
    connect(&msgSocket,&MessageSocket::disconnected,
            this,&MainWindow::onServerDisconnected);
    connect(&msgSocket,
            static_cast<void (MessageSocket::*)(QString)>
            (&MessageSocket::newMessage),
            this,&MainWindow::onNewMessage);
    connect(this,&MainWindow::sendMessage,
            &msgSocket,&MessageSocket::sendMessage);
    connect(&dataSocket,&DataSocket::connected,
            this,&MainWindow::onServerConnected);
    connect(&dataSocket,&DataSocket::newData,
            ui->canvas,&Canvas::onNewData);
    connect(ui->canvas,&Canvas::sendData,
            &dataSocket,&DataSocket::sendData);
    connect(&dataSocket,&DataSocket::disconnected,
            this,&MainWindow::onServerDisconnected);

    connect(ui->canvas, &Canvas::newBrushSettings,
            this, &MainWindow::onBrushSettingsChanged);

    connect(ui->layerWidget,&LayerWidget::itemHide,
            ui->canvas, &Canvas::hideLayer);
    connect(ui->layerWidget,&LayerWidget::itemShow,
            ui->canvas, &Canvas::showLayer);
    connect(ui->layerWidget,&LayerWidget::itemLock,
            ui->canvas, &Canvas::lockLayer);
    connect(ui->layerWidget,&LayerWidget::itemUnlock,
            ui->canvas,  &Canvas::unlockLayer);
    connect(ui->layerWidget,&LayerWidget::itemSelected,
            ui->canvas, &Canvas::layerSelected);

    connect(ui->colorBox, &ColorBox::colorChanged,
            this, &MainWindow::brushColorChange);
    connect(this, &MainWindow::brushColorChange,
            ui->canvas, &Canvas::setBrushColor);
    connect(ui->canvas, &Canvas::pickColorComplete,
            this, &MainWindow::onPickColorComplete);

    connect(ui->colorGrid,
            static_cast<void (ColorGrid::*)(const int&)>
            (&ColorGrid::colorDroped),
            this, &MainWindow::onColorGridDroped);
    connect(ui->colorGrid, &ColorGrid::colorPicked,
            this, &MainWindow::onColorGridPicked);
    connect(ui->panorama, &PanoramaWidget::refresh,
            this, &MainWindow::onPanoramaRefresh);
    connect(ui->centralWidget, &CanvasContainer::rectChanged,
            ui->panorama, &PanoramaWidget::onRectChange);
    // use lambda to avoid that long static_cast :)
    connect(ui->panorama, &PanoramaWidget::moveTo,
            [&](const QPointF &p){
        ui->centralWidget->centerOn(p);
    });

    layerWidgetInit();
    colorGridInit();
    toolbarInit();
    viewInit();

    shortcutInit();
    //    stylize();
    cmdSocketRouterInit();

    QTimer *t = new QTimer(this);
    t->setInterval(5000);
    connect(t, &QTimer::timeout,
            this, &MainWindow::requestOnlinelist);
    t->start();
}

void MainWindow::cmdSocketRouterInit()
{
    cmdRouter_.regHandler("action",
                          "close",
                          std::bind(&MainWindow::onCommandActionClose,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("response",
                          "close",
                          std::bind(&MainWindow::onCommandResponseClose,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("action",
                          "clearall",
                          std::bind(&MainWindow::onCommandActionClearAll,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("response",
                          "clearall",
                          std::bind(&MainWindow::onCommandResponseClearAll,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("response",
                          "onlinelist",
                          std::bind(&MainWindow::onCommandResponseOnlinelist,
                                    this,
                                    std::placeholders::_1));
}

void MainWindow::layerWidgetInit()
{
    for(int i=0;i<10;++i){
        addLayer();
    }
    ui->layerWidget->itemAt(0)->setSelect(true);
}

void MainWindow::colorGridInit()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QByteArray data = settings.value("colorgrid/pal")
            .toByteArray();
    if(data.isEmpty()){
        return;
    }else{
        ui->colorGrid->dataImport(data);
    }
}

void MainWindow::viewInit()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QByteArray data = settings.value("mainwindow/view")
            .toByteArray();
    if(data.isEmpty()){
        return;
    }else{
        restoreState(data);
    }
}

void MainWindow::toolbarInit()
{
    toolbar_ = new QToolBar("Brushes", this);
    toolbar_->setObjectName("toolbar");
    this->addToolBar(Qt::TopToolBarArea, toolbar_);
    brushActionGroup_ = new QActionGroup(this);

    // always remember last action
    auto restoreAction =  [this](){
        if(lastBrushAction){
            lastBrushAction->trigger();
        }
    };

    auto brushes = BrushManager::allBrushes();

    for(auto item: brushes){
        // create action on tool bar
        QAction * action = toolbar_->addAction(item->icon(),
                                               item->displayName());
        action->setObjectName(item->name());
        connect(action, &QAction::triggered,
                this, &MainWindow::onBrushTypeChange);
        action->setCheckable(true);
        action->setAutoRepeat(false);
        brushActionGroup_->addAction(action);

        // set shortcut for the brush
        SingleShortcut *shortcut = new SingleShortcut(this);
        shortcut->setKey(item->shortcut());
        connect(shortcut, &SingleShortcut::activated,
                [=](){
            lastBrushAction = brushActionGroup_->checkedAction();
            action->trigger();
        });
        connect(shortcut, &SingleShortcut::inactivated,
                restoreAction);
        action->setToolTip(
                    tr("Shortcut: %1")
                    .arg(shortcut->key()
                         .toString()));
        if(toolbar_->actions().count() < 2){
            action->trigger();
        }
    }


    // doing hacking to color picker
    QIcon colorpickerIcon;
    colorpickerIcon.addFile("iconset/ui/picker-1.png",
                  QSize(), QIcon::Disabled);
    colorpickerIcon.addFile("iconset/ui/picker-2.png",
                  QSize(), QIcon::Active);
    colorpickerIcon.addFile("iconset/ui/picker-3.png",
                  QSize(), QIcon::Selected);
    colorpickerIcon.addFile("iconset/ui/picker-3.png",
                  QSize(), QIcon::Normal, QIcon::On);
    colorpickerIcon.addFile("iconset/ui/picker-4.png",
                  QSize(), QIcon::Normal);
    QAction *colorpicker = toolbar_->addAction(colorpickerIcon,
                                               tr("Color Picker"));
    colorpicker->setCheckable(true);
    colorpicker->setAutoRepeat(false);
    // we need a real QToolButton to know weather the picker is
    // canceled by hand
    auto l = colorpicker->associatedWidgets();
    if(l.count() > 1){
        QToolButton *b = qobject_cast<QToolButton *>(l[1]);
        if(b){
            colorPickerButton_ = b;
            connect(b, &QToolButton::clicked,
                    this, &MainWindow::onColorPickerPressed);

            SingleShortcut *pickerShortcut = new SingleShortcut(this);
            pickerShortcut->setKey(Qt::Key_C);
            connect(pickerShortcut, &SingleShortcut::activated,
                    b, &QToolButton::click);
            connect(pickerShortcut, &SingleShortcut::inactivated,
                    b, &QToolButton::click);
            colorpicker->setToolTip(
                        tr("Shortcut: %1")
                        .arg(pickerShortcut->key()
                             .toString()));
        }
    }

    // for brush width
    BrushWidthWidget * widthWidget = new BrushWidthWidget(this);
    connect(widthWidget, &BrushWidthWidget::valueChanged,
            ui->canvas, &Canvas::setBrushWidth);

    QShortcut* widthActionSub = new QShortcut(this);
    widthActionSub->setKey(Qt::Key_Q);
    connect(widthActionSub, SIGNAL(activated()),
            widthWidget, SLOT(down()));
    QShortcut* widthActionAdd = new QShortcut(this);
    widthActionAdd->setKey(Qt::Key_W);
    connect(widthActionAdd, SIGNAL(activated()),
            widthWidget, SLOT(up()));
    widthControl_ = widthWidget;

    toolbar_->addSeparator();
    toolbar_->addWidget(widthWidget);

    //TODO: locking before complete connect
}

QVariant MainWindow::getRoomKey()
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(roomName_.toUtf8());
    QString hashed_name = hash.result().toHex();
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.sync();
    if( !settings.contains("rooms/"+hashed_name) ){
        // Tell user that he's not owner
        qDebug()<<"hashed_name"<<hashed_name
               <<" key cannot found!";
        QMessageBox::warning(this,
                             tr("Sorry"),
                             tr("Only room owner is authorized "
                                "to close the room.\n"
                                "It seems you're not room manager."));
        return QVariant();
    }
    QVariant key = settings.value("rooms/"+hashed_name);
    if(key.isNull()){
        // Tell user that he's not owner
        return QVariant();
    }
    return key;
}

void MainWindow::requestOnlinelist()
{
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("request", QString("onlinelist"));
    obj.insert("clientid", CommandSocket::clientId());
    doc.setObject(obj);
    qDebug()<<"clientid: "<<CommandSocket::clientId();
    CommandSocket::cmdSocket()->sendData(doc.toJson());
}

void MainWindow::shortcutInit()
{
    connect(ui->action_Quit, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(ui->actionExport_All, SIGNAL(triggered()),
            this, SLOT(exportAllToFile()));
    connect(ui->actionExport_Visiable, SIGNAL(triggered()),
            this, SLOT(exportVisibleToFile()));
    connect(ui->actionExport_All_To_Clipboard, SIGNAL(triggered()),
            this, SLOT(exportAllToClipboard()));
    connect(ui->actionExport_Visible_To_ClipBorad, SIGNAL(triggered()),
            this, SLOT(exportVisibleToClipboard()));
    connect(ui->actionReset_View, SIGNAL(triggered()),
            this, SLOT(resetView()));
    connect(ui->action_About_Mr_Paint, SIGNAL(triggered()),
            this, SLOT(about()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()),
            qApp, SLOT(aboutQt()));
    connect(ui->actionClose_Room, &QAction::triggered,
            [&](){
        QVariantMap map;
        QVariant r_key = getRoomKey();
        map.insert("request", "close");
        if(r_key.isNull()){
            return;
        }
        map.insert("key", r_key);
        CommandSocket::cmdSocket()
                ->sendData(toJson(QVariant(map)));
    });
    connect(ui->actionAll_Layers, &QAction::triggered,
            this, &MainWindow::clearAllLayer);
}

void MainWindow::socketInit(int dataPort, int msgPort)
{
    connect(CommandSocket::cmdSocket(), &CommandSocket::newData,
            this, &MainWindow::onCmdServerData);


    ui->canvas->setHistorySize(historySize_);
    ui->textEdit->insertPlainText(tr("Connecting to server...\n"));
    msgSocket.connectToHost(QHostAddress(GlobalDef::HOST_ADDR),msgPort);
    dataSocket.connectToHost(QHostAddress(GlobalDef::HOST_ADDR),dataPort);
}

void MainWindow::setNickName(const QString &name)
{
    nickName_ = name;
}

void MainWindow::setRoomName(const QString &name)
{
    roomName_ = name;
    setWindowTitle(name+tr(" - Mr.Paint"));
}

void MainWindow::setHistorySize(const quint64 &size)
{
    historySize_ = size;
}

void MainWindow::setCanvasSize(const QSize &size)
{
    canvasSize_ = size;
    ui->canvas->resize(canvasSize_);
}

void MainWindow::onServerConnected()
{
    if(dataSocket.isConnected() && msgSocket.isConnected()){
        ui->textEdit->insertPlainText(tr("Server Connected.\n"));
        ui->canvas->setEnabled(true);
        ui->layerWidget->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->pushButton->setEnabled(true);
        //        ui->colorPicker->setEnabled(true);
    }
}

void MainWindow::onServerDisconnected()
{
    ui->textEdit->insertPlainText(tr("Server Connection Failed.\n"));
    ui->canvas->setEnabled(false);
}

void MainWindow::onCmdServerDisconnected()
{
    //
}

void MainWindow::onCmdServerData(const QByteArray &data)
{
    cmdRouter_.onData(data);
}

void MainWindow::onCommandActionClose(const QJsonObject &)
{
    QMessageBox::warning(this,
                         tr("Closing"),
                         tr("Warning, the room owner has "
                            "closed the room. This room will close"
                            " when everyone leaves.\n"
                            "Save your work if you like it!"));
}

void MainWindow::onCommandResponseClose(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        QMessageBox::critical(this,
                              tr("Sorry"),
                              tr("Sorry, it seems you're not"
                                 "room owner."));
    }else{
        // Since server accepted close request, we can
        // wait for close now.
        // of course, delete the key. it's useless.
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(roomName_.toUtf8());
        QString hashed_name = hash.result().toHex();
        QSettings settings(GlobalDef::SETTINGS_NAME,
                           QSettings::defaultFormat(),
                           qApp);
        settings.remove("rooms/"+hashed_name);
        settings.sync();
    }
}

void MainWindow::onCommandResponseClearAll(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        QMessageBox::critical(this,
                              tr("Sorry"),
                              tr("Sorry, it seems you're not"
                                 "room owner."));
    }
}

void MainWindow::onCommandActionClearAll(const QJsonObject &)
{
    ui->canvas->clearAllLayer();
}

void MainWindow::onCommandResponseOnlinelist(const QJsonObject &o)
{
    QJsonArray list = o.value("onlinelist").toArray();
    qDebug()<<list;
    MemberList l;
    for(int i=0;i<list.count();++i){
        QJsonObject obj = list[i].toObject();
        QString id = obj.value("clientid").toString();
        QString nick = obj.value("name").toString();
        QVariantList vl;
        vl.append(nick);
        l.insert(id, vl);
    }
    ui->memberList->setMemberList(l);
}

void MainWindow::onNewMessage(const QString &content)
{
    QTextCursor c = ui->textEdit->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(c);
    ui->textEdit->insertPlainText(content);
    ui->textEdit->verticalScrollBar()
            ->setValue(ui->textEdit->verticalScrollBar()
                       ->maximum());
    // BUG: looks it does nothing!?
    qApp->alert(this, 3000);
}

QByteArray MainWindow::toJson(const QVariant &m)
{
    return QJsonDocument::fromVariant(m).toJson();
}

QVariant MainWindow::fromJson(const QByteArray &d)
{
    return QJsonDocument::fromJson(d).toVariant();
}

void MainWindow::onSendPressed()
{
    QString string(ui->lineEdit->text());
    if(string.isEmpty() || string.count()>256){
        qDebug()<<"Warnning: text too long or empty.";
        return;
    }
    string.prepend(nickName_+": ");
    string.append('\n');
    emit sendMessage(string);

    QTextCursor c = ui->textEdit->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(c);
    ui->textEdit->insertPlainText(string);
    ui->textEdit->verticalScrollBar()
            ->setValue(ui->textEdit->verticalScrollBar()
                       ->maximum());
    ui->lineEdit->clear();
}

void MainWindow::onColorGridDroped(int id)
{
    QColor c = ui->colorBox->color();
    ui->colorGrid->setColor(id, c);
}

void MainWindow::onColorGridPicked(int, const QColor &c)
{
    ui->colorBox->setColor(c);
}

void MainWindow::onBrushTypeChange()
{
    ui->canvas->changeBrush(sender()->objectName());
}

void MainWindow::onBrushSettingsChanged(const QVariantMap &m)
{
    int w = m["width"].toInt();
    QVariantMap colorMap = m["color"].toMap();
    QColor c(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());

    // INFO: to prevent scaled to 1px, should always
    // change width first
    if(widthControl_){
        if(widthControl_->value() != w)
            widthControl_->setValue(w);
    }
    if(ui->colorBox->color() != c)
        ui->colorBox->setColor(c);

}

void MainWindow::onPanoramaRefresh()
{
    ui->panorama->onImageChange(ui->canvas->grab(),
                                ui->centralWidget->visualRect().toRect());
}

void MainWindow::onColorPickerPressed(bool c)
{
    ui->canvas->onColorPicker(c);
    if(brushActionGroup_){
        brushActionGroup_->setDisabled(c);
    }
}

void MainWindow::onPickColorComplete()
{
    if(brushActionGroup_){
        brushActionGroup_->setDisabled(false);
    }
    if(colorPickerButton_){
        colorPickerButton_->setChecked(false);
    }
}

void MainWindow::remoteAddLayer(const QString &layerName)
{
    if( layerName.isEmpty() ){
        return;
    }

    LayerItem *item = new LayerItem;
    item->setVisibleIcon(QIcon("iconset/visibility.png"));
    item->setLockIcon(QIcon("iconset/lock.png"));
    item->setLabel(layerName);
    ui->layerWidget->addItem(item);
}

void MainWindow::addLayer(const QString &layerName)
{
    QString name = layerName;
    if(name.isNull() || name.isEmpty())
        name = QString::number(ui->canvas->layerNum());

    LayerItem *item = new LayerItem;
    item->setVisibleIcon(QIcon("iconset/visibility.png"));
    item->setLockIcon(QIcon("iconset/lock.png"));
    item->setLabel(name);
    ui->layerWidget->addItem(item);
    ui->canvas->addLayer(name);

    // NOTICE: disable single layer clear due to lack of
    // a way to store this action in server history
    //    QAction *clearOne = new QAction(this);
    //    ui->menuClear_Canvas->insertAction(ui->actionAll_Layers,
    //                                       clearOne);
    //    clearOne->setText(tr("Layer ")+name);
    //    connect(clearOne, &QAction::triggered,
    //            [this, name, clearOne](){
    //        this->clearLayer(name);
    //    });
}

void MainWindow::deleteLayer()
{
    LayerItem * item = ui->layerWidget->selected();
    QString text = item->label();
    bool sucess = ui->canvas->deleteLayer(text);
    if(sucess) ui->layerWidget->removeItem(item);
}

void MainWindow::clearLayer(const QString &name)
{
    auto result = QMessageBox::question(this,
                                        tr("OMG"),
                                        tr("You're going to clear layer %1. "
                                           "All the work of that layer"
                                           "will be deleted and CANNOT be undone.\n"
                                           "Do you really want to do so?").arg(name),
                                        QMessageBox::Yes|QMessageBox::No);
    if(result == QMessageBox::Yes){
        ui->canvas->clearLayer(name);
        QVariantMap map;
        map.insert("request", "clear");
        map.insert("key", getRoomKey());
        map.insert("layer", name);
        CommandSocket::cmdSocket()
                ->sendData(toJson(QVariant(map)));
    }

}

void MainWindow::clearAllLayer()
{
    auto result = QMessageBox::question(this,
                                        tr("OMG"),
                                        tr("You're going to clear ALL LAYERS"
                                           ". All of work in this room"
                                           "will be deleted and CANNOT be undone.\n"
                                           "Do you really want to do so?"),
                                        QMessageBox::Yes|QMessageBox::No);
    if(result == QMessageBox::Yes){
        QVariantMap map;
        QVariant r_key = getRoomKey();
        if(r_key.isNull()){
            return;
        }
        map.insert("request", "clearall");
        map.insert("key", getRoomKey());
        CommandSocket::cmdSocket()
                ->sendData(toJson(QVariant(map)));
    }
}

void MainWindow::deleteLayer(const QString &name)
{
    bool sucess = ui->canvas->deleteLayer(name);
    if(sucess) ui->layerWidget->removeItem(name);
}

void MainWindow::closeEvent( QCloseEvent * event )
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.setValue("colorgrid/pal",
                      ui->colorGrid->dataExport());
    settings.setValue("mainwindow/view",
                      saveState());
    settings.sync();
    event->accept();
}

void MainWindow::exportAllToFile()
{
    QString fileName =
            QFileDialog::getSaveFileName(this,
                                         tr("Export all to file"),
                                         QDir::currentPath(),
                                         tr("Images (*.png)"));
    fileName = fileName.trimmed();
    if(fileName.isEmpty()){
        return;
    }
    if(!fileName.endsWith(".png", Qt::CaseInsensitive)){
        fileName = fileName + ".png";
    }
    QPixmap image = ui->canvas->allCanvas();
    image.save(fileName, "PNG");
}

void MainWindow::exportVisibleToFile()
{
    QString fileName =
            QFileDialog::getSaveFileName(this,
                                         tr("Export visible part to file"),
                                         QDir::currentPath(),
                                         tr("Images (*.png)"));
    fileName = fileName.trimmed();
    if(fileName.isEmpty()){
        return;
    }
    if(!fileName.endsWith(".png", Qt::CaseInsensitive)){
        fileName = fileName + ".png";
    }
    QPixmap image = ui->canvas->currentCanvas();
    image.save(fileName, "PNG");
}

void MainWindow::exportAllToClipboard()
{
    QClipboard *cb = qApp->clipboard();
    QPixmap image = ui->canvas->allCanvas();
    cb->setPixmap(image);
}

void MainWindow::exportVisibleToClipboard()
{
    QClipboard *cb = qApp->clipboard();
    QPixmap image = ui->canvas->currentCanvas();
    cb->setPixmap(image);
}

void MainWindow::resetView()
{
    restoreState(defaultView);
}

void MainWindow::about()
{
    AboutDialog dialog(this);
    dialog.exec();
}
