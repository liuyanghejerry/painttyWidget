#include "clientsocket.h"
#include <QJsonDocument>
#include <QJsonObject>

ClientSocket::ClientSocket(QObject *parent) :
    Socket(parent)
{
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

void ClientSocket::dispatch(const QByteArray& bytes)
{
    auto doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();
    if(!obj.contains("type")){
        return;
    }
    QString j_type = obj.value("type").toString();
    if(j_type == "data"){
        emit dataPack(obj);
    }else if(j_type == "message"){
        emit msgPack(obj);
        onNewMessage(obj);
    }else if(j_type == "command"){
        emit cmdPack(obj);
    }else if(j_type == "manager"){
        emit managerPack(obj);
    }else{
        qWarning()<<"unexpcted json type"<<j_type;
    }
}
