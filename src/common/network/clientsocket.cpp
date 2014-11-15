#include "clientsocket.h"
#include "../../common/binary.h"
#include "../../common/common.h"
#include "../misc/archivefile.h"
#include "../misc/singleton.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QMetaMethod>
#include <QSettings>
#include <QTimer>
#include <QRegularExpression>
#include <QDateTime>

using std::tuple;


static QByteArray jsonToBuffer(const QJsonObject& obj)
{
    QJsonDocument doc;
    doc.setObject(obj);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    QByteArray buffer = doc.toJson(QJsonDocument::Compact);
#else
    QByteArray buffer = doc.toJson();
#endif
    return buffer;
}

ClientSocket::ClientSocket(QObject *parent) :
    Socket(parent),
    schedualDataLength_(0),
    leftDataLength_(0),
    poolEnabled_(false),
    timer_(new QTimer(this)),
    archive_(Singleton<ArchiveFile>::instance()),
    no_save_(false),
    remove_after_close_(false),
    canceled_(false),
    hb_timer_(new QTimer(this)),
    state_(INIT),
    roomDelay_(-1)
{
    initRouter();
    connect(this, &ClientSocket::newData,
            this, &ClientSocket::onPending);
    connect(this, &ClientSocket::managerPack,
            this, &ClientSocket::onManagerPack);
    connect(timer_, &QTimer::timeout,
            this, &ClientSocket::processPending);
    timer_->start(WAIT_TIME);

    connect(hb_timer_, &QTimer::timeout,
            this, &ClientSocket::sendHeartbeat);
}

ClientSocket::State ClientSocket::currentState() const
{
    return state_;
}

void ClientSocket::setPoolEnabled(bool on)
{
    poolEnabled_ = on;
    if(!poolEnabled_){
        timer_->start(WAIT_TIME);
        processPending();
    }else{
        timer_->stop();
    }
}

bool ClientSocket::isPoolEnabled()
{
    return poolEnabled_;
}

void ClientSocket::setSchedualDataLength(quint64 length)
{
    leftDataLength_ = schedualDataLength_ = length;
    if(leftDataLength_ <= 0){
        emit archiveLoaded(schedualDataLength_);
    }
}

quint64 ClientSocket::schedualDataLength()
{
    return schedualDataLength_;
}

QString ClientSocket::archiveSignature() const
{
    return archive_.signature();
}

void ClientSocket::setArchiveSignature(const QString &as)
{
    archive_.setSignature(as);
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat());
    bool skip_replay = settings.value("canvas/skip_replay", true).toBool();
    if(!skip_replay){
        no_save_ = true;
        unpack(archive_.readAll());
        no_save_ = false;
    }else{
        leftDataLength_ -= archive_.size();
    }
}

quint64 ClientSocket::archiveSize() const
{
    return archive_.size();
}

void ClientSocket::setRoomCloseFlag()
{
    remove_after_close_ = true;
}

QString ClientSocket::toUrl() const
{
    return genRoomUrl(address().toString(),
                            port(),
                            passwd());
}

void ClientSocket::connectToManager(const QHostAddress &addr, const int port)
{
    disconnect(this, &ClientSocket::connected, this, 0);
    setPoolEnabled(false);
    connect(this, &ClientSocket::connected,
            [this] {
        state_ = MANAGER_CONNECTED;
        emit managerConnected();
    });
    connect(this, &ClientSocket::disconnected,
            [this]{
        // TODO: mannully or suppose to close?
//        router_.clear();
    });
    connectToHost(addr, port);
    state_ = CONNECTING_MANAGER;
}

void ClientSocket::requestRoomList()
{
    QJsonObject map;
    map.insert("request", QString("roomlist"));
    sendManagerPack(map);
    state_ = REQUESTING_ROOMLIST;
}

void ClientSocket::requestNewRoom(const QJsonObject &m)
{
    QJsonObject map;
    map["request"] = QString("newroom");
    map["info"] = m;
    setRoomName(m["name"].toString());
    setPasswd(m["password"].toString());
    sendManagerPack(map);
    state_ = REQUESTING_NEWROOM;
}

void ClientSocket::tryJoinRoom(const QString &url)
{
    auto decoded_info = decodeRoomUrl(url);
    setPasswd(decoded_info.passwd);
    tryJoinRoom(QHostAddress(decoded_info.addr), decoded_info.port);
}

void ClientSocket::onResponseRoomList(const QJsonObject &obj)
{
    if(!obj["result"].toBool())
        return;
    QJsonArray list = obj["roomlist"].toArray();
    QHash<QString, QJsonObject> roomsInfo;
    for(auto info: list){
        QJsonObject m = info.toObject();
        QString name = m["name"].toString();
        roomsInfo.insert(name, m);
    }
    // TODO: emit roomslist
    emit roomlistFetched(roomsInfo);
}

