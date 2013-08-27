#include "clientsocket.h"
#include "../../common/binary.h"
#include "../misc/archivefile.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QTimer>

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
    archive_(new ArchiveFile(this)),
    no_save_(false),
    remove_after_close_(false)
{
    connect(this, &ClientSocket::newData,
            this, &ClientSocket::onPending);
    connect(timer_, &QTimer::timeout,
            this, &ClientSocket::processPending);
    timer_->start(WAIT_TIME);
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
}

quint64 ClientSocket::schedualDataLength()
{
    return schedualDataLength_;
}

QString ClientSocket::archiveSignature() const
{
    return archive_->signature();
}

void ClientSocket::setArchiveSignature(const QString &as)
{
    archive_->setSignature(as);
    no_save_ = true;
    unpack(archive_->readAll());
    no_save_ = false;
}

quint64 ClientSocket::archiveSize() const
{
    return archive_->size();
}

void ClientSocket::setRoomCloseFlag()
{
    remove_after_close_ = true;
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
    archive_->setName(name);
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

void ClientSocket::close()
{
    if(archive_){
        archive_->flush();
    }
    if(remove_after_close_){
        archive_->remove();
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
    while(!pool_.isEmpty()){
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
                archive_->appendData(pack(bytes));
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
    poolEnabled_ = false;
    username_.clear();
    clientid_.clear();
    roomname_.clear();
    canvassize_ = QSize();
    router_.clear();
    timer_->start(WAIT_TIME);
    pool_.clear();
}

