#include "contactListWidget.hpp"

ContactListWidget::ContactListWidget(
    Api     *a_api,
    QWidget *a_parent)
    :   QWidget(a_parent)
{
    setObjectName("contactListWidget");
    ApiCommandArgs args;

    m_api           = a_api;
    m_conferences   = new ConferencesWidget(m_api);

    setupUi(this);

    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventUpdateContactList *)),
        this,
        SLOT(apiEvent(ApiEventUpdateContactList *))
    );
    connect(
        m_treeWidget,
        SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
        this,
        SLOT(itemDoubleClicked(QTreeWidgetItem *, int))
    );
    connect(
        m_api,
        SIGNAL(itemContactChanged(ApiContactInfo *)),
        this,
        SLOT(itemContactChangedSlot(ApiContactInfo *))
    );
}

ContactListWidget::~ContactListWidget()
{
}

void ContactListWidget::createActions()
{
    // add group
    m_actAddGroup = new QAction(tr("Add &group"), this);
    m_actAddGroup->setStatusTip(
        tr("Add new group in contact list")
    );
    connect(
        m_actAddGroup,      SIGNAL(triggered()),
        this,               SLOT(addGroup())
    );

    // add..
    m_actAdd = new QAction(tr("&Add.."), this);
    m_actAdd->setStatusTip(
        tr("Add new item in contact list")
    );
    connect(
        m_actAdd,           SIGNAL(triggered()),
        this,               SLOT(add())
    );

    // add contact
    m_actAddContact = new QAction(tr("Add &contact"), this);
    m_actAddContact->setStatusTip(
        tr("Add new contact in contact list")
    );
    connect(
        m_actAddContact,    SIGNAL(triggered()),
        this,               SLOT(addContact())
    );

    // del group
    m_actDelGroup = new QAction(tr("Del g&roup"), this);
    m_actDelGroup->setStatusTip(
        tr("Delete group from contact list")
    );
    connect(
        m_actDelGroup,      SIGNAL(triggered()),
        this,               SLOT(delGroup())
    );

    // search user
    m_actSearchUser = new QAction(tr("&Search user"), this);
    m_actSearchUser->setStatusTip(
        tr("Search user")
    );
    connect(
        m_actSearchUser,    SIGNAL(triggered()),
        this,               SLOT(searchUser())
    );

}

void ContactListWidget::setupUi(QWidget *a_parent)
{
    m_vbox          = new QVBoxLayout(a_parent);
    m_treeWidget    = new QTreeWidget(a_parent);

    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->header()->setVisible(false);

    m_vbox->addWidget(m_treeWidget);

    createActions();
}

void ContactListWidget::itemDoubleClicked(
    ContactListItem *a_item,
    int)    // column
{
    m_conferences->itemClicked(a_item->getContactId());
    m_conferences->show();
}

void ContactListWidget::itemDoubleClicked(
    ContactListGroup *, // a_group,
    int)                // column
{
}

void ContactListWidget::itemDoubleClicked(
    QTreeWidgetItem     *a_item,
    int                 column)
{
    ContactListTreeItem *tree_item = NULL;
    QString type;

    tree_item   = (ContactListTreeItem *)a_item;
    type        = tree_item->getType();

    if ("item" == type){
        ContactListItem *item = NULL;
        item = (ContactListItem *)tree_item;
        itemDoubleClicked(item, column);
    } else if ("group" == type){
        ContactListGroup *group = NULL;
        group = (ContactListGroup *)tree_item;
        itemDoubleClicked(group, column);
    } else {
        PERROR("Unsupported type: '%s'\n",
            type.toStdString().c_str());
    }
}

void ContactListWidget::addGroup(
    QDomElement         &a_group_el,
    QTreeWidgetItem     *a_group)
{
    int i;
    QList<QDomElement>  res;
    QTreeWidgetItem     *group = NULL;
    QString             group_id;
    QString             group_name;
    QString             is_system;

    // get group info
    group_id    = a_group_el.attribute("id");
    group_name  = a_group_el.attribute("name");
    is_system   = a_group_el.attribute("is_system");

    // create new group
    group = new ContactListGroup(
        m_api,
        group_id,
        group_name,
        ("true" == is_system),
        a_group
    );

    // maybe it is top item?
    if (!a_group){
        m_treeWidget->addTopLevelItem(group);
    }

    // get child groups
    res.clear();
    m_api->getElementsByTagName(
        a_group_el,
        "group",
        res
    );
    for (i = 0; i < res.count(); i++){
        QDomElement group_el = res.at(i);
        addGroup(group_el, group);
    }

    // add group items if exist
    res.clear();
    m_api->getElementsByTagName(
        a_group_el,
        "group_item",
        res
    );
    for (i = 0; i < res.count(); i++){
        ContactListItem     *item           = NULL;
        ApiContactInfo      *info           = NULL;
        QDomElement         group_item_el   = res.at(i);
        QString             contact_id_str;
        int32_t             contact_id;

        contact_id_str = group_item_el.attribute("contact_id");
        if (not contact_id_str.size()){
            PERROR("Impty contact_id at"
                " ContactListWidget::addGroup\n");
            continue;
        }
        contact_id = contact_id_str.toInt();
        info = m_api->getContactInfo(contact_id);
        item = new ContactListItem(
            contact_id,
            m_api,
            group,
            QStringList(info->getNickName())
        );
        itemContactChanged(NULL, item);
    }
}

