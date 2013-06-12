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
#include <QRegularExpression>
#include <QProcess>
#include <QTimer>

#include "../misc/singleshortcut.h"
#include "layerwidget.h"
#include "layeritem.h"
#include "colorgrid.h"
#include "aboutdialog.h"
#include "configuredialog.h"
#include "brushsettingswidget.h"
#include "../../common/network/commandsocket.h"
#include "../../common/network/localnetworkinterface.h"
#include "../paintingTools/brush/brushmanager.h"
#include "../../common/common.h"
#include "../misc/platformextend.h"
#include "../misc/singleton.h"
#include "../misc/shortcutmanager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    msgSocket(this),
    dataSocket(this),
    lastBrushAction(nullptr),
    brushSettingControl_(nullptr),
    toolbar_(nullptr),
    brushActionGroup_(nullptr),
    colorPickerButton_(nullptr)
{
    ui->setupUi(this);
//    auto&& roomName = Singleton<CommandSocket>::instance().roomName();
//    setWindowTitle(roomName+tr(" - Mr.Paint"));
//    ui->canvas->resize(Singleton<CommandSocket>::instance().canvasSize());
    defaultView = saveState();
    init();
}

MainWindow::~MainWindow()
{
    msgSocket.close();
    dataSocket.close();
    Singleton<CommandSocket>::instance().close();
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
    auto&& roomName = Singleton<CommandSocket>::instance().roomName();
    setWindowTitle(roomName+tr(" - Mr.Paint"));
    ui->canvas->resize(Singleton<CommandSocket>::instance().canvasSize());

    ui->centralWidget->setBackgroundRole(QPalette::Dark);
    ui->centralWidget->setCanvas(ui->canvas);
    ui->canvas->setDisabled(true);
    ui->layerWidget->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    ui->pushButton->setDisabled(true);

    connect(ui->panorama, &PanoramaWidget::scaled,
            ui->centralWidget, &CanvasContainer::setScaleFactor);
    connect(ui->centralWidget, &CanvasContainer::scaled,
            ui->panorama, &PanoramaWidget::setScaled);

    connect(ui->lineEdit,&QLineEdit::returnPressed,
            this,&MainWindow::onSendPressed);
    connect(ui->pushButton,&QPushButton::clicked,
            this,&MainWindow::onSendPressed);

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
    connect(ui->panorama, &PanoramaWidget::moveTo,
            ui->centralWidget,
            static_cast<void (CanvasContainer::*)(const QPointF&)>
            (&CanvasContainer::centerOn));
    connect(ui->centralWidget, &CanvasContainer::tabletDetected,
            ui->canvas, &Canvas::foundTablet);

    layerWidgetInit();
    colorGridInit();
    toolbarInit();
    viewInit();
    shortcutInit();
    //    stylize();
    socketInit();

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
    cmdRouter_.regHandler("response",
                          "checkout",
                          std::bind(&MainWindow::onCommandResponseCheckout,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("action",
                          "notify",
                          std::bind(&MainWindow::onActionNotify,
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
    toolbar_ = new QToolBar(tr("Brushes"), this);
    toolbar_->setObjectName("BrushToolbar");
    this->addToolBar(Qt::TopToolBarArea, toolbar_);
    brushActionGroup_ = new QActionGroup(this);

    // always remember last action
    auto restoreAction =  [this](){
        if(lastBrushAction){
            lastBrushAction->trigger();
        }
    };

    auto brushes = Singleton<BrushManager>::instance().allBrushes();

    for(auto &item: brushes){
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
                    tr("%1\n"
                       "Shortcut: %2")
                    .arg(item->displayName())
                    .arg(shortcut->key()
                         .toString()));
        if(toolbar_->actions().count() < 2){
            action->trigger();
        }
    }


    // doing hacking to color picker
    QIcon colorpickerIcon;
    colorpickerIcon.addFile(":/iconset/ui/picker-1.png",
                            QSize(), QIcon::Disabled);
    colorpickerIcon.addFile(":/iconset/ui/picker-2.png",
                            QSize(), QIcon::Active);
    colorpickerIcon.addFile(":/iconset/ui/picker-3.png",
                            QSize(), QIcon::Selected);
    colorpickerIcon.addFile(":/iconset/ui/picker-3.png",
                            QSize(), QIcon::Normal, QIcon::On);
    colorpickerIcon.addFile(":/iconset/ui/picker-4.png",
                            QSize(), QIcon::Normal);
    QAction *colorpicker = toolbar_->addAction(colorpickerIcon,
                                               tr("Color Picker"));
    colorpicker->setCheckable(true);
    colorpicker->setAutoRepeat(false);
    // we need the real QToolButton to know weather the picker is
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
                        tr("%1\n"
                           "Shortcut: %2")
                        .arg(colorpicker->text())
                        .arg(pickerShortcut->key()
                             .toString()));
        }
    }

    // for brush width
    QToolBar *brushSettingToolbar = new QToolBar(tr("Brush Settings"), this);
    brushSettingToolbar->setObjectName("BrushSettingToolbar");
    this->addToolBar(Qt::TopToolBarArea, brushSettingToolbar);
    BrushSettingsWidget * brushSettingWidget = new BrushSettingsWidget(this);
    connect(brushSettingWidget, &BrushSettingsWidget::widthChanged,
            ui->canvas, &Canvas::setBrushWidth);
    connect(brushSettingWidget, &BrushSettingsWidget::hardnessChanged,
            ui->canvas, &Canvas::setBrushHardness);
    connect(brushSettingToolbar, &QToolBar::orientationChanged,
            brushSettingWidget, &BrushSettingsWidget::setOrientation);

    // shortcuts for width control
    ShortcutManager &stctmgr = Singleton<ShortcutManager>::instance();
    QShortcut* widthActionSub = new QShortcut(this);
    widthActionSub->setKey(stctmgr.shortcut("subwidth")["key"].toString());
    connect(widthActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::widthDown);
    QShortcut* widthActionAdd = new QShortcut(this);
    widthActionAdd->setKey(stctmgr.shortcut("addwidth")["key"].toString());
    connect(widthActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::widthUp);
    // shortcuts for hardness control
    QShortcut* hardnessActionSub = new QShortcut(this);
    hardnessActionSub->setKey(stctmgr.shortcut("subhardness")["key"].toString());
    connect(hardnessActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::hardnessDown);
    QShortcut* hardnessActionAdd = new QShortcut(this);
    hardnessActionAdd->setKey(stctmgr.shortcut("addhardness")["key"].toString());
    connect(hardnessActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::hardnessUp);

    brushSettingControl_ = brushSettingWidget;
    brushSettingToolbar->addWidget(brushSettingWidget);
}

