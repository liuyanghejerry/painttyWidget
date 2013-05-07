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
    void connectToHost(const QHostAddress & address, quint16 port);
    void connectToHost(const QString & hostName, quint16 port);
    bool isConnected();
    QHostAddress address();
    int port();
    quint64 pastSize();
    void clearPastSize();
    void setCompressed(bool b);
    bool compressed();
    QString errorString() const;
    
signals:
    void disconnected();
    void connected();
    void newData(const QByteArray &);
    void error(QAbstractSocket::SocketError socketError);
    
public slots:
    void sendData(const QByteArray &data);
    void close();
protected slots:
    void onReceipt();
protected:
    quint32 dataSize;
    quint64 historySize;
    bool commandStarted;
    QTcpSocket *socket;
    bool compressed_;
    
};

#endif // SOCKET_H
