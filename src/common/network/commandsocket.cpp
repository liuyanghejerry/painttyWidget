#include "commandsocket.h"
#include <QDebug>

CommandSocket::CommandSocket(QObject *parent) :
    Socket(parent)
{
    qDebug()<<"CommandSocket init.";
}

void CommandSocket::setClientId(const QString &id)
{
    info_.insert("clientid", id);
}

QString CommandSocket::clientId() const
{
    return info_.value("clientid").toString();
}

void CommandSocket::setUserName(const QString &name)
{
    username_ = name;
}

QString CommandSocket::userName() const
{
    return username_;
}

void CommandSocket::setRoomName(const QString &name)
{
    roomname_ = name;
}

QString CommandSocket::roomName() const
{
    return roomname_;
}

void CommandSocket::setCanvasSize(const QSize &size)
{
    canvassize_ = size;
}

QSize CommandSocket::canvasSize() const
{
    return canvassize_;
}

void CommandSocket::setHistorySize(int size)
{
    historysize_ = size;
}

int CommandSocket::historySize() const
{
    return historysize_;
}

void CommandSocket::setDataPort(int port)
{
    dataPort_ = port;
}

int CommandSocket::dataPort() const
{
    return dataPort_;
}
void CommandSocket::setMsgPort(int port)
{
    msgPort_ = port;
}

int CommandSocket::msgPort() const
{
    return msgPort_;
}
