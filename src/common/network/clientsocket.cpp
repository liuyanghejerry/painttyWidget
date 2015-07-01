#include "clientsocket.h"
#include "../../common/binary.h"
#include "../../common/common.h"
#include "../misc/archivefile.h"
#include "../misc/singleton.h"
#include <QJsonDocument>
#include <QApplication>
#include <QJsonObject>
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
    hb_timer_(new QTimer(this))
{
    connect(this, &ClientSocket::newData,
            this, &ClientSocket::onPending);
    connect(timer_, &QTimer::timeout,
            this, &ClientSocket::processPending);
    timer_->start(WAIT_TIME);

    connect(hb_timer_, &QTimer::timeout,
            this, &ClientSocket::sendHeartbeat);
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
    return this->genRoomUrl(this->address().toString(),
                            this->port(),
                            this->passwd());
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
    this->sendData(assamblePack(true, MESSAGE, jsonToBuffer(map)));
}

void ClientSocket::onNewMessage(const QJsonObject &map)
{
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
}

void ClientSocket::sendHeartbeat()
{
    QJsonObject obj;
    obj.insert("request", QString("heartbeat"));
    int now = QDateTime::currentMSecsSinceEpoch() / 1000;
    obj.insert("timestamp", now);
    this->sendCmdPack(obj);
}

void ClientSocket::sendDataPack(const QByteArray &content)
{
    this->sendData(assamblePack(true, DATA, content));
}

void ClientSocket::sendDataPack(const QJsonObject &content)
{
    this->sendData(assamblePack(true, DATA, jsonToBuffer(content)));
}

void ClientSocket::sendCmdPack(const QJsonObject &content)
{
    this->sendData(assamblePack(true, COMMAND, jsonToBuffer(content)));
}

void ClientSocket::sendManagerPack(const QJsonObject &content)
{
    this->sendData(assamblePack(true, MANAGER, jsonToBuffer(content)));
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
