#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QHostAddress>

class QTcpSocket;

class Socket : public QObject
{
    Q_OBJECT
public:
    explicit Socket(QObject *parent = 0);
    ~Socket();
    void connectToHost(const QHostAddress& address, quint16 port);
    void connectToHost(const QString & hostName, quint16 port);
    bool waitForConnected(int msecs = 30000);
    bool isConnected();
    QHostAddress address() const;
    bool isIPv4Address() const;
    bool isIPv6Address() const;
    int port() const;
    QString errorString() const;
    
signals:
    void disconnected();
    void connected();
    void newData(const QByteArray &);
    void error(QAbstractSocket::SocketError socketError);
    
public slots:
    void sendData(const QByteArray &data);
    virtual void close();
protected slots:
    void onReceipt();
protected:
    QByteArray pack(const QByteArray &content);
    void unpack(const QByteArray &content);
    quint32 dataSize;
    QTcpSocket *socket;
    bool commandStarted;
private:
    Q_DISABLE_COPY(Socket)
};

#endif // SOCKET_H
