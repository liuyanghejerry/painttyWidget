#include "eventcaptor.h"
#include "tabletsupport.h"
#include <QTabletEvent>
#include <QWidget>
#include <QApplication>
#include <QDebug>

EventCaptor::EventCaptor(TabletSupport &parent) :
    QThread(0),
    tbl_(parent)
{
}

void EventCaptor::run()
{
    constexpr int num = 125;
    PACKET buff[num];
    qDebug()<<"capture event started.";
    auto preRange_s = tbl_.normalPressureInfo();
    int preRange = preRange_s.axMax - preRange_s.axMin +1;
    auto tpreRange_s = tbl_.tangentialPressureInfo();
    int tpreRange = tpreRange_s.axMax - tpreRange_s.axMin +1;
//    int xTilt = 0;
//    int yTilt = 0;
//    bool supportTilt = tbl_.supportTilt();
    while(true){
        int real_num = tbl_.callFunc().ptrWTPacketsGet(tbl_.tabapis.context_,
                                                  num,
                                                  buff);
        QPointF old_point;
        QPointF new_point;
        QPointF window_point;
        for(int i=0;i<real_num;++i){
            new_point.setX(buff[i].pkX);
            new_point.setY(buff[i].pkY);
            window_point = tbl_.window_->mapFromGlobal(new_point.toPoint());
            // TODO: caculate tilt
//            if(supportTilt){
//                //
//            }
            // TODO: identify what device we have
            // TODO: identify what pointer type now
            if(HIWORD(buff[i].pkButtons) == TBN_DOWN) {
                QTabletEvent *te = new QTabletEvent(QEvent::TabletPress,
                                window_point,
                                new_point,
                                QTabletEvent::Stylus,
                                QTabletEvent::Pen,
                                buff[i].pkNormalPressure/qreal(preRange),
                                0,// TODO: xTilt
                                0,// TODO: yTilt
                                // tangentialPressure
                                buff[i].pkTangentPressure/qreal(tpreRange),
                                0.0,// TODO: rotation
                                0,// z
                                Qt::NoModifier, // TODO: get modfier
                                0// TODO: uniqueID
                                );
                qApp->postEvent(tbl_.window_, te);
            }else if(HIWORD(buff[i].pkButtons) == TBN_UP){
                QTabletEvent *te = new QTabletEvent(QEvent::TabletRelease,
                                window_point,
                                new_point,
                                QTabletEvent::Stylus,
                                QTabletEvent::Pen,
                                buff[i].pkNormalPressure/qreal(preRange),
                                0,// TODO: xTilt
                                0,// TODO: yTilt
                                // tangentialPressure
                                buff[i].pkTangentPressure/qreal(tpreRange),
                                0.0,// TODO: rotation
                                0,// z
                                Qt::NoModifier, // TODO: get modfier
                                0// TODO: uniqueID
                                );
                qApp->postEvent(tbl_.window_, te);
            }else if(HIWORD(buff[i].pkButtons) == TBN_NONE){
                if(new_point != old_point){
                    QTabletEvent *te = new QTabletEvent(QEvent::TabletMove,
                                    window_point,
                                    new_point,
                                    QTabletEvent::Stylus,
                                    QTabletEvent::Pen,
                                    buff[i].pkNormalPressure/qreal(preRange),
                                    0,// TODO: xTilt
                                    0,// TODO: yTilt
                                    // tangentialPressure
                                    buff[i].pkTangentPressure/qreal(tpreRange),
                                    0.0,// TODO: rotation
                                    buff[i].pkZ,// z
                                    Qt::NoModifier, // TODO: get modfier
                                    0// TODO: uniqueID
                                    );
                    qApp->postEvent(tbl_.window_, te);
                }
            }
            old_point = new_point;
        }
    }
    qDebug()<<"capture event stoped.";
}
