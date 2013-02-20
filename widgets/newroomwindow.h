#ifndef NEWROOMWINDOW_H
#define NEWROOMWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVariantMap>

#include <QDebug>
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
    void newRoom(const QVariantMap &m);
public slots:
    void onServerResponse(const QVariantMap &m);
private slots:
    void onOk();
    void onCancel();
    void onNameChanged(const QString &name);
    
private:
    void disableEdit(bool t);
    Ui::NewRoomWindow *ui;
};

#endif // NEWROOMWINDOW_H
