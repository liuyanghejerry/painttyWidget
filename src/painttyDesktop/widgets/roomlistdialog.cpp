#include "roomlistdialog.h"
#include "ui_roomlistdialog.h"

#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QTableWidgetItem>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>
#include <QDateTime>
#include <QCryptographicHash>

#include "../../common/common.h"
#include "../../common/network/clientsocket.h"
#include "../../common/network/localnetworkinterface.h"
#include "newroomwindow.h"
#include "../misc/singleton.h"
#include "configuredialog.h"
#include "../misc/errortable.h"

RoomListDialog::RoomListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomListDialog),
    historySize_(0),
    timer(new QTimer(this)),
    newRoomWindow(new NewRoomWindow(this)),
    state_(Init)
{
    ui->setupUi(this);
    connect(ui->pushButton_5, &QPushButton::clicked,
            this, &RoomListDialog::openConfigure);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::reject);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::saveNick);
    connect(ui->pushButton_3,&QPushButton::clicked,
            this, &RoomListDialog::requestRoomList);
    connect(ui->pushButton_2,&QPushButton::clicked,
            this, &RoomListDialog::requestJoin);
    connect(ui->tableWidget,&QTableWidget::cellDoubleClicked,
            this, &RoomListDialog::requestJoin);
    connect(ui->checkBox, &QCheckBox::clicked,
            this, &RoomListDialog::filterRoomList);
    connect(ui->search_box, &QLineEdit::textChanged,
            this, &RoomListDialog::filterRoomList);

    connect(ui->pushButton, &QPushButton::clicked,
            newRoomWindow, &NewRoomWindow::show);
    connect(newRoomWindow, &NewRoomWindow::newRoom,
            this,&RoomListDialog::requestNewRoom);
    connect(newRoomWindow, &NewRoomWindow::finished,
            this,&RoomListDialog::requestRoomList);

    connect(timer,&QTimer::timeout,
            this,&RoomListDialog::requestRoomList);

    ui->counter_label->setText(tr("Rooms: %1, Members: %2")
                               .arg("?")
                               .arg("?"));
    tableInit();
    state_ = Ready;
    socketInit();
    loadNick();
    clientId_ = loadClientId();

}

RoomListDialog::~RoomListDialog()
{
    auto& socket = Singleton<ClientSocket>::instance();
    disconnect(&socket, &ClientSocket::connected,
               this, &RoomListDialog::onManagerServerConnected);
    disconnect(&socket, &ClientSocket::disconnected,
               this, &RoomListDialog::onManagerServerClosed);
    disconnect(&socket, &ClientSocket::managerPack,
            this, &RoomListDialog::onManagerData);
    socket.close();
    delete ui;
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

void RoomListDialog::connectToManager()
{
    state_ = ManagerConnecting;
    ui->progressBar->setRange(0,0);
    auto& client_socket = Singleton<ClientSocket>::instance();

    connect(&client_socket, &ClientSocket::managerPack,
            this, &RoomListDialog::onManagerData);
    connect(&client_socket, &ClientSocket::connected,
            this, &RoomListDialog::onManagerServerConnected);
    connect(&client_socket, &ClientSocket::disconnected,
            this, &RoomListDialog::onManagerServerClosed);
    client_socket.setPoolEnabled(false);
    QHostAddress addr;
    if(LocalNetworkInterface::supportIpv6()){
        addr = GlobalDef::HOST_ADDR[1];
        qDebug()<<"using ipv6 address to connect server";
    }else{
        addr = GlobalDef::HOST_ADDR[0];
        qDebug()<<"using ipv4 address to connect server";
    }

    client_socket.connectToHost(addr,
                                GlobalDef::HOST_MGR_PORT);
}

void RoomListDialog::socketInit()
{
    managerSocketRouter_.regHandler("response",
                                    "roomlist",
                                    std::bind(&RoomListDialog::onManagerResponseRoomlist,
                                              this,
                                              std::placeholders::_1));
    managerSocketRouter_.regHandler("response",
                                    "newroom",
                                    std::bind(&RoomListDialog::onNewRoomRespnse,
                                              this,
                                              std::placeholders::_1));
}

bool RoomListDialog::collectUserInfo()
{
    ui->lineEdit->setText(ui->lineEdit->text().trimmed());
    nickName_ = ui->lineEdit->text();
    if(nickName_.isEmpty()){
        QMessageBox::warning( this,
                              tr("Warning"),
                              tr("You must have a valid nick name."),
                              QMessageBox::Close);
        return false;
    }
    return true;
}

void RoomListDialog::requestJoin()
{
    if(state_ < 1){
        return;
    }
    state_ = RoomConnecting;
    wantedRoomName_.clear();
    wantedPassword_.clear();
    if(!collectUserInfo()){
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
        return;
    }
    //    QHostAddress address(
    //                roomsInfo[roomName].value("serverAddress").toString());
    //    QHostAddress address = Singleton<ClientSocket>::instance().address();
    int port = roomsInfo[roomName_].value("port").toDouble();
    connectRoomByPort(port);
}

void RoomListDialog::requestRoomList()
{
    if(state_ < 1){
        return;
    }else if(state_ == ManagerConnecting){
        QMessageBox::critical(newRoomWindow, tr("Error!"),
                              tr("Cannot connect to server.\n" \
                                 "If this situation continues," \
                                 " you should consider <a href='http://mrspaint.com'>update</a> manually."),
                              QMessageBox::Ok);
        state_ = Error;
        return;
    }else if(state_ == ManagerConnected){
        state_ = RequestingList;
        QJsonObject map;
        map.insert("request", QString("roomlist"));

        Singleton<ClientSocket>::instance().sendManagerPack(map);
        ui->progressBar->setRange(0,0);
    }else{
        qDebug()<<"Unexpected State in requestRoomList"<<state_;
    }
}

void RoomListDialog::requestNewRoom(const QJsonObject &m)
{
    if(state_ < 1){
        return;
    }else if(state_ == ManagerConnecting){
        QMessageBox::critical(newRoomWindow, tr("Error!"),
                              tr("Cannot connect to server.\n" \
                                 "If this situation continues," \
                                 " you should consider <a href='http://mrspaint.com'>update</a> manually."),
                              QMessageBox::Ok);
        state_ = Error;
        return;
    }else if(state_ == ManagerConnected){
        state_ = RequestingNewRoom;
        QJsonObject map;
        map["request"] = QString("newroom");
        map["info"] = m;
        // TODO: add owner info

        Singleton<ClientSocket>::instance().sendManagerPack(map);

        ui->progressBar->setRange(0,0);
        wantedRoomName_ = m["name"].toString();
        wantedPassword_ = m["password"].toString();
    }else{
        qDebug()<<"Unexpected State in requestNewRoom"<<state_;
    }
}

void RoomListDialog::connectRoomByPort(const int &p)
{
    auto& client_socket = Singleton<ClientSocket>::instance();
    QHostAddress address = client_socket.address();

    disconnect(&client_socket, &ClientSocket::connected,
               this, &RoomListDialog::onManagerServerConnected);
    disconnect(&client_socket, &ClientSocket::disconnected,
               this, &RoomListDialog::onManagerServerClosed);
    disconnect(&client_socket, &ClientSocket::managerPack,
            this, &RoomListDialog::onManagerData);
    client_socket.close();

    connect(&client_socket, &ClientSocket::connected,
            this, &RoomListDialog::onCmdServerConnected);
    connect(&client_socket, &ClientSocket::cmdPack,
            this, &RoomListDialog::onCmdData);

    qDebug()<<"start connecting room";
    client_socket.connectToHost(address, p);
}

void RoomListDialog::tryJoinRoomManually()
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

    Singleton<ClientSocket>::instance().sendManagerPack(map);
    ui->progressBar->setRange(0, 0);
}

