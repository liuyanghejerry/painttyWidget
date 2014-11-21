#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "socket.h"
#include "../../common/binary.h"
//#include "../../common/network/packparser.h"
#include "../misc/router.h"
#include <QSize>
#include <QMutex>
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
        ROOM_DISCONNECTED,
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
    void setSchedualDataLength(quint64 length);
    quint64 schedualDataLength();
    void reset();
    QString archiveSignature() const;
    void setArchiveSignature(const QString &as);
    quint64 archiveSize() const;
    void setPoolEnabled(bool on);
    void setRoomCloseFlag();
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
    void roomlistFetched(QHash<QString, QJsonObject>);
    void roomCreated();
    void roomJoined();

    void roomAboutToClose();
    void layerAllCleared();
    void memberListFetched(const QHash<QString, QVariantList> &list);
    void getNotified(const QString &content);
    void getKicked();

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
//    PackParser parser_;
    QString clientid_;
    QString username_;
    QString roomname_;
    QSize canvassize_;
    QString passwd_;
    QString roomKey_;
    quint64 schedualDataLength_;
    quint64 leftDataLength_;
    Router<> router_;
    QList<QByteArray> pool_;
    bool poolEnabled_;
    QTimer *timer_;
    ArchiveFile& archive_;
    bool no_save_;
    bool remove_after_close_;
    bool canceled_;
    const static int WAIT_TIME = 1000;
    const static int HEARTBEAT_RATE = 30; // sends 30 heartbeat packs per min
    QTimer *hb_timer_;
    State state_;
    int roomDelay_;
private slots:
    void initRouter();
    void setClientId(const QString &id);
    void setRoomName(const QString &name);
    void setCanvasSize(const QSize &size);
    ParserResult parserPack(const QByteArray& data);
    QByteArray assamblePack(bool compress, PACK_TYPE pt, const QByteArray& bytes);
    void onPending(const QByteArray& bytes);
    void processPending();
    bool dispatch(const QByteArray& bytes);
    void onNewMessage(const QJsonObject &map);
    void onManagerPack(const QJsonObject &data);
};

#endif // CLIENTSOCKET_H
