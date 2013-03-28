#ifndef LAYERLABEL_H
#define LAYERLABEL_H

#include <QWidget>
#include <QStaticText>

class LayerLabel : public QWidget
{
    Q_OBJECT
public:
    explicit LayerLabel(QWidget *parent = 0);
    QString text();
    void setText(const QString &string);
    void setSelected(bool s);
    void setEditFlag(bool b);
    QSize sizeHint () const;
    QSize minimumSizeHint () const;
    
signals:
    
public slots:
protected:
    void paintEvent ( QPaintEvent * e );
private:
    QString prefix_;
    QString text_;
    QStaticText static_text_;
    bool isSelected_;
    bool isEdited_;
};

#endif // LAYERLABEL_H
