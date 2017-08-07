#include "xmlEditor.h"

#define ITEM_FLAGS Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
#define NEW_TREE_ELEMENT_NAME "new_item"

XmlEditor::XmlEditor(QWidget *parent, const QByteArray &data)
    : QWidget(parent)
{
    QDomDocument        *dom    = NULL;

    bool namespaceProcessing    = false;
    QString errorMsg            = "";
    int errorLine               = -1;
    int errorColumn             = -1;
    bool res                    = false;

    setupUi(this);

    dom     = new QDomDocument();
    res     = dom->setContent(
        data, namespaceProcessing,
        &errorMsg, &errorLine, &errorColumn
    );

    if (!res){
        fprintf(stderr, "Cannot parse XML: '%s'\n", data.data());
        return;
    }

    treeWidget->setColumnCount(2);

    QDomElement element = dom->documentElement();
    dom2Tree(element);

    treeWidget->expandAll();

    treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    
    connect(
        treeWidget, SIGNAL(itemExpanded(  QTreeWidgetItem *)),
        this, SLOT(treeItemClicked( QTreeWidgetItem *))
    );

    connect(
        treeWidget, SIGNAL(itemPressed(   QTreeWidgetItem *, int)),
        this, SLOT(treeItemClicked( QTreeWidgetItem *, int))
    );

    // context menu for tree
    createTreeItemAct   = new QAction(tr("Create child item"),  this);
    editTreeItemAct     = new QAction(tr("Edit item"),          this);
    deleteTreeItemAct   = new QAction(tr("Delete item"),        this);
    connect(createTreeItemAct,      SIGNAL(triggered()), this, SLOT(createTreeItem()));
    connect(editTreeItemAct,        SIGNAL(triggered()), this, SLOT(editTreeItem()));
    connect(deleteTreeItemAct,      SIGNAL(triggered()), this, SLOT(deleteTreeItem()));
    connect (
        treeWidget,     SIGNAL(customContextMenuRequested(const QPoint &)),
        this,           SLOT(treeContextMenuSlot(const QPoint &))
    );

    // context menu for table
    createTableItemAct   = new QAction(tr("Create item"),   this);
    editTableItemAct     = new QAction(tr("Edit item"),     this);
    deleteTableItemAct   = new QAction(tr("Delete item"),   this);
    connect(createTableItemAct,     SIGNAL(triggered()), this, SLOT(createTableItem()));
    connect(editTableItemAct,       SIGNAL(triggered()), this, SLOT(editTableItem()));
    connect(deleteTableItemAct,     SIGNAL(triggered()), this, SLOT(deleteTableItem()));
    connect(
        tableWidget,    SIGNAL(customContextMenuRequested(const QPoint &)),
        this,           SLOT(tableContextMenuSlot(const QPoint &))
    );

    connect(
        tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)),
        this,        SLOT(tableItemChanged(QTableWidgetItem *))
    );

    // resize all for context
    resizeAllWidgets();
}

XmlEditor::~XmlEditor()
{

}

// tree context menu

void XmlEditor::createTreeItem()
{
    QTreeWidgetItem *new_item       = NULL; 
    QTreeWidgetItem *current_item   = treeWidget->currentItem();
    QTreeWidgetItem *root_item      = treeWidget->topLevelItem(0);

    if (root_item == NULL){
        // gui
        new_item = new QTreeWidgetItem(treeWidget);
        // dom
        QVariant v;
        QDomDocument    dom;
        QDomElement     element = dom.createElement(NEW_TREE_ELEMENT_NAME);
        dom.appendChild(element);
        // qvariant
        TreeElement xmlElement;
        xmlElement.element = element;
        v.setValue(xmlElement);
        new_item->setData(0, Qt::UserRole, v);
        // gui
        treeWidget->addTopLevelItem(new_item);
    } else if (current_item != NULL){
        // gui
        new_item = new QTreeWidgetItem(current_item);
        // dom
        QVariant cur_v              = current_item->data(0, Qt::UserRole);
        TreeElement cur_xmlElement  = cur_v.value<TreeElement>();
        // qvariant
        QVariant v;
        TreeElement     xmlElement;
        QDomDocument    dom     = cur_xmlElement.element.ownerDocument();
        QDomElement     element = dom.createElement(NEW_TREE_ELEMENT_NAME);
        xmlElement.element      = element;
        cur_xmlElement.element.appendChild(element);
        v.setValue(xmlElement);
        new_item->setData(0, Qt::UserRole, v);
        // gui
        current_item->addChild(new_item);
    } else {
        return;
    }

    new_item->setText(0, NEW_TREE_ELEMENT_NAME);
    new_item->setFlags(ITEM_FLAGS);

    resizeAllWidgets();
}

