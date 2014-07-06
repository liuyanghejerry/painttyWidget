#include <QApplication>
#include <QProcess>
#include <QTranslator>
#include <QSettings>
#include <QFontDatabase>
#include <QVariant>
#include <QDir>
#include <QMessageBox>
#include "../common/common.h"
#include "widgets/mainwindow.h"
#include "widgets/roomlistdialog.h"
#include "widgets/gradualbox.h"
#include "widgets/waitupdaterdialog.h"
#include "misc/singleton.h"

namespace mainOnly
{
QPalette& rePalette(QPalette &p)
{
    p.setColor(QPalette::ToolTipText, QColor::fromRgb(38, 38, 53));
    p.setColor(QPalette::WindowText, QColor::fromRgb(38, 38, 53));
    p.setColor(QPalette::Text, QColor::fromRgb(38, 38, 53));
    p.setColor(QPalette::Button, QColor::fromRgb(249, 249, 249));
    p.setColor(QPalette::ButtonText, QColor::fromRgb(38, 38, 53));
    p.setColor(QPalette::BrightText, QColor::fromRgb(249, 249, 249));
    p.setColor(QPalette::Highlight, QColor::fromRgb(255, 68, 0));
    p.setColor(QPalette::HighlightedText, QColor::fromRgb(249, 249, 249));
    p.setColor(QPalette::Window, QColor::fromRgb(249, 249, 249));
    p.setColor(QPalette::Base, QColor::fromRgb(249, 249, 249));
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
void initFonts()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool use_droid_font = settings.value("global/use_droid_font", false)
            .toBool();
    if(!use_droid_font){
        return;
    }

    int  ret = QFontDatabase::addApplicationFont(":/fonts/DroidSansFallback.ttf");
    if(ret < 0){
        qDebug()<<"Cannot load fonts!";
    }
    QStringList strList(QFontDatabase::applicationFontFamilies(ret));
    if (strList.count() > 0){
        QFont fontThis(strList.at(0));
        //        fontThis.setPointSize(9);
        if(qApp->font().pointSize() == -1){
            fontThis.setPixelSize(qApp->font().pixelSize());
        }else{
            fontThis.setPointSize(qApp->font().pointSize());
        }

        qApp->setFont(fontThis);
    }
}

bool runUpdater()
{
    QStringList args;
    args<<"-v"<< QString::number(GlobalDef::CLIENT_VER, 10)
       <<"-p"<< QString::number(qApp->applicationPid(), 10);

    qDebug()<<"try to start updater: "<<args;

    // TODO: considering using detached way,
    // which won't kill updater when Mr.Paint is get killed or be closed
    //    QProcess::startDetached(QDir::current().filePath("updater"), args, QDir::currentPath());

    QProcess *process = new QProcess(qApp);
    process->setWorkingDirectory(QDir::currentPath());
    process->start(QDir::current().filePath("updater"), args);
    if (!process->waitForStarted()){
        GradualBox::showText(QObject::tr("We cannot find updater.\n"
                                         "You may need to check update yourself."));
        return false;
    }
    WaitUpdaterDialog *dialog = new WaitUpdaterDialog;
    WaitUpdaterDialog::connect(dialog, &WaitUpdaterDialog::rejected,
                               process, &QProcess::terminate);
    WaitUpdaterDialog::connect(process,
                               static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                               dialog, &WaitUpdaterDialog::close);
    dialog->exec();
    dialog->deleteLater();
    return true;
}

void adjustLog()
{
    auto msg_pattern = "["\
            "%{if-debug}D%{endif}"\
            "%{if-warning}W%{endif}"\
            "%{if-critical}C%{endif}"\
            "%{if-fatal}F%{endif}]"\
            "%{file}:%{line} - %{message}";
    qSetMessagePattern(msg_pattern);
}

} // namespace mainOnly

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainOnly::adjustLog();

#ifdef Q_OS_MACX
    QDir::setCurrent(a.applicationDirPath());
#endif
    mainOnly::initStyle();
    mainOnly::initSettings();
    mainOnly::initTranslation();
    mainOnly::initFonts();

    RoomListDialog *dialog = new RoomListDialog;
    int exitCode = 0;
#ifndef Q_OS_MACX
    dialog->show();
#endif
    mainOnly::runUpdater();
    while( !exitCode && dialog->exec() ) {
        dialog->hide();
        MainWindow w;
        w.showMaximized();
        exitCode = a.exec();
        //        qDebug()<<"exit code: "<<exitCode;
    }

    delete dialog;
    a.quit();

    return 0;
}
