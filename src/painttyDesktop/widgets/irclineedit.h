#ifndef QIRCLINEEDIT_H
#define QIRCLINEEDIT_H

#include <QLineEdit>
#include <QList>
class QKeyEvent;

class IRCLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    IRCLineEdit(QWidget* parent = 0);
private:
    void keyPressEvent(QKeyEvent *);
    QList<QString> string_list_;
    quint16 current_index_;
public slots:
    void commit();
};

#endif // QIRCLINEEDIT_H
