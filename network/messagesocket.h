#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
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
    void newMessage(const QByteArray &array);
private:
    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);
};

#endif // SOCKETTHREAD_H
