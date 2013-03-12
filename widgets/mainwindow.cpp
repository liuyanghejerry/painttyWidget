#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QSize& canvasSize, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    msgSocket(this),
    dataSocket(this),
    historySize_(0),
    canvasSize_(canvasSize),
    lastBrushButton(nullptr)
{
    ui->setupUi(this);
    ui->canvas->resize(canvasSize_);
    defaultView = saveState();
}

MainWindow::~MainWindow()
{
    msgSocket.close();
    dataSocket.close();
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
    ui->colorPicker->setDisabled(true);

    connect(ui->spinBox,SIGNAL(valueChanged(int)),
            ui->canvas,SLOT(setBrushWidth(int)));
    connect(ui->lineEdit,SIGNAL(returnPressed()),
            this,SLOT(onSendPressed()));
    connect(ui->pushButton,SIGNAL(clicked()),
            this,SLOT(onSendPressed()));

    connect(&msgSocket,SIGNAL(connected()),
            this,SLOT(onServerConnected()));
    connect(&msgSocket,SIGNAL(disconnected()),
            this,SLOT(onServerDisconnected()));
    connect(&msgSocket,SIGNAL(newMessage(QString)),
            this,SLOT(onNewMessage(const QString &)));
    connect(this,SIGNAL(sendMessage(QString)),
            &msgSocket,SLOT(sendMessage(QString)));
    connect(&dataSocket,SIGNAL(connected()),
            this,SLOT(onServerConnected()));
    connect(&dataSocket,SIGNAL(newData(QByteArray)),
            ui->canvas,SLOT(onNewData(const QByteArray&)));
    connect(ui->canvas,SIGNAL(sendData(const QByteArray &)),
            &dataSocket,SLOT(sendData(QByteArray)));
    connect(&dataSocket,SIGNAL(disconnected()),
            this,SLOT(onServerDisconnected()));

    connect(ui->canvas, SIGNAL(newBrushSettings(QVariantMap)),
            this, SLOT(onBrushSettingsChanged(QVariantMap)));

    connect(ui->layerWidget,SIGNAL(itemHide(QString)),
            ui->canvas, SLOT(hideLayer(QString)));
    connect(ui->layerWidget,SIGNAL(itemShow(QString)),
            ui->canvas, SLOT(showLayer(QString)));
    connect(ui->layerWidget,SIGNAL(itemLock(QString)),
            ui->canvas, SLOT(lockLayer(QString)));
    connect(ui->layerWidget,SIGNAL(itemUnlock(QString)),
            ui->canvas, SLOT(unlockLayer(QString)));
    connect(ui->layerWidget,SIGNAL(itemSelected(QString)),
            ui->canvas, SLOT(layerSelected(QString)));

    connect(ui->brushButton, SIGNAL(clicked()),
            this, SLOT(onBrushTypeChange()));
    connect(ui->pencilButton, SIGNAL(clicked()),
            this, SLOT(onBrushTypeChange()));
    connect(ui->sketchButton, SIGNAL(clicked()),
            this, SLOT(onBrushTypeChange()));
    connect(ui->eraserButton, SIGNAL(clicked()),
            this, SLOT(onBrushTypeChange()));

    connect(ui->colorPicker, SIGNAL(clicked(bool)),
            this, SLOT(onColorPickerPressed(bool)));
    connect(ui->colorBox, SIGNAL(colorChanged(QColor)),
            this, SIGNAL(brushColorChange(QColor)));
    connect(this, SIGNAL(brushColorChange(QColor)),
            ui->canvas, SLOT(setBrushColor(QColor)));
    connect(ui->canvas, SIGNAL(pickColorComplete()),
            this, SLOT(onPickColorComplete()));

    connect(ui->colorGrid, SIGNAL(colorDroped(int)),
            this, SLOT(onColorGridDroped(int)));
    connect(ui->colorGrid, SIGNAL(colorPicked(int,QColor)),
            this, SLOT(onColorGridPicked(int,QColor)));
    connect(ui->panorama, SIGNAL(refresh()),
            this, SLOT(onPanoramaRefresh()));
    connect(ui->centralWidget, SIGNAL(rectChanged(QRect)),
            ui->panorama, SLOT(onRectChange(QRect)));
    // use lambda to avoid that long static_cast :)
    connect(ui->panorama, &PanoramaWidget::moveTo,
            [&](const QPointF &p){
        ui->centralWidget->centerOn(p);
    });

    layerWidgetInit();
    colorGridInit();
    viewInit();
    // TODO: change below ugly way to load first brush
    ui->pencilButton->click();

    DeveloperConsole *console = new DeveloperConsole(this);

    devConsoleShortCut = new QShortcut(this);
    devConsoleShortCut->setKey(QString("F12"));
    connect(devConsoleShortCut, SIGNAL(activated()),
            console, SLOT(show()));

    shortcutInit();
    //    stylize();
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

void MainWindow::shortcutInit()
{
    QShortcut* widthActionSub = new QShortcut(this);
    widthActionSub->setKey(Qt::Key_Q);
    connect(widthActionSub, SIGNAL(activated()),
            ui->spinBox, SLOT(stepDown()));
    QShortcut* widthActionAdd = new QShortcut(this);
    widthActionAdd->setKey(Qt::Key_W);
    connect(widthActionAdd, SIGNAL(activated()),
            ui->spinBox, SLOT(stepUp()));

    SingleShortcut *pencilShort = new SingleShortcut(this);
    pencilShort->setKey(Qt::Key_Z);
    connect(pencilShort, &SingleShortcut::activated,
            [&](){
        lastBrushButton =
                ui->buttonGroup->checkedButton();
        if(lastBrushButton)
            ui->pencilButton->click();
    });
    connect(pencilShort, &SingleShortcut::inactivated,
            [&](){
        if(lastBrushButton)
            lastBrushButton->click();
    });

    SingleShortcut *brushShort = new SingleShortcut(this);
    brushShort->setKey(Qt::Key_A);
    connect(brushShort, &SingleShortcut::activated,
            [&](){
        lastBrushButton =
                ui->buttonGroup->checkedButton();
        if(lastBrushButton)
            ui->brushButton->click();
    });
    connect(brushShort, &SingleShortcut::inactivated,
            [&](){
        if(lastBrushButton)
            lastBrushButton->click();
    });

    SingleShortcut *sketchShort = new SingleShortcut(this);
    sketchShort->setKey(Qt::Key_S);
    connect(sketchShort, &SingleShortcut::activated,
            [&](){
        lastBrushButton =
                ui->buttonGroup->checkedButton();
        if(lastBrushButton)
            ui->sketchButton->click();
    });
    connect(sketchShort, &SingleShortcut::inactivated,
            [&](){
        if(lastBrushButton)
            lastBrushButton->click();
    });

    SingleShortcut *eraserShort = new SingleShortcut(this);
    eraserShort->setKey(Qt::Key_E);
    connect(eraserShort, &SingleShortcut::activated,
            [&](){
        lastBrushButton =
                ui->buttonGroup->checkedButton();
        if(lastBrushButton)
            ui->eraserButton->click();
    });
    connect(eraserShort, &SingleShortcut::inactivated,
            [&](){
        if(lastBrushButton)
            lastBrushButton->click();
    });

    SingleShortcut *pickerShort = new SingleShortcut(this);
    pickerShort->setKey(Qt::Key_C);
    connect(pickerShort, &SingleShortcut::activated,
            [&](){
        lastBrushButton =
                ui->buttonGroup->checkedButton();
        if(lastBrushButton)
            ui->colorPicker->click();
    });
    connect(pickerShort, &SingleShortcut::inactivated,
            [&](){
        if(lastBrushButton)
            lastBrushButton->click();
    });

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
        map.insert("request", "close");
        map.insert("key", getRoomKey());
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
        ui->colorPicker->setEnabled(true);
    }
}