QVariant MainWindow::getRoomKey()
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    auto&& roomName = Singleton<CommandSocket>::instance().roomName();
    hash.addData(roomName.toUtf8());
    QString hashed_name = hash.result().toHex();
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.sync();
    if( !settings.contains("rooms/"+hashed_name) ){
        // Tell user that he's not owner
        qDebug()<<"hashed_name"<<hashed_name
               <<" key cannot found!";
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
    obj.insert("clientid", Singleton<CommandSocket>::instance().clientId());
    doc.setObject(obj);
    qDebug()<<"clientid: "<<Singleton<CommandSocket>::instance().clientId();
    Singleton<CommandSocket>::instance().sendData(doc.toJson());
}

void MainWindow::requestCheckout()
{
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("request", QString("checkout"));
    obj.insert("key", getRoomKey().toString());
    doc.setObject(obj);
    qDebug()<<"checkout with key: "<<getRoomKey();
    Singleton<CommandSocket>::instance().sendData(doc.toJson());
}

void MainWindow::shortcutInit()
{
    connect(ui->action_Quit, &QAction::triggered,
            this, &MainWindow::close);
    connect(ui->actionExport_All, &QAction::triggered,
            this, &MainWindow::exportAllToFile);
    connect(ui->actionExport_Visiable, &QAction::triggered,
            this, &MainWindow::exportVisibleToFile);
    connect(ui->actionExport_All_To_Clipboard, &QAction::triggered,
            this, &MainWindow::exportAllToClipboard);
    connect(ui->actionExport_Visible_To_ClipBorad, &QAction::triggered,
            this, &MainWindow::exportVisibleToClipboard);
    connect(ui->actionReset_View, &QAction::triggered,
            this, &MainWindow::resetView);
    connect(ui->action_About_Mr_Paint, &QAction::triggered,
            this, &MainWindow::about);
    connect(ui->actionAbout_Qt, &QAction::triggered,
            &QApplication::aboutQt);
    connect(ui->actionClose_Room, &QAction::triggered,
            [&](){
        QVariantMap map;
        QVariant r_key = getRoomKey();
        map.insert("request", "close");
        if(r_key.isNull()){
            QMessageBox::warning(this,
                                 tr("Sorry"),
                                 tr("Only room owner is authorized "
                                    "to close the room.\n"
                                    "It seems you're not room manager."));
            return;
        }
        map.insert("key", r_key);
        Singleton<CommandSocket>::instance().sendData(toJson(QVariant(map)));
    });
    connect(ui->actionAll_Layers, &QAction::triggered,
            this, &MainWindow::clearAllLayer);
    connect(ui->actionConfiguration, &QAction::triggered,
            [this](){
        ConfigureDialog conf_dialog;
        conf_dialog.exec();
    });
}

