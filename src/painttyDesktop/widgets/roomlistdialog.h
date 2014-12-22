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
    void requestNewRoom(const QJsonObject &m);
    void requestRoomList();
    void filterRoomList();
    void connectRoomByUrl(const QString& url);
private slots:
    void onManagerServerConnected();
    void onRoomlist(const QHash<QString, QJsonObject> &obj);
    void onManagerServerClosed();
    void onNewRoomCreated();
    void onClientSocketError(int errcode);
    void loadNick();
    void saveNick();
    void openConfigure();
protected:
    void hideEvent(QHideEvent *e);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    
    bool collectUserInfo();
    void tryJoinRoomManually();
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
    QString nickName_;
    QTimer *timer;
    NewRoomWindow *newRoomWindow;
    QHash<QString, QJsonObject> roomsInfo;
    State state_;
    void tableInit();
    void connectToManager();
    QByteArray loadClientId();
};

#endif // ROOMLISTDIALOG_H
