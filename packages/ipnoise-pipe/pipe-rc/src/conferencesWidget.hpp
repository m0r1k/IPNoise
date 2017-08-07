#ifndef CONFERENCES_WIDGET_HPP
#define CONFERENCES_WIDGET_HPP

#include <QObject>
#include <QString>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include "api.hpp"
#include "conferenceWidget.hpp"
#include "conferenceHelperWidget.hpp"

class PendingInvite
{
    public:
        QString m_conf_id;
        QString m_huid;
};

class ConferencesWidget
    :   public QWidget
{
    Q_OBJECT

    public:
        ConferencesWidget(
            Api     *a_api,
            QWidget *a_parent = 0
        );
        virtual ~ConferencesWidget();

        void    setupUi(QWidget *);
        void    getByHuid(
            const QString               &a_huid,
            QList<ConferenceWidget *>   &a_out
        );
        ConferenceWidget *          getById(const QString &);
        ConferenceHelperWidget *    getHelper();
        void                        removeHelper();
        ConferenceHelperWidget *    getCreateHelper(
            int32_t a_contact_id
        );

        void        removeTab(int a_index);
        QWidget *   getTabWidget(int a_index);
        int         getTabIndex(QWidget *a_widget);
        QString     getTabName(QWidget *a_widget);
        void        setTabName(
            QWidget         *a_widget,
            const QString   &a_tab_name
        );
        void        selectTab(QWidget *a_widget);
        void        itemClicked(int32_t a_contact_id);
        void        setPendingInvite(
            const QString &a_conf_id,
            const QString &a_huid
        );

    protected slots:
        void    apiEvent(ApiEventConferenceCreated *);
        void    apiEvent(ApiEventConferenceJoin *);
        void    apiEvent(ApiEventConferenceMsg *);
        void    apiEvent(ApiEventConferenceInvited *);
        void    apiEvent(ApiEventUpdateConference *);
        void    apiEvent(ApiEventConferenceMsgDelivered *);

    private:
        Api             *m_api;
        QVBoxLayout     *m_vbox;
        QTabWidget      *m_tabs;
        PendingInvite   *m_pending_invite;
};

#endif

