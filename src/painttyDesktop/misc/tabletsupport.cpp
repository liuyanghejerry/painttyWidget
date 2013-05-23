#include "tabletsupport.h"
#include <QWidget>
#include <QTabletEvent>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QDebug>

TabletSupport::TabletSupport(QWidget *window):
    wintab_module(nullptr),
    window_(window),
    logContext(nullptr)
{
    if(!loadWintab()) {
        return;
    }
    if(!mapWintabFuns()){
        qCritical()<<"Error with function mapping!";
        return;
    }
    if(!hasDevice()){
        qCritical()<<"No Device found!";
        return;
    }
    logContext = new tagLOGCONTEXTA;
    auto handle = (HWND)window_->winId();

    callFunc().ptrWTInfoA(WTI_DEFSYSCTX, 0, logContext);
    logContext->lcOptions |= CXO_MESSAGES;
    logContext->lcMoveMask = PACKETDATA;
    logContext->lcBtnUpMask = logContext->lcBtnDnMask;

    AXIS TabletX;
    AXIS TabletY;
    callFunc().ptrWTInfoA( WTI_DEVICES, DVC_X, &TabletX );
    callFunc().ptrWTInfoA( WTI_DEVICES, DVC_Y, &TabletY );

    logContext->lcInOrgX = 0;
    logContext->lcInOrgY = 0;
    logContext->lcInExtX = TabletX.axMax;
    logContext->lcInExtY = TabletY.axMax;

    /* output the data in screen coords */
    logContext->lcOutOrgX = logContext->lcOutOrgY = 0;
    logContext->lcOutExtX = GetSystemMetrics(SM_CXSCREEN);
    /* move origin to upper left */
    logContext->lcOutExtY = -GetSystemMetrics(SM_CYSCREEN);

    logContext->lcPktData = PACKETDATA;
    logContext->lcPktMode = PACKETMODE;

    tabapis.context_ = callFunc().ptrWTOpenA(handle,
                                             (LPLOGCONTEXTA)logContext,
                                             true);
    qDebug()<<deviceString();
}

TabletSupport::~TabletSupport()
{
    stop();
    freeWintab();
    delete logContext;
}

void TabletSupport::start()
{
    if(hasDevice()) {
        auto dispacher = QAbstractEventDispatcher::instance(window_->thread());
        dispacher->installNativeEventFilter(this);
    }
}

void TabletSupport::stop()
{
    if(tabapis.context_){
        callFunc().ptrWTClose(tabapis.context_);
    }
    auto dispacher = QAbstractEventDispatcher::instance(window_->thread());
    dispacher->removeNativeEventFilter(this);
}

bool TabletSupport::hasDevice() const
{
    return callFunc().ptrWTInfoA(0, 0, NULL);
}

QString TabletSupport::deviceString() const
{
    int charLen = callFunc().ptrWTInfoA(WTI_DEVICES, DVC_NAME, NULL);
    if(!charLen){
        return QString();
    }
    TCHAR *buf = new TCHAR[charLen];
    callFunc().ptrWTInfoA(WTI_DEVICES, DVC_NAME, buf);
    QString info((char*)buf);
    delete [] buf;
    return info;
}

tagAXIS TabletSupport::normalPressureInfo() const
{
    int charLen = callFunc().ptrWTInfoA(WTI_DEVICES, DVC_NPRESSURE, NULL);
    tagAXIS buf;
    if(!charLen){
        return buf;
    }
    callFunc().ptrWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &buf);
    return buf;
}

tagAXIS TabletSupport::tangentialPressureInfo() const
{
    int charLen = callFunc().ptrWTInfoA(WTI_DEVICES, DVC_TPRESSURE, NULL);
    tagAXIS buf;
    if(!charLen){
        return buf;
    }
    callFunc().ptrWTInfoA(WTI_DEVICES, DVC_TPRESSURE, &buf);
    return buf;
}

