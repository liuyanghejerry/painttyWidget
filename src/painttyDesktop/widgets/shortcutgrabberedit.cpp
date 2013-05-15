#include "shortcutgrabberedit.h"
#include <QKeyEvent>

ShortcutGrabberEdit::ShortcutGrabberEdit(QWidget *parent,
                                         const QKeySequence &shortcut) :
    QLineEdit(parent),
    shortcutSequence(shortcut),
    previousFullSequence(shortcut),
    fullSequence(false)
{
    QFont f = font();
    f.setFamily("Courier New");
    setFont(f);

    normalPalette = palette();
    focusPalette = normalPalette;
    focusPalette.setColor(QPalette::Base, Qt::blue);
    focusPalette.setColor(QPalette::Text, Qt::white);
    focusPalette.setColor(QPalette::Highlight, Qt::blue);
    focusPalette.setColor(QPalette::HighlightedText, Qt::white);

    setReadOnly(true);
    setFocusPolicy(Qt::ClickFocus);
    updateDisplayText();
}

const QKeySequence& ShortcutGrabberEdit::shortcut() const
{
    return shortcutSequence;
}

void ShortcutGrabberEdit::setShortcut(const QKeySequence &shortcut)
{
    shortcutSequence = shortcut;
    previousFullSequence = shortcut;
    updateDisplayText();
}

inline void ShortcutGrabberEdit::updateDisplayText()
{
    setText(shortcutSequence.toString(QKeySequence::NativeText));
}

void ShortcutGrabberEdit::focusInEvent(QFocusEvent *e)
{
    setPalette(focusPalette);
    QLineEdit::focusInEvent(e);
}

void ShortcutGrabberEdit::focusOutEvent(QFocusEvent *e)
{
    if (!fullSequence)
    {
        shortcutSequence = previousFullSequence;
        updateDisplayText();
    }
    setPalette(normalPalette);
    QLineEdit::focusOutEvent(e);
}

void ShortcutGrabberEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int keyCode = e->modifiers() & (~Qt::KeypadModifier);   //when NumLock is on,
                                                            //events for number key on keypad will
                                                            //contain KeypadModifier, we filt it out.
    fullSequence = false;
    switch (e->key())
    {
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_Meta:
    case Qt::Key_NumLock:   //it seems Qt cannot handle numlock key.
    case Qt::Key_Cancel:    //what is Cancel key?
        break;
    default:
        keyCode |= e->key();
        fullSequence = true;
    }
    shortcutSequence = QKeySequence(keyCode);
    if (fullSequence)
        previousFullSequence = shortcutSequence;
    updateDisplayText();
}

void ShortcutGrabberEdit::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int modifier = e->modifiers() & (~Qt::KeypadModifier);
#if defined(Q_OS_WIN)
    if (!modifier)      //on Windows, when a modifier key is released,
                        //modifiers() is empty and key() returns the modifier.
#elif defined(Q_OS_MACX)
    if(((modifier == Qt::ShiftModifier && e->key() == Qt::Key_Shift) ||
        (modifier == Qt::ControlModifier && e->key() == Qt::Key_Control) ||
        (modifier == Qt::AltModifier && e->key() == Qt::Key_Alt) ||
        (modifier == Qt::MetaModifier && e->key() == Qt::Key_Meta) ||
        modifier == Qt::NoModifier))    //on Mac OS X, when a modifier key is released,
                                        //modifiers() contains the modifier that just released.
#elif defined(Q_OS_LINUX)
    if (!modifier)    //TODO: We haven't tested on Linux!
#endif
    {
        if (fullSequence)//when every key is released and sequence is a full one, we emit the signal.
        {
            fullSequence = false;
            emit grabFinished(shortcutSequence);
        }
        else
        {
            shortcutSequence = previousFullSequence;
            updateDisplayText();
        }
    }
}
