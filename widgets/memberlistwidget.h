#ifndef MEMBERLISTWIDGET_H
#define MEMBERLISTWIDGET_H

#include <QListWidget>
#include <QHash>

class MemberListWidget : public QListWidget
{
    Q_OBJECT
    
public:
    MemberListWidget(QWidget *parent = 0);
    void addMember(const QString &id, const QString &nickName);
    void removeMember(const QString &id);
    QString nickName(const QString &id) const;
    QString currentMemberId() const;
    QString currentMemberNickName() const;

private:
    QHash<QString, QListWidgetItem*> memberList;
};

#endif // MEMBERLISTWIDGET_H
