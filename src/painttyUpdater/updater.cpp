#include "updater.h"
#include <QHostAddress>
#include <QUrl>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include "../common/common.h"
#include "../common/network/socket.h"
#include "../common/network/localnetworkinterface.h"


Updater::Updater() :
    socket(new Socket(this)),
    state_(State::READY)
{
    checkNewestVersion();
}

Updater::~Updater()
{
    socket->close();
}

void Updater::checkNewestVersion()
{
    connect(socket, &Socket::connected,
            [this](){
        state_ = State::CHK_VERSION;
        QJsonDocument doc;
        QJsonObject obj;
        obj.insert("request", QString("check"));
        // NOTICE: to ensure no ambigous,
        // all JSON should be in lower case
        QString locale = QLocale(QLocale::system().uiLanguages().at(0))
                .name().toLower();
        obj.insert("language", locale);
        obj.insert("platform", QString("windows x86"));
        doc.setObject(obj);
        socket->sendData(doc.toJson());
    });
    connect(socket, &Socket::error,
            [this](){
        switch(state_){
        case State::CHK_VERSION:
            state_ = State::CHK_ERROR;
            break;
        case State::DOWNLOAD_NEW:
            state_ = State::DOWNLOAD_ERROR;
            break;
        case State::OVERLAP:
            state_ = State::OVERLAP_ERROR;
            break;
        default:
            state_ = State::UNKNOWN_ERROR;
        }

        qDebug()<<socket->errorString();
        qApp->exit(1);
    });
    connect(socket, &Socket::newData,
            [this](const QByteArray& data){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        if(obj.isEmpty()){
            state_ = State::CHK_ERROR;
            qDebug()<<"Check version error!";
            qApp->exit(1);
        }
        QJsonObject info = obj.value("info").toObject();
        if(info.isEmpty()){
            state_ = State::CHK_ERROR;
            qDebug()<<"Check version error!";
            qApp->exit(1);
        }

        // close connection right after we have the result.
        socket->close();

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
        index = index>0?index:commandList.lastIndexOf("-v");
        if(index < 0 || index >= commandList.count()){
            qDebug()<<"parsing error!"<<"cannot find --version or -v";
            printUsage();
            qApp->exit(1);
        }
        QString old_version = commandList[index+1].trimmed();
        if(old_version.isEmpty()){
            qDebug()<<"parsing error!"<<"version number is empty";
            printUsage();
            qApp->exit(1);
        }
        if(version != old_version){
            QMessageBox msgBox;
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setWindowTitle(tr("New version!"));
            if(level < 3) {
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("There's a new version of Mr.Paint.\n"
                                  "We suggest you download it <a href='%1'>here</a>.")
                               .arg(url.toString()));
            }else{
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("There's a critical update of Mr.Paint.\n"
                                  "You can connect to server "
                                  "ONLY if you've <a href='%1'>updated</a>!")
                               .arg(url.toString()));
            }
            if(!changelog.isEmpty()){
                msgBox.setDetailedText(changelog);
            }

            msgBox.exec();
            qApp->exit(0);
        }

    });
    //
    QHostAddress addr;
    if(LocalNetworkInterface::supportIpv6()){
        addr = GlobalDef::HOST_ADDR[1];
        qDebug()<<"using ipv6 address to connect server";
    }else{
        addr = GlobalDef::HOST_ADDR[0];
        qDebug()<<"using ipv4 address to connect server";
    }
    socket->connectToHost(addr, GlobalDef::HOST_UPDATER_PORT);

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

void Updater::printUsage()
{
    qDebug()<<"painttyUpdater "<<GlobalDef::UPDATER_VERSION<<endl
           <<"Usage: "<<"painttyUpdater OPTIONS"<<endl<<endl
          <<"-v, --version VERSION: tell updater the current "
            "version of main program.";
}
