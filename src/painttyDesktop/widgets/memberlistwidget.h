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
    void clearMemberList();
    void addMember(const QString &id, const QString &nickName);
    void removeMember(const QString &id);
    QString nickName(const QString &id) const;
    QString currentMemberId() const;
    QString currentMemberNickName() const;

signals:
    void memberGetKicked(const QString& id);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
private:
    QHash<QString, QListWidgetItem*> memberItemList;
};

#endif // MEMBERLISTWIDGET_H
