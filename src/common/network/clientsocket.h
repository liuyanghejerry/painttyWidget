#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../misc/router.h"
#include <QSize>
class QTimer;

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
    void setPoolEnabled(bool on);
    bool isPoolEnabled();
    void reset();
signals:
    void dataPack(const QJsonObject&);
    void msgPack(const QJsonObject&);
    void cmdPack(const QJsonObject&);
    void managerPack(const QJsonObject&);
    void newMessage(const QString&);
    
public slots:
    void sendMessage(const QString &content);
private:
    QVariantMap info_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    int historysize_;
    Router<> router_;
    QList<QByteArray> pool_;
    bool poolEnabled_;
    QTimer *timer_;
    const static int WAIT_TIME = 1000;
private slots:
    void onPending(const QByteArray& bytes);
    void processPending();
    bool dispatch(const QByteArray& bytes);
    void onNewMessage(const QJsonObject &map);
};

#endif // CLIENTSOCKET_H
