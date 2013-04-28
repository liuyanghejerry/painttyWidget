#include "shortcutmanager.h"
#include "../common.h"
#include <QSettings>
#include <QKeySequence>
#include <QApplication>

ShortcutManager::ShortcutManager(QObject *parent) :
    QObject(parent)
{
}

bool ShortcutManager::loadFromConfigure()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.sync();

    if( !settings.contains("shortcuts/user") ){
        return false;
    }

    QVariant confs = settings.value("shortcuts/user");
    if(confs.isNull()){
        return false;
    }

    shortcut_conf = confs.toMap();

    return true;
}

bool ShortcutManager::saveToConfigure()
{
    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.setValue("shortcuts/user", shortcut_conf);
    settings.sync();

    return true;
}

bool ShortcutManager::setShortcut(const QString& s,
                 const QKeySequence& k,
                 ShortcutType t)
{
    QVariantMap map;
    map.insert("name", s);
    map.insert("key", k);
    map.insert("type", t);
    shortcut_conf.insert(s, map);
    return true;
}

void ShortcutManager::resetShortcut(const QString&)
{
    // TODO
}

void ShortcutManager::resetAllShortcuts()
{
    // TODO
}
