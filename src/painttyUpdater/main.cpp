#include "updater.h"
#include <QApplication>
#include <QTranslator>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include "../common/common.h"

void initTranslation()
{
    QString locale = QLocale(QLocale::system().uiLanguages().at(0)).name();

    QTranslator *qtTranslator = new QTranslator(qApp);
    qtTranslator->load(QString("qt_%1").arg(locale), ":/translation", "_", ".qm");
    QCoreApplication::installTranslator(qtTranslator);

    QTranslator *myappTranslator = new QTranslator(qApp);
    myappTranslator->load(QString("updater_%1").arg(locale), ":/translation", "_", ".qm");
    QCoreApplication::installTranslator(myappTranslator);
}

void initSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    QString updaterVersion = settings.value("global/version/updater",
                                           GlobalDef::UPDATER_VERSION)
            .toString();
    settings.setValue("global/version/updater", updaterVersion);
    settings.sync();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    initTranslation();
    Updater up;
    return a.exec();
}
