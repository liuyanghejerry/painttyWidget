#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include "socket.h"

class CommandSocket : public Socket
{
    Q_OBJECT
public:
    static CommandSocket * cmdSocket();
signals:
    
public slots:
private:
    CommandSocket(QObject *parent = 0);
    static CommandSocket *socket;
};

#endif // COMMANDSOCKET_H
