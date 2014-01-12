#include "shortcutmanager.h"
#include "../../common/common.h"
#include <QSettings>
#include <QKeySequence>
#include <QApplication>

ShortcutManager::ShortcutManager(QObject *parent) :
    QObject(parent)
{
    QVariantMap map;
    map.insert("name", "binarybrush");
    map.insert("key", QKeySequence("Z"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Pencil"));
    default_conf.insert("pencil", map);

    map.insert("name", "basicbrush");
    map.insert("key", QKeySequence("P"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Brush"));
    default_conf.insert("brush", map);

    map.insert("name", "sketchbrush");
    map.insert("key", QKeySequence("S"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Sketch"));
    default_conf.insert("sketch", map);

    map.insert("name", "basiceraser");
    map.insert("key", QKeySequence("E"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Eraser"));
    default_conf.insert("eraser", map);

    map.insert("name", "addwidth");
    map.insert("key", QKeySequence("W"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Increase brush width"));
    default_conf.insert("addwidth", map);

    map.insert("name", "subwidth");
    map.insert("key", QKeySequence("Q"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Decrease brush width"));
    default_conf.insert("subwidth", map);

    map.insert("name", "addhardness");
    map.insert("key", QKeySequence("F"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Increase brush hardness"));
    default_conf.insert("addhardness", map);

    map.insert("name", "subhardness");
    map.insert("key", QKeySequence("D"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Decrease brush hardness"));
    default_conf.insert("subhardness", map);

    if( !loadFromConfigure() ){
        shortcut_conf = default_conf;
    }

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
    QVariantMap map = shortcut_conf.value(s).toMap();
    map.insert("name", s);
    map.insert("key", k);
    map.insert("type", t);
    shortcut_conf.insert(s, map);
    emit shortcutChange(map);
    return true;
}

const QVariantMap& ShortcutManager::allShortcutMap() const
{
    return shortcut_conf;
}

QVariantMap ShortcutManager::shortcut(const QString& s)
{
    return shortcut_conf[s].toMap();
}

void ShortcutManager::resetShortcut(const QString& s)
{
    shortcut_conf[s] = default_conf[s];
    saveToConfigure();
    emit shortcutChange(shortcut_conf[s].toMap());
}

void ShortcutManager::resetAllShortcuts()
{
    shortcut_conf = default_conf;
    saveToConfigure();
    for(const QVariant& v: shortcut_conf){
        emit shortcutChange(v.toMap());
    }
}