void XmlEditor::editTreeItem()
{
    QTreeWidgetItem *new_item       = NULL; 
    QTreeWidgetItem *current_item   = treeWidget->currentItem();
    if (current_item == NULL){
        return;
    }
}

void XmlEditor::deleteTreeItem()
{
    QTreeWidgetItem *new_item       = NULL; 
    QTreeWidgetItem *current_item   = treeWidget->currentItem();
    if (current_item == NULL){
        return;
    }
    delete current_item;
}

// table context menu

void XmlEditor::createTableItem()
{
    int cur_rows_count                  = 0;
    QTreeWidgetItem     *current_item   = treeWidget->currentItem();

    if (current_item == NULL){
        // we have not tree element selected
        return;
    }

    cur_rows_count = tableWidget->rowCount();
    cur_rows_count++;
    tableWidget->setRowCount(cur_rows_count);

    resizeAllWidgets();
}

void XmlEditor::tableItemChanged(QTableWidgetItem *item)
{
    int changed = 0, row = 0;
    QTableWidgetItem    *key            = NULL; 
    QTableWidgetItem    *value          = NULL; 
    QTreeWidgetItem     *current_item   = treeWidget->currentItem();

    if (item == NULL){
        // we have't item :) "Trust no one" (c) X-Files
        return;
    }

    if (current_item == NULL){
        // we have not tree element selected
        return;
    }

    // get item row
    row = item->row();
    
    // ok, now search key and value for this element
    key     = tableWidget->item(row, 0);
    value   = tableWidget->item(row, 1);

    if (key == NULL){
        key = new QTableWidgetItem("");
    }

    if (value == NULL){
        value = new QTableWidgetItem("");
    }

    QVariant v                  = key->data(Qt::UserRole);
    TableElement tableElement   = v.value<TableElement>();

    // good, now we search dom element for current tree element
    QVariant vvv            = current_item->data(0, Qt::UserRole);
    TreeElement xmlElement  = vvv.value<TreeElement>();

    // if old key value exist, remove it
    if (tableElement.old_key_value != ""){
        xmlElement.element.removeAttribute(tableElement.old_key_value);
    }

    if (key->text() == ""){
        return;
    }

    // ok, now we create new attributes for element
    xmlElement.element.setAttribute(key->text(), value->text());

    if (tableElement.old_key_value == key->text()){
        // already changed
        return;
    }

    // store as old value
    QVariant vv;
    TableElement new_tableElement;
    new_tableElement.old_key_value = key->text();
    vv.setValue(new_tableElement);
    key->setData(Qt::UserRole, vv);
    value->setData(Qt::UserRole, vv);

    // request to resize
    resizeAllWidgets();
}

void XmlEditor::editTableItem()
{
    QTableWidgetItem *current_item = tableWidget->currentItem();
    if (current_item == NULL){
        return;
    }
    // start edit
    tableWidget->editItem(current_item);
}

void XmlEditor::deleteTableItem()
{
    QTableWidgetItem *new_item       = NULL; 
    QTableWidgetItem *current_item   = tableWidget->currentItem();
    if (current_item == NULL){
        return;
    }
    delete current_item;
}

void XmlEditor::resizeAllWidgets()
{
    int i;
    for (i = 0; i < treeWidget->columnCount(); i++){
        treeWidget->resizeColumnToContents(i);
    }
    for (i = 0; i < tableWidget->columnCount(); i++){
        tableWidget->resizeColumnToContents(i);
    }
    tableWidget->sortItems(0, Qt::AscendingOrder);
}