void ClientSocket::onResponseNewRoom(const QJsonObject &m)
{
    if(m["result"].toBool()){
        QHostAddress addr(address());
        int roomPort = 0;
        if(m.contains("info")){
            QJsonObject info = m.value("info").toObject();
            roomPort = info.value("port").toDouble();
            if(info.value("address").toString("0.0.0.0") != "0.0.0.0") {
                addr = QHostAddress(info.value("address").toString());
            }
            roomKey_ = info.value("key").toString();

            QCryptographicHash hash(QCryptographicHash::Md5);
            hash.addData(roomName().toUtf8());
            QString hashed_name = hash.result().toHex();
            QSettings settings(GlobalDef::SETTINGS_NAME,
                               QSettings::defaultFormat(),
                               qApp);
            settings.setValue("rooms/"+hashed_name, roomKey_);
            settings.sync();
        }

        emit roomCreated();

        if(roomPort){
            // Since full new room info comes later,
            // we must fake it to join it now
            tryJoinRoom(addr, roomPort);
        }
        return;
    }

    if(!m.contains("errcode")){
        return;
    }else{
        int errcode = m["errcode"].toDouble();
        emit clientSocketError(errcode);
    }
}

void ClientSocket::onResponseLogin(const QJsonObject &map)
{
    setPoolEnabled(true);

    if(!map.contains("result"))
        return;
    if(!map["result"].toBool()){
        int errcode = 300;
        if(map.contains("errcode")){
            errcode = map["errcode"].toDouble();
        }
        emit clientSocketError(errcode);
    }else{
        if(!map.contains("info"))
            return;
        QJsonObject info = map["info"].toObject();
        if(!info.contains("historysize")){
            return;
        }
        setSchedualDataLength(info["historysize"].toDouble());
        if(info.contains("size")){
            QJsonObject sizeMap = info["size"].toObject();
            int width = sizeMap["width"].toDouble();
            int height = sizeMap["height"].toDouble();
            setCanvasSize(QSize(width, height));
        }
        if(info.contains("clientid")){
            QString clientid = info["clientid"].toString();
            setClientId(clientid);
            qDebug()<<"clientid assign"
                   <<clientid;
        }
        if(info.contains("name")){
            QString name = info["name"].toString();
            setRoomName(name);
            qDebug()<<"room name assign"
                   <<name;
        }

        setPoolEnabled(false);
        requestArchiveSign();
        // TODO: checkout if own the room
        if(roomKey().length()) {
            requestCheckout();
        }
        startHeartbeat();
        state_ = ROOM_JOINED;
        emit roomJoined();
    }
    //
}

void ClientSocket::tryJoinRoom(const QHostAddress &addr, const int port)
{
    qDebug()<<"tryJoinRoom"<<addr<<port;
    close();
    disconnect(this, &ClientSocket::connected, this, 0);
    disconnect(this, &ClientSocket::cmdPack, this, 0);
    connect(this, &ClientSocket::connected,
            [this]() {
        QJsonObject map;
        map.insert("request", QString("login"));
        map.insert("name", userName());
        map.insert("password", passwd());

        qDebug()<<"try auto join room";
        sendCmdPack(map);
        state_ = JOINING_ROOM;

//        disconnect(this, &ClientSocket::connected, 0, 0);
    });
    connect(this, &ClientSocket::cmdPack,
            [this](const QJsonObject &map) {
        qDebug()<<"onCmdData"<<map;
        router_.onData(map);
    });
    connectToHost(addr, port);
    state_ = CONNECTING_ROOM;
}

void ClientSocket::requestArchive()
{
    QJsonObject obj;
    obj.insert("request", QString("archive"));
    obj.insert("start", (int)archiveSize());
    qDebug()<<"request archive"<<obj;
    sendCmdPack(obj);
}

void ClientSocket::requestArchiveSign()
{
    QJsonObject obj;
    obj.insert("request", QString("archivesign"));
    qDebug()<<"request archive signature";
    sendCmdPack(obj);
}

void ClientSocket::onResponseArchiveSign(const QJsonObject &o)
{
    qDebug()<<o;
    if(!o.contains("result") || !o.contains("signature")){
        return;
    }
    int errcode = 800;
    if(!o.value("result").toBool()){
        emit clientSocketError(errcode);
        return;
    }

    QString signature = o.value("signature").toString();

    setArchiveSignature(signature);
    requestArchive();
}

