#ifndef NEWROOMWINDOW_H
#define NEWROOMWINDOW_H

#include <QDialog>

#include "../misc/errortable.h"

namespace Ui {
class NewRoomWindow;
}

class NewRoomWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewRoomWindow(QWidget *parent = 0);
    ~NewRoomWindow();
    QString roomName() const;
    void complete();
signals:
    void newRoom(const QJsonObject &m);
public slots:
    void onOk();
    void onCancel();
private slots:
    void onNameChanged(const QString &name);
    
private:
    void disableEdit(bool t);
    Ui::NewRoomWindow *ui;
};

#endif // NEWROOMWINDOW_H