void MainWindow::socketInit()
{
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

    connect(&Singleton<CommandSocket>::instance(), &CommandSocket::newData,
            this, &MainWindow::onCmdServerData);
    cmdSocketRouterInit();
    // checkout if client is room owner
    if(!getRoomKey().isNull()){
        requestCheckout();
    }

    ui->textEdit->insertPlainText(tr("Connecting to server...\n"));
    QHostAddress addr;
    if(LocalNetworkInterface::supportIpv6()){
        addr = GlobalDef::HOST_ADDR[1];
        qDebug()<<"using ipv6 address to connect server";
    }else{
        addr = GlobalDef::HOST_ADDR[0];
        qDebug()<<"using ipv4 address to connect server";
    }
    auto& cmdInstance = Singleton<CommandSocket>::instance();
    msgSocket.connectToHost(addr,
                            cmdInstance.msgPort());
    dataSocket.connectToHost(addr,
                             cmdInstance.dataPort());
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
        auto&& roomName = Singleton<CommandSocket>::instance().roomName();
        hash.addData(roomName.toUtf8());
        QString hashed_name = hash.result().toHex();
        QSettings settings(GlobalDef::SETTINGS_NAME,
                           QSettings::defaultFormat(),
                           qApp);
        settings.remove("rooms/"+hashed_name);
        settings.sync();
        ui->statusBar->showMessage(tr("Close Request Completed."),
                                   5000);
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
    ui->statusBar->showMessage(tr("Clear Request Completed."),
                               5000);
}

void MainWindow::onCommandResponseCheckout(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        //        QMessageBox::critical(this,
        //                              tr("Sorry"),
        //                              tr("Sorry, it seems you're not"
        //                                 "room owner."));
    }else{
        int hours = m["cycle"].toDouble();
        if(hours){
            // prepare next checkout
            // this rarely happens, but still need
            QTimer * checkoutTimer = new QTimer(this);
            checkoutTimer->setSingleShot(true);
            hours--;
            checkoutTimer->setInterval(hours * 3600*1000);
            connect(checkoutTimer, &QTimer::timeout,
                    this, &MainWindow::requestCheckout);
        }
        ui->statusBar->showMessage(tr("Checkout Completed."),
                                   5000);
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
    ui->statusBar->showMessage(tr("Online List Refreshed."),
                               2000);
}

void MainWindow::onActionNotify(const QJsonObject &o)
{
    QString content = o.value("content").toString();
    if(content.isEmpty()){
        return;
    }

    QTextCursor c = ui->textEdit->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(c);
    ui->textEdit->insertHtml(content);
    ui->textEdit->verticalScrollBar()
            ->setValue(ui->textEdit->verticalScrollBar()
                       ->maximum());
    ui->textEdit->insertPlainText("\n");

    qDebug()<<"notified with: "<<o;
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
    qDebug()<<"new message with: "<<content;

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool msg_notify = settings.value("chat/msg_notify").toBool();
    if(!this->isActiveWindow() && msg_notify)
        PlatformExtend::notify(this);
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
    string.prepend(Singleton<CommandSocket>::instance().userName()
                   + ": ");
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
    int h = m["hardness"].toInt();
    QVariantMap colorMap = m["color"].toMap();
    QColor c(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());

    // INFO: to prevent scaled to 1px, should always
    // change width first
    if(brushSettingControl_){
        if(brushSettingControl_->width() != w)
            brushSettingControl_->setWidth(w);
    }
    if(brushSettingControl_){
        if(brushSettingControl_->hardness() != h)
            brushSettingControl_->setHardness(h);
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
    item->setVisibleIcon(QIcon(":/iconset/visibility-on.png"));
    item->setLockIcon(QIcon(":/iconset/lock.png"));
    item->setLabel(layerName);
    ui->layerWidget->addItem(item);
}

void MainWindow::addLayer(const QString &layerName)
{
    QString name = layerName;
    if(name.isNull() || name.isEmpty())
        name = QString::number(ui->canvas->layerNum());

    LayerItem *item = new LayerItem;
    item->setVisibleIcon(QIcon(":/iconset/visibility-on.png"));
    item->setLockIcon(QIcon(":/iconset/lock.png"));
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
        Singleton<CommandSocket>::instance().sendData(toJson(QVariant(map)));
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
            QMessageBox::warning(this,
                                 tr("Sorry"),
                                 tr("Only room owner is authorized "
                                    "to clear the canvas.\n"
                                    "It seems you're not room manager."));
            return;
        }
        map.insert("request", "clearall");
        map.insert("key", getRoomKey());
        Singleton<CommandSocket>::instance().sendData(toJson(QVariant(map)));
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
