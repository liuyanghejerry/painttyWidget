#ifndef ROOMLISTDIALOG_H
#define ROOMLISTDIALOG_H

#include <QDialog>

#include "../misc/router.h"
#include "../network/clientsocket.h"

class NewRoomWindow;

namespace Ui {
class RoomListDialog;
}

// TODO: re-arrange APIs

class RoomListDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RoomListDialog(QWidget *parent = 0);
    ~RoomListDialog();

public slots:
    void requestJoin();
    void requestNewRoom(const QJsonObject &m);
    void requestRoomList();
    void filterRoomList();
    void connectRoomByUrl(const QString& url);
private slots:
    void onManagerServerConnected();
    void onManagerResponseRoomlist(const QJsonObject &obj);
    void onManagerData(const QJsonObject &array);
    void onManagerServerClosed();
    void onCmdServerConnected();
    void onCmdData(const QJsonObject &map);
    void onNewRoomRespnse(const QJsonObject &m);
    void loadNick();
    void saveNick();
    void openConfigure();
protected:
    void hideEvent(QHideEvent *e);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    
    bool collectUserInfo();
    void connectRoomByPort(const int &p);
    void tryJoinRoomManually();
    void tryJoinRoomAutomated();
    void tryJoinRoomByUrl(const ClientSocket::RoomUrl& url);
private:
    enum State{
        Error = -999,
        ConnectFailed,
        Init,
        Ready,
        ManagerConnecting = 0,
        ManagerConnected,
        RequestingList,
        RequestingNewRoom,
        RoomConnecting,
        RoomConnected,
        RoomJoined
    };

    Ui::RoomListDialog *ui;
    static const int REFRESH_TIME = 10000;
    QString roomName_;
    QString wantedRoomName_;
    QString wantedPassword_;
    QString nickName_;
    QTimer *timer;
    NewRoomWindow *newRoomWindow;
    QHash<QString, QJsonObject> roomsInfo;
    Router<> managerSocketRouter_;
    QByteArray clientId_;
    State state_;
    void tableInit();
    void connectToManager();
    void routerInit();
    QByteArray loadClientId();
};

#endif // ROOMLISTDIALOG_H
