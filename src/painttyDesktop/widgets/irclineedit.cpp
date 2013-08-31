#include "irclineedit.h"
#include <QKeyEvent>
#include <QDebug>

IRCLineEdit::IRCLineEdit(QWidget *parent)
    :QLineEdit(parent),
      current_index_(-1)
{
    connect(this, &IRCLineEdit::editingFinished,
            this, &IRCLineEdit::commit);
}

void IRCLineEdit::keyPressEvent(QKeyEvent *e)
{
    // keep in mind, current_index_ is always BEFORE the real current pos
    if (e->key() == Qt::Key_Up) {
        if(string_list_.isEmpty()){
            return;
        }
        setText(string_list_[current_index_]);
        if(current_index_ > 0){
            current_index_--;
        }
    } else if (e->key() == Qt::Key_Down) {
        if(string_list_.isEmpty()){
            setText(QString());
            return;
        }
        if(string_list_.size() - 1 > current_index_){
            current_index_++;
            setText(string_list_[current_index_]);
        }else{
            setText(QString());
        }
    } else {
        QLineEdit::keyPressEvent(e);
    }
}

void IRCLineEdit::commit()
{
    if (text().isEmpty())
        return;

    string_list_.push_back(text());
    current_index_ = string_list_.size() - 1;
    clear();
}
