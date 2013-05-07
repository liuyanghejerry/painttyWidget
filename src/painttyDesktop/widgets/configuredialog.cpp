#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include "configuredialog.h"
#include "ui_configuredialog.h"
#include "../../common/common.h"

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);
    readSettings();
    initLanguageList();
    initUi();

    connect(this, &ConfigureDialog::accepted, this, &ConfigureDialog::acceptConfigure);
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

void ConfigureDialog::initUi()
{
    ui->ipv6CheckBox->setChecked(tryIpv6);
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
    if (ui->ipv6CheckBox->isChecked() ^ tryIpv6)
    {
        settings.setValue("global/ipv6", ui->ipv6CheckBox->isChecked());
        needRestart = true;
    }

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
