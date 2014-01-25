#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QItemDelegate>

class ShortcutModel;
class QTreeWidgetItem;
class ShortcutDelegate;

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
    bool enable_tablet;
    bool skip_replay;
    bool use_droid_font;
    bool fullspeed_replay;

    bool use_defalut_server;
    QString IPv4_addr;
    QString IPv6_addr;
    quint16 server_port;

    void readSettings();
    void initLanguageList();
    void initShortcutList();
    void initServerSettings();
    void initUi();

private slots:
    void acceptConfigure();
};


class ShortcutDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ShortcutDelegate(QObject *parent = 0);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CONFIGUREDIALOG_H
