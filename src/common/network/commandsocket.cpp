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

QString CommandSocket::clientId()
{
    return info_.value("clientid").toString();
}
