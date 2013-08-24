#ifndef CLEARLINEEDIT_H
#define CLEARLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class ClearLineEdit : public QLineEdit
{
    Q_OBJECT
    
public:
    ClearLineEdit(QWidget *parent = 0);
    ~ClearLineEdit();

private:
    QToolButton *clearButton;

    bool eventFilter(QObject *o, QEvent *e);
};

#endif // CLEARLINEEDIT_H
