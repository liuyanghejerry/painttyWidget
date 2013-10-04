#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include <QTreeWidgetItem>
#include <QMapIterator>
#include <QComboBox>
#include "configuredialog.h"
#include "ui_configuredialog.h"
#include "../../common/common.h"
#include "../misc/shortcutmanager.h"
#include "../misc/singleton.h"
#include "shortcutgrabberedit.h"

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    tryIpv6(false),
    msg_notify(false),
    auto_disable_ime(false),
    skip_replay(false),
    use_defalut_server(true),
    server_port(0)
{
    ui->setupUi(this);
    readSettings();
    initLanguageList();
    initShortcutList();
    initServerSettings();
    initUi();

    connect(this, &ConfigureDialog::accepted,
            this, &ConfigureDialog::acceptConfigure);
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::readSettings()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    selectedLanguage = settings.value("global/language").toString();
    tryIpv6 = settings.value("global/ipv6", false).toBool();
    msg_notify = settings.value("chat/msg_notify", false).toBool();
    auto_disable_ime = settings.value("canvas/auto_disable_ime", false).toBool();
    enable_tablet = settings.value("canvas/enable_tablet", false).toBool();
    use_defalut_server = settings.value("global/server/use_default", true).toBool();
    IPv4_addr = settings.value("global/server/ipv4_addr", QString()).toString();
    IPv6_addr = settings.value("global/server/ipv6_addr", QString()).toString();
    server_port = settings.value("global/server/server_port", 0).toUInt();
    skip_replay = settings.value("canvas/skip_replay", false).toBool();
}

void ConfigureDialog::initLanguageList()
{
    QDir qmDir(":/translation");
    QStringList qmList = qmDir.entryList(QStringList() << "paintty_*.qm",
                                         QDir::Files);
    ui->languageComboBox->addItem(tr("System Default"), QString());
    for (QString &qmFile: qmList)
    {
        qmFile.remove(QRegularExpression(".?paintty_", QRegularExpression::CaseInsensitiveOption));
        qmFile.remove(".qm", Qt::CaseInsensitive);
        QString languageName = QLocale(qmFile).nativeLanguageName();
        ui->languageComboBox->addItem(languageName, qmFile);
        if (selectedLanguage == qmFile)
            ui->languageComboBox->setCurrentIndex(ui->languageComboBox->count() - 1);
    }
}

void ConfigureDialog::initShortcutList()
{
    const ShortcutManager& manager = Singleton<ShortcutManager>::instance();
    const QVariantMap& shortcutMap = manager.allShortcutMap();
    ShortcutDelegate *delegate = new ShortcutDelegate(ui->shortcutList);
    ui->shortcutList->setItemDelegate(delegate);
    QTreeWidgetItem *categoryItem = new QTreeWidgetItem(ui->shortcutList);
    categoryItem->setText(0, tr("Brushes"));
    ui->shortcutList->addTopLevelItem(categoryItem);
    QMapIterator<QString, QVariant> iterator(shortcutMap);
    while (iterator.hasNext())
    {
        iterator.next();
        QTreeWidgetItem *shortcutItem = new QTreeWidgetItem(categoryItem);
        QVariantMap singleEntry = iterator.value()  //we get QVariant for a single QVariantMap entry
                .toMap();                           //we get QVariantMap for a single entry
        QKeySequence sequence = singleEntry.value("key")    //we get QVariant for a QKeySequence
                .value<QKeySequence>();                     //we get QKeySequence
        ShortcutManager::ShortcutType type =
                ShortcutManager::ShortcutType(singleEntry.value("type").toInt());
        shortcutItem->setText(0, singleEntry.value("description").toString());
        shortcutItem->setText(1,sequence.toString(QKeySequence::NativeText));
        if (type == ShortcutManager::Single)
            shortcutItem->setText(2, tr("Immediately"));
        else if (type == ShortcutManager::Multiple)
            shortcutItem->setText(2, tr("When Release"));
        shortcutItem->setData(0, Qt::UserRole, iterator.key());
        shortcutItem->setData(1, Qt::UserRole, sequence);
        shortcutItem->setData(2, Qt::UserRole, type);
        shortcutItem->setFlags(shortcutItem->flags() | Qt::ItemIsEditable);
    }
    ui->shortcutList->expandAll();
    ui->shortcutList->resizeColumnToContents(2);
    ui->shortcutList->resizeColumnToContents(0);
}

