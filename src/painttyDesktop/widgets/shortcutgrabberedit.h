#ifndef SHORTCUTGRABBEREDIT_H
#define SHORTCUTGRABBEREDIT_H

#include <QLineEdit>
#include <QKeySequence>

class ShortcutGrabberEdit : public QLineEdit
{
    Q_OBJECT
public:
    ShortcutGrabberEdit(QWidget *parent = 0, const QKeySequence &shortcut = QKeySequence());
    const QKeySequence& shortcut() const;
    
signals:
    void grabFinished(const QKeySequence &shortcut);

public slots:
    void setShortcut(const QKeySequence & shortcut);

private:
    QPalette normalPalette;
    QPalette focusPalette;
    QKeySequence shortcutSequence;
    QKeySequence previousFullSequence;
    bool fullSequence;

    void updateDisplayText();
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    
};

#endif // SHORTCUTGRABBEREDIT_H
