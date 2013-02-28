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
//    socket->setCompressed(false);
    connect(socket,&Socket::newData,
            this,&RoomListDialog::onRoomListUpdate);
    connect(socket,&Socket::connected,
            this,&RoomListDialog::requestRoomList);
    connect(socket,&Socket::disconnected,
            this,&RoomListDialog::onServerClosed);
    socket->connectToHost(GlobalDef::HOST_ADDR, GlobalDef::HOST_MGR_PORT);
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
    int cmdPort = roomsInfo[roomName_].value("cmdport").toInt();
    CommandSocket::cmdSocket()->connectToHost(address, cmdPort);

}

void RoomListDialog::requestRoomList()
{
    QByteArray array;

    QVariantMap map;
    map.insert("request", QString("roomlist"));
    QVariant data(map);

    array = QJsonDocument::fromVariant(data).toJson();

    socket->sendData(array);
    ui->progressBar->setRange(0,0);
}

void RoomListDialog::requestNewRoom(const QVariantMap &m)
{
    QVariantMap map;
    map["request"] = "newroom";
    map["info"] = m;
    // TODO: add owner info

    auto array = QJsonDocument::fromVariant(
                QVariant(map)).toJson();

    socket->sendData(array);
    ui->progressBar->setRange(0,0);
    //TODO: auto connect to room
}

void RoomListDialog::onRoomListUpdate(const QByteArray &array)
{
    auto var = QJsonDocument::fromJson(array).toVariant();
    QVariantMap map = var.toMap();
    QString response = map["response"].toString();

    if(response == "roomlist"){
        // TODO: add errcode
        if(!map["result"].toBool())
            return;
        QVariantList list;
        QTableWidgetItem *item = 0;
        list = map["roomlist"].toList();
        QVariant info;
        int row = 0;
        int column = 0;
        ui->progressBar->setMaximum(100);
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
        QHash<QString, QVariantMap> tmpRoomsInfo;
        ui->tableWidget->clearContents();
        ui->tableWidget->setSortingEnabled(false);
        foreach(info, list){
            // TODO: we need more validate!
            QVariantMap m = info.toMap();
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
            item->setData(Qt::DisplayRole, m["currentload"].toInt());
            ui->tableWidget->setItem(0, column++, item);

            item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setData(Qt::DisplayRole, m["maxload"].toInt());
            ui->tableWidget->setItem(0, column++, item);

            row++;
            ui->progressBar->setValue(100*row/list.count());
        }
        roomsInfo = tmpRoomsInfo;
        ui->progressBar->setValue(100);
        ui->tableWidget->setSortingEnabled(true);
    }else if(response == "newroom"){
        newRoomWindow->onServerResponse(map);
        requestRoomList();
    }
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
    int current = roomsInfo[roomName_].value("currentload").toInt();
    int max = roomsInfo[roomName_].value("maxload").toInt();

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


    QVariantMap map;
    map.insert("request", "login");
    map.insert("name", nickName_);
    map.insert("password", passwd);

    auto array = QJsonDocument::fromVariant(QVariant(map)).toJson();

    CommandSocket::cmdSocket()->sendData(array);
    ui->progressBar->setRange(0, 0);
}

void RoomListDialog::onCmdServerData(const QByteArray &array)
{
    auto var  = QJsonDocument::fromJson(array).toVariant();
    QVariantMap map = var.toMap();
    QString response = map["response"].toString();

    if(response == "login"){
        if(!map.contains("result"))
            return;
        bool res = map["result"].toBool();
        if(!res){
            int errcode = 300;
            if(map.contains("errcode")){
                errcode = map["errcode"].toInt();
            }
            QMessageBox::critical(this,
                                  tr("Error"),
                                  tr("Sorry, an error occurred.\nError:")
                                  +QString::number(errcode));
        }else{
            if(!map.contains("info"))
                return;
            QVariantMap info = map["info"].toMap();
            if(!info.contains("dataport")
                    || !info.contains("msgport")
                    || !info.contains("historysize")){
                return;
            }
            dataPort_ = info["dataport"].toInt();
            msgPort_ = info["msgport"].toInt();
            historySize_ = info["historysize"].toULongLong();
            //            socket->close();
            accept();
        }
    }
}

void RoomListDialog::filterRoomList()
{
    //TODO: change to local filter
    requestRoomList();
}

QString RoomListDialog::roomName()
{
    return roomName_;
}

QString RoomListDialog::nick()
{
    return nickName_;
}

int RoomListDialog::historySize()
{
    return historySize_;
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
