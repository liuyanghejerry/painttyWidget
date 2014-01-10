#ifndef WAITUPDATERDIALOG_H
#define WAITUPDATERDIALOG_H

#include <QDialog>

namespace Ui {
class WaitUpdaterDialog;
}

class WaitUpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitUpdaterDialog(QWidget *parent = 0);
    ~WaitUpdaterDialog();

private:
    Ui::WaitUpdaterDialog *ui;
};

#endif // WAITUPDATERDIALOG_H
