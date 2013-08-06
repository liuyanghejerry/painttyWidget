#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../misc/router.h"
#include <QSize>

class ClientSocket : public Socket
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = 0);
    void setClientId(const QString &id);
    QString clientId() const;
    void setUserName(const QString &name);
    QString userName() const;
    void setRoomName(const QString &name);
    QString roomName() const;
    void setCanvasSize(const QSize &size);
    QSize canvasSize() const;
    void setHistorySize(int size);
    int historySize() const;
signals:
    void dataPack(const QJsonObject&);
    void msgPack(const QJsonObject&);
    void cmdPack(const QJsonObject&);
    void managerPack(const QJsonObject&);
    void newMessage(const QString&);
    
public slots:
    void onNewMessage(const QJsonObject &map);
    void sendMessage(const QString &content);
private:
    QVariantMap info_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    int historysize_;
    Router<> router_;
    void dispatch(const QByteArray& bytes);
};

#endif // CLIENTSOCKET_H