void ContactListWidget::getContactsById(
    int32_t                     &a_contact_id,
    QList<ContactListItem *>    &a_out)
{
    int i;
    for (i = 0; i < m_treeWidget->topLevelItemCount(); i++){
        getContactsById(
            a_contact_id,
            a_out,
            (ContactListTreeItem *)m_treeWidget
                ->topLevelItem(i)
        );
    }
}

void ContactListWidget::getContactsById(
    int32_t                     &a_contact_id,
    QList<ContactListItem *>    &a_out,
    ContactListTreeItem         *a_item)
{
    int i;

    if (not a_item){
        goto out;
    }

    if ("item" == a_item->getType()){
        ContactListItem *item = NULL;
        item = (ContactListItem *)a_item;
        if (a_contact_id  == item->getContactId()){
            // collect item
            a_out.push_back(item);
        }
    }

    for (i = 0; i < a_item->childCount(); i++){
        getContactsById(
            a_contact_id,
            a_out,
            (ContactListTreeItem *)a_item->child(i)
        );
    }

out:
    return;
}

void ContactListWidget::itemContactChanged(
    ApiContactInfo      *a_info,
    ContactListItem     *a_item)
{
    int32_t                             contact_id;
    QList<ContactListItem *>            items;
    QList<ContactListItem *>::iterator  items_it;

    if (not a_info && not a_item){
        goto out;
    }

    // search huid
    if (a_info){
        contact_id = a_info->getContactId();
    }
    if (a_item){
        contact_id = a_item->getContactId();
    }

    // search info
    if (not a_info){
        a_info = m_api->getContactInfo(contact_id);
    }

    // search items
    if (a_item){
        items.push_back(a_item);
    }

    getContactsById(contact_id, items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        ContactListItem *item = NULL;
        // update items
        item = *items_it;
        if (not a_info->isOnline()){
            item->setForeground(0, QBrush(Qt::darkGray));
        } else {
            item->setForeground(0, QBrush(Qt::black));
        }
    }

out:
    return;
}

void ContactListWidget::itemContactChangedSlot(
    ApiContactInfo     *a_info)
{
    itemContactChanged(a_info, NULL);
}

void ContactListWidget::apiEvent(
    ApiEventUpdateContactList *a_event)
{
    QDomNode        node;
    QDomElement     groups_el;
    QDomElement     group_el;
    QDomElement     el = a_event->getElement();

    // TODO XXX
    // update only changed items/groups
    // keep 'expanded', 'not expanded' state
    m_treeWidget->clear();

    // get groups
    groups_el = m_api->getElementByTagName(el, "groups");
    if (groups_el.isNull()){
        goto out;
    }

    // get group
    group_el = m_api->getElementByTagName(groups_el, "group");
    if (group_el.isNull()){
        goto out;
    }

    // fill recursive
    addGroup(group_el);

out:
    return;
}

void ContactListWidget::contextMenuEvent(
    QContextMenuEvent   *event)
{
    ContactListTreeItem     *tree_item = NULL;
    ContactListPath         contact_path;

    getCurPath(contact_path);
    if (not contact_path.count()){
        goto out;
    }
    tree_item = contact_path.at(contact_path.count() - 1);
    if ("group" == tree_item->getType()){
        QMenu menu(this);
        ContactListGroup *group = NULL;
        group = (ContactListGroup *)tree_item;
        if (not group->isSystem()){
            menu.addAction(m_actAdd);
            menu.addAction(m_actAddGroup);
            menu.addAction(m_actAddContact);
            menu.addAction(m_actDelGroup);
        }
        menu.exec(event->globalPos());
    } else if ("item" == tree_item->getType()){
        QMenu menu(this);
        menu.addAction(m_actSearchUser);
        menu.exec(event->globalPos());
    }

out:
    return;
}

void ContactListWidget::getCurPath(
    ContactListPath &a_out)
{
    ContactListTreeItem *item = NULL;
    item = (ContactListTreeItem *)m_treeWidget->currentItem();
    while (item){
        a_out.push_front(item);
        item = (ContactListTreeItem *)item->parent();
    }
}

void ContactListWidget::add()
{
    WizardAdd           *wizard = NULL;
    ContactListPath     contact_path;

    getCurPath(contact_path);
    wizard = new WizardAdd(m_api, contact_path);
    wizard->show();
}

void ContactListWidget::addContact()
{
    WizardAddContact    *wizard = NULL;
    ContactListPath     contact_path;

    getCurPath(contact_path);
    wizard = new WizardAddContact(m_api, contact_path);
    wizard->show();
}

void ContactListWidget::addGroup()
{
    WizardAddGroup      *wizard = NULL;
    ContactListPath     contact_path;

    getCurPath(contact_path);
    wizard = new WizardAddGroup(m_api, contact_path);
    wizard->show();
}

void ContactListWidget::delGroup()
{
    WizardDelGroup      *wizard = NULL;
    ContactListPath     contact_path;

    getCurPath(contact_path);
    wizard = new WizardDelGroup(m_api, contact_path);
    wizard->show();
}

void ContactListWidget::searchUser()
{
    WizardSearchUser    *wizard = NULL;
    ContactListPath     contact_path;

    getCurPath(contact_path);
    wizard = new WizardSearchUser(m_api, contact_path);
    wizard->show();
}