int TabletSupport::eventRate() const
{
    int charLen = callFunc().ptrWTInfoA(WTI_DEVICES, DVC_PKTRATE, NULL);
    UINT buf;
    if(!charLen){
        return buf;
    }
    callFunc().ptrWTInfoA(WTI_DEVICES, DVC_PKTRATE, &buf);
    return buf;
}

bool TabletSupport::supportTilt() const
{
    // Get info. about tilt
    struct tagAXIS tiltOrient[3];
    bool tiltSupport = false;
    tiltSupport = callFunc().ptrWTInfoA(WTI_DEVICES,
                                        DVC_ORIENTATION,
                                        &tiltOrient);
    if (tiltSupport){
        // Does the tablet support azimuth and altitude
        if (tiltOrient[0].axResolution && tiltOrient[1].axResolution){
            // Get resolution
            //            auto azimuth = tiltOrient[0].axResolution;
            //            auto altitude = tiltOrient[1].axResolution;
        }else{
            tiltSupport = false;
        }
    }
    return tiltSupport;
}

const WinTabAPI& TabletSupport::callFunc() const
{
    return tabapis;
}

bool TabletSupport::loadWintab()
{
    wintab_module = LoadLibraryA("Wintab32.dll");
    if(!wintab_module) {
        DWORD err = GetLastError();
        qCritical()<<"Cannot load wintab32.dll:"<<err;
        return false;
    }
    return true;
}

bool TabletSupport::freeWintab()
{
    if(wintab_module) {
        bool ok = FreeLibrary(wintab_module);
        if(!ok) {
            DWORD err = GetLastError();
            qCritical()<<"Error while free wintab32.dll:"<<err;
            return false;
        }
        return true;
    }
    return true;
}

bool TabletSupport::mapWintabFuns()
{
    bool isOk = true;
    isOk = isOk && getProcAddr<WinTabAPI::WTINFOA>(tabapis.ptrWTInfoA, "WTInfoA");
    isOk = isOk && getProcAddr<WinTabAPI::WTOPENA>(tabapis.ptrWTOpenA, "WTOpenA");
    isOk = isOk && getProcAddr<WinTabAPI::WTGETA>(tabapis.ptrWTGetA, "WTGetA");
    isOk = isOk && getProcAddr<WinTabAPI::WTSETA>(tabapis.ptrWTSetA, "WTSetA");
    isOk = isOk && getProcAddr<WinTabAPI::WTOPENA>(tabapis.ptrWTOpenA, "WTOpenA");
    isOk = isOk && getProcAddr<WinTabAPI::WTCLOSE>(tabapis.ptrWTClose, "WTClose");
    isOk = isOk && getProcAddr<WinTabAPI::WTPACKET>(tabapis.ptrWTPacket, "WTPacket");
    isOk = isOk && getProcAddr<WinTabAPI::WTOVERLAP>(tabapis.ptrWTOverlap, "WTOverlap");
    isOk = isOk && getProcAddr<WinTabAPI::WTSAVE>(tabapis.ptrWTSave, "WTSave");
    isOk = isOk && getProcAddr<WinTabAPI::WTCONFIG>(tabapis.ptrWTConfig, "WTConfig");
    isOk = isOk && getProcAddr<WinTabAPI::WTRESTORE>(tabapis.ptrWTRestore, "WTRestore");
    isOk = isOk && getProcAddr<WinTabAPI::WTEXTSET>(tabapis.ptrWTExtSet, "WTExtSet");
    isOk = isOk && getProcAddr<WinTabAPI::WTEXTGET>(tabapis.ptrWTExtGet, "WTExtGet");
    isOk = isOk && getProcAddr<WinTabAPI::WTQUEUESIZESET>(tabapis.ptrWTQueueSizeSet, "WTQueueSizeSet");
    isOk = isOk && getProcAddr<WinTabAPI::WTDATAPEEK>(tabapis.ptrWTDataPeek, "WTDataPeek");
    isOk = isOk && getProcAddr<WinTabAPI::WTPACKETSGET>(tabapis.ptrWTPacketsGet, "WTPacketsGet");
    isOk = isOk && getProcAddr<WinTabAPI::WTMGROPEN>(tabapis.ptrWTMgrOpen, "WTMgrOpen");
    isOk = isOk && getProcAddr<WinTabAPI::WTMGRCLOSE>(tabapis.ptrWTMgrClose, "WTMgrClose");
    isOk = isOk && getProcAddr<WinTabAPI::WTMGRDEFCONTEXT>(tabapis.ptrWTMgrDefContext, "WTMgrDefContext");
    isOk = isOk && getProcAddr<WinTabAPI::WTMGRDEFCONTEXTEX>(tabapis.ptrWTMgrDefContextEx, "WTMgrDefContextEx");
    return isOk;
}


