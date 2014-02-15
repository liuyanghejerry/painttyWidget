#ifndef NETWORKINDICATOR_H
#define NETWORKINDICATOR_H

#include <QWidget>

class NetworkIndicator : public QWidget
{
    Q_OBJECT
public:
    enum LEVEL {
        NONE,
        LOW,
        MEDIUM,
        GOOD,
        UNKNOWN = -1
    };

    explicit NetworkIndicator(QWidget *parent = 0);

    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    LEVEL level() const;
    void setLevel(const LEVEL &level);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent * event) Q_DECL_OVERRIDE;

private:
    LEVEL level_;
    QString display_text_;
    QColor display_color_;
};

#endif // NETWORKINDICATOR_H