void RoomListDialog::tryJoinRoomAutomated()
{
    if(!collectUserInfo()){
        return;
    }
    QJsonObject map;
    map.insert("request", QString("login"));
    map.insert("name", nickName_);
    map.insert("password", wantedPassword_);
    map.insert("clientid", QString::fromUtf8(clientId_.toHex()));

    qDebug()<<"try auto join room";
    Singleton<ClientSocket>::instance().sendCmdPack(map);
    ui->progressBar->setRange(0, 0);
}

void RoomListDialog::onManagerData(const QJsonObject &array)
{
    qDebug()<<"onManagerData"<<array;
    managerSocketRouter_.onData(array);
}

void RoomListDialog::onManagerResponseRoomlist(const QJsonObject &obj)
{
    state_ = ManagerConnected;
    if(!obj["result"].toBool())
        return;
    QJsonArray list = obj["roomlist"].toArray();
    QHash<QString, QJsonObject> tmpRoomsInfo;
    for(auto info: list){
        QJsonObject m = info.toObject();
        QString name = m["name"].toString();
        tmpRoomsInfo.insert(name, m);
    }
    roomsInfo = tmpRoomsInfo;
    filterRoomList();
    ui->progressBar->setValue(100);

}

void RoomListDialog::onManagerServerConnected()
{
    qDebug()<<"Manager connected";
    state_ = ManagerConnected;
    ui->progressBar->setValue(100);
    requestRoomList();
    timer->start(REFRESH_TIME);
}

