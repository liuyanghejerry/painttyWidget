#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QShortcut>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>
#include <QFileDialog>
#include <QFile>
#include <QClipboard>
#include <QJsonDocument>
#include <QSettings>

#include "common.h"
#include "layerwidget.h"
#include "colorgrid.h"
#include "developerconsole.h"
#include "aboutdialog.h"
#include "../network/messagesocket.h"
#include "../network/datasocket.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
    void cmdSocketInit(const QHostAddress &add, int port);
    void socketInit(int dataPort, int msgPort);
    void setNickName(const QString &name);
    void setRoomName(const QString &name);
    void setHistorySize(const quint64 &size);
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
    QByteArray toJson(const QVariant &m);
    QVariant fromJson(const QByteArray &d);

    Ui::MainWindow *ui;
    MessageSocket msgSocket;
    DataSocket dataSocket;
    DataSocket cmdSocket;

    QString nickName_;
    QString roomName_;
    quint64 historySize_;
    QShortcut *devConsoleShortCut;
    QByteArray defaultView;
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
    void onCmdServerConnected();
    void onCmdServerDisconnected();
    void onCanvasMoveBy(const QPoint &p);
    void onPanoramaRefresh();

};

#endif // MAINWINDOW_H