bool TabletSupport::nativeEventFilter(const QByteArray &eventType,
                                      void *message, long *)
{
    if (eventType == "windows_generic_MSG") {
        MSG* ev = static_cast<MSG *>(message);
        switch(ev->message){
        case WT_PACKET:
            PACKET pkt;
            if(!callFunc().ptrWTPacket((HCTX)ev->lParam,
                                       ev->wParam,
                                       &pkt)){
                return false;
            }
            QPointF new_point(pkt.pkX, pkt.pkY);
            QRect r = window_->visibleRegion().boundingRect();
            if(!r.contains(new_point.toPoint() - window_->pos())){
                return false;
            }

            QPointF window_point = window_->mapFromGlobal(new_point.toPoint());

            auto preRange_s = normalPressureInfo();
            int preRange = preRange_s.axMax - preRange_s.axMin +1;
            auto tpreRange_s = tangentialPressureInfo();
            int tpreRange = tpreRange_s.axMax - tpreRange_s.axMin +1;

            auto btn_state = HIWORD(pkt.pkButtons);
            if(btn_state == TBN_DOWN) {
                QTabletEvent *te = new QTabletEvent(QEvent::TabletPress,
                                                    window_point,
                                                    new_point,
                                                    QTabletEvent::Stylus,
                                                    QTabletEvent::Pen,
                                                    pkt.pkNormalPressure/qreal(preRange),
                                                    0,// TODO: xTilt
                                                    0,// TODO: yTilt
                                                    // tangentialPressure
                                                    pkt.pkTangentPressure/qreal(tpreRange),
                                                    0.0,// TODO: rotation
                                                    0,// z
                                                    Qt::NoModifier, // TODO: get modfier
                                                    0// TODO: uniqueID
                                                    );
                qApp->postEvent(window_, te);
                qDebug()<<"TabletPress";
            }else if(btn_state == TBN_UP){
                QTabletEvent *te = new QTabletEvent(QEvent::TabletRelease,
                                                    window_point,
                                                    new_point,
                                                    QTabletEvent::Stylus,
                                                    QTabletEvent::Pen,
                                                    pkt.pkNormalPressure/qreal(preRange),
                                                    0,// TODO: xTilt
                                                    0,// TODO: yTilt
                                                    // tangentialPressure
                                                    pkt.pkTangentPressure/qreal(tpreRange),
                                                    0.0,// TODO: rotation
                                                    0,// z
                                                    Qt::NoModifier, // TODO: get modfier
                                                    0// TODO: uniqueID
                                                    );
                qApp->postEvent(window_, te);
                qDebug()<<"TabletRelease";
            }else{
                if( !new_point.isNull() ){
                    QTabletEvent *te = new QTabletEvent(QEvent::TabletMove,
                                                        window_point,
                                                        new_point,
                                                        QTabletEvent::Stylus,
                                                        QTabletEvent::Pen,
                                                        pkt.pkNormalPressure/qreal(preRange),
                                                        0,// TODO: xTilt
                                                        0,// TODO: yTilt
                                                        // tangentialPressure
                                                        pkt.pkTangentPressure/qreal(tpreRange),
                                                        0.0,// TODO: rotation
                                                        pkt.pkZ,// z
                                                        Qt::NoModifier, // TODO: get modfier
                                                        0// TODO: uniqueID
                                                        );
                    qApp->postEvent(window_, te);
                    qDebug()<<"TabletMove";
                }
            }
            break;
        }
    }
    return false;
}
