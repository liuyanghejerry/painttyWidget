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
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QCryptographicHash>
#include <QHostAddress>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QProcess>
#include <QTimer>
#include <QScriptEngine>

#include "../misc/singleshortcut.h"
#include "layerwidget.h"
#include "layeritem.h"
#include "colorgrid.h"
#include "aboutdialog.h"
#include "configuredialog.h"
#include "brushsettingswidget.h"
#include "gradualbox.h"
#include "roomsharebar.h"
#include "developerconsole.h"
#include "../../common/network/clientsocket.h"
#include "../../common/network/localnetworkinterface.h"
#include "../paintingTools/brush/brushmanager.h"
#include "../../common/common.h"
#include "../misc/platformextend.h"
#include "../misc/singleton.h"
#include "../misc/shortcutmanager.h"
#include "../misc/errortable.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastBrushAction(nullptr),
    brushSettingControl_(nullptr),
    toolbar_(nullptr),
    brushActionGroup_(nullptr),
    colorPickerButton_(nullptr),
    scriptEngine_(nullptr),
    console_(nullptr)
{
    ui->setupUi(this);
    defaultView = saveState();
    init();
}

MainWindow::~MainWindow()
{
    Singleton<ClientSocket>::instance().reset();
    Singleton<ClientSocket>::instance().close();
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
    auto& client_socket = Singleton<ClientSocket>::instance();
    auto&& roomName = client_socket.roomName();
    setWindowTitle(roomName+tr(" - Mr.Paint"));
    ui->canvas->resize(client_socket.canvasSize());

    ui->centralWidget->setBackgroundRole(QPalette::Dark);
    ui->centralWidget->setCanvas(ui->canvas);

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

    layerWidgetInit();
    colorGridInit();
    toolbarInit();
    viewInit();
    shortcutInit();
    //    stylize();
    socketInit();
    scriptInit();

}

void MainWindow::cmdRouterInit()
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
    cmdRouter_.regHandler("response",
                          "archivesign",
                          std::bind(&MainWindow::onResponseArchiveSign,
                                    this,
                                    std::placeholders::_1));
    cmdRouter_.regHandler("response",
                          "archive",
                          std::bind(&MainWindow::onResponseArchive,
                                    this,
                                    std::placeholders::_1));
}

void MainWindow::scriptInit()
{
    scriptEngine_ = new QScriptEngine;

    QScriptValue scriptMainWindow = scriptEngine_->newQObject(this);
    scriptEngine_->globalObject().setProperty("mainwindow", scriptMainWindow);

    QScriptValue scriptCanvas = scriptEngine_->newQObject(this->ui->canvas);
    scriptEngine_->globalObject().setProperty("canvas", scriptCanvas);

    QScriptValue scriptClientSocket = scriptEngine_->newQObject(&Singleton<ClientSocket>::instance());
    scriptEngine_->globalObject().setProperty("clientsocket", scriptClientSocket);
}

