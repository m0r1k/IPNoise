#include "eventsWindow.h"
#include "xmlEditor.h"

class Columns
{
    public:
        enum {
            DATE = 0,
            TYPE,
            DATA,
            TOTAL
        };
};

EventsWindow::EventsWindow(QWidget *parent)
    : QWidget(parent)
{
    horizontalLayout = NULL;

    setupUi(this);
 
    eventsWidget->setRowCount(0);
    eventsWidget->setColumnCount(Columns::TOTAL);
    eventsWidget->setColumnHidden(Columns::DATA, true);

    connect(eventsWidget,   SIGNAL(itemClicked(QTableWidgetItem *)),
            this,           SLOT(itemClicked(QTableWidgetItem *)));

}

EventsWindow::~EventsWindow()
{
}

void EventsWindow::itemClicked(QTableWidgetItem *item)
{
    assert(item != NULL);

    QTextEdit           *text       = NULL;
    QTextDocument       *doc        = NULL;
    XmlEditor           *xmlEditor  = NULL;

    QTableWidgetItem    *data_item  = NULL;
    QTableWidgetItem    *type_item  = NULL;


    int i, row = item->row();

    data_item = eventsWidget->item(row, Columns::DATA);
    type_item = eventsWidget->item(row, Columns::TYPE);

    if (data_item == NULL || type_item == NULL){
        return;
    }

    QObjectList childs = eventWidget->children();
    for (i = 0; i < childs.size(); ++i){
        QObject *child = childs.at(i);
        delete child;
    }

    QSizePolicy sizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
 
    horizontalLayout = new QHBoxLayout(eventWidget);

    horizontalLayout->setObjectName(
        QString::fromUtf8("horizontalLayout1")
    );
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
   
    if (type_item->text() == "command"){
        const QByteArray xml(data_item->text().toAscii().data());
        xmlEditor = new XmlEditor(eventWidget, xml);
        horizontalLayout->addWidget(xmlEditor);

//        xmlEditor->setSizePolicy(sizePolicy);
//        xmlEditor->show();
    } else {
        text = new QTextEdit(eventWidget);
        text->setWordWrapMode(QTextOption::NoWrap);

        doc = new QTextDocument(data_item->text(), text);
        text->setDocument(doc);
        horizontalLayout->addWidget(text);

//        text->setSizePolicy(sizePolicy);
//        text->show();
    }

    eventWidget->show();
}

void EventsWindow::appendItem(
    const char *_date,
    const char *_type,
    const char *_data)
{
    QTableWidgetItem *date  = NULL;
    QTableWidgetItem *type  = NULL;
    QTableWidgetItem *data  = NULL;
    int cur_rows_count;

    date = new QTableWidgetItem(QObject::tr(_date));
    type = new QTableWidgetItem(QObject::tr(_type));
    data = new QTableWidgetItem(QObject::tr(_data));

    data->setFlags(Qt::NoItemFlags);
    type->setFlags(Qt::NoItemFlags);
    date->setFlags(Qt::NoItemFlags);

    cur_rows_count = eventsWidget->rowCount();
    cur_rows_count++;
    eventsWidget->setRowCount(cur_rows_count);
    eventsWidget->setItem(cur_rows_count - 1, 0, date);
    eventsWidget->setItem(cur_rows_count - 1, 1, type);
    eventsWidget->setItem(cur_rows_count - 1, 2, data);

    eventsWidget->sortItems(0, Qt::DescendingOrder);
}



