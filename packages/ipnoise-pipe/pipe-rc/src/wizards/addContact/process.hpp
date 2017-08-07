#ifndef WIDGET_ADD_CONTACT_PAGE_PROCESS_HPP
#define WIDGET_ADD_CONTACT_PAGE_PROCESS_HPP

#include <QWidget>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QDialog>
#include <QMap>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "api.hpp"

#include "linkWidget.hpp"
#include "links/tcp.hpp"
#include "links/udp.hpp"
#include "wizards/addLink.hpp"

typedef QMap<QString, LinkWidget *>  Links;
typedef Links::iterator              LinksIt;

#define REGISTER_LINK(name)                                 \
    do {                                                    \
        LinksIt      links_it;                              \
        LinkWidget   *link = NULL;                          \
        QString         link_name;                          \
                                                            \
        link = new Link##name##Widget(this);                \
        link_name = link->getName();                        \
                                                            \
        links_it = m_links.find(link_name);                 \
        if (m_links.end() != links_it){                     \
            PERROR("Attempt to double register"             \
                " link: '%s'",                              \
                link_name.toStdString().c_str()             \
            );                                              \
            delete link;                                    \
            break;                                          \
        };                                                  \
        m_links[link_name] = link;                          \
    } while(0);

class WizardAddContactPageProcess
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardAddContactPageProcess(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardAddContactPageProcess();

        enum LinksColumns {
            COLUMN_LINK_ICON    = 0,
            COLUMN_LINK_NAME,
            COLUMN_LINK_DISPLAY_NAME,
            COLUMN_LINK_ADDR,
            COLUMNS_TOTAL
        };

        void    setupUi(QWidget *a_parent);
        void    AddInfoTab(QWidget *a_parent);
        void    AddLinksTab(QWidget *a_parent);
        void    AddInfoField(
            const QString   &a_display_name,
            const QString   &a_name
        );

        void    getContactInfo(
            QMap<QString, QString> &a_out
        );

        void    getContactLinks(
            QMap<QString, QString> &a_out
        );

        LinkWidget * getLink(const QString &a_name) const;

    protected slots:
        void    addLink();
        void    addLinkFinished(int);
        void    addInfoField();

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QVBoxLayout         *m_vbox;
        QTabWidget          *m_tabs;
        QPushButton         *m_add_button;
        QComboBox           *m_add_type;
        Links               m_links;
        WizardAddLink       *m_wizard;
        QGroupBox           *m_links_box;
        QTableWidget        *m_links_table;
        QFormLayout         *m_info_form;
        QComboBox           *m_info_combo;
        QPushButton         *m_info_add;
};

#endif

