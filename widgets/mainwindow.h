#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../network/messagesocket.h"
#include "../network/datasocket.h"
#include "../misc/router.h"

class QToolButton;
class BrushSettingsWidget;
class QActionGroup;

typedef Router<> RegularRouter;

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
    void requestCheckout();
    static QByteArray toJson(const QVariant &m);
    static QVariant fromJson(const QByteArray &d);
    QMenu* languageMenu();

    Ui::MainWindow *ui;
    MessageSocket msgSocket;
    DataSocket dataSocket;

    QString nickName_;
    QString roomName_;
    quint64 historySize_;
    QSize canvasSize_;
    QByteArray defaultView;
    QAction *lastBrushAction;
    RegularRouter cmdRouter_;
    BrushSettingsWidget *brushSettingControl_;
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
    void onCommandResponseCheckout(const QJsonObject &m);
    void onCommandActionClearAll(const QJsonObject &);
    void onCommandResponseOnlinelist(const QJsonObject &o);
    void onActionNotify(const QJsonObject &o);

};

#endif // MAINWINDOW_H
