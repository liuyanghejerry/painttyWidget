#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include "socket.h"
#include <QSize>

class CommandSocket : public Socket
{
    Q_OBJECT
public:
    CommandSocket(QObject *parent = 0);
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
    void setDataPort(int port);
    int dataPort() const;
    void setMsgPort(int port);
    int msgPort() const;
signals:
    
public slots:
private:
    CommandSocket *socket;
    QVariantMap info_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    int historysize_;
    int dataPort_;
    int msgPort_;
};

#endif // COMMANDSOCKET_H
