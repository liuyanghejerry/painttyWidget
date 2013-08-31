#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../../common/binary.h"
//#include "../../common/network/packparser.h"
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

//    typedef std::tuple<PACK_TYPE, QByteArray> ParserResult;
    struct ParserResult
    {
        ParserResult(PACK_TYPE t, const QByteArray& d):
            pack_type(t),
            pack_data(d)
        {
        }

        PACK_TYPE pack_type;
        QByteArray pack_data;

    };
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
    QString passwd() const;
    void setPasswd(const QString &passwd);
    void setPoolEnabled(bool on);
    bool isPoolEnabled();
    void setSchedualDataLength(quint64 length);
    quint64 schedualDataLength();
    void reset();
    QString archiveSignature() const;
    void setArchiveSignature(const QString &as);
    quint64 archiveSize() const;
    void setRoomCloseFlag();
    QString toUrl() const;

    static QString genRoomUrl(const QString& addr,
                              const quint16 port,
                              const QString& passwd=QString());
    struct RoomUrl
    {
        QString scheme;
        QString addr;
        quint16 port;
        QString passwd;
        QString misc;
    };

    static RoomUrl decodeRoomUrl(const QString& url);



signals:
    void dataPack(const QJsonObject&);
    void msgPack(const QJsonObject&);
    void cmdPack(const QJsonObject&);
    void managerPack(const QJsonObject&);
    void newMessage(const QString&);
    void archiveLoaded(int s_size);
    // internal use
    void newPack(PACK_TYPE t, const QByteArray& bytes);
    
public slots:
    void sendMessage(const QString &content);
    void sendDataPack(const QByteArray &content);
    void sendDataPack(const QJsonObject &content);
    void sendCmdPack(const QJsonObject &content);
    void sendManagerPack(const QJsonObject &content);
    void close() Q_DECL_OVERRIDE;
private:
    Q_DISABLE_COPY(ClientSocket)
//    PackParser parser_;
    QString clientid_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    QString passwd_;
    quint64 schedualDataLength_;
    quint64 leftDataLength_;
    Router<> router_;
    QList<QByteArray> pool_;
    bool poolEnabled_;
    QTimer *timer_;
    ArchiveFile* archive_;
    bool no_save_;
    bool remove_after_close_;
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
