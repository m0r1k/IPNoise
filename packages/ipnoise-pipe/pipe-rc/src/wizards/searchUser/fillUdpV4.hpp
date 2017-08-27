#ifndef WIDGET_SEARCH_USER_PAGE_FILL_UDP_V4_HPP
#define WIDGET_SEARCH_USER_PAGE_FILL_UDP_V4_HPP

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include "contactListTreeItem.hpp"
#include "api.hpp"
#include "linkWidget.hpp"

class WizardSearchUserPageFillUdpV4
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardSearchUserPageFillUdpV4(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardSearchUserPageFillUdpV4();

        void    setupUi(QWidget *a_parent);
        QString getAddr();

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

