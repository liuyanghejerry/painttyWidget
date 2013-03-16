#include "memberlistwidget.h"

MemberListWidget::MemberListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

void MemberListWidget::addMember(const QString &id, const QString &nickName)
{
    QListWidgetItem *item = new QListWidgetItem(nickName);
    addItem(item);
    memberList.insert(id, item);;
}

void MemberListWidget::removeMember(const QString &id)
{
    QListWidgetItem *item = memberList.take(id);
    if (item)
        delete item;
}

QString MemberListWidget::nickName(const QString &id) const
{
    QListWidgetItem *item = memberList.value(id, 0);
    if (item)
        return item->text();
    return QString();
}

QString MemberListWidget::currentMemberId() const
{
    QListWidgetItem *item = currentItem();
    if (item)
        return memberList.key(currentItem(), QString());
    return QString();
}

QString MemberListWidget::currentMemberNickName() const
{
    QListWidgetItem *item = currentItem();
    if (item)
        return currentItem()->text();
    return QString();
}
