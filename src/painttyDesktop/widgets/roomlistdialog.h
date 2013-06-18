#ifndef ROOMLISTDIALOG_H
#define ROOMLISTDIALOG_H

#include <QDialog>

#include "../misc/router.h"

class Socket;
class NewRoomWindow;

namespace Ui {
class RoomListDialog;
}

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
    void onServerData(const QByteArray &array);
    void onServerClosed();
    void filterRoomList();
private slots:
    void onManagerServerConnected();
    void onCmdServerConnected();
    void onCmdServerData(const QByteArray &array);
    void onNewRoomRespnse(const QJsonObject &m);
    void loadNick();
    void saveNick();
    void openConfigure();
    void commitToGlobal();
protected:
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *e);
    
    QString roomName() const;
    QString nick() const;
    int historySize() const;
    QSize canvasSize() const {return canvasSize_;}
    int dataPort() const {return dataPort_;}
    int msgPort() const {return msgPort_;}
    void connectRoomByPort(const int &p);
    void tryJoinRoomManually();
    void tryJoinRoomAutomated();
private:
    enum State{
        Error = -999,
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
    int dataPort_;
    int msgPort_;
    quint64 historySize_;
    QSize canvasSize_;
    Socket *socket;
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
    void socketInit();
    QByteArray loadClientId();
    void onManagerResponseRoomlist(const QJsonObject &obj);
};

#endif // ROOMLISTDIALOG_H