void MainWindow::layerWidgetInit()
{
    for(int i=0;i<10;++i){
        addLayer();
    }
    ui->layerWidget->itemAt(0)->setSelect(true);
    ui->canvas->loadLayers();
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

            auto colorpicker_key = Singleton<ShortcutManager>::instance()
                    .shortcut("colorpicker")["key"].toString();
            SingleShortcut *pickerShortcut = new SingleShortcut(this);
            pickerShortcut->setKey(colorpicker_key);
            connect(pickerShortcut, &SingleShortcut::activated,
                    b, &QToolButton::click);
            connect(pickerShortcut, &SingleShortcut::inactivated,
                    b, &QToolButton::click);
            colorpicker->setToolTip(
                        tr("%1\n"
                           "Shortcut: %2")
                        .arg(colorpicker->text())
                        .arg(colorpicker_key));
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
    connect(brushSettingWidget, &BrushSettingsWidget::thicknessChanged,
            ui->canvas, &Canvas::setBrushThickness);
    connect(brushSettingWidget, &BrushSettingsWidget::waterChanged,
            ui->canvas, &Canvas::setBrushWater);
    connect(brushSettingWidget, &BrushSettingsWidget::extendChanged,
            ui->canvas, &Canvas::setBrushExtend);
    connect(brushSettingWidget, &BrushSettingsWidget::mixinChanged,
            ui->canvas, &Canvas::setBrushMixin);
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
    // shortcuts for thickness control
    QShortcut* thicknessActionSub = new QShortcut(this);
    thicknessActionSub->setKey(stctmgr.shortcut("subthickness")["key"].toString());
    connect(thicknessActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::thicknessDown);
    QShortcut* thicknessActionAdd = new QShortcut(this);
    thicknessActionAdd->setKey(stctmgr.shortcut("addthickness")["key"].toString());
    connect(thicknessActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::thicknessUp);
    // shortcuts for water control
    QShortcut* waterActionSub = new QShortcut(this);
    waterActionSub->setKey(stctmgr.shortcut("subwater")["key"].toString());
    connect(waterActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::waterDown);
    QShortcut* waterActionAdd = new QShortcut(this);
    waterActionAdd->setKey(stctmgr.shortcut("addwater")["key"].toString());
    connect(waterActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::waterUp);
    // shortcuts for extend control
    QShortcut* extendActionSub = new QShortcut(this);
    extendActionSub->setKey(stctmgr.shortcut("subextend")["key"].toString());
    connect(extendActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::extendDown);
    QShortcut* extendActionAdd = new QShortcut(this);
    extendActionAdd->setKey(stctmgr.shortcut("addextend")["key"].toString());
    connect(extendActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::extendUp);
    // shortcuts for mixin control
    QShortcut* mixinActionSub = new QShortcut(this);
    mixinActionSub->setKey(stctmgr.shortcut("submixin")["key"].toString());
    connect(mixinActionSub, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::mixinDown);
    QShortcut* mixinActionAdd = new QShortcut(this);
    mixinActionAdd->setKey(stctmgr.shortcut("addmixin")["key"].toString());
    connect(mixinActionAdd, &QShortcut::activated,
            brushSettingWidget, &BrushSettingsWidget::mixinUp);

    brushSettingControl_ = brushSettingWidget;
    brushSettingToolbar->addWidget(brushSettingWidget);

    changeToBrush("BasicBrush");

    // for room share
    auto& client_socket = Singleton<ClientSocket>::instance();
    QToolBar* roomShareToolbar = new QToolBar(tr("Room Share"), this);
    roomShareToolbar->setObjectName("RoomShareToolbar");
    this->addToolBar(Qt::TopToolBarArea, roomShareToolbar);
    RoomShareBar* rsb = new RoomShareBar(this);
    rsb->setAddress(client_socket.toUrl());
    roomShareToolbar->addWidget(rsb);

    if(client_socket.isIPv6Address()){
        auto f = [this](){
            GradualBox::showText(tr("Notice, we detected you're using IPv6 protocol"\
                                    " which may result in that your Room URL is not available"\
                                    " for IPv4 users."));
        };

        GlobalDef::delayJob(f, 5000);
    }
}

QString MainWindow::getRoomKey()
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    auto&& roomName = Singleton<ClientSocket>::instance().roomName();
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
        return QString();
    }
    QVariant key = settings.value("rooms/"+hashed_name);
    return key.toString();
}

void MainWindow::requestOnlinelist()
{
    QJsonObject obj;
    obj.insert("request", QString("onlinelist"));
    obj.insert("type", QString("command"));
    obj.insert("clientid", Singleton<ClientSocket>::instance().clientId());
//    qDebug()<<"clientid: "<<Singleton<ClientSocket>::instance().clientId();

    Singleton<ClientSocket>::instance().sendCmdPack(obj);
}

void MainWindow::requestCheckout()
{
    QJsonObject obj;
    obj.insert("request", QString("checkout"));
    obj.insert("type", QString("command"));
    obj.insert("key", getRoomKey());
    qDebug()<<"checkout with key: "<<getRoomKey();

    Singleton<ClientSocket>::instance().sendCmdPack(obj);
}

void MainWindow::requestArchiveSign()
{
    QJsonObject obj;
    obj.insert("request", QString("archivesign"));
    qDebug()<<"request archive signature";

    Singleton<ClientSocket>::instance().sendCmdPack(obj);
}

