#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QAbstractItemModel>

class ShortcutModel;

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
    bool msg_notify;
    bool auto_disable_ime;

    void readSettings();
    void initLanguageList();
    void initShortcutList();
    void initUi();

private slots:
    void acceptConfigure();
};

#endif // CONFIGUREDIALOG_H