void XmlEditor::treeContextMenuSlot(const QPoint & pos)
{
    // tree widget context menu event
    QPoint p = QWidget::mapToGlobal(treeWidget->pos()) + pos;
    QTreeWidgetItem *tree_item = NULL;

    tree_item = treeWidget->itemAt(pos);

    if (tree_item != NULL){
        QMenu menu(this);
        menu.addAction(createTreeItemAct);
        menu.addAction(editTreeItemAct);
        menu.addAction(deleteTreeItemAct);
        menu.exec(p);
    } else {
        QMenu menu(this);
        menu.addAction(createTreeItemAct);
        menu.exec(p);
    }
}

void XmlEditor::tableContextMenuSlot(const QPoint & pos)
{
    // table widget context menu event
    QMenu menu(this);
    QPoint p = QWidget::mapToGlobal(tableWidget->pos()) + pos;
    QTableWidgetItem    *table_item     = NULL;
    QTreeWidgetItem     *current_item   = treeWidget->currentItem();

    if (current_item == NULL){
        // we have not tree element selected
        return;
    }

    table_item = tableWidget->itemAt(pos);

    menu.addAction(createTableItemAct);
    if (table_item != NULL){
        menu.addAction(editTableItemAct);
        menu.addAction(deleteTableItemAct);
    }
    menu.exec(p);
}

void XmlEditor::treeItemClicked(QTreeWidgetItem *item, int column)
{
    int i;
    QVariant v              = item->data(0, Qt::UserRole);
    TreeElement xmlElement  = v.value<TreeElement>();

    QDomNamedNodeMap attrs_map;
    QTableWidgetItem    *key    = NULL;
    QTableWidgetItem    *value  = NULL;
    int cur_rows_count;

    while (tableWidget->rowCount() > 0){
        tableWidget->removeRow(0);
    }

    attrs_map = xmlElement.element.attributes();
    for (i = 0; i < attrs_map.size(); i++){
        QDomNode item  = attrs_map.item(i);
        cur_rows_count = tableWidget->rowCount();
        cur_rows_count++;
        tableWidget->setRowCount(cur_rows_count);

        key     = new QTableWidgetItem(item.nodeName());
        value   = new QTableWidgetItem(item.nodeValue());

        key->setFlags(ITEM_FLAGS);
        value->setFlags(ITEM_FLAGS);

        tableWidget->setItem(cur_rows_count - 1, 1, value);
        tableWidget->setItem(cur_rows_count - 1, 0, key);
    }

    QDomNode n = xmlElement.element.firstChild();
    while (!n.isNull()){
        QDomText t = n.toText();
        if (!t.isNull()){
            cur_rows_count = tableWidget->rowCount();
            cur_rows_count++;
            tableWidget->setRowCount(cur_rows_count);

            key     = new QTableWidgetItem("[TEXT NODE]");
            value   = new QTableWidgetItem(t.data());

            key->setFlags(Qt::NoItemFlags);
            value->setFlags(ITEM_FLAGS);

            tableWidget->setItem(cur_rows_count - 1, 0, key);
            tableWidget->setItem(cur_rows_count - 1, 1, value);
        }
        n = n.nextSibling();
    }

    resizeAllWidgets();
}

void XmlEditor::treeItemClicked(QTreeWidgetItem *item)
{
    treeItemClicked(item, 0);
}

void XmlEditor::dom2Tree(
    QDomElement &element)
{
    return dom2Tree(NULL, element);
}

void XmlEditor::dom2Tree(
    QTreeWidgetItem     *item,
    QDomElement         &element)
{
    QTreeWidgetItem     *new_item   = NULL;
    QString             name        = "";
    QString             attrs       = "";

    name = element.tagName();

    if (item == NULL){
        new_item = new QTreeWidgetItem(treeWidget);
        treeWidget->addTopLevelItem(new_item);
    } else {
        new_item = new QTreeWidgetItem(item);
        item->addChild(new_item);
    }
    new_item->setFlags(ITEM_FLAGS);

    QVariant v;
    TreeElement xmlElement;
    xmlElement.element = element;
    v.setValue(xmlElement);
    new_item->setData(0, Qt::UserRole, v);

    QDomNode n = element.firstChild();
    while (!n.isNull()){
        QDomElement e = n.toElement();
        if (!e.isNull()){
            dom2Tree(new_item, e);
        }
        n = n.nextSibling();
    }

    new_item->setText(0, name);
    new_item->setText(1, attrs);
}

