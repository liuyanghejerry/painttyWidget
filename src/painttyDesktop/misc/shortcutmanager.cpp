#include "shortcutmanager.h"
#include "../../common/common.h"
#include <QSettings>
#include <QKeySequence>
#include <QApplication>

ShortcutManager::ShortcutManager(QObject *parent) :
    QObject(parent)
{
    QVariantMap map;

    map.insert("name", "basicbrush");
    map.insert("key", QKeySequence("B"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Brush"));
    default_conf.insert("basicbrush", map);

    map.insert("name", "basiceraser");
    map.insert("key", QKeySequence("N"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Eraser"));
    default_conf.insert("basiceraser", map);

    map.insert("name", "binarybrush");
    map.insert("key", QKeySequence("M"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("BinaryBrush"));
    default_conf.insert("binarybrush", map);

    map.insert("name", "crayon");
    map.insert("key", QKeySequence(","));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Crayon"));
    default_conf.insert("crayon", map);

    map.insert("name", "sketchbrush");
    map.insert("key", QKeySequence("."));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Sketch"));
    default_conf.insert("sketchbrush", map);

    // TODO
//    map.insert("name", "waterbrush");
//    map.insert("key", QKeySequence("/"));
//    map.insert("type", ShortcutType::Single);
//    map.insert("description", tr("WaterBrush"));
//    default_conf.insert("waterbrush", map);

    map.insert("name", "colorpicker");
    map.insert("key", QKeySequence("V"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("ColorPicker"));
    default_conf.insert("colorpicker", map);

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
    map.insert("key", QKeySequence("S"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Increase brush hardness"));
    default_conf.insert("addhardness", map);

    map.insert("name", "subhardness");
    map.insert("key", QKeySequence("A"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Decrease brush hardness"));
    default_conf.insert("subhardness", map);

    map.insert("name", "addthickness");
    map.insert("key", QKeySequence("X"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Increase brush thickness"));
    default_conf.insert("addthickness", map);

    map.insert("name", "subthickness");
    map.insert("key", QKeySequence("Z"));
    map.insert("type", ShortcutType::Single);
    map.insert("description", tr("Decrease brush thickness"));
    default_conf.insert("subthickness", map);

//    map.insert("name", "addwater");
//    map.insert("key", QKeySequence("S"));
//    map.insert("type", ShortcutType::Single);
//    map.insert("description", tr("Decrease brush hardness"));
//    default_conf.insert("subhardness", map);

    map.insert("name", "zoomin");
    map.insert("key", QKeySequence("="));
    map.insert("type", ShortcutType::Multiple);
    map.insert("description", tr("Zoom in canvas"));
    default_conf.insert("zoomin", map);

    map.insert("name", "zoomout");
    map.insert("key", QKeySequence("-"));
    map.insert("type", ShortcutType::Multiple);
    map.insert("description", tr("Zoom out canvas"));
    default_conf.insert("zoomout", map);

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
