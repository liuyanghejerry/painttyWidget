#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include "socket.h"

class CommandSocket : public Socket
{
    Q_OBJECT
public:
    CommandSocket(QObject *parent = 0);
    void setClientId(const QString &id);
    QString clientId();
signals:
    
public slots:
private:
    CommandSocket *socket;
    QVariantMap info_;
};

#endif // COMMANDSOCKET_H