void ConfigureDialog::initServerSettings()
{
    connect(ui->use_default_server_checkbox, &QCheckBox::stateChanged,
            [this](int n_state){
        if(n_state == Qt::Checked){
            ui->ipv4_lineedit->setDisabled(true);
            ui->ipv6_lineedit->setDisabled(true);
            ui->port_lineedit->setDisabled(true);
            ui->server_notice_label->setVisible(false);
        }else{
            ui->ipv4_lineedit->setDisabled(false);
            ui->ipv6_lineedit->setDisabled(false);
            ui->port_lineedit->setDisabled(false);
            ui->server_notice_label->setVisible(true);
        }
    });
    ui->use_default_server_checkbox->setChecked(use_defalut_server);

    ui->ipv4_lineedit->setText(IPv4_addr);
    ui->ipv6_lineedit->setText(IPv6_addr);
    if(!server_port){
        ui->port_lineedit->clear();
    }else{
        ui->port_lineedit->setText(QString::number(server_port, 10));
    }
}

void ConfigureDialog::initUi()
{
    ui->ipv6CheckBox->setChecked(tryIpv6);
    ui->msg_notify_checkbox->setChecked(msg_notify);
    ui->auto_disable_ime_checkbox->setChecked(auto_disable_ime);
    ui->enable_tablet->setChecked(enable_tablet);
    ui->skip_replay->setChecked(skip_replay);
    connect(ui->clearCache, &QPushButton::clicked,
            [](){
        QDir cacheDir("cache");
        cacheDir.removeRecursively();
    });
}