void MainWindow::onServerDisconnected()
{
    ui->textEdit->insertPlainText(tr("Server Connection Failed.\n"));
    ui->canvas->setEnabled(false);
}

void MainWindow::onCmdServerConnected()
{
    //    QVariantMap map;
    //    map.insert("request", "login");
    //    QVariantMap info;
    //    info.insert("password");
}

void MainWindow::onCmdServerDisconnected()
{
    //
}

void MainWindow::onCmdServerData(const QByteArray &data)
{
    QVariantMap m = fromJson(data).toMap();
    if(m.contains("action")){
        QString action = m["action"].toString();
        if(action == "close"){
            QMessageBox::warning(this,
                                 tr("Closing"),
                                 tr("Warning, the room owner has "
                                    "closed the room. This room will close"
                                    " when everyone leaves.\n"
                                    "Save your work if you like it!"));
        }
    }

    if(m.contains("response")){
        QString response = m["response"].toString();
        bool result = m["result"].toBool();
        if(response == "close"){
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
    }
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
}

QByteArray MainWindow::toJson(const QVariant &m)
{
    auto jsonS = QJsonDocument::fromVariant(m);
    return jsonS.toJson();
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
            colorMap["blue"].toInt(),
            colorMap["alpha"].toInt());

    // INFO: to prevent scaled to 1px, should always
    // change width first
    if(ui->spinBox->value() != w)
        ui->spinBox->setValue(w);
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
    ui->pencilButton->setEnabled(!c);
    ui->brushButton->setEnabled(!c);
    ui->sketchButton->setEnabled(!c);
    ui->eraserButton->setEnabled(!c);
}

void MainWindow::onPickColorComplete()
{
    ui->colorPicker->setChecked(false);
    ui->pencilButton->setEnabled(true);
    ui->brushButton->setEnabled(true);
    ui->sketchButton->setEnabled(true);
    ui->eraserButton->setEnabled(true);
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
    QAction *clearOne = new QAction(this);
    ui->menuClear_Canvas->insertAction(ui->actionAll_Layers,
                                       clearOne);
    clearOne->setText(tr("Layer ")+name);
    connect(clearOne, &QAction::triggered,
            [this, name, clearOne](){
        this->clearLayer(name);
    });
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
                                        tr("You're going to clear layer ")
                                        +name
                                        +tr(". All the work of that layer"
                                            "will be deleted and CANNOT be undone.\n"
                                            "Do you really want to do so?"),
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
        ui->canvas->clearAllLayer();
        QVariantMap map;
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

void MainWindow::closeEvent ( QCloseEvent * event )
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
