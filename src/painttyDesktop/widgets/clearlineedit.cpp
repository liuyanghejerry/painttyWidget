#include "clearlineedit.h"
#include <QToolButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QPalette>

ClearLineEdit::ClearLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    clearButton = new QToolButton(this);
    clearButton->setIcon(QIcon(":/iconset/ui/cross_16x16.png"));
    clearButton->setFocusPolicy(Qt::NoFocus);
    clearButton->setStyleSheet("QToolButton {border: none;}");
    clearButton->installEventFilter(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->addWidget(clearButton, 0, Qt::AlignRight);
    setLayout(layout);

    QMargins margins = textMargins();
    margins.setRight(24);
    setTextMargins(margins);

    connect(clearButton, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, [this](const QString &text) {
        clearButton->setVisible(!text.isEmpty());
    });

    QMetaObject::invokeMethod(clearButton, "hide", Qt::QueuedConnection);
}

ClearLineEdit::~ClearLineEdit()
{
}

bool ClearLineEdit::eventFilter(QObject *o, QEvent *e)
{
    if (o == clearButton && e->type() == QEvent::Enter)
    {
        clearButton->setCursor(Qt::ArrowCursor);
    }
    return false;
}
