#ifndef ICONCHECKBOX_H
#define ICONCHECKBOX_H

#include <QAbstractButton>
#include <QPainter>
#include <QStyleOption>
#include <QBrush>

class IconCheckBox : public QAbstractButton
{
    Q_OBJECT
public:
    explicit IconCheckBox(QWidget *parent = 0);
    QSize sizeHint () const;

    
signals:
//    void checked(bool);
    
public slots:
    
protected:
    void paintEvent ( QPaintEvent * e );
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    bool hitButton (const QPoint &) const;
private:
    QSize size_;
};

#endif // ICONCHECKBOX_H
