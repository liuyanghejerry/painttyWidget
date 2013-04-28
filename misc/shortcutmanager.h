#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QVariantMap>

class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    enum ShortcutType {
        Single,
        Multiple
    };

    explicit ShortcutManager(QObject *parent = 0);
    
signals:
    
public slots:
    bool setShortcut(const QString&,
                     const QKeySequence&,
                     ShortcutType);
    void resetShortcut(const QString&);
    void resetAllShortcuts();
    
private:
    QVariantMap shortcut_conf;
    // each item in QVariantMap has scheme:
    // name: QString,
    // key: QKeySequence,
    // type: ShortcutType
    bool loadFromConfigure();
    bool saveToConfigure();
};

#endif // SHORTCUTMANAGER_H
