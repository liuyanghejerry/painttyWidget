#include "roomlistdialog.h"
#include "ui_roomlistdialog.h"

RoomListDialog::RoomListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomListDialog),
    dataPort_(0),
    msgPort_(0),
    historySize_(0),
    socket(nullptr),
    newRoomWindow(new NewRoomWindow(this))
{
    ui->setupUi(this);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::reject);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::saveNick);
    connect(ui->pushButton_3,&QPushButton::clicked,
            this, &RoomListDialog::requestRoomList);
    connect(ui->pushButton_2,&QPushButton::clicked,
            this, &RoomListDialog::requestJoin);
    connect(ui->tableWidget,&QTableWidget::cellDoubleClicked,
            this,&RoomListDialog::requestJoin);
    connect(ui->checkBox, &QCheckBox::clicked,
            this,&RoomListDialog::filterRoomList);

    connect(ui->pushButton, &QPushButton::clicked,
            newRoomWindow, &NewRoomWindow::show);
    connect(newRoomWindow, &NewRoomWindow::newRoom,
            this,&RoomListDialog::requestNewRoom);
    connect(CommandSocket::cmdSocket(), &CommandSocket::connected,
            this, &RoomListDialog::onCmdServerConnected);
    connect(CommandSocket::cmdSocket(), &CommandSocket::newData,
            this, &RoomListDialog::onCmdServerData);

    tableInit();
    socketInit();
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,
            this,&RoomListDialog::requestRoomList);
    timer->start(10000);
    loadNick();
    clientId_ = loadClientId();

}

RoomListDialog::~RoomListDialog()
{
    disconnect(socket,&Socket::disconnected,
               this,&RoomListDialog::onServerClosed);
    socket->close();
    delete ui;
    delete socket;
}

void RoomListDialog::tableInit()
{
    ui->tableWidget->setColumnCount(4);
    QStringList list;
    list << QString(tr("Room Name"))
         << QString(tr("Privacy"))
         << QString(tr("Current Member"))
         << QString(tr("Max Member"));
    ui->tableWidget->setHorizontalHeaderLabels(list);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void RoomListDialog::socketInit()
{
    ui->progressBar->setRange(0,0);
    socket = new Socket;
    connect(socket,&Socket::newData,
            this,&RoomListDialog::onServerData);
    connect(socket,&Socket::connected,
            this,&RoomListDialog::requestRoomList);
    connect(socket,&Socket::disconnected,
            this,&RoomListDialog::onServerClosed);
    socket->connectToHost(GlobalDef::HOST_ADDR,
                          GlobalDef::HOST_MGR_PORT);
    managerSocketRouter_.regHandler("response",
                                    "roomlist",
                                    std::bind(&RoomListDialog::onManagerResponseRoomlist,
                                              this,
                                              std::placeholders::_1));
    managerSocketRouter_.regHandler("response",
                                    "newroom",
                                    std::bind(&NewRoomWindow::onServerResponse,
                                              newRoomWindow,
                                              std::placeholders::_1));
}

void RoomListDialog::requestJoin()
{
    CommandSocket::cmdSocket()->close();
    ui->lineEdit->setText(ui->lineEdit->text().trimmed());
    nickName_ = ui->lineEdit->text();
    if(nickName_.isEmpty()){
        QMessageBox::warning( this,
                              tr("Warning"),
                              tr("You must have a valid nick name."),
                              QMessageBox::Close);
        return;
    }
    QList<QTableWidgetItem *> list = ui->tableWidget->selectedItems();
    if(list.isEmpty()){
        QMessageBox::warning( this,
                              tr("Warning"),
                              tr("You didn't choose any room."),
                              QMessageBox::Close);
        return;
    }

    roomName_ = ui->tableWidget->item(list.at(0)->row(), 0)->text();
    if(!roomsInfo.contains(roomName_)){
        // TODO: not found!
        return;
    }
    //    QHostAddress address(
    //                roomsInfo[roomName].value("serverAddress").toString());
    QHostAddress address = socket->address();
    int cmdPort = roomsInfo[roomName_].value("cmdport").toDouble();
    CommandSocket::cmdSocket()->connectToHost(address, cmdPort);

}

void RoomListDialog::requestRoomList()
{
    QJsonObject map;
    map.insert("request", QString("roomlist"));

    QJsonDocument doc;
    doc.setObject(map);
    socket->sendData(doc.toJson());
    ui->progressBar->setRange(0,0);
}

void RoomListDialog::requestNewRoom(const QJsonObject &m)
{
    QJsonObject map;
    map["request"] = QString("newroom");
    map["info"] = m;
    // TODO: add owner info

    QJsonDocument doc;
    doc.setObject(map);

    socket->sendData(doc.toJson());
    ui->progressBar->setRange(0,0);
    //TODO: auto connect to room
}

void RoomListDialog::onServerData(const QByteArray &array)
{
    managerSocketRouter_.onData(array);
}

void RoomListDialog::onManagerResponseRoomlist(const QJsonObject &obj)
{
    // TODO: add errcode
    if(!obj["result"].toBool())
        return;
    QJsonArray list;
    QTableWidgetItem *item = 0;
    list = obj["roomlist"].toArray();
    QJsonValue info;
    int row = 0;
    int column = 0;
    ui->progressBar->setMaximum(100);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    QHash<QString, QJsonObject> tmpRoomsInfo;
    ui->tableWidget->clearContents();
    ui->tableWidget->setSortingEnabled(false);
    foreach(info, list){
        // TODO: we need more validate!
        QJsonObject m = info.toObject();
        QString name = m["name"].toString();
        tmpRoomsInfo.insert(name, m);
        if(ui->checkBox->isChecked()){
            //Don't show it if room is full
            if(m["maxload"] == m["currentload"]){
                continue;
            }
        }

        if(row >= ui->tableWidget->rowCount())
            ui->tableWidget->insertRow(0);
        column = 0;

        item = new QTableWidgetItem(name);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, column++, item);

        bool isPrivate = m["private"].toBool();
        item = new QTableWidgetItem(
                    isPrivate?tr("Private"):tr("Public"));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, column++, item);

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::DisplayRole, m["currentload"].toDouble());
        ui->tableWidget->setItem(0, column++, item);

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::DisplayRole, m["maxload"].toDouble());
        ui->tableWidget->setItem(0, column++, item);

        row++;
        ui->progressBar->setValue(100*row/list.count());
    }
    roomsInfo = tmpRoomsInfo;
    ui->progressBar->setValue(100);
    ui->tableWidget->setSortingEnabled(true);
}

