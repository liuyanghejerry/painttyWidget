#include "messagesocket.h"

MessageSocket::MessageSocket(QObject *parent) :
    Socket(parent)
{
//    serializer.setIndentMode(QJson::IndentCompact);
    connect(this,SIGNAL(newData(QByteArray)),
            this,SLOT(newMessage(QByteArray)));
}

MessageSocket::~MessageSocket()
{
    close();
}

void MessageSocket::sendMessage(const QString &content)
{
    QVariantMap map;
    map.insert("content", content);
    QByteArray buffer = toJson(QVariant(map));

    this->sendData(buffer);
}

void MessageSocket::newMessage(const QByteArray &array)
{
    QVariantMap map = fromJson(array).toMap();
    if(!map.contains("content")) return;

    QString string = map["content"].toString();
    emit newMessage(string);
}

QByteArray MessageSocket::toJson(const QVariant &m)
{
    return QJsonDocument::fromVariant(m).toJson();
}

QVariant MessageSocket::fromJson(const QByteArray &d)
{
    return QJsonDocument::fromJson(d).toVariant();
}
