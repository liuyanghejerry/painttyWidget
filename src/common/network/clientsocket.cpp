#include "clientsocket.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QTimer>

ClientSocket::ClientSocket(QObject *parent) :
    Socket(parent),
    poolEnabled_(false),
    timer_(new QTimer(this))
{
    connect(this, &ClientSocket::newData,
            this, &ClientSocket::onPending);
    connect(timer_, &QTimer::timeout,
            [this](){
        this->processPending();
    });
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
}

int ClientSocket::historySize() const
{
    return historysize_;
}

void ClientSocket::sendMessage(const QString &content)
{
    QJsonObject map;
    QJsonDocument doc;
    map.insert("content", content);
    map.insert("type", QString("message"));
    doc.setObject(map);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    QByteArray buffer = doc.toJson(QJsonDocument::Compact);
#else
    QByteArray buffer = doc.toJson();
#endif

    this->sendData(buffer);
}

void ClientSocket::onNewMessage(const QJsonObject &map)
{
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
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

bool ClientSocket::dispatch(const QByteArray& bytes)
{
    auto doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();
    if(!obj.contains("type")){
        return true;
    }
    QString j_type = obj.value("type").toString();
    if(j_type == "data"){
        static const auto sig = QMetaMethod::fromSignal(&ClientSocket::dataPack);
        if(isSignalConnected(sig)){
            emit dataPack(obj);
            return true;
        }else{
            return false;
        }
    }else if(j_type == "message"){
        static const auto sig = QMetaMethod::fromSignal(&ClientSocket::newMessage);
        if(isSignalConnected(sig)){
            emit msgPack(obj);
            onNewMessage(obj);
            return true;
        }else{
            return false;
        }
    }else if(j_type == "command"){
        static const auto sig = QMetaMethod::fromSignal(&ClientSocket::cmdPack);
        if(isSignalConnected(sig)){
            emit cmdPack(obj);
            return true;
        }else{
            return false;
        }
    }else if(j_type == "manager"){
        static const auto sig = QMetaMethod::fromSignal(&ClientSocket::managerPack);
        if(isSignalConnected(sig)){
            emit managerPack(obj);
            return true;
        }else{
            return false;
        }
    }else{
        qWarning()<<"unexpcted json type"<<j_type;
        return true;
    }
}

