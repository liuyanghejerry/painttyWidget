#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <QVariant>
#include <QDir>
#include "../common/common.h"
#include "widgets/mainwindow.h"
#include "widgets/roomlistdialog.h"
#include "network/commandsocket.h"
#include "misc/singleton.h"

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

    //    p.setColor(QPalette::Text, QColor::fromRgb(54, 86, 60));
    //    p.setColor(QPalette::Button, QColor::fromRgb(168, 216, 185));
    //    p.setColor(QPalette::ButtonText, QColor::fromRgb(0, 0, 0));
    //    p.setColor(QPalette::BrightText, QColor::fromRgb(168, 216, 185));
    //    p.setColor(QPalette::Highlight, QColor::fromRgb(168, 216, 185));
    //    p.setColor(QPalette::HighlightedText, QColor::fromRgb(54, 86, 60));

    // TODO: new palette, Cthulhu Rises
    p.setColor(QPalette::ToolTipText, QColor::fromRgb(85, 51, 51));
    p.setColor(QPalette::WindowText, QColor::fromRgb(85, 51, 51));
    p.setColor(QPalette::Text, QColor::fromRgb(85, 51, 51));
    p.setColor(QPalette::Button, QColor::fromRgb(170, 187, 170));
    p.setColor(QPalette::ButtonText, QColor::fromRgb(85, 51, 51));
    p.setColor(QPalette::BrightText, QColor::fromRgb(153, 136, 102));
    p.setColor(QPalette::Highlight, QColor::fromRgb(153, 170, 170));
    p.setColor(QPalette::HighlightedText, QColor::fromRgb(85, 51, 51));
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
    if(locale.isEmpty())
        locale = QLocale(QLocale::system().uiLanguages().at(0)).name();

    qtTranslator->load(QString("qt_%1").arg(locale), ":/translation", "_", ".qm");
    myappTranslator->load(QString("paintty_%1").arg(locale), ":/translation", "_", ".qm");
    QCoreApplication::installTranslator(qtTranslator);
    QCoreApplication::installTranslator(myappTranslator);
}

bool runUpdater()
{
    QStringList args;
    args<<"-v"<<GlobalDef::CLIENT_VER;
    QProcess *process = new QProcess(qApp);
    process->setWorkingDirectory(QDir::currentPath());
    QObject::connect(qApp, &QApplication::aboutToQuit, process, &QProcess::kill);
    process->start(QDir::current().filePath("updater"), args);
    if (!process->waitForStarted())
    {
        QMessageBox::warning(0, QObject::tr("No Updater?"),
                             QObject::tr("We cannot find updater.\n"
                                         "You may need to check update yourself."),
                             QMessageBox::Ok);
        return false;
    }
    return true;
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
    int exitCode = 0;
    dialog->show();
    mainOnly::runUpdater();
    while( !exitCode && dialog->exec() ) {
        dialog->hide();
        MainWindow w;
#ifdef Q_OS_MACX
        w.showFullScreen();
#else
        w.showMaximized();
#endif
        exitCode = a.exec();
        qDebug()<<"exit code: "<<exitCode;
    }

    delete dialog;
    a.quit();

    return 0;
}
