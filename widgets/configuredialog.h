#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConfigureDialog(QWidget *parent = 0);
    ~ConfigureDialog();
    
private:
    Ui::ConfigureDialog *ui;
    QString selectedLanguage;
    bool tryIpv6;

    void readSettings();
    void initLanguageList();
    void initUi();

private slots:
    void acceptConfigure();
};

#endif // CONFIGUREDIALOG_H