void RoomListDialog::onManagerServerClosed()
{
    state_ = Error;
    QMessageBox::critical(this,
                          tr("Connection"),
                          tr("Sorry, server has closed.") );
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void RoomListDialog::onCmdServerConnected()
{
    state_ = RoomConnected;
    timer->stop();
    if(wantedRoomName_.isEmpty()){
        tryJoinRoomManually();
    }else{
        tryJoinRoomAutomated();
    }
}

void RoomListDialog::onCmdData(const QJsonObject &map)
{
    auto& client_socket = Singleton<ClientSocket>::instance();
    client_socket.setPoolEnabled(true);
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
                                  tr("Sorry, an error occurred.\n"
                                     "Error: %1, %2").arg(errcode)
                                  .arg(ErrorTable::toString(errcode)));
        }else{
            if(!map.contains("info"))
                return;
            QJsonObject info = map["info"].toObject();
            if(!info.contains("historysize")){
                return;
            }
            historySize_ = info["historysize"].toDouble();
            if(info.contains("size")){
                QJsonObject sizeMap = info["size"].toObject();
                int width = sizeMap["width"].toDouble();
                int height = sizeMap["height"].toDouble();
                canvasSize_ = QSize(width, height);
            }
            if(info.contains("clientid")){
                QString clientid = info["clientid"].toString();
                Singleton<ClientSocket>::instance().setClientId(clientid);
                qDebug()<<"clientid assign"
                       <<Singleton<ClientSocket>::instance().clientId();
            }
            state_ = RoomJoined;

            disconnect(&client_socket, &ClientSocket::connected,
                       this, &RoomListDialog::onCmdServerConnected);
            disconnect(&client_socket, &ClientSocket::cmdPack,
                       this, &RoomListDialog::onCmdData);
            accept();
            return;
        }
        state_ = ManagerConnected;
        ui->progressBar->setValue(100);
    }
}

void RoomListDialog::onNewRoomRespnse(const QJsonObject &m)
{
    state_ = ManagerConnected;
    if(m["result"].toBool()){
        newRoomWindow->complete();
        QString msg = tr("Succeed!");
        QMessageBox::information(newRoomWindow, tr("Go get your room!"),
                                 msg,
                                 QMessageBox::Ok);
        int port = 0;
        if(m.contains("info")){
            QJsonObject info = m.value("info").toObject();
            port = info.value("port").toDouble();
            QString key = info.value("key").toString();

            QCryptographicHash hash(QCryptographicHash::Md5);
            hash.addData(newRoomWindow->roomName().toUtf8());
            QString hashed_name = hash.result().toHex();
            QSettings settings(GlobalDef::SETTINGS_NAME,
                               QSettings::defaultFormat(),
                               qApp);
            settings.setValue("rooms/"+hashed_name, key);
            settings.sync();
        }
        newRoomWindow->accept();

        if(port){
            // Since full new room info comes later,
            // we must fake it to join it now
            roomName_ = wantedRoomName_;
            connectRoomByPort(port);
        }
        return;
    }
    newRoomWindow->failed();

    if(!m.contains("errcode")){
        return;
    }else{
        int errcode = m["errcode"].toDouble();
        QString errmsg = tr("Error: %1, %2\n"
                            "Do you want to retry?")
                .arg(errcode).arg(ErrorTable::toString(errcode));
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(newRoomWindow, tr("Error!"),
                                      errmsg,
                                      QMessageBox::Retry|
                                      QMessageBox::Abort);
        if(reply == QMessageBox::Retry){
            newRoomWindow->onOk();
        }else{
            return;
        }
    }
}

void RoomListDialog::filterRoomList()
{
    QString&& searchText = ui->search_box->text();
    QTableWidgetItem *item = 0;
    int row = 0;
    int column = 0;
    int members = 0;
    ui->progressBar->setMaximum(100);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setSortingEnabled(false);
    for(auto& info: roomsInfo){
        QString name = info["name"].toString();
        if(ui->checkBox->isChecked()){
            //Don't show it if room is full
            if(info["maxload"] == info["currentload"]){
                continue;
            }
        }

        if(!name.contains(searchText)){
            continue;
        }

        if(row >= ui->tableWidget->rowCount())
            ui->tableWidget->insertRow(0);
        column = 0;

        item = new QTableWidgetItem(name);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, column++, item);

        bool isPrivate = info["private"].toBool();
        item = new QTableWidgetItem(
                    isPrivate?tr("Private"):tr("Public"));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, column++, item);

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::DisplayRole, info["currentload"].toDouble());
        ui->tableWidget->setItem(0, column++, item);
        members += info["currentload"].toDouble();

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::DisplayRole, info["maxload"].toDouble());
        ui->tableWidget->setItem(0, column++, item);

        row++;
        //        ui->progressBar->setValue(100*row/roomsInfo.count());
    }
    ui->counter_label->setText(tr("Rooms: %1, Members: %2")
                               .arg(row)
                               .arg(members));
    ui->tableWidget->setSortingEnabled(true);
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
    commitToGlobal();
}

void RoomListDialog::openConfigure()
{
    ConfigureDialog w;
    w.exec();
}

void RoomListDialog::commitToGlobal()
{
    auto &instance = Singleton<ClientSocket>::instance();
    instance.setUserName(this->nick());
    instance.setCanvasSize(this->canvasSize());
    instance.setHistorySize(this->historySize());
    instance.setRoomName(this->roomName());
}

void RoomListDialog::hideEvent(QHideEvent *e)
{
    saveNick();
    QDialog::hideEvent(e);
}

void RoomListDialog::showEvent(QShowEvent *e)
{
    connectToManager();
    QDialog::showEvent(e);
}

void RoomListDialog::closeEvent(QCloseEvent *e)
{
    saveNick();
    e->accept();
}
