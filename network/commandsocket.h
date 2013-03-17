#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include "socket.h"

class CommandSocket : public Socket
{
    Q_OBJECT
public:
    static CommandSocket * cmdSocket();
    static void setClientId(const QString &id);
    static QString clientId();
signals:
    
public slots:
private:
    CommandSocket(QObject *parent = 0);
    static CommandSocket *socket;
    static QVariantMap info_;
};

#endif // COMMANDSOCKET_H
