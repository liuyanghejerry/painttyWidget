#ifndef QGRADUALBOX_H
#define QGRADUALBOX_H

#include <QWidget>
#include <QQueue>

class GradualBox : public QWidget
{
    Q_OBJECT

public:
    Q_PROPERTY(qreal opacity MEMBER opacity_ WRITE setOpacity)

    GradualBox(QWidget *p = 0);
    GradualBox(const QString& pMessage, QWidget *p = 0);
    ~GradualBox();

    qreal maxOpacity() const;
    void setMaxOpacity(const qreal& maxOpacity);
    qreal minOpacity() const;
    void setMinOpacity(const qreal& minOpacity);
    int duration() const;
    void setDuration(int duration);
    bool autoPosition() const;
    void setAutoPosition(bool autoPosition);
    bool hideOnClick() const;
    void setHideOnClick(bool hideOnClick);

    static void showText(const QString& msg, bool hideOnClick = false, int duration = 5000);

signals:
    void clicked();
    void finished();
    void textShown();

public slots:
    void setText(const QString & pMessage);

protected:
    void paintEvent( QPaintEvent * event );
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void setOpacity(const qreal& op);
    void prepare();
private:
    qreal maxOpacity_;
    qreal minOpacity_;
    qreal opacity_;
    int duration_;
    QString content_;
    bool autoPosition_;
    bool hideOnClick_;
    bool in_pressed_;
};

#endif // QGRADUALBOX_H
