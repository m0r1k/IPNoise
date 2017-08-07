#include "contactList/group.hpp"
#include "contactList/item.hpp"
#include "contactListTreeItem.hpp"

ContactListTreeItem::ContactListTreeItem(
    const QString       &a_type,
    Api                 *a_api,
    QTreeWidgetItem     *a_item,
    const QStringList   &a_list)
    :   QTreeWidgetItem(a_item, a_list)
{
    m_type  = a_type;
    m_api   = a_api;
}

ContactListTreeItem::~ContactListTreeItem()
{
}

QString ContactListTreeItem::getType()
{
    return m_type;
}

QString ContactListTreeItem::getName()
{
    return text(0);
}

// ------------------------- static -------------------------

QString ContactListTreeItem::getContactName(
    ContactListPath &a_contact_path)
{
    QString             ret;
    ContactListTreeItem *item = NULL;

    if (not a_contact_path.count()){
        goto out;
    }

    item    = a_contact_path.at(a_contact_path.count() - 1);
    ret     = item->getName();

out:
    return ret;
}

QString ContactListTreeItem::getContactGroupsPath(
    ContactListPath &a_contact_path)
{
    int i;
    QString ret;

    for (i = 0; i < a_contact_path.count(); i++){
        ContactListTreeItem *item = a_contact_path.at(i);
        if ("group" != item->getType()){
            continue;
        }
        if (ret.size()){
            ret += "->";
        }
        ret += item->getName();
    }

    return ret;
}

ContactListItem * ContactListTreeItem::getContactItem(
    ContactListPath &a_contact_path)
{
    int i;
    ContactListItem *ret = NULL;

    for (i = a_contact_path.count() - 1;
        i >= 0;
        i--)
    {
        ContactListTreeItem *item = a_contact_path.at(i);
        if ("item" != item->getType()){
            continue;
        }
        ret = (ContactListItem *)item;
        break;
    }

    return ret;
}

ContactListGroup * ContactListTreeItem::getContactGroup(
    ContactListPath &a_contact_path)
{
    int i;
    ContactListGroup *ret = NULL;

    for (i = a_contact_path.count() - 1;
        i >= 0;
        i--)
    {
        ContactListTreeItem *item = a_contact_path.at(i);
        if ("group" != item->getType()){
            continue;
        }
        ret = (ContactListGroup *)item;
        break;
    }

    return ret;
}

ContactListGroup * ContactListTreeItem::getContactParentGroup(
    ContactListPath &a_contact_path)
{
    int              i, last = a_contact_path.count() - 1;
    ContactListGroup *ret            = NULL;
    ContactListGroup *first_group    = NULL;
    int              get_first_group = 0;

    for (i = last; i >= 0; i--){
        ContactListTreeItem *item = a_contact_path.at(i);
        if (i == last){
            // check last element
            if ("group" != item->getType()){
                // it is not group, so search first group
                get_first_group = 1;
                continue;
            }
        }
        if ("group" == item->getType()){
            if (not first_group){
                // we have found first group
                first_group = (ContactListGroup *)item;
                if (get_first_group){
                    ret = first_group;
                    break;
                }
            } else {
                // first group exist, get second
                ret = (ContactListGroup *)item;
                break;
            }
        }
    }

    return ret;
}


