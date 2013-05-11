#ifndef EVENTCAPTOR_H
#define EVENTCAPTOR_H

#include <QThread>

class TabletSupport;

class EventCaptor : public QThread
{
    Q_OBJECT
public:
    EventCaptor(TabletSupport &parent);
    
signals:
    
public slots:
protected:
    void run();
    TabletSupport& tbl_;
    
};

#endif // EVENTCAPTOR_H
