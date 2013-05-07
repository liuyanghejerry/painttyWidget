#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include "socket.h"


class MessageSocket : public Socket
{
    Q_OBJECT
public:
    explicit MessageSocket(QObject *parent = 0);
    ~MessageSocket();
    
signals:
    void newMessage(QString);
    
public slots:
    void sendMessage(const QString &content);
    void onNewMessage(const QByteArray &array);
};

#endif // SOCKETTHREAD_H