void RoomListDialog::onServerClosed()
{
    QMessageBox::critical(this,
                          tr("Connection"),
                          tr("Sorry, server has closed.") );
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void RoomListDialog::onCmdServerConnected()
{
    int current = roomsInfo[roomName_].value("currentload").toDouble();
    int max = roomsInfo[roomName_].value("maxload").toDouble();

    if(current >= max){
        QMessageBox::critical(this,
                              tr("Full loaded"),
                              tr("Cannot join a full loaded room."),
                              QMessageBox::Close);
        return;
    }
    bool isPrivate = roomsInfo[roomName_].value("private").toBool();
    QString passwd;
    if(isPrivate){
        bool isOk = false;
        passwd = QInputDialog::getText(this,
                                       tr("Password"),
                                       tr("This is a private room, please input password:"),
                                       QLineEdit::PasswordEchoOnEdit,
                                       QString(),
                                       &isOk);
        if(!isOk) {
            return;
        }
        passwd.truncate(16);
    }


    QJsonObject map;
    map.insert("request", QString("login"));
    map.insert("name", nickName_);
    map.insert("password", passwd);
    map.insert("clientid", QString::fromUtf8(clientId_.toHex()));

    QJsonDocument doc;
    doc.setObject(map);

    auto array = doc.toJson();

    CommandSocket::cmdSocket()->sendData(array);
    ui->progressBar->setRange(0, 0);
}

void RoomListDialog::onCmdServerData(const QByteArray &array)
{
    QJsonObject map  = QJsonDocument::fromJson(array).object();
    QString response = map["response"].toString();

    if(response == "login"){
        if(!map.contains("result"))
            return;
        bool res = map["result"].toBool();
        if(!res){
            int errcode = 300;
            if(map.contains("errcode")){
                errcode = map["errcode"].toDouble();
            }
            QMessageBox::critical(this,
                                  tr("Error"),
                                  tr("Sorry, an error occurred.\nError:")
                                  +tr(", ")
                                  +QString::number(errcode)
                                  +ErrorTable::toString(errcode));
        }else{
            if(!map.contains("info"))
                return;
            QJsonObject info = map["info"].toObject();
            if(!info.contains("dataport")
                    || !info.contains("msgport")
                    || !info.contains("historysize")){
                return;
            }
            dataPort_ = info["dataport"].toDouble();
            msgPort_ = info["msgport"].toDouble();
            historySize_ = info["historysize"].toDouble();
            if(info.contains("size")){
                QJsonObject sizeMap = info["size"].toObject();
                int width = sizeMap["width"].toDouble();
                int height = sizeMap["height"].toDouble();
                canvasSize_ = QSize(width, height);
            }
            if(info.contains("clientid")){
                QString clientid = info["clientid"].toString();
                CommandSocket::setClientId(clientid);
                qDebug()<<"clientid assign"
                       <<CommandSocket::clientId();
            }
            accept();
        }
    }
}

void RoomListDialog::filterRoomList()
{
    //TODO: change to local filter
    requestRoomList();
}

QString RoomListDialog::roomName() const
{
    return roomName_;
}

QString RoomListDialog::nick() const
{
    return nickName_;
}

int RoomListDialog::historySize() const
{
    return historySize_;
}

QByteArray RoomListDialog::loadClientId()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QString identy = QDateTime::currentDateTimeUtc()
            .toString(Qt::ISODate)
            + this->nickName_;
    QCryptographicHash hashed_identy(QCryptographicHash::Sha1);
    hashed_identy.addData(identy.toUtf8());
    QByteArray data = hashed_identy.result();
    settings.setValue("global/personal/clientid",
                      data);
    settings.sync();
    return data;
}

void RoomListDialog::loadNick()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QByteArray data = settings.value("global/personal/nick")
            .toByteArray();
    QString name = QString::fromUtf8(data);
    if(name.isEmpty()
            || name.length() > 16){
        QMessageBox::information(this,
                                 tr("Notice"),
                                 tr("We're still in alpha test. "
                                    "This means the program may crash at any time "
                                    "in any condition.\nUse this software "
                                    "only when you accept it."));
    }else{
        ui->lineEdit->setText(name);
    }
}

void RoomListDialog::saveNick()
{
    QString name = ui->lineEdit->text();
    if(name.length() > 16){
        return;
    }
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.setValue("global/personal/nick",
                      name.toUtf8());
    settings.sync();
}

void RoomListDialog::hideEvent( QHideEvent * )
{
    if(this->isHidden()){
        timer->stop();
    }else{
        timer->start(10000);
    }
}

void RoomListDialog::closeEvent(QCloseEvent *e)
{
    saveNick();
    e->accept();
}
