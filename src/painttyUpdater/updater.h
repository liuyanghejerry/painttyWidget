#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
class Socket;

class Updater : public QObject
{
    Q_OBJECT
    
public:
    Updater();
    void checkNewestVersion();
    bool download();
    bool overlap();
    void printUsage();
    ~Updater();
    
private:
    enum State {
        READY = 0,
        CHK_VERSION,
        DOWNLOAD_NEW,
        OVERLAP,
        CHK_ERROR = -100,
        DOWNLOAD_ERROR,
        OVERLAP_ERROR,
        UNKNOWN_ERROR
    };

    Socket *socket;
    State state_;
};

#endif // MAINWINDOW_H
