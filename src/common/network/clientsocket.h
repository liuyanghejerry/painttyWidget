#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../misc/binary.h"
#include "../misc/router.h"
#include <QSize>
class QTimer;
class ArchiveFile;

class ClientSocket : public Socket
{
    Q_OBJECT

    enum PACK_TYPE : unsigned char {
        MANAGER = binL<0>::value,
        COMMAND = binL<1>::value,
        DATA = binL<10>::value,
        MESSAGE = binL<11>::value
    };

    typedef std::tuple<PACK_TYPE, QByteArray> ParserResult;
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
    void setPoolEnabled(bool on);
    bool isPoolEnabled();
    void setSchedualDataLength(quint64 length);
    void reset();
    QString archiveSignature() const;
    void setArchiveSignature(const QString &as);
    quint64 archiveSize() const;

signals:
    void dataPack(const QJsonObject&);
    void msgPack(const QJsonObject&);
    void cmdPack(const QJsonObject&);
    void managerPack(const QJsonObject&);
    void newMessage(const QString&);
    void historyLoaded(int s_size);
    // internal use
    void newPack(PACK_TYPE t, const QByteArray& bytes);
    
public slots:
    void sendMessage(const QString &content);
    void sendDataPack(const QByteArray &content);
    void sendCmdPack(const QJsonObject &content);
    void sendManagerPack(const QJsonObject &content);
private:
    Q_DISABLE_COPY(ClientSocket)
    QString clientid_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    quint64 schedualDataLength_;
    quint64 leftDataLength_;
    QString archiveSignature_;
    Router<> router_;
    QList<QByteArray> pool_;
    bool poolEnabled_;
    QTimer *timer_;
    ArchiveFile* archive_;
    const static int WAIT_TIME = 1000;
private slots:
    ParserResult parserPack(const QByteArray& data);
    QByteArray assamblePack(bool compress, PACK_TYPE pt, const QByteArray& bytes);
    void onPending(const QByteArray& bytes);
    void processPending();
    bool dispatch(const QByteArray& bytes);
    void onNewMessage(const QJsonObject &map);
};

#endif // CLIENTSOCKET_H
