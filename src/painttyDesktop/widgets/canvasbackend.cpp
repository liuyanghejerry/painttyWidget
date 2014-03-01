#include "canvasbackend.h"
#include "../../common/network/clientsocket.h"
#include "../misc/singleton.h"
#include "../../common/common.h"

#include <QTimerEvent>
#include <QDateTime>
#include <QJsonDocument>
#include <QSettings>

CanvasBackend::CanvasBackend(QObject *parent)
    :QObject(parent),
      parse_timer_id_(0),
      archive_loaded_(false),
      is_parsed_signal_sent(false),
      pause_(false),
      fullspeed_replay(false)
{
    parse_timer_id_ = this->startTimer(50);

    auto& client_socket = Singleton<ClientSocket>::instance();

    // NOTICE: This is a direct call,
    // hence CanvasBackend must be inited at main thread.
    cached_clientid_ = client_socket.clientId();

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat());

    fullspeed_replay = settings.value("canvas/fullspeed_replay",
                                       false).toBool();

    connect(&client_socket, &ClientSocket::dataPack,
            this, &CanvasBackend::onIncomingData);
    connect(this, &CanvasBackend::newDataGroup,
            &client_socket,
            static_cast<void (ClientSocket::*)(const QByteArray&)>
            (&ClientSocket::sendDataPack));
    connect(&client_socket, &ClientSocket::archiveLoaded,
            this, &CanvasBackend::onArchiveLoaded);
}

CanvasBackend::~CanvasBackend()
{
    auto& client_socket = Singleton<ClientSocket>::instance();
    disconnect(&client_socket, &ClientSocket::dataPack,
               this, &CanvasBackend::onIncomingData);
    this->disconnect();
    if(parse_timer_id_)
        killTimer(parse_timer_id_);
}

void CanvasBackend::pauseParse()
{
    pause_ = true;
}

void CanvasBackend::resumeParse()
{
    pause_ = false;
}

void CanvasBackend::onDataBlock(const QVariantMap info)
{
    QString author = info["name"].toString();
    QString clientid = info["clientid"].toString();
    upsertFootprint(clientid, author);

    auto data = toJson(QVariant(info));
    emit newDataGroup(data);
}

void CanvasBackend::onIncomingData(const QJsonObject& obj)
{
    incoming_store_.enqueue(obj);
    if(fullspeed_replay && !pause_){
        parseIncoming();
    }
}

void CanvasBackend::parseIncoming()
{
    auto dataBlock = [this](const QVariantMap& m){
        QString clientid(m["clientid"].toString());
        // don't draw your own move from remote
        if(clientid == cached_clientid_){
            return;
        }
        QVariantList list(m["block"].toList());
        if(list.length() < 1) {
            return;
        }

        QString layerName(m["layer"].toString());
        QVariantMap brushInfo(m["brush"].toMap());

        // parse first point as drawpoint
        QVariantMap first_set(list.takeFirst().toMap());
        QPoint point(first_set.value("x", 0).toInt(), first_set.value("y", 0).toInt());
        qreal pressure = 1.0;
        if(first_set.contains("pressure")) {
            pressure = first_set.value("pressure").toDouble();
        }
        QString author;
        bool has_author = m.contains("name");
        if(has_author){
            author = m["name"].toString();
            upsertFootprint(clientid, author, point);
        }

        emit remoteDrawPoint(point, brushInfo,
                             layerName, clientid,
                             pressure);

        // parse points as drawlines, with first point as start
        QPoint start_point(point);
        QPoint end_point;
        QVariantMap end;
        while(list.length()){
            end = list.takeFirst().toMap();
            end_point.setX(end.value("x").toInt());
            end_point.setY(end.value("y").toInt());
            qreal pressure = 1.0;
            if(end.contains("pressure")) {
                pressure = end.value("pressure").toDouble();
            }
            if(has_author){
                upsertFootprint(clientid, author, end_point);
            }
            emit remoteDrawLine(start_point, end_point,
                                brushInfo, layerName,
                                clientid, pressure);
            start_point = end_point;
        }
    };

    static bool need_repaint = false;

    if(incoming_store_.length()){
        auto obj = incoming_store_.dequeue();
        QString action = obj.value("action").toString().toLower();
        if(action == "block"){
            dataBlock(obj.toVariantMap());
            if(fullspeed_replay){
                need_repaint = true;
            }else{
                emit repaintHint();
            }
        }
    }else{
        if(need_repaint){
            emit repaintHint();
        }
        if(archive_loaded_ && !is_parsed_signal_sent){
            emit archiveParsed();
            is_parsed_signal_sent = true;
        }
    }
}

void CanvasBackend::requestMembers(MSI index)
{
    //    qDebug()<<"Members requested!";
    typedef QList<MS> MSL;

    MSL&& list = memberHistory_.values();
    qSort(list.begin(), list.end(), [index](const MS &e1,
          const MS &e2) {
        // Note, std::get<> never receive dynamic index.
        // And that's why we have to use switch :(
        switch (index){
        case Name:
            return std::get<Name>(e1) < std::get<Name>(e2);
            break;
        default: // fall-through
        case Count:
            return std::get<Count>(e1) < std::get<Count>(e2);
            break;
        }
    });

    emit membersSorted(list);
}

void CanvasBackend::clearMembers()
{
    memberHistory_.clear();
}

void CanvasBackend::upsertFootprint(const QString& id,
                                    const QString& name,
                                    const QPoint& point)
{
    qint64 stamp = QDateTime::currentMSecsSinceEpoch();
    if( memberHistory_.contains(id) ) {
        auto& member = memberHistory_[id];
        std::get<MSI::Count>( member )++;
        std::get<MSI::Footprint>( member ) = point;
        std::get<MSI::Name>( member ) = name;
        std::get<MSI::LastActiveStamp>( member ) = stamp;
    }else{
        memberHistory_.insert(id, MemberSection(id,
                                                name,
                                                1,
                                                point,
                                                stamp));
    }
}

void CanvasBackend::upsertFootprint(const QString& id,
                                    const QString& name)
{
    if( memberHistory_.contains(id) ) {
        auto& member = memberHistory_[id];
        std::get<MSI::Count>( member )++;
        std::get<MSI::Name>( member ) = name;
    }else{
        memberHistory_.insert(id, MemberSection(id,
                                                name,
                                                1,
                                                QPoint(),
                                                QDateTime::currentMSecsSinceEpoch()));
    }
}

void CanvasBackend::timerEvent(QTimerEvent * event)
{
    if(event->timerId() == parse_timer_id_){
        if(!pause_){
            this->parseIncoming();
        }
    }
}

QByteArray CanvasBackend::toJson(const QVariant &m)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    return QJsonDocument::fromVariant(m).toJson(QJsonDocument::Compact);
#else
    return QJsonDocument::fromVariant(m).toJson();
#endif
}

QVariant CanvasBackend::fromJson(const QByteArray &d)
{
    return QJsonDocument::fromJson(d).toVariant();
}

void CanvasBackend::onArchiveLoaded()
{
    archive_loaded_ = true;
}
