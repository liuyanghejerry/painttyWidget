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
#include "gradualbox.h"

#define client_socket (Singleton<ClientSocket>::instance())

RoomListDialog::RoomListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomListDialog),
    timer(new QTimer(this)),
    newRoomWindow(new NewRoomWindow(this)),
    state_(Init)
{
    ui->setupUi(this);
    //resize(width()*logicalDpiX()/96, height()*logicalDpiY()/96);
    connect(ui->pushButton_6, &QPushButton::clicked,
            [this](){
        bool ok;
        QString text = QInputDialog::getText(this, tr("Room Url"),
                                             tr("Input room url:"), QLineEdit::Normal,
                                             "paintty://", &ok);
        if (ok && !text.isEmpty()){
            connectRoomByUrl(text);
        }
    });
    connect(ui->pushButton_5, &QPushButton::clicked,
            this, &RoomListDialog::openConfigure);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::reject);
    connect(ui->pushButton_4,&QPushButton::clicked,
            this, &RoomListDialog::saveNick);
    connect(ui->pushButton_3,&QPushButton::clicked,
            this, &RoomListDialog::requestRoomList);
    connect(ui->pushButton_2,&QPushButton::clicked,
            this, &RoomListDialog::tryJoinRoomManually);
    connect(ui->tableWidget,&QTableWidget::cellDoubleClicked,
            this, &RoomListDialog::tryJoinRoomManually);
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

    connect(&client_socket, &ClientSocket::managerConnected,
            [this](){
        state_ = ManagerConnected;
    });
    connect(&client_socket, &ClientSocket::managerConnected,
            this, &RoomListDialog::requestRoomList);
    connect(&client_socket, &ClientSocket::roomCreated,
            this, &RoomListDialog::onNewRoomCreated);
    connect(&client_socket, &ClientSocket::roomListFetched,
            this, &RoomListDialog::onRoomlist);
    connect(&client_socket, &ClientSocket::roomJoined,
            this, &RoomListDialog::accept);

    ui->counter_label->setText(tr("Rooms: %1, Members: %2")
                               .arg("?")
                               .arg("?"));
    tableInit();
    state_ = Ready;
    loadNick();
}

RoomListDialog::~RoomListDialog()
{
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
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->sortByColumn(2, Qt::DescendingOrder);
}

void RoomListDialog::connectToManager()
{
    state_ = ManagerConnecting;
    ui->progressBar->setRange(0,0);

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool use_defalut_server = settings.value("global/server/use_default", true).toBool();
    QString IPv4_addr = settings.value("global/server/ipv4_addr", QString()).toString();
    QString IPv6_addr = settings.value("global/server/ipv6_addr", QString()).toString();
    quint16 server_port = settings.value("global/server/server_port", 0).toUInt();

    QHostAddress addr[2];
    quint16 port = 0;

    if(!use_defalut_server){
        qDebug()<<"using address in settings";
        addr[0] = QHostAddress(IPv4_addr);
        addr[1] = QHostAddress(IPv6_addr);
        port = server_port;
    }else{
        qDebug()<<"using default address";
        addr[0] = GlobalDef::HOST_ADDR[0];
        addr[1] = GlobalDef::HOST_ADDR[1];
        port = GlobalDef::HOST_MGR_PORT;
    }

    if(LocalNetworkInterface::supportIpv6()){
        qDebug()<<"using ipv6 address to connect server";
        client_socket.connectToManager(addr[1], port);
    }else{
        qDebug()<<"using ipv4 address to connect server";
        client_socket.connectToManager(addr[0], port);
    }
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
        client_socket.requestRoomList();
        ui->progressBar->setRange(0,0);
    }else{
        qDebug()<<"Unexpected State in requestRoomList"<<state_;
//        state_ = ConnectFailed;
//        GradualBox::showText(tr("Seems you have trouble on connecting to server."));
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
        client_socket.requestNewRoom(m);
        ui->progressBar->setRange(0,0);
    }else{
        qDebug()<<"Unexpected State in requestNewRoom"<<state_;
    }
}

void RoomListDialog::tryJoinRoomManually()
{
    if(state_ < 1 || !collectUserInfo()){
        return;
    }
    state_ = RoomConnecting;
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
            connectToManager();
            return;
        }
        passwd.truncate(16);
    }

    client_socket.setPasswd(passwd);
    client_socket.setUserName(nickName_);
    QHostAddress addr(client_socket.address());
    if(roomsInfo[roomName_].value("serveraddress").toString("0.0.0.0") != "0.0.0.0") {
        addr = roomsInfo[roomName_].value("serveraddress").toString();
    }
    client_socket.tryJoinRoom(addr,
                              roomsInfo[roomName_].value("port").toInt());
    ui->progressBar->setRange(0, 0);
}

void RoomListDialog::connectRoomByUrl(const QString& url)
{
    state_ = RoomConnecting;
    client_socket.setUserName(nickName_);
    client_socket.tryJoinRoom(url);
}

void RoomListDialog::onRoomlist(const QHash<QString, QJsonObject> &obj)
{
    qDebug()<<"onRoomlist";
    state_ = ManagerConnected;
    roomsInfo = obj;
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
    qDebug()<<"Room connected";
    state_ = RoomConnected;
    timer->stop();
}

void RoomListDialog::onNewRoomCreated()
{
    state_ = ManagerConnected;
    newRoomWindow->complete();
    QString msg = tr("Succeed!");
    QMessageBox::information(newRoomWindow, tr("Go get your room!"),
                             msg,
                             QMessageBox::Ok);
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
        int currentLoad = info["currentload"].toDouble();
        int maxLoad = info["maxload"].toDouble();
        if(ui->checkBox->isChecked()){
            //Don't show it if room is full
            if(currentLoad >= maxLoad){
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
        // if current > max, there must be people joining via url
        if(currentLoad > maxLoad) {
            item->setData(Qt::DisplayRole, QString("%1+%1").arg(maxLoad).arg(currentLoad - maxLoad));
        } else {
            item->setData(Qt::DisplayRole, currentLoad);
        }
        ui->tableWidget->setItem(0, column++, item);
        members += currentLoad;

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::DisplayRole, maxLoad);
        ui->tableWidget->setItem(0, column++, item);

        row++;
    }
    ui->counter_label->setText(tr("Rooms: %1, Members: %2")
                               .arg(row)
                               .arg(members));
    ui->tableWidget->setSortingEnabled(true);
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
    client_socket.setUserName(nickName_);
}

void RoomListDialog::openConfigure()
{
    ConfigureDialog w;
    w.exec();
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
