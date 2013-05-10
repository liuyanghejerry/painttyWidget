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
    const QVariantMap& allShortcutMap() const;
    
signals:
    // QVariantMap is the changed shortcut
    void shortcutChange(const QVariantMap&);
    
public slots:
    bool setShortcut(const QString&,
                     const QKeySequence&,
                     ShortcutType);
    QVariantMap shortcut(const QString& s);
    void resetShortcut(const QString&);
    void resetAllShortcuts();
    
private:
    QVariantMap shortcut_conf;
    QVariantMap default_conf;
    // each item in QVariantMap has scheme:
    // name: QString,
    // key: QKeySequence,
    // type: ShortcutType
    bool loadFromConfigure();
    bool saveToConfigure();
};

#endif // SHORTCUTMANAGER_H
