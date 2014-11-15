#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../misc/shortcutmanager.h"

class QToolButton;
class BrushSettingsWidget;
class QActionGroup;
class QScriptEngine;
class DeveloperConsole;
class NetworkIndicator;

typedef ShortcutManager::ShortcutType ShT;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    template<typename T, typename U>
    bool regShortcut(const QString& name, T func, U func2);
    template<typename T>
    bool regShortcut(const QString& name, T func);
    template<typename T, typename U>
    bool regShortcut(const QKeySequence& k, T func, U func2);
    template<typename T>
    bool regShortcut(const QKeySequence& k, T func);

public slots:
    void exportAllToFile();
    void exportVisibleToFile();
    void exportAllToClipboard();
    void exportVisibleToClipboard();
    void exportToPSD();
    void resetView();
    void about();
    void onCanvasToolComplete();
    void openConsole();
    void changeToBrush(const QString& brushName);

    /* layer operations */
    void remoteAddLayer(const QString &layerName);
    void addLayer(const QString &name = QString());
    void deleteLayer();
    void deleteLayer(const QString &name);
    void clearLayer(const QString &name);
    void clearAllLayer();

    /* script */
    void evaluateScript(const QString& script);
    void runScript(const QString& script);
signals:
    void sendMessage(QString);
    void brushColorChange(const QColor &color);
    void scriptResult(QString);
protected:
    void closeEvent( QCloseEvent * event ) ;
private:
    void init();
    void stylize();
    void layerWidgetInit();
    void colorGridInit();
    void viewInit();
    void statusBarInit();
    void toolbarInit();
    void shortcutInit();
    void socketInit();
    void routerInit();
    void scriptInit();
    void setNickName();
    void setRoomName();
    void setHistorySize();
    void setCanvasSize();
    QString getRoomKey();
    void requestCloseRoom();
    void requestKickUser(const QString& id);

    Ui::MainWindow *ui;

    QByteArray defaultView;
    QAction *lastBrushAction;
    BrushSettingsWidget *brushSettingControl_;
    QToolBar *toolbar_;
    QActionGroup *brushActionGroup_;
    QToolButton *colorPickerButton_;
    QToolButton *moveToolButton_;
    QScriptEngine* scriptEngine_;
    DeveloperConsole* console_;
    NetworkIndicator* networkIndicator_;
    QHash<QString, bool> keyMap_;

private slots:
    void onServerDisconnected();
    void onNewMessage(const QString &content);
    void onSendPressed();
    void onColorGridDroped(int);
    void onColorGridPicked(int, const QColor &);
    void onBrushTypeChange();
    void onBrushSettingsChanged(const QVariantMap &m);
    void onColorPickerPressed(bool c);
    void onMoveToolPressed(bool c);
    void onPanoramaRefresh();

    void onAboutToClose();
    void onAllLayerCleared();
    void onMemberlistFetched(const QHash<QString, QVariantList> &list);
    void onNotify(const QString &content);
    void onKicked();
//    void onResponseHeartbeat(const QJsonObject &o);
    void onClientSocketError(const int code);
};

#endif // MAINWINDOW_H
