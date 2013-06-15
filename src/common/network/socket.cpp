#include "socket.h"

#include <QTcpSocket>

Socket::Socket(QObject *parent) :
    QObject(parent),
    dataSize(0),
    historySize(0),
    commandStarted(false),
    compressed_(true)
{
    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket,&QTcpSocket::readyRead,
            this,&Socket::onReceipt);
    connect(socket, &QTcpSocket::connected,
            this,&Socket::connected);
    connect(socket, &QTcpSocket::disconnected,
            this,&Socket::disconnected);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this,SIGNAL(error(QAbstractSocket::SocketError)));
}

Socket::~Socket()
{
    socket->close();
}

QHostAddress Socket::address()
{
    return socket->peerAddress();
}

int Socket::port()
{
    return socket->peerPort();
}

quint64 Socket::pastSize()
{
    return historySize;
}

QString Socket::errorString() const
{
    return socket->errorString();
}

void Socket::clearPastSize()
{
    historySize = 0;
}

void Socket::setCompressed(bool b)
{
    compressed_ = b;
}

bool Socket::compressed()
{
    return compressed_;
}

void Socket::connectToHost(const QHostAddress & address, quint16 port)
{
    socket->connectToHost(address, port);
}

void Socket::connectToHost(const QString & hostName, quint16 port)
{
    socket->connectToHost(hostName, port);
}

bool Socket::isConnected()
{
    return (socket->state() == QAbstractSocket::ConnectedState);
}

void Socket::sendData(const QByteArray &content)
{
    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray buffer;
        if(compressed_){
            buffer = qCompress(content);
        }else{
            buffer = content;
        }
        quint32 length = buffer.length()+1; // one more byte to
                                            // store extra information
        uchar c1, c2, c3, c4;
        c1 = length & 0xFF;
        length >>= 8;
        c2 = length & 0xFF;
        length >>= 8;
        c3 = length & 0xFF;
        length >>= 8;
        c4 = length & 0xFF;

        socket->putChar(c4);
        socket->putChar(c3);
        socket->putChar(c2);
        socket->putChar(c1);
        // lowest bit for compressed or not
        if(compressed_){
            socket->putChar(0x1);
        }else{
            socket->putChar(0);
        }
        socket->write(buffer);
    }
}

void Socket::onReceipt()
{
    //    qDebug() << "thread" << QThread::currentThreadId();
    if (commandStarted == false) {
        /* There it's a new message we are receiving.
           To start receiving it we must know its length, i.e. the 2 first bytes */
        if (socket->bytesAvailable() < 5) {
            return;
        }
        /* Ok now we can start */
        commandStarted=true;
        /* getting the length of the message */
        char c1, c2, c3, c4;
        socket->getChar(&c1), socket->getChar(&c2);
        socket->getChar(&c3), socket->getChar(&c4);
        historySize += 4;
        dataSize= (uchar(c1) << 24) + (uchar(c2) << 16)
                + (uchar(c3) << 8) + uchar(c4);
        /* Recursive call to write less code =) */
        onReceipt();
    } else {
        /* Checking if the command is complete! */
        if (socket->bytesAvailable() >= dataSize) {
            QByteArray info = socket->read(dataSize);
            historySize += dataSize;
            bool isCompressed = info[0] & 0x1;
            if(isCompressed){
                auto tmp = qUncompress(
                    info.right(info.length()-1));
                if(!tmp.isEmpty()){
                    emit newData(tmp);
                }else{
                    qDebug()<<"bad input";
                }
            }else{
                emit newData(info.right(info.length()-1));
            }
            commandStarted = false;
            /* Recursive call to spare code =), there may be still data pending */
            onReceipt();
        }
    }
}

void Socket::close()
{
    socket->disconnectFromHost();
}
