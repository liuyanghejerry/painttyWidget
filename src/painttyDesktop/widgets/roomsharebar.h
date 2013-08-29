#ifndef ROOMSHAREBAR_H
#define ROOMSHAREBAR_H

#include <QWidget>

class QHBoxLayout;
class EasyCopyLineEdit;
class QLabel;

class RoomShareBar : public QWidget
{
    Q_OBJECT
public:
    explicit RoomShareBar(QWidget *parent = 0);
    void setAddress(const QString&);
    QString address() const;
signals:
    
public slots:
private:
    QLabel* roomShareLB_;
    EasyCopyLineEdit* roomAddrLE_;
    QHBoxLayout* layout_;
    
};

#endif // ROOMSHAREBAR_H