void ClientSocket::onResponseArchive(const QJsonObject &o)
{
    if(!o.contains("result")|| !o.contains("datalength")){
        return;
    }
    int errcode = 900;
    if(!o.value("result").toBool()){
        emit clientSocketError(errcode);
        return;
    }

    quint64 datalength = o.value("datalength").toDouble();

    // TODO: re-match signature and receive archive data
    setSchedualDataLength(datalength);
}

void ClientSocket::onCommandActionClose(const QJsonObject &)
{
    emit roomAboutToClose();
    setRoomCloseFlag();
}

void ClientSocket::onCommandResponseClose(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        emit requestUnauthed();
    }else{
        // Since server accepted close request, we can
        // wait for close now.
        // of course, delete the key. it's useless.
        QCryptographicHash hash(QCryptographicHash::Md5);
        auto name = roomName();
        hash.addData(name.toUtf8());
        QString hashed_name = hash.result().toHex();
        QSettings settings(GlobalDef::SETTINGS_NAME,
                           QSettings::defaultFormat(),
                           qApp);
        settings.remove("rooms/"+hashed_name);
        settings.sync();
    }
}

void ClientSocket::onCommandResponseClearAll(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        emit requestUnauthed();
    }
}

void ClientSocket::onCommandResponseCheckout(const QJsonObject &m)
{
    bool result = m["result"].toBool();
    if(!result){
        //
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
                    this, &ClientSocket::requestCheckout);
        }
    }
}

void ClientSocket::onCommandActionClearAll(const QJsonObject &obj)
{
    qDebug()<<"on action clearall"<<obj;
    if(obj.contains("signature")){
        auto&& s = obj.value("signature").toString();
        setArchiveSignature(s);
    }
    emit layerAllCleared();
}

void ClientSocket::onCommandResponseOnlinelist(const QJsonObject &o)
{
    QJsonArray list = o.value("onlinelist").toArray();
    QHash<QString, QVariantList> l;
    for(int i=0;i<list.count();++i){
        QJsonObject obj = list[i].toObject();
        QString id = obj.value("clientid").toString();
        QString nick = obj.value("name").toString();
        QVariantList vl;
        vl.append(nick);
        l.insert(id, vl);
    }
    emit memberListFetched(l);
}

void ClientSocket::onActionNotify(const QJsonObject &o)
{
    QString content = o.value("content").toString();
    if(content.isEmpty()){
        return;
    }

    emit getNotified(content);
    qDebug()<<"notified with: "<<o;
}

void ClientSocket::onActionKick(const QJsonObject &)
{
    qDebug()<<"Get kicked";
    emit getKicked();
}

void ClientSocket::onResponseHeartbeat(const QJsonObject &o)
{
    if(!o.contains("timestamp")){
        return;
    }
    qDebug()<<o;
    int server_time = o.value("timestamp").toInt();
    int now = QDateTime::currentMSecsSinceEpoch() / 1000;
    int delta = now - server_time;
    roomDelay_ = delta;
}


void ClientSocket::exitFromRoom()
{
    reset();
    close();
}

void ClientSocket::requestOnlinelist()
{
    QJsonObject obj;
    obj.insert("request", QString("onlinelist"));
    obj.insert("type", QString("command"));
    obj.insert("clientid", clientId());

    sendCmdPack(obj);
}

bool ClientSocket::requestCheckout()
{
    if(roomKey().length()) {
        QJsonObject obj;
        obj.insert("request", QString("checkout"));
        obj.insert("type", QString("command"));
        obj.insert("key", roomKey());
        qDebug()<<"checkout with key: "<<roomKey();

        sendCmdPack(obj);
    }
    return roomKey().length();
}

bool ClientSocket::requestCloseRoom()
{
    if(roomKey().length()) {
        QJsonObject obj;
        obj.insert("request", QString("checkout"));
        obj.insert("key", roomKey());
        qDebug()<<"close with key: "<<roomKey();

        sendCmdPack(obj);
    }
    return roomKey().length();
}

bool ClientSocket::requestKickUser(const QString& id)
{
    if(roomKey().length()) {
        QJsonObject obj;
        obj.insert("request", QString("kick"));
        obj.insert("clientid", id);
        obj.insert("key", roomKey());
        qDebug()<<"kick with key: "<<roomKey();

        sendCmdPack(obj);
    }
    return roomKey().length();
}

void ClientSocket::startHeartbeat()
{
    hb_timer_->start(1000 * 60 / HEARTBEAT_RATE);
}

void ClientSocket::stopHeartbeat()
{
    hb_timer_->stop();
}

void ClientSocket::setClientId(const QString &id)
{
    clientid_ = id;
}

