#include "process.hpp"

WizardAddContactPageProcess::WizardAddContactPageProcess(
    Api                 *a_api,
    ContactListPath     &a_contact_path,
    QWidget             *a_parent)
    :   QWizardPage(a_parent)
{
    m_api           = a_api;
    m_contact_path  = a_contact_path;
    m_wizard        = NULL;

    REGISTER_LINK(Udp);
    REGISTER_LINK(Tcp);

    setupUi(this);
}

WizardAddContactPageProcess::~WizardAddContactPageProcess()
{
}

void WizardAddContactPageProcess::setupUi(QWidget *a_parent)
{
    QString group_path;

    group_path = ContactListTreeItem::getContactGroupsPath(
        m_contact_path
    );

    setTitle(tr("Add contact"));
    setSubTitle(tr("Please specify item information"));
    setPixmap(
        QWizard::LogoPixmap,
        QPixmap("/opt/QtSDK/Examples/4.7/dialogs/classwizard/images/logo2.png")
    );

    m_vbox = new QVBoxLayout(a_parent);
    m_tabs = new QTabWidget(a_parent);

    AddInfoTab(a_parent);
    AddLinksTab(a_parent);

    m_vbox->addWidget(m_tabs);
    setLayout(m_vbox);
}

void WizardAddContactPageProcess::addInfoField()
{
    QString field_display_name;
    QString field_name;
    int     index = m_info_combo->currentIndex();

    field_name = m_info_combo->itemData(index).toString();
    field_display_name = m_info_combo->itemText(index);

    if (    not field_name.size()
        ||  not field_display_name.size())
    {
        goto out;
    }

    AddInfoField(field_name, field_display_name);

out:
    return;
}

void WizardAddContactPageProcess::AddInfoField(
    const QString   &a_name,
    const QString   &a_display_name)
{
    QLineEdit   *edit   = new QLineEdit;
    int         row     = m_info_form->rowCount();

    edit->setObjectName(a_name);
    m_info_form->insertRow(
        row,
        a_display_name,
        edit
    );
}

void WizardAddContactPageProcess::AddInfoTab(
    QWidget *a_parent)
{
    QWidget     *widget     = new QWidget(a_parent);
    QVBoxLayout *vbox       = new QVBoxLayout;
//    QHBoxLayout *hbox       = NULL;

    // Create form
    m_info_form = new QFormLayout;
    vbox->addItem(m_info_form);

    // add defailt field in form
    AddInfoField("nickname",   tr("Nick name"));

/*
    TODO need implement dynamic field adding
    20120823 morik@

    // add new field
    hbox = new QHBoxLayout;
    // field type
    m_info_combo = new QComboBox;
    m_info_combo->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
    );
    m_info_combo->addItem("", "");
    m_info_combo->addItem(
        "First Name",
        "first_name"
    );
    hbox->addWidget(m_info_combo);
    // add button
    m_info_add = new QPushButton("Add");
    connect(
        m_info_add, SIGNAL(clicked(bool)),
        this,       SLOT(addInfoField())
    );
    hbox->addWidget(m_info_add);
    vbox->addItem(hbox);
*/

    vbox->addStretch(1);

    widget->setLayout(vbox);
    m_tabs->addTab(widget, "&Info");
}

void WizardAddContactPageProcess::AddLinksTab(
    QWidget *a_parent)
{
    LinksIt  links_it;
    QWidget     *widget = new QWidget(a_parent);
    QVBoxLayout *vbox   = new QVBoxLayout;
    QHBoxLayout *hbox   = new QHBoxLayout;

    // button
    m_add_button = new QPushButton(
        "&Add new link",
        widget
    );
    m_add_button->setSizePolicy(
        QSizePolicy::Minimum,
        QSizePolicy::Minimum
    );

    connect(
        m_add_button,
        SIGNAL(clicked(bool)),
        this,
        SLOT(addLink())
    );

    hbox->addWidget(m_add_button);

    vbox->addItem(hbox);

    // links table
    m_links_table = new QTableWidget(0, COLUMNS_TOTAL, widget);
    m_links_table->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );

    QStringList labels;
    labels  += "";
    labels  += "";
    labels  += "Link type";
    labels  += "Link address";
    m_links_table->setColumnHidden(COLUMN_LINK_NAME, true);
    m_links_table->setHorizontalHeaderLabels(labels);
    m_links_table->horizontalHeader()
        ->setStretchLastSection(true);
    //m_links_table->horizontalHeader()->setResizeMode(
    //    QHeaderView::ResizeToContents
    //);

    vbox->addWidget(m_links_table);

    widget->setLayout(vbox);
    m_tabs->addTab(widget, "&Links");
}

void WizardAddContactPageProcess::addLinkFinished(int)
{
    LinkWidget  *link       = NULL;
    int         row_count   = 0;

    link = m_wizard->getLink();
    if (!link){
        goto out;
    }

    row_count = m_links_table->rowCount();
    m_links_table->setRowCount(row_count + 1);

    // icon
    m_links_table->setItem(
        row_count, COLUMN_LINK_ICON,
        new QTableWidgetItem(link->getIcon(), "")
    );

    // name
    m_links_table->setItem(
        row_count, COLUMN_LINK_NAME,
        new QTableWidgetItem(link->getName())
    );

    // display name
    m_links_table->setItem(
        row_count, COLUMN_LINK_DISPLAY_NAME,
        new QTableWidgetItem(link->getDisplayName())
    );

    // address
    m_links_table->setItem(
        row_count, COLUMN_LINK_ADDR,
        new QTableWidgetItem(link->getAddr())
    );

out:
    return;
}

void WizardAddContactPageProcess::getContactInfo(
    QMap<QString, QString> &a_out)
{
    int i, rows = m_info_form->rowCount();
    for (i = 0; i < rows; i++){
        QString         info_name;
        QString         info_value;
        QLayoutItem     *item = NULL;
        QLineEdit       *edit = NULL;
        item = m_info_form->itemAt(
            i,
            QFormLayout::FieldRole
        );
        edit        = (QLineEdit *)item->widget();
        info_name   = edit->objectName();
        info_value  = edit->text();

        a_out[info_name] = info_value;
    }
}

void WizardAddContactPageProcess::getContactLinks(
    QMap<QString, QString> &a_out)
{
    int i, rows = m_links_table->rowCount();

    for (i = 0; i < rows; i++){
        QTableWidgetItem *link_name_item = NULL;
        QTableWidgetItem *link_addr_item = NULL;
        QString link_name, link_addr;

        link_name_item = m_links_table->item(
            i, COLUMN_LINK_NAME);
        link_addr_item = m_links_table->item(
            i, COLUMN_LINK_ADDR);

        link_name = link_name_item->text();
        link_addr = link_addr_item->text();
        a_out[link_name] = link_addr;
    }
}

void WizardAddContactPageProcess::addLink()
{
    m_wizard = new WizardAddLink(
        m_api,
        m_contact_path
    );
    m_wizard->setModal(true);
    m_wizard->show();

    connect(
        m_wizard,   SIGNAL(finished(int)),
        this,       SLOT(addLinkFinished(int))
    );
}

