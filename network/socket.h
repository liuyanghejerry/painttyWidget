#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>

class Socket : public QObject
{
    Q_OBJECT
public:
    explicit Socket(QObject *parent = 0);
    ~Socket();
    void connectToHost(const QHostAddress & address, quint16 port);
    void connectToHost(const QString & hostName, quint16 port);
    bool isConnected();
    QHostAddress address(){return socket->peerAddress();}
    int port(){return socket->peerPort();}
    void setCompressed(bool b);
    bool compressed();
    
signals:
    void disconnected();
    void connected();
    void newData(const QByteArray &);
    
public slots:
    void sendData(const QByteArray &data);
    void close();
protected slots:
    void onReceipt();
protected:
    quint32 dataSize;
    bool commandStarted;
    QTcpSocket *socket;
    bool compressed_;
    
};

#endif // SOCKET_H
