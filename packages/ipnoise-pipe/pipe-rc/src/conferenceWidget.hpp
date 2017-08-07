#ifndef CONFERENCE_WIDGET_HPP
#define CONFERENCE_WIDGET_HPP

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QApplication>

#include "textEditWidget.hpp"
#include "conferenceItemWidget.hpp"
#include "conferenceMsgWidget.hpp"

class ConferencesWidget;

#include "api.hpp"

class ConferenceWidget
    :   public QWidget
{
    Q_OBJECT

    public:
        ConferenceWidget(
            const QString       &a_id,
            Api                 *a_api,
            ConferencesWidget   *a_conferences,
            QWidget             *a_parent = 0
        );
        virtual ~ConferenceWidget();

        void            setupUi(QWidget *);

        ConferenceItemWidget *getItemByHuid(
            const QString &a_huid
        );
        ConferenceItemWidget * getCreateHuid(
            const QString &a_huid
        );

        bool haveHuid(const QString &a_huid);
        QString         getTopic();
        void            setTopic(const QString &a_topic);
        void            setId(const QString &a_id);
        QString         getId();
        void            addHuid(const QString &a_huid);
        void            addMsg(
            const QString &a_msg_id,
            const QString &a_msg_value,
            const QString &a_msg_type
        );
        void            msgDelivered(
            const QString &a_msg_id
        );


    protected:
        void itemInfoChanged(
            ApiItemInfo             *a_info,
            ConferenceItemWidget    *a_item
        );

    protected slots:
        void    sendMsg();
        void    itemInfoChangedSlot(ApiItemInfo *);

    private:
        QString             m_id;
        Api                 *m_api;
        QVBoxLayout         *m_vbox;
        QHBoxLayout         *m_hbox1;
        QListWidget         *m_history;
        QVBoxLayout         *m_vbox1;
        QListWidget         *m_members;
        QPushButton         *m_invite;
        QLabel              *m_status;
        TextEditWidget      *m_edit;
        ConferencesWidget   *m_conferences;
};

#endif

