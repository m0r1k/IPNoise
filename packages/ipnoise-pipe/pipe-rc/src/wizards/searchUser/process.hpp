#ifndef WIDGET_SEARCH_USER_PAGE_PROCESS_HPP
#define WIDGET_SEARCH_USER_PAGE_PROCESS_HPP

#include <QWidget>
#include <QWizardPage>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <QTimer>
#include <QProgressBar>
#include <QCoreApplication>
#include <QSpacerItem>

#include <ipnoise-common/log_common.h>
#include "contactListTreeItem.hpp"
#include "api.hpp"

#define MAX_SEARCH_TIME_SECS 30

class WizardSearchUserPageProcess
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardSearchUserPageProcess(
            Api                 *a_api,
            ContactListPath     &a_contact_path,
            QWidget             *a_parent = 0
        );
        virtual ~WizardSearchUserPageProcess();

        void setupUi(QWidget *a_parent);

        virtual void initializePage();
        virtual int nextId() const;

        QString getHuid();
        void    clearResults();

    protected slots:
        void apiEvent(ApiEventSearchUsersAnswer *);
        void timerSlot();

    private:
        Api                     *m_api;
        ContactListPath         m_contact_path;
        QVBoxLayout             *m_vbox;
        QGroupBox               *m_gbox;
        QVBoxLayout             *m_radios_box;
        QList<QRadioButton *>   m_radios;
        QTimer                  *m_timer;
        QProgressBar            *m_progress;
        QSpacerItem             *m_vertical_spacer;
};

#endif

