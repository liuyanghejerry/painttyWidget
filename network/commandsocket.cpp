#include "commandsocket.h"

CommandSocket * CommandSocket::socket = 0;

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
