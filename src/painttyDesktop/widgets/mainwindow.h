#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../misc/router.h"
#include "../misc/shortcutmanager.h"

class QToolButton;
class BrushSettingsWidget;
class QActionGroup;
class QScriptEngine;
class DeveloperConsole;

typedef Router<> RegularRouter;
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
    void onPickColorComplete();
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
    void toolbarInit();
    void shortcutInit();
    void socketInit();
    void cmdRouterInit();
    void scriptInit();
    void setNickName();
    void setRoomName();
    void setHistorySize();
    void setCanvasSize();
    QString getRoomKey();
    void requestOnlinelist();
    void requestCheckout();
    void requestArchiveSign();
    void requestArchive();
    void requestCloseRoom();

    Ui::MainWindow *ui;

    QByteArray defaultView;
    QAction *lastBrushAction;
    RegularRouter cmdRouter_;
    BrushSettingsWidget *brushSettingControl_;
    QToolBar *toolbar_;
    QActionGroup *brushActionGroup_;
    QToolButton *colorPickerButton_;
    QScriptEngine* scriptEngine_;
    DeveloperConsole* console_;
private slots:
    void onServerDisconnected();
    void onNewMessage(const QString &content);
    void onSendPressed();
    void onColorGridDroped(int);
    void onColorGridPicked(int, const QColor &);
    void onBrushTypeChange();
    void onBrushSettingsChanged(const QVariantMap &m);
    void onColorPickerPressed(bool c);
    void onCmdData(const QJsonObject &data);
    void onPanoramaRefresh();
    void onCommandActionClose(const QJsonObject &);
    void onCommandResponseClose(const QJsonObject &m);
    void onCommandResponseClearAll(const QJsonObject &m);
    void onCommandResponseCheckout(const QJsonObject &m);
    void onCommandActionClearAll(const QJsonObject &);
    void onCommandResponseOnlinelist(const QJsonObject &o);
    void onActionNotify(const QJsonObject &o);
    void onResponseArchiveSign(const QJsonObject &o);
    void onResponseArchive(const QJsonObject &o);
};

#endif // MAINWINDOW_H
