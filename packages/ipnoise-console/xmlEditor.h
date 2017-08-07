#ifndef XML_EDITOR_H
#define XML_EDITOR_H

#include <QTreeWidget>
#include <QDomDocument>
#include <QByteArray>
#include <QVariant>
#include <QObject>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include "ui_xmlEditor.h"

struct TreeElement
{
    QDomElement element;
};

Q_DECLARE_METATYPE(TreeElement);

struct TableElement
{
    QString old_key_value;
};

Q_DECLARE_METATYPE(TableElement);


class XmlEditor : public QWidget, public Ui_XmlEditor
{
    Q_OBJECT

    public:
        XmlEditor(QWidget *parent, const QByteArray &data);
        ~XmlEditor();

    private:
        void dom2Tree(
            QTreeWidgetItem     *item,
            QDomElement         &element
        );
        void dom2Tree(
            QDomElement         &element
        );

        // context for tree
        QAction *createTreeItemAct;
        QAction *editTreeItemAct;
        QAction *deleteTreeItemAct;

        // context for table
        QAction *createTableItemAct;
        QAction *editTableItemAct;
        QAction *deleteTableItemAct;

    public slots:

        void treeItemClicked(QTreeWidgetItem *item);
        void treeItemClicked(QTreeWidgetItem *item, int column);
        void resizeAllWidgets();

        // tree widget
        void createTreeItem();
        void editTreeItem();
        void deleteTreeItem();
        void treeContextMenuSlot(const QPoint & pos);

        // table widget
        void createTableItem();
        void editTableItem();
        void deleteTableItem();
        void tableContextMenuSlot(const QPoint & pos);
        void tableItemChanged(QTableWidgetItem *item);

};

#endif

