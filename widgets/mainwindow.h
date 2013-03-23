#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <tuple>

#include <QMainWindow>
#include <QCloseEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QToolButton>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QShortcut>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <QCryptographicHash>

#include "../common.h"
#include "../misc/singleshortcut.h"
#include "layerwidget.h"
#include "colorgrid.h"
#include "aboutdialog.h"
#include "brushwidthwidget.h"
#include "../network/messagesocket.h"
#include "../network/datasocket.h"
#include "../network/commandsocket.h"
#include "../misc/router.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(const QSize& canvasSize, QWidget *parent = 0);
    ~MainWindow();
    void init();
    void socketInit(int dataPort, int msgPort);
    void setNickName(const QString &name);
    void setRoomName(const QString &name);
    void setHistorySize(const quint64 &size);
    void setCanvasSize(const QSize &size);
public slots:
    void exportAllToFile();
    void exportVisibleToFile();
    void exportAllToClipboard();
    void exportVisibleToClipboard();
    void resetView();
    void about();
    void onPickColorComplete();

    /* layer operations */
    void remoteAddLayer(const QString &layerName);
    void addLayer(const QString &name = QString());
    void deleteLayer();
    void deleteLayer(const QString &name);
    void clearLayer(const QString &name);
    void clearAllLayer();
signals:
    void sendMessage(QString);
    void brushColorChange(const QColor &color);
protected:
    void closeEvent( QCloseEvent * event ) ;
private:
    void stylize();
    void layerWidgetInit();
    void colorGridInit();
    void viewInit();
    void toolbarInit();
    void shortcutInit();
    void cmdSocketRouterInit();
    QVariant getRoomKey();
    void requestOnlinelist();
    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);

    Ui::MainWindow *ui;
    MessageSocket msgSocket;
    DataSocket dataSocket;

    QString nickName_;
    QString roomName_;
    quint64 historySize_;
    QSize canvasSize_;
    QByteArray defaultView;
    QAction *lastBrushAction;
    Router<> cmdRouter_;
    BrushWidthWidget *widthControl_;
    QToolBar *toolbar_;
    QActionGroup *brushActionGroup_;
    QToolButton *colorPickerButton_;
private slots:
    void onServerConnected();
    void onServerDisconnected();
    void onNewMessage(const QString &content);
    void onSendPressed();
    void onColorGridDroped(int);
    void onColorGridPicked(int, const QColor &);
    void onBrushTypeChange();
    void onBrushSettingsChanged(const QVariantMap &m);
    void onColorPickerPressed(bool c);
    void onCmdServerDisconnected();
    void onCmdServerData(const QByteArray &data);
    void onPanoramaRefresh();
    void onCommandActionClose(const QJsonObject &);
    void onCommandResponseClose(const QJsonObject &m);
    void onCommandResponseClearAll(const QJsonObject &m);
    void onCommandActionClearAll(const QJsonObject &);
    void onCommandResponseOnlinelist(const QJsonObject &o);

};

#endif // MAINWINDOW_H
