#ifndef CONFERENCE_MSG_WIDGET_HPP
#define CONFERENCE_MSG_WIDGET_HPP

#include <QString>
#include <QListWidgetItem>

class ConferenceMsgWidget
    :   public  QListWidgetItem
{
    public:
        ConferenceMsgWidget(QListWidget *a_parent = NULL);
        virtual ~ConferenceMsgWidget();

        void        setMsgId(const QString &);
        QString     getMsgId();

    private:
        QString     m_msg_id;
};

#endif

