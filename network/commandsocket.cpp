#include "commandsocket.h"

CommandSocket * CommandSocket::socket = nullptr;
QVariantMap CommandSocket::info_;

CommandSocket * CommandSocket::cmdSocket()
{
    // notice, this is NOT a thread-safe class.
    if(!CommandSocket::socket){
        CommandSocket::socket = new CommandSocket;
    }
    return CommandSocket::socket;
}

CommandSocket::CommandSocket(QObject *parent) :
    Socket(parent)
{
}

void CommandSocket::setClientId(const QString &id)
{
    info_.insert("clientid", id);
}

QString CommandSocket::clientId()
{
    return info_.value("clientid").toString();
}
