#ifndef NEWROOMWINDOW_H
#define NEWROOMWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QCryptographicHash>
#include <QSettings>
#include <QJsonObject>

#include <QDebug>
#include "../common.h"
namespace Ui {
class NewRoomWindow;
}

class NewRoomWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewRoomWindow(QWidget *parent = 0);
    ~NewRoomWindow();
signals:
    void newRoom(const QJsonObject &m);
public slots:
    void onServerResponse(const QJsonObject &m);
private slots:
    void onOk();
    void onCancel();
    void onNameChanged(const QString &name);
    
private:
    void disableEdit(bool t);
    Ui::NewRoomWindow *ui;
};

#endif // NEWROOMWINDOW_H
