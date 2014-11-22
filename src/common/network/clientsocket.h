#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../../common/binary.h"
#include "../misc/router.h"
#include <QSize>
#include <QMutex>
#include <QAtomicInt>
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

    enum State {
        INIT,
        CONNECTING_MANAGER,
        MANAGER_CONNECTED,
        MANAGER_EXITED,
        MANAGER_DISCONNECTED,
        REQUESTING_ROOMLIST,
        REQUESTING_NEWROOM,
        CONNECTING_ROOM,
        JOINING_ROOM,
        ROOM_JOINED,
        ROOM_EXITED,
        ROOM_OFFLINE,
        ROOM_KICKED
    };

    explicit ClientSocket(QObject *parent = 0);
    State currentState() const;
    QString clientId() const;
    void setUserName(const QString &name);
    QString userName() const;
    QString roomName() const;
    QSize canvasSize() const;
    QString passwd() const;
    void setPasswd(const QString &passwd);
    bool isPoolEnabled();
    quint64 schedualDataLength();
    void reset();
    QString archiveSignature() const;
    quint64 archiveSize() const;
    void setPoolEnabled(bool on);
    void setRoomCloseFlag();
    int getDelay() const;
    QString roomKey() const;
    QString toUrl() const;

    void connectToManager(const QHostAddress& addr,
                          const int port);
    void requestRoomList();
    void requestNewRoom(const QJsonObject &m);
    void tryJoinRoom(const QHostAddress &addr,
                     const int port);
    void tryJoinRoom(const QString& url);
    void onResponseRoomList(const QJsonObject &);
    void onResponseNewRoom(const QJsonObject &);
    void onResponseLogin(const QJsonObject &);
    void requestArchive();
    void requestArchiveSign();
    void exitFromRoom();

    void requestOnlinelist();
    bool requestCheckout();
    bool requestCloseRoom();
    bool requestKickUser(const QString &id);

    void onCommandActionClose(const QJsonObject &);
    void onCommandResponseClose(const QJsonObject &m);
    void onCommandResponseClearAll(const QJsonObject &m);
    void onCommandResponseCheckout(const QJsonObject &m);
    void onCommandActionClearAll(const QJsonObject &);
    void onCommandResponseOnlinelist(const QJsonObject &o);
    void onActionNotify(const QJsonObject &o);
    void onActionKick(const QJsonObject &o);
    void onResponseArchiveSign(const QJsonObject &o);
    void onResponseArchive(const QJsonObject &o);
    void onResponseHeartbeat(const QJsonObject &o);

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
    void clientSocketError(int);
    void requestUnauthed();
    void managerConnected();
    void roomListFetched(QHash<QString, QJsonObject>);
    void roomCreated();
    void roomJoined();
    void roomOfflined();

    void roomAboutToClose();
    void layerAllCleared();
    void memberListFetched(const QHash<QString, QVariantList> &list);
    void getNotified(const QString &content);
    void getKicked();
    void delayGet(int);

    void newClientId(const QString&);

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
    void cancelPendings();
    void sendHeartbeat();
    void startHeartbeat();
    void stopHeartbeat();
    void close() Q_DECL_OVERRIDE;
private:
    Q_DISABLE_COPY(ClientSocket)
    QString clientid_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    QString passwd_;
    QString roomKey_;
    quint64 schedualDataLength_;
    quint64 leftDataLength_;
    Router<> router_;
    QList<QByteArray> inputPool_;
    QList<QByteArray> outputPool_;
    State state_;
    QAtomicInt roomDelay_;
    QTimer *loopTimer_;
    QTimer *heartBeatTimer_;
    ArchiveFile& archive_;
    bool poolEnabled_;
    bool no_save_;
    bool remove_after_close_;
    bool canceled_;
    const static int WAIT_TIME = 1000;
    const static int HEARTBEAT_RATE = 30; // sends 30 heartbeat packs per min
private slots:
    void initRouter();
    void setClientId(const QString &id);
    void setRoomName(const QString &name);
    void setCanvasSize(const QSize &size);
    void setArchiveSignature(const QString &as);
    void setSchedualDataLength(quint64 length);
    ParserResult parserPack(const QByteArray& data);
    QByteArray assamblePack(bool compress, PACK_TYPE pt, const QByteArray& bytes);
    void onInputPending(const QByteArray& bytes);
    void processInputPending();
    bool dispatch(const QByteArray& bytes);
    void onNewMessage(const QJsonObject &map);
    void onManagerPack(const QJsonObject &data);
    void trySendData(const QByteArray &content);
    void processOutputPending();
    void onServerDisconnected();
    void tryRejoinRoom();
};

#endif // CLIENTSOCKET_H
