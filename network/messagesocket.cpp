#include "messagesocket.h"

#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

MessageSocket::MessageSocket(QObject *parent) :
    Socket(parent)
{
    connect(this,&Socket::newData,
            this,&MessageSocket::onNewMessage);
}

MessageSocket::~MessageSocket()
{
    close();
}

void MessageSocket::sendMessage(const QString &content)
{
    QJsonObject map;
    QJsonDocument doc;
    map.insert("content", content);
    doc.setObject(map);
    QByteArray buffer = doc.toJson();

    this->sendData(buffer);
}

void MessageSocket::onNewMessage(const QByteArray &array)
{
    QJsonObject map = QJsonDocument::fromJson(array).object();
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
}
