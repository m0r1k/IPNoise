#ifndef WIDGET_SEARCH_USER_PAGE_TCP_HPP
#define WIDGET_SEARCH_USER_PAGE_TCP_HPP

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
#include <QFormLayout>
#include <QLineEdit>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"
#include "linkWidget.hpp"

class WizardSearchUserPageFillTcpV4
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardSearchUserPageFillTcpV4(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardSearchUserPageFillTcpV4();

        void setupUi(QWidget *a_parent);

        virtual int nextId() const;
        void        updateAddr();

    protected slots:
        void    ipChanged(const QString &);
        void    portChanged(const QString &);

    private:
        Api                 *m_api;
        ContactListPath     m_contact_path;
        QVBoxLayout         *m_vbox;
        LinkWidget          *m_link;
        QFormLayout         *m_form;
        QLabel              *m_ip_label;
        QLineEdit           *m_ip_edit;
        QLabel              *m_port_label;
        QLineEdit           *m_port_edit;
};

#endif

