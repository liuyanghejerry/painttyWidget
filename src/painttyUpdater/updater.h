#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QTextStream>
#include <QTemporaryFile>
#include "updatedialog.h"

class QNetworkAccessManager;

class Updater : public QObject
{
    Q_OBJECT
    
public:
    Updater();
    void run();
    void checkNewestVersion();
    bool download(const QUrl &url);
    bool uncompress(QTemporaryFile &file);
    bool overlap();
    void printUsage();
    void timeout();
    ~Updater();
    void quit();

protected slots:
    void onCheck();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

protected:
    int queryOldVersion();
    QString querySystem();
    QString queryLanguage();
    
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

    QNetworkAccessManager *manager_;
    State state_;
    QTimer *timer_;
    QTextStream output;
    UpdateDialog dialog;
};

#endif // MAINWINDOW_H