void ConfigureDialog::acceptConfigure()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    bool needRestart = false;

    //save language settings
    QString newLanguage = ui->languageComboBox->itemData(ui->languageComboBox->currentIndex())
            .toString();
    if (newLanguage != selectedLanguage)
    {
        settings.setValue("global/language", newLanguage);
        needRestart = true;
    }

    //save ipv6 settings
    if (ui->ipv6CheckBox->isChecked() != tryIpv6)
    {
        settings.setValue("global/ipv6", ui->ipv6CheckBox->isChecked());
        needRestart = true;
    }

    //save shortcut settings
    ShortcutManager& manager = Singleton<ShortcutManager>::instance();
    QVariantMap shortcutMap = manager.allShortcutMap();
    for (int i = 0; i < ui->shortcutList->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *categoryItem = ui->shortcutList->topLevelItem(i);
        if (!categoryItem)
            continue;
        for (int k = 0; k < categoryItem->childCount(); k++)
        {
            QTreeWidgetItem *shortcutItem = categoryItem->child(k);
            if (!shortcutItem)
                return;
            QVariantMap oldEntry =
                    shortcutMap.value(shortcutItem->data(0, Qt::UserRole).toString()).toMap();
            QKeySequence oldSequence = oldEntry.value("key").value<QKeySequence>();
            QKeySequence newSequence = shortcutItem->data(1, Qt::UserRole).value<QKeySequence>();
            ShortcutManager::ShortcutType oldType =
                    ShortcutManager::ShortcutType(oldEntry.value("type").toInt());
            ShortcutManager::ShortcutType newType =
                    ShortcutManager::ShortcutType(shortcutItem->data(2, Qt::UserRole).toInt());
            if (oldSequence != newSequence || oldType != newType) //we compare old sequence with new one
                //to see if we need restart and set new value.
            {
                needRestart = true;
                manager.setShortcut(shortcutItem->data(0, Qt::UserRole).toString(),
                                    newSequence,
                                    newType);
            }
        }
    }
    manager.saveToConfigure();

    //save msg notify settings
    if (ui->msg_notify_checkbox->isChecked() != msg_notify)
    {
        settings.setValue("chat/msg_notify", ui->msg_notify_checkbox->isChecked());
        needRestart = true;
    }

    //save auto disable IME settings
    if (ui->auto_disable_ime_checkbox->isChecked() != auto_disable_ime)
    {
        settings.setValue("canvas/auto_disable_ime",
                          ui->auto_disable_ime_checkbox->isChecked());
        needRestart = true;
    }

    //save tablet settings
    if (ui->enable_tablet->isChecked() != enable_tablet)
    {
        settings.setValue("canvas/enable_tablet",
                          ui->enable_tablet->isChecked());
        needRestart = true;
    }

    // save server settings
    settings.setValue("global/server/use_default", use_defalut_server);
    settings.setValue("global/server/ipv4_addr", IPv4_addr);
    settings.setValue("global/server/ipv6_addr", IPv6_addr);
    settings.setValue("global/server/server_port", server_port);
    needRestart = true;

    // save canvas replay-skip settings
    if (ui->skip_replay->isChecked() != skip_replay)
    {
        settings.setValue("canvas/skip_replay",
                          ui->skip_replay->isChecked());
        needRestart = true;
    }

    settings.sync();

    //see if we need to restart
    if (needRestart)
    {
        int result = QMessageBox::warning(this, tr("Restart"),
                                          tr("Application must restart to "
                                             "enable some of the settings.\n"
                                             "Do you want to restart right now?"),
                                          QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            qApp->closeAllWindows();
            qApp->exit(1);
            QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        }
        else if (result == QMessageBox::No)
        {
            QMessageBox::warning(this , tr("Restart"),
                                 tr("New settings will be applied on next start."));
        }
    }
}

ShortcutDelegate::ShortcutDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget* ShortcutDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if (!index.isValid() || !index.parent().isValid() || !index.column())
        return 0;
    if (index.column() == 1)
        return new ShortcutGrabberEdit(parent);
    else if (index.column() == 2)
    {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(QStringList() << tr("Immediately")
                           << tr("When Release"));
        return comboBox;
    }
    else
        return 0;
}

void ShortcutDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid() || !index.parent().isValid() || !index.column())
        return;
    if (index.column() == 1)
    {
        ShortcutGrabberEdit *shortcutEditor = qobject_cast<ShortcutGrabberEdit*>(editor);
        shortcutEditor->setShortcut(index.data(Qt::UserRole).value<QKeySequence>());
    }
    else if (index.column() == 2)
    {
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        if (index.data(Qt::UserRole).toInt() == ShortcutManager::Single)
            comboBox->setCurrentText(tr("Immediately"));
        else if (index.data(Qt::UserRole).toInt() == ShortcutManager::Multiple)
            comboBox->setCurrentText(tr("When Release"));
    }
}

void ShortcutDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid() || !index.parent().isValid() || !index.column())
        return;
    if (index.column() == 1)
    {
        ShortcutGrabberEdit *shortcutEditor = qobject_cast<ShortcutGrabberEdit*>(editor);
        model->setData(index, shortcutEditor->shortcut(), Qt::UserRole);
        model->setData(index, shortcutEditor->text(), Qt::DisplayRole);
    }
    else if (index.column() == 2)
    {
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        if (comboBox->currentText() == tr("Immediately"))
            model->setData(index, ShortcutManager::Single, Qt::UserRole);
        else if (comboBox->currentText() == tr("When Release"))
            model->setData(index, ShortcutManager::Multiple, Qt::UserRole);
        model->setData(index, comboBox->currentText(), Qt::DisplayRole);
    }
}

void ShortcutDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid() || !index.parent().isValid() || !index.column())
        return;
    editor->setGeometry(option.rect);
}
