#include "commandsocket.h"

CommandSocket * CommandSocket::socket = 0;
QString CommandSocket::clientid_ = QString();

CommandSocket * CommandSocket::cmdSocket()
{
    // notice, this is not a thread-safe class.
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
    CommandSocket::clientid_ = id;
}

QString CommandSocket::clientId()
{
    return CommandSocket::clientid_;
}