QString ClientSocket::clientId() const
{
    return clientid_;
}

void ClientSocket::setUserName(const QString &name)
{
    username_ = name;
}

QString ClientSocket::userName() const
{
    return username_;
}

void ClientSocket::setRoomName(const QString &name)
{
    roomname_ = name;
    archive_.setName(name);
}

QString ClientSocket::roomName() const
{
    return roomname_;
}

void ClientSocket::setCanvasSize(const QSize &size)
{
    canvassize_ = size;
}

QSize ClientSocket::canvasSize() const
{
    return canvassize_;
}

QString ClientSocket::passwd() const
{
    return passwd_;
}

void ClientSocket::setPasswd(const QString &passwd)
{
    passwd_ = passwd;
}

void ClientSocket::sendMessage(const QString &content)
{
    QJsonObject map;
    map.insert("content", content);
    sendData(assamblePack(true, MESSAGE, jsonToBuffer(map)));
}

void ClientSocket::onNewMessage(const QJsonObject &map)
{
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
}

void ClientSocket::onManagerPack(const QJsonObject &data)
{
    router_.onData(data);
}

void ClientSocket::sendHeartbeat()
{
    QJsonObject obj;
    obj.insert("request", QString("heartbeat"));
    int now = QDateTime::currentMSecsSinceEpoch() / 1000;
    obj.insert("timestamp", now);
    sendCmdPack(obj);
}

void ClientSocket::sendDataPack(const QByteArray &content)
{
    sendData(assamblePack(true, DATA, content));
}

void ClientSocket::sendDataPack(const QJsonObject &content)
{
    sendData(assamblePack(true, DATA, jsonToBuffer(content)));
}

void ClientSocket::sendCmdPack(const QJsonObject &content)
{
    sendData(assamblePack(true, COMMAND, jsonToBuffer(content)));
}

void ClientSocket::sendManagerPack(const QJsonObject &content)
{
    sendData(assamblePack(true, MANAGER, jsonToBuffer(content)));
}

void ClientSocket::cancelPendings()
{
    canceled_ = true;
    pool_.clear();
}

void ClientSocket::close()
{
    archive_.flush();
    if(remove_after_close_){
        archive_.remove();
    }
    Socket::close();
}