void MainWindow::requestArchive()
{
    auto& socket = Singleton<ClientSocket>::instance();
    QJsonObject obj;
    obj.insert("request", QString("archive"));
    obj.insert("start", (int)socket.archiveSize());
    qDebug()<<"request archive"<<obj;

    socket.sendCmdPack(obj);
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
//    connect(ui->actionExport_to_PSD, &QAction::triggered,
//            this, &MainWindow::exportToPSD);
    connect(ui->actionClose_Room, &QAction::triggered,
            [&](){
        QJsonObject obj;
        QString r_key = getRoomKey();
        obj.insert("request", QString("close"));
        if(r_key.isEmpty()){
            QMessageBox::warning(this,
                                 tr("Sorry"),
                                 tr("Only room owner is authorized "
                                    "to close the room.\n"
                                    "It seems you're not room manager."));
            return;
        }
        obj.insert("key", r_key);
        Singleton<ClientSocket>::instance().sendCmdPack(obj);
    });
    connect(ui->actionAll_Layers, &QAction::triggered,
            this, &MainWindow::clearAllLayer);
    connect(ui->actionConfiguration, &QAction::triggered,
            [this](){
        ConfigureDialog conf_dialog;
        conf_dialog.exec();
    });

    QShortcut* console_shortcut = new QShortcut(QKeySequence("F12"), this);
    connect(console_shortcut, &QShortcut::activated,
            this, &MainWindow::openConsole);

    auto& sm = Singleton<ShortcutManager>::instance();
    {
        QShortcut* zoomin_shortcut = new QShortcut(QKeySequence(sm.shortcut("zoomin")["key"].toString()), this);
        connect(zoomin_shortcut, &QShortcut::activated,
                [this](){
            // TODO: should be configurable
            this->ui->centralWidget->scaleBy(1.2);
        });
    }
    {
        QShortcut* zoomout_shortcut = new QShortcut(QKeySequence(sm.shortcut("zoomout")["key"].toString()), this);
        connect(zoomout_shortcut, &QShortcut::activated,
                [this](){
            // TODO: should be configurable
            this->ui->centralWidget->scaleBy(0.8);
        });
    }
}

void MainWindow::socketInit()
{
    auto& client_socket = Singleton<ClientSocket>::instance();

    connect(&client_socket, &ClientSocket::newMessage,
            this, &MainWindow::onNewMessage);
    connect(this, &MainWindow::sendMessage,
            &client_socket, &ClientSocket::sendMessage);

    connect(&client_socket, &ClientSocket::cmdPack,
            this, &MainWindow::onCmdData);
    connect(&client_socket, &ClientSocket::disconnected,
            this, &MainWindow::onServerDisconnected);
    cmdRouterInit();
    auto fff = [this](){
        Singleton<ClientSocket>::instance().setPoolEnabled(false);
        requestArchiveSign();

        // checkout if client is room owner
        if(!getRoomKey().isNull()){
            requestCheckout();
        }
    };
    GlobalDef::delayJob(fff);
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout,
            this, &MainWindow::requestOnlinelist);
    t->start(5000);
}

void MainWindow::onServerDisconnected()
{
    GradualBox::showText(tr("Server Connection Failed."));
    ui->canvas->setEnabled(false);
    // TODO: reconnect to room and request login
}

void MainWindow::onCmdData(const QJsonObject &data)
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
    Singleton<ClientSocket>::instance().setRoomCloseFlag();
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
        auto&& roomName = Singleton<ClientSocket>::instance().roomName();
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

void MainWindow::onCommandActionClearAll(const QJsonObject &obj)
{
    qDebug()<<"on action clearall"<<obj;
    if(obj.contains("signature")){
        auto&& s = obj.value("signature").toString();
        Singleton<ClientSocket>::instance().setArchiveSignature(s);
    }
    ui->canvas->clearAllLayer();
}

void MainWindow::onCommandResponseOnlinelist(const QJsonObject &o)
{
    QJsonArray list = o.value("onlinelist").toArray();
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

void MainWindow::onResponseArchiveSign(const QJsonObject &o)
{
    if(!o.contains("result") || !o.contains("signature")){
        return;
    }
    int errcode = 800;
    if(!o.value("result").toBool()){
        errcode = o.value("errcode").toDouble();
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Sorry, an error occurred.\n"
                                 "Error: %1, %2").arg(errcode)
                              .arg(ErrorTable::toString(errcode)));
        return;
    }

    QString signature = o.value("signature").toString();

    auto& socket = Singleton<ClientSocket>::instance();
    socket.setArchiveSignature(signature);
    requestArchive();
}

