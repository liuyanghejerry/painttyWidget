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
    canceled_(false)
{
    connect(this, &ClientSocket::newData,
            this, &ClientSocket::onPending);
    connect(timer_, &QTimer::timeout,
            this, &ClientSocket::processPending);
    timer_->start(WAIT_TIME);
}

void ClientSocket::setPoolEnabled(bool on)
{
    mutex_.lock();
    poolEnabled_ = on;
    if(!poolEnabled_){
        timer_->start(WAIT_TIME);
        processPending();
    }else{
        timer_->stop();
    }
    mutex_.unlock();
}

bool ClientSocket::isPoolEnabled()
{
    return poolEnabled_;
}

void ClientSocket::setSchedualDataLength(quint64 length)
{
    mutex_.lock();
    leftDataLength_ = schedualDataLength_ = length;
    mutex_.unlock();
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
    mutex_.lock();
    archive_.setSignature(as);
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool skip_replay = settings.value("canvas/skip_replay", false).toBool();
    if(!skip_replay){
        no_save_ = true;
        unpack(archive_.readAll());
        no_save_ = false;
    }else{
        leftDataLength_ -= archive_.size();
    }
    mutex_.unlock();
}

quint64 ClientSocket::archiveSize() const
{
    return archive_.size();
}

void ClientSocket::setRoomCloseFlag()
{
    mutex_.lock();
    remove_after_close_ = true;
    mutex_.unlock();
}

QString ClientSocket::toUrl() const
{
    return this->genRoomUrl(this->address().toString(),
                            this->port(),
                            this->passwd());
}

void ClientSocket::setClientId(const QString &id)
{
    mutex_.lock();
    clientid_ = id;
    mutex_.unlock();
}

QString ClientSocket::clientId() const
{
    return clientid_;
}

void ClientSocket::setUserName(const QString &name)
{
    mutex_.lock();
    username_ = name;
    mutex_.unlock();
}

QString ClientSocket::userName() const
{
    return username_;
}

void ClientSocket::setRoomName(const QString &name)
{
    mutex_.lock();
    roomname_ = name;
    archive_.setName(name);
    mutex_.unlock();
}

QString ClientSocket::roomName() const
{
    return roomname_;
}

void ClientSocket::setCanvasSize(const QSize &size)
{
    mutex_.lock();
    canvassize_ = size;
    mutex_.unlock();
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
    mutex_.lock();
    passwd_ = passwd;
    mutex_.unlock();
}

void ClientSocket::sendMessage(const QString &content)
{
    mutex_.lock();
    QJsonObject map;
    map.insert("content", content);
    this->sendData(assamblePack(true, MESSAGE, jsonToBuffer(map)));
    mutex_.unlock();
}

void ClientSocket::onNewMessage(const QJsonObject &map)
{
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
}

void ClientSocket::sendDataPack(const QByteArray &content)
{
    mutex_.lock();
    this->sendData(assamblePack(true, DATA, content));
    mutex_.unlock();
}

void ClientSocket::sendDataPack(const QJsonObject &content)
{
    mutex_.lock();
    this->sendData(assamblePack(true, DATA, jsonToBuffer(content)));
    mutex_.unlock();
}

void ClientSocket::sendCmdPack(const QJsonObject &content)
{
    mutex_.lock();
    this->sendData(assamblePack(true, COMMAND, jsonToBuffer(content)));
    mutex_.unlock();
}

void ClientSocket::sendManagerPack(const QJsonObject &content)
{
    mutex_.lock();
    this->sendData(assamblePack(true, MANAGER, jsonToBuffer(content)));
    mutex_.unlock();
}

void ClientSocket::cancelPendings()
{
    mutex_.lock();
    canceled_ = true;
    pool_.clear();
    mutex_.unlock();
}

void ClientSocket::close()
{
    mutex_.lock();
    archive_.flush();
    if(remove_after_close_){
        archive_.remove();
    }
    Socket::close();
    mutex_.unlock();
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
    mutex_.lock();
    poolEnabled_ = false;
    canceled_ = false;
    username_.clear();
    clientid_.clear();
    roomname_.clear();
    canvassize_ = QSize();
    router_.clear();
    timer_->start(WAIT_TIME);
    pool_.clear();
    mutex_.unlock();
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