void ClientSocket::initRouter()
{
    router_.clear();
    router_.regHandler("response",
                       "roomlist",
                       std::bind(&ClientSocket::onResponseRoomList,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "newroom",
                       std::bind(&ClientSocket::onResponseNewRoom,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "login",
                       std::bind(&ClientSocket::onResponseLogin,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("action",
                       "close",
                       std::bind(&ClientSocket::onCommandActionClose,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "close",
                       std::bind(&ClientSocket::onCommandResponseClose,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("action",
                       "clearall",
                       std::bind(&ClientSocket::onCommandActionClearAll,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "clearall",
                       std::bind(&ClientSocket::onCommandResponseClearAll,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "onlinelist",
                       std::bind(&ClientSocket::onCommandResponseOnlinelist,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "checkout",
                       std::bind(&ClientSocket::onCommandResponseCheckout,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("action",
                       "notify",
                       std::bind(&ClientSocket::onActionNotify,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("action",
                       "kick",
                       std::bind(&ClientSocket::onActionKick,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "archivesign",
                       std::bind(&ClientSocket::onResponseArchiveSign,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "archive",
                       std::bind(&ClientSocket::onResponseArchive,
                                 this,
                                 std::placeholders::_1));
    router_.regHandler("response",
                       "heartbeat",
                       std::bind(&ClientSocket::onResponseHeartbeat,
                                 this,
                                 std::placeholders::_1));
}
QString ClientSocket::roomKey() const
{
    return roomKey_;
}

ClientSocket::ParserResult ClientSocket::parserPack(const QByteArray &data)
{
    bool isCompressed = data[0] & 0x1;
    PACK_TYPE pack_type = PACK_TYPE((data[0] & binL<110>::value) >> 0x1);
    QByteArray data_without_header = data.right(data.length()-1);
    if(isCompressed){
        QByteArray tmp = qUncompress(data_without_header);
        if(tmp.isEmpty()){
            qDebug()<<"bad input"<<data_without_header.toHex();
        }
        return ParserResult(pack_type, tmp);
    }else{
        return ParserResult(pack_type, data_without_header);
    }
}

QByteArray ClientSocket::assamblePack(bool compress, PACK_TYPE pt, const QByteArray& bytes)
{
    auto body = bytes;
    if(compress){
        body = qCompress(bytes);
    }
    QByteArray result;
    char header = (compress & 0x1) | (pt << 0x1);
    result.append(header);
    result.append(body);
    return result;
}

void ClientSocket::onPending(const QByteArray& bytes)
{
    if(canceled_){
        return;
    }

    if(poolEnabled_){
        pool_.append(bytes);
    }else{
        if(pool_.count()){
            pool_.append(bytes);
            processPending();
        }else{
            dispatch(bytes);
        }
    }
}

void ClientSocket::processPending()
{
    //    qDebug()<<"process pending";
    while(!pool_.isEmpty() && !canceled_){
        if(dispatch(pool_.first())){
            pool_.pop_front();
        }else{
            break;
        }
    }
}

bool ClientSocket::dispatch(const QByteArray& bytes)
{
    //    qDebug()<<"try to dispatch";
    QByteArray data;
    PACK_TYPE p_type;
    //    std::tie(p_type, data) = parserPack(bytes);
    auto re = parserPack(bytes);
    p_type = re.pack_type;
    data = re.pack_data;
    if(data.isEmpty()){
        return true;
    }
    auto doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    static const auto sig_d = QMetaMethod::fromSignal(&ClientSocket::dataPack);
    static const auto sig_m = QMetaMethod::fromSignal(&ClientSocket::newMessage);
    static const auto sig_c = QMetaMethod::fromSignal(&ClientSocket::cmdPack);
    static const auto sig_n = QMetaMethod::fromSignal(&ClientSocket::managerPack);

    bool ret = true;

    switch(p_type){
    case DATA:
        if(isSignalConnected(sig_d)){
            emit dataPack(obj);
            leftDataLength_ -= bytes.length()+4;
            if(!no_save_)
                archive_.appendData(pack(bytes));
            if(leftDataLength_ <= 0){
                emit archiveLoaded(schedualDataLength_);
            }
            ret = true;
        }else{
            ret = false;
        }
        break;
    case MESSAGE:
        if(isSignalConnected(sig_m)){
            emit msgPack(obj);
            onNewMessage(obj);
            ret = true;
        }else{
            ret = false;
        }
        break;
    case COMMAND:
        if(isSignalConnected(sig_c)){
            emit cmdPack(obj);
            ret = true;
        }else{
            ret = false;
        }
        break;
    case MANAGER:
        if(isSignalConnected(sig_n)){
            //            qDebug()<<obj;
            emit managerPack(obj);
            ret = true;
        }else{
            ret = false;
        }
        break;
    default:
        qWarning()<<"unexpcted json type"<<p_type;
        ret = true;
        break;
    }
    return ret;
}

void ClientSocket::reset()
{
//    mutex_.lock();
    poolEnabled_ = false;
    canceled_ = false;
    username_.clear();
    clientid_.clear();
    roomname_.clear();
    canvassize_ = QSize();
    router_.clear();
    timer_->start(WAIT_TIME);
    pool_.clear();
//    mutex_.unlock();
}


QString ClientSocket::genRoomUrl(const QString& addr,
                                 const quint16 port,
                                 const QString& passwd)
{
    QString raw_url;
    if(passwd.isEmpty()){
        raw_url = QString("%1@%2").arg(port).arg(addr);
    }else{
        raw_url = QString("%1@%2|%3").arg(port).arg(addr).arg(passwd);
    }
    return QString("paintty://")+QString::fromUtf8(raw_url.toUtf8().toBase64());
}

ClientSocket::RoomUrl ClientSocket::decodeRoomUrl(const QString& url)
{
    QRegularExpression re("([A-Za-z]+)://([A-Za-z0-9\\+\\/\\=]+)[#]*(.*)");
    QRegularExpressionMatch match = re.match(url);
    if(!match.hasMatch()){
        qWarning()<<"Invalid url"<<url;
        return RoomUrl();
    }
    RoomUrl url_struct;

    url_struct.scheme = match.captured(1);
    QString decoded_url = QByteArray::fromBase64(match.captured(2).toUtf8());

    url_struct.port = decoded_url.section("@", 0, 0).toInt(0, 10);
    QString leftpart = decoded_url.section("@", 1, 1);
    url_struct.addr = leftpart.section("|", 0, 0);
    url_struct.passwd = leftpart.section("|", 1, 1);
    url_struct.misc = match.captured(3);

    if( url_struct.scheme.isEmpty()
            ||url_struct.addr.isEmpty()
            ||!url_struct.port ){
        qWarning()<<"Invalid url"<<decoded_url;
    }else{
        qDebug()<<"Valid url"
               <<url_struct.scheme
              <<url_struct.port
             <<url_struct.addr
            <<url_struct.passwd
           <<url_struct.misc;
    }

    return url_struct;
}