void MainWindow::onResponseArchive(const QJsonObject &o)
{
    if(!o.contains("result")|| !o.contains("datalength")){
        return;
    }
    int errcode = 900;
    if(!o.value("result").toBool()){
        errcode = o.value("errcode").toDouble();
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Sorry, an error occurred.\n"
                                 "Error: %1, %2").arg(errcode)
                              .arg(ErrorTable::toString(errcode)));
        return;
    }

    quint64 datalength = o.value("datalength").toDouble();

    // TODO: re-match signature and receive archive data
    auto& socket = Singleton<ClientSocket>::instance();
    socket.setSchedualDataLength(datalength);
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

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool msg_notify = settings.value("chat/msg_notify").toBool();
    if(!this->isActiveWindow() && msg_notify)
        PlatformExtend::notify(this);
}

void MainWindow::onSendPressed()
{
    QString string(ui->lineEdit->text());
    if(string.isEmpty() || string.count()>256){
        qDebug()<<"Warnning: text too long or empty.";
        return;
    }
    string.prepend(Singleton<ClientSocket>::instance().userName()
                   + ": ");
    string.append('\n');
    emit sendMessage(string);
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
    changeToBrush(sender()->objectName());
}

void MainWindow::onBrushSettingsChanged(const QVariantMap &m)
{
    int width = m["width"].toInt();
    int hardness = m["hardness"].toInt();
    int thickness = m["thickness"].toInt();
    int water = m["water"].toInt();
    int extend = m["extend"].toInt();
    int mixin = m["mixin"].toInt();
    QVariantMap colorMap = m["color"].toMap();
    QColor c(colorMap["red"].toInt(),
            colorMap["green"].toInt(),
            colorMap["blue"].toInt());

    // INFO: to prevent scaled to 1px, should always
    // change width first
    if(brushSettingControl_){
        if(brushSettingControl_->width() != width)
            brushSettingControl_->setWidth(width);
        if(brushSettingControl_->hardness() != hardness)
            brushSettingControl_->setHardness(hardness);
        if(brushSettingControl_->thickness() != thickness)
            brushSettingControl_->setThickness(thickness);
        if(brushSettingControl_->water() != water)
            brushSettingControl_->setWater(water);
        if(brushSettingControl_->extend() != extend)
            brushSettingControl_->setExtend(extend);
        if(brushSettingControl_->mixin() != mixin)
            brushSettingControl_->setMixin(mixin);
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

void MainWindow::openConsole()
{
    if(!console_){
        console_ = new DeveloperConsole(this);
        connect(this, &MainWindow::scriptResult,
                console_, &DeveloperConsole::append);
        connect(console_, &DeveloperConsole::evaluate,
                this, &MainWindow::evaluateScript);
    }
    console_->show();
}

void MainWindow::changeToBrush(const QString &brushName)
{
    ui->canvas->changeBrush(brushName);
    auto f = ui->canvas->brushFeatures();
    if(!this->brushSettingControl_){
        return;
    }
    this->brushSettingControl_->setHardnessEnabled(f.support(BrushFeature::HARDNESS));
    this->brushSettingControl_->setThicknessEnabled(f.support(BrushFeature::THICKNESS));
    this->brushSettingControl_->setWaterEnabled(f.support(BrushFeature::WATER));
    this->brushSettingControl_->setExtendEnabled(f.support(BrushFeature::EXTEND));
    this->brushSettingControl_->setMixinEnabled(f.support(BrushFeature::MIXIN));
    onBrushSettingsChanged(ui->canvas->brushSettings());
}

void MainWindow::remoteAddLayer(const QString &layerName)
{
    if( layerName.isEmpty() ){
        return;
    }

    LayerItem *item = new LayerItem;
    QIcon visibility(":/iconset/ui/visibility-on.png");
    visibility.addFile(":/iconset/ui/visibility-off.png",
                       QSize(),
                       QIcon::Selected,
                       QIcon::On);
    item->setVisibleIcon(visibility);
    QIcon lock(":/iconset/ui/lock.png");
    lock.addFile(":/iconset/ui/unlock.png",
                 QSize(),
                 QIcon::Selected,
                 QIcon::On);
    item->setLockIcon(lock);
    item->setLabel(layerName);
    ui->layerWidget->addItem(item);
}

void MainWindow::addLayer(const QString &layerName)
{
    QString name = layerName;
    if(name.isNull() || name.isEmpty())
        name = QString::number(ui->canvas->layerNum());

    LayerItem *item = new LayerItem;
    QIcon visibility(":/iconset/ui/visibility-on.png");
    visibility.addFile(":/iconset/ui/visibility-off.png",
                       QSize(),
                       QIcon::Selected,
                       QIcon::On);
    item->setVisibleIcon(visibility);
    QIcon lock(":/iconset/ui/lock.png");
    lock.addFile(":/iconset/ui/unlock.png",
                 QSize(),
                 QIcon::Selected,
                 QIcon::On);
    item->setLockIcon(lock);
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
        QJsonObject map;
        map.insert("request", QString("clear"));
        map.insert("type", QString("command"));
        map.insert("key", getRoomKey());
        map.insert("layer", name);
        Singleton<ClientSocket>::instance().sendCmdPack(map);
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
        QJsonObject map;
        QString r_key = getRoomKey();
        if(r_key.isEmpty()){
            QMessageBox::warning(this,
                                 tr("Sorry"),
                                 tr("Only room owner is authorized "
                                    "to clear the canvas.\n"
                                    "It seems you're not room manager."));
            return;
        }
        map.insert("request", QString("clearall"));
        map.insert("type", QString("command"));
        map.insert("key", getRoomKey());
        Singleton<ClientSocket>::instance().sendCmdPack(map);
    }
}

void MainWindow::evaluateScript(const QString &script)
{
    if(!scriptEngine_){
        qWarning()<<"Cannot evaluate script before script engine init!";
        return;
    }

    // pause event process
    if(scriptEngine_->processEventsInterval() > 0){
        scriptEngine_->setProcessEventsInterval(-1);
    }

    emit scriptResult(scriptEngine_->evaluate(script).toString());
}

void MainWindow::runScript(const QString &script)
{
    if(!scriptEngine_){
        qWarning()<<"Cannot run script before script engine init!";
        return;
    }
    scriptEngine_->setProcessEventsInterval(300);
    emit scriptResult(scriptEngine_->evaluate(script).toString());
}

void MainWindow::deleteLayer(const QString &name)
{
    bool sucess = ui->canvas->deleteLayer(name);
    if(sucess) ui->layerWidget->removeItem(name);
}

void MainWindow::closeEvent( QCloseEvent * event )
{
    auto& client_socket = Singleton<ClientSocket>::instance();

    client_socket.cancelPendings();
    ui->canvas->pause();

    QMessageBox msgBox;
    msgBox.setText(tr("Waiting for sync, please do not close.\n"\
                   "This will cost you 1 minute at most."));
    msgBox.setStandardButtons(QMessageBox::NoButton);
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setModal(true);
    msgBox.setWindowFlags(Qt::WindowTitleHint
                          | Qt::CustomizeWindowHint);
    msgBox.show();
    // This is a workaround to make msgBox text shown
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.setValue("colorgrid/pal",
                      ui->colorGrid->dataExport());
    settings.setValue("mainwindow/view",
                      saveState());
    bool skip_replay = settings.value("canvas/skip_replay", false).toBool();
    if(skip_replay){
        qDebug()<<"skip replay detected, save layers";
        ui->canvas->saveLayers();
    }
    settings.sync();

    disconnect(&client_socket, &ClientSocket::disconnected,
            this, &MainWindow::onServerDisconnected);

    client_socket.close();
    client_socket.reset();
    msgBox.close();

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
    QImage image = ui->canvas->allCanvas();
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
    QImage image = ui->canvas->currentCanvas();
    image.save(fileName, "PNG");
}

// TODO
void MainWindow::exportToPSD()
{
    QString fileName =
            QFileDialog::getSaveFileName(this,
                                         tr("Export contents to psd file"),
                                         QDir::currentPath(),
                                         tr("Photoshop Images (*.psd)"));
    fileName = fileName.trimmed();
    if(fileName.isEmpty()){
        return;
    }
    if(!fileName.endsWith(".psd", Qt::CaseInsensitive)){
        fileName = fileName + ".psd";
    }
    //QImage image = ui->canvas->currentCanvas();

}

void MainWindow::exportAllToClipboard()
{
    QClipboard *cb = qApp->clipboard();
    QImage image = ui->canvas->allCanvas();
    cb->setImage(image);
}

void MainWindow::exportVisibleToClipboard()
{
    QClipboard *cb = qApp->clipboard();
    QImage image = ui->canvas->currentCanvas();
    cb->setImage(image);
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
