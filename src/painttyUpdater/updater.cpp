#include "updater.h"
#include <QUrl>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include <QMessageBox>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>
#include "../network/localnetworkinterface.h"
#include "common.h"
#include "updatedialog.h"

Updater::Updater() :
    manager_(new QNetworkAccessManager(this)),
    state_(State::READY),
    timer_(new QTimer(this)),
    output(stdout)
{
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout,
            this, &Updater::timeout);
}

void Updater::run()
{
    timer_->start(20*1000);
    checkNewestVersion();
}

Updater::~Updater()
{
    //
}

void Updater::quit()
{
    qApp->exit();
}

void Updater::onCheck()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) {
        qDebug()<<"cannot find reply object";
        quit();
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

    if(doc.isEmpty()) {
        qDebug()<<"return value is empty";
        quit();
        return;
    }
    QJsonObject info = doc.object();
    if( (!info.value("result").toBool()) || (info.value("response").toString() != "version") ) {
        qDebug()<<"server error.";
        quit();
        return;
    }
    info = info.value("info").toObject();
    qDebug()<<info;

    QString version = info.value("version").toString().trimmed();
    QString changelog = info.value("changelog").toString();
    int level = info.value("level").toDouble();

    // try to use url fetched from remote server
    QUrl url = QUrl::fromUserInput(GlobalDef::DOWNLOAD_URL);
    QString fetched_url = info.value("url").toString();

    // if we cannot get a valid url, use predefined url
    if(!fetched_url.isEmpty()){
        url = QUrl::fromUserInput(fetched_url);
    }

    QString old_version = queryOldVersion();
    if(old_version.isEmpty()){
        output<<"parsing error!"<<"version number is empty";
        printUsage();
        quit();
    }

    if(version != old_version){
        QString find_new_version = QString(tr("Found new version, %1\nLevel: %2\nChangelog:\n%3\n"))
                .arg(version)
                .arg(level)
                .arg(changelog);
        dialog.print(find_new_version);
        download(url);
    }
    reply->deleteLater();
}

void Updater::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    dialog.updateProgress(bytesReceived/bytesTotal * 80);
}

void Updater::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) {
        qDebug()<<"cannot find reply object";
        quit();
        return;
    }
    QTemporaryFile tmp_file("mrspaint.tmp");
    tmp_file.setAutoRemove(true);
    if(!tmp_file.open()) {
        dialog.print(tr("Error encounter!\nAbort."));
        qDebug()<<"cannot create a temp file.";
        quit();
        return;
    }
    tmp_file.write(reply->readAll());
//    reply->readAll();
}

QString Updater::queryOldVersion()
{
    QStringList commandList = qApp->arguments();
    // --version should be considered first
    int index = commandList.lastIndexOf("--version");
    // then we check if there is -v
    index = index > 0 ? index : commandList.lastIndexOf("-v");
    if(index < 0 || index >= commandList.count()){
        return QString();
    }
    QString old_version = commandList[index+1].trimmed();
    return old_version;
}

QString Updater::querySystem()
{
#if defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)
    return QString("windows x86");
#elif defined(Q_OS_WIN64)
    return QString("windows x64");
#elif defined(Q_OS_OSX)
    return QString("mac");
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_32)
    return QString("linux x86");
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)
    return QString("linux x64");
#endif
    return QString();
}

QString Updater::queryLanguage()
{
    return QLocale(QLocale::system().uiLanguages().at(0))
            .name().toLower();
}

void Updater::checkNewestVersion()
{
    dialog.setWindowTitle(tr("Checking new version"));
    dialog.show();
    state_ = State::CHK_VERSION;

    QJsonObject obj;
    obj.insert("request", QString("check"));
    // NOTICE: to ensure no ambigous,
    // all JSON should be in lower case
    QString locale = queryLanguage();
    if(locale.isEmpty()) {
        output << "error when query system locale.";
        quit();
    }
    QString system = querySystem();
    if(system.isEmpty()) {
        output << "error when query os. Unsupported system?";
        quit();
    }
    obj.insert("language", locale);
    obj.insert("platform", system);
    QJsonDocument doc;
    doc.setObject(obj);

    QUrl update_addr;
    if(LocalNetworkInterface::supportIpv6()) {
        update_addr = QUrl(GlobalDef::UPDATER_ADDR_IPV6);
    } else {
        update_addr = QUrl(GlobalDef::UPDATER_ADDR_IPV4);
    }
//    qDebug()<<update_addr;
    QNetworkRequest request(update_addr);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    QNetworkReply *reply = manager_->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished,
            this, &Updater::onCheck);
    return;
}

bool Updater::download(const QUrl& url)
{
    if(url.isEmpty()) {
        return false;
    }
//    QDesktopServices::openUrl(url);
    // TODO: auto download the new version

    dialog.setWindowTitle(tr("Downloading"));
    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &Updater::onDownloadProgress);
    connect(reply, &QNetworkReply::finished,
            this, &Updater::onDownloadFinished);
    dialog.print(tr("downloading..."));
    return true;
}

bool Updater::overlap()
{
    // TODO: overlap the old version
    return false;
}

void Updater::timeout()
{
    // TODO: should stop all network activities before quit
    if(state_ <= State::CHK_VERSION)
        quit();
}

void Updater::printUsage()
{
    output<<"painttyUpdater "<<GlobalDef::UPDATER_VERSION<<endl
           <<"Usage: "<<"painttyUpdater OPTIONS"<<endl<<endl
          <<"-v, --version VERSION: tell updater the current "
            "version of main program.";
}
