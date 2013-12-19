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
#include "common.h"

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

void Updater::onCheck()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if(doc.isNull() || doc.isEmpty()) {
        return;
    }
    QJsonObject info = doc.object();

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

    QStringList commandList = qApp->arguments();
    // --version should be considered first
    int index = commandList.lastIndexOf("--version");
    // then we check if there is -v
    index = index > 0 ? index : commandList.lastIndexOf("-v");
    if(index < 0 || index >= commandList.count()){
        output<<"parsing error!"<<"cannot find --version or -v";
        printUsage();
        qApp->exit(1);
    }
    QString old_version = commandList[index+1].trimmed();
    if(old_version.isEmpty()){
        output<<"parsing error!"<<"version number is empty";
        printUsage();
        qApp->exit(1);
    }
}

void Updater::checkNewestVersion()
{
    state_ = State::CHK_VERSION;

    QJsonObject obj;
    obj.insert("request", QString("check"));
    // NOTICE: to ensure no ambigous,
    // all JSON should be in lower case
    QString locale = QLocale(QLocale::system().uiLanguages().at(0))
            .name().toLower();
    obj.insert("language", locale);
    obj.insert("platform", QString("windows x86"));
    QJsonDocument doc;
    doc.setObject(obj);

    QNetworkRequest request(QUrl("http://localhost:7979"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    QNetworkReply *reply = manager_->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished,
            this, &Updater::onCheck);

    return;
}

bool Updater::download()
{
    // TODO: auto download the new version
    return false;
}

bool Updater::overlap()
{
    // TODO: overlap the old version
    return false;
}

void Updater::timeout()
{
    // TODO: should stop all network activities before quit
    if(state_ < State::CHK_VERSION)
        qApp->exit(1);
}

void Updater::printUsage()
{
    output<<"painttyUpdater "<<GlobalDef::UPDATER_VERSION<<endl
           <<"Usage: "<<"painttyUpdater OPTIONS"<<endl<<endl
          <<"-v, --version VERSION: tell updater the current "
            "version of main program.";
}
