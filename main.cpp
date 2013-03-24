#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QJsonDocument>
#include <QVariant>
#include "common.h"
#include "widgets/mainwindow.h"
#include "widgets/roomlistdialog.h"

namespace mainOnly
{
QPalette& rePalette(QPalette &p)
{
    //    p.setColor(QPalette::Window, QColor::fromRgb(54, 86, 60));
    //    p.setColor(QPalette::WindowText, QColor::fromRgb(168, 216, 185));
    //    p.setColor(QPalette::Base, QColor::fromRgb(34, 125, 81));
    //    p.setColor(QPalette::AlternateBase, QColor::fromRgb(27, 129, 62));
    //    p.setColor(QPalette::ToolTipBase, QColor::fromRgb(27, 129, 62));
    //    p.setColor(QPalette::ToolTipText, QColor::fromRgb(54, 86, 60));
    p.setColor(QPalette::Text, QColor::fromRgb(54, 86, 60));
    p.setColor(QPalette::Button, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::ButtonText, QColor::fromRgb(0, 0, 0));
    p.setColor(QPalette::BrightText, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::Highlight, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::HighlightedText, QColor::fromRgb(54, 86, 60));
    return p;
}

void initStyle()
{
    QApplication::setStyle("Fusion");
    auto p = QApplication::palette();
    p = mainOnly::rePalette(p);
    QApplication::setPalette(p);
}

void initSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QString clientVersion = settings.value("global/version/client",
                                           GlobalDef::CLIENT_VER)
            .toString();
    settings.setValue("global/version/client", clientVersion);
    settings.sync();
}

void initTranslation()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);

    QTranslator *qtTranslator = new QTranslator(qApp);
    QTranslator *myappTranslator = new QTranslator(qApp);

    QString locale = settings.value("global/language", "")
            .toString();
    if(locale.isEmpty()) {
        qtTranslator->load(QLocale::system(), "qt", "_", ":/translation", ".qm");
        myappTranslator->load(QLocale::system(), "paintty", "_", ":/translation", ".qm");
    }
    else {
        qtTranslator->load(QString("qt_%1").arg(locale), ":/translation", "_", ".qm");
        myappTranslator->load(QString("paintty_%1").arg(locale), ":/translation", "_", ".qm");
    }
    QCoreApplication::installTranslator(qtTranslator);
    QCoreApplication::installTranslator(myappTranslator);
}

} // namespace mainOnly

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef Q_OS_MACX
    QDir::setCurrent(a.applicationDirPath());
#endif
    mainOnly::initStyle();
    mainOnly::initSettings();
    mainOnly::initTranslation();

    RoomListDialog *dialog = new RoomListDialog;
    while(dialog->exec()) {
        dialog->hide();
        MainWindow w(dialog->canvasSize());
        w.init();
        w.setNickName(dialog->nick());
        w.setRoomName(dialog->roomName());
        w.setHistorySize(dialog->historySize());
        w.socketInit(dialog->dataPort(), dialog->msgPort());
#ifdef Q_OS_MACX
        w.showFullScreen();
#else
        w.showMaximized();
#endif
        a.exec();
    }

    delete dialog;
    a.quit();

    return 0;
}
