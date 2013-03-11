#ifndef ROOMLISTDIALOG_H
#define ROOMLISTDIALOG_H

#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QTableWidgetItem>
#include <QHostAddress>
#include <QVariant>
#include <QJsonDocument>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>

#include "../common.h"
#include "../network/commandsocket.h"
#include "newroomwindow.h"

namespace Ui {
class RoomListDialog;
}

class RoomListDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RoomListDialog(QWidget *parent = 0);
    ~RoomListDialog();
    QString roomName() const;
    QString nick() const;
    int historySize() const;
    QSize canvasSize() const {return canvasSize_;}
    int dataPort() const {return dataPort_;}
    int msgPort() const {return msgPort_;}
public slots:
    void requestJoin();
    void requestNewRoom(const QVariantMap &m);
    void requestRoomList();
    void onRoomListUpdate(const QByteArray &array);
    void onServerClosed();
    void filterRoomList();
private slots:
//    void onNewRoomButtonPressed();
    void onCmdServerConnected();
    void onCmdServerData(const QByteArray &array);
    void loadNick();
    void saveNick();
protected:
    void hideEvent(QHideEvent *);
    void closeEvent(QCloseEvent *e);
    
private:
    Ui::RoomListDialog *ui;
    int dataPort_;
    int msgPort_;
    quint64 historySize_;
    QSize canvasSize_;
    Socket *socket;
    QString roomName_;
    QString nickName_;
    QTimer *timer;
    NewRoomWindow *newRoomWindow;
    QHash<QString, QVariantMap> roomsInfo;
    void tableInit();
    void socketInit();
};

#endif // ROOMLISTDIALOG_H
