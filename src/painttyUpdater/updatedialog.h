#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = 0);
    ~UpdateDialog();
    void print(const QString& content);
    void setMaxProgress(int value);
    void updateProgress(int value);

private:
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
