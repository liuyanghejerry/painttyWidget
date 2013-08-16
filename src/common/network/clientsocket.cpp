#include "clientsocket.h"
#include "../misc/binary.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QTimer>

using std::tuple;

ClientSocket::ClientSocket(QObject *parent) :
    Socket(parent),
    historysize_(0),
    counted_history_(0),
    poolEnabled_(false),
    timer_(new QTimer(this))
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

void ClientSocket::setClientId(const QString &id)
{
    info_.insert("clientid", id);
}

QString ClientSocket::clientId() const
{
    return info_.value("clientid").toString();
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

void ClientSocket::setHistorySize(int size)
{
    historysize_ = size;
    counted_history_ = 0;
}

int ClientSocket::historySize() const
{
    return historysize_;
}

void ClientSocket::sendMessage(const QString &content)
{
    QJsonObject map;
    map.insert("content", content);
    this->sendData(assamblePack(true, MESSAGE, jsonToArray(map)));
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

void ClientSocket::sendCmdPack(const QJsonObject &content)
{
    this->sendData(assamblePack(true, COMMAND, jsonToArray(content)));
}

void ClientSocket::sendManagerPack(const QJsonObject &content)
{
    this->sendData(assamblePack(true, MANAGER, jsonToArray(content)));
}


ClientSocket::ParserResult ClientSocket::parserPack(const QByteArray &data)
{
    bool isCompressed = data[0] & 0x1;
    PACK_TYPE pack_type = PACK_TYPE((data[0] & binL<110>::value) >> 0x1);
    QByteArray data_without_header = data.right(data.length()-1);
    if(isCompressed){
        QByteArray tmp = qUncompress(data_without_header);
        if(tmp.isEmpty()){
            qDebug()<<"bad input";
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
    while(!pool_.isEmpty()){
        if(dispatch(pool_.first())){
            pool_.pop_front();
        }else{
            break;
        }
    }
}

void ClientSocket::tryIncHistory(int s)
{
    if(historysize_){
        counted_history_ += s;
        if(counted_history_ >= historysize_){
            historysize_ = 0;
            emit historyLoaded(counted_history_);
        }
    }
}

bool ClientSocket::dispatch(const QByteArray& bytes)
{
    QByteArray data = bytes.right(bytes.length()-4);
    PACK_TYPE p_type;
    std::tie(p_type, data) = parserPack(data);
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
    qDebug()<<"dispatch"<<p_type<<obj;

    switch(p_type){
    case DATA:
        if(isSignalConnected(sig_d)){
            tryIncHistory(bytes.count());
            emit dataPack(obj);
            ret = true;
        }else{
            ret = false;
        }
        break;
    case MESSAGE:
        if(isSignalConnected(sig_m)){
            tryIncHistory(bytes.count());
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
    info_.clear();
    roomname_.clear();
    canvassize_ = QSize();
    historysize_ = 0;
    counted_history_ = 0;
    router_.clear();
    timer_->start(WAIT_TIME);
    pool_.clear();
}

QByteArray ClientSocket::jsonToArray(const QJsonObject& obj)
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
