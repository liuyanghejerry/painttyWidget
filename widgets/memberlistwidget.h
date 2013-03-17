#ifndef MEMBERLISTWIDGET_H
#define MEMBERLISTWIDGET_H

#include <QListWidget>
#include <QHash>

typedef QHash<QString, QVariantList> MemberList;

class MemberListWidget : public QListWidget
{
    Q_OBJECT
    
public:
    MemberListWidget(QWidget *parent = 0);
    void setMemberList(const MemberList &memberList);
    void addMember(const QString &id, const QString &nickName);
    void removeMember(const QString &id);
    QString nickName(const QString &id) const;
    QString currentMemberId() const;
    QString currentMemberNickName() const;

private:
    QHash<QString, QListWidgetItem*> memberItemList;
};

#endif // MEMBERLISTWIDGET_H
