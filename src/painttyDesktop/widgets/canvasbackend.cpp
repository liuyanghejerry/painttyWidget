#include "canvasbackend.h"
#include "../../common/network/clientsocket.h"
#include "../misc/singleton.h"

#include <QTimerEvent>
#include <QDateTime>
#include <QJsonDocument>

CanvasBackend::CanvasBackend(QObject *parent)
    :QObject(parent),
      blocklevel_(MEDIUM),
      send_timer_id_(0),
      parse_timer_id_(0),
      archive_loaded_(false),
      is_parsed_signal_sent(false),
      pause_(false)
{
    send_timer_id_ = this->startTimer(1000*10);
    parse_timer_id_ = this->startTimer(50);

    auto& client_socket = Singleton<ClientSocket>::instance();

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
    if(send_timer_id_)
        killTimer(send_timer_id_);
    if(parse_timer_id_)
        killTimer(parse_timer_id_);
}

void CanvasBackend::commit()
{
    if(blocklevel_ == NONE){
        while(!tempStore.isEmpty()){
            QVariant element = tempStore.takeFirst();
            auto data = toJson(element);
            emit newDataGroup(data);
        }
    }else{
        if(!tempStore.isEmpty()){
            QVariantMap doc;
            doc.insert("action", "block");
            doc.insert("block", tempStore);
            auto data = toJson(QVariant(doc));
            emit newDataGroup(data);
            tempStore.clear();
        }
    }
}

void CanvasBackend::setBlockLevel(const BlockLevel le)
{
    blocklevel_ = le;
}

void CanvasBackend::pauseParse()
{
    pause_ = true;
}

void CanvasBackend::resumeParse()
{
    pause_ = false;
}

CanvasBackend::BlockLevel CanvasBackend::blockLevel() const
{
    return blocklevel_;
}

void CanvasBackend::onDataBlock(const QVariantMap d)
{
    tempStore.append(d);

    QVariantMap info = d["info"].toMap();
    QString author = info["name"].toString();
    QString clientid = info["clientid"].toString();
    upsertFootprint(clientid, author);

    if(blocklevel_ == NONE){
        commit();
    }else{
        if(tempStore.length() >= blocklevel_ ){
            commit();
        }
    }
}

void CanvasBackend::onIncomingData(const QJsonObject& obj)
{
    incoming_store_.append(obj);
}

void CanvasBackend::parseIncoming()
{
    auto drawPoint = [this](const QVariantMap& m){
        QPoint point;
        QString layerName;

        QVariantMap map = m["info"].toMap();
        QString clientid = map["clientid"].toString();
        // don't draw your own move from remote
        if(clientid == Singleton<ClientSocket>::instance().clientId()){
            return;
        }

        QVariantMap point_j = map["point"].toMap();
        point.setX(point_j["x"].toInt());
        point.setY(point_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        qreal pressure = 1.0;
        if(map.contains("pressure")){
            pressure = map["pressure"].toDouble();
        }

        if(map.contains("name")){
            QString author = map["name"].toString();
            upsertFootprint(clientid, author, point);
        }

        emit remoteDrawPoint(point, brushInfo,
                             layerName, clientid,
                             pressure);
    };

    auto drawLine = [this](const QVariantMap& m){
        QPoint start;
        QPoint end;
        QString layerName;

        QVariantMap map = m["info"].toMap();

        QString clientid = map["clientid"].toString();
        // don't draw your own move from remote
        if(clientid == Singleton<ClientSocket>::instance().clientId()){
            return;
        }

        QVariantMap start_j = map["start"].toMap();
        start.setX(start_j["x"].toInt());
        start.setY(start_j["y"].toInt());
        QVariantMap end_j = map["end"].toMap();
        end.setX(end_j["x"].toInt());
        end.setY(end_j["y"].toInt());
        layerName = map["layer"].toString();
        QVariantMap brushInfo = map["brush"].toMap();
        qreal pressure = 1.0;
        if(map.contains("pressure")){
            pressure = map["pressure"].toDouble();
        }

        if(map.contains("name")){
            QString author = map["name"].toString();
            upsertFootprint(clientid, author, end);
        }

        emit remoteDrawLine(start, end,
                            brushInfo, layerName,
                            clientid, pressure);
    };

    auto dataBlock = [&drawPoint,
            &drawLine](const QVariantMap& m){
        QVariantList list = m["block"].toList();
        for(auto &item: list){
            QVariantMap singleData = item.toMap();
            QString action = singleData["action"].toString().toLower();
            if(action == "drawpoint"){
                drawPoint(singleData);
            }else if(action == "drawline"){
                drawLine(singleData);
            }
        }
    };
    for(int i=0;i<3;++i){
        if(incoming_store_.length()){
            auto obj = incoming_store_.takeFirst();
            QString action = obj.value("action").toString().toLower();

            if(action == "drawpoint"){
                drawPoint(obj.toVariantMap());
            }else if(action == "drawline"){
                drawLine(obj.toVariantMap());
            }else if(action == "block"){
                dataBlock(obj.toVariantMap());
            }
        }else{
            if(archive_loaded_ && !is_parsed_signal_sent){
                emit archiveParsed();
                is_parsed_signal_sent = true;
            }
            break;
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
    if(event->timerId() == send_timer_id_){
        this->commit();
    }else if(event->timerId() == parse_timer_id_){
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
