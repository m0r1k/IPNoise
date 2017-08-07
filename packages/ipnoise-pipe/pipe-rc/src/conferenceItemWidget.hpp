#ifndef CONFERENCE_ITEM_WIDGET
#define CONFERENCE_ITEM_WIDGET

#include <QListWidget>
#include <QListWidgetItem>
#include <QString>

#include "api.hpp"

class ConferenceItemWidget
    :   public  QListWidgetItem
{
    public:
        ConferenceItemWidget(
            Api             *a_api,
            QListWidget     *parent,
            const QString   &a_huid
        );
        virtual ~ConferenceItemWidget();

        QString getHuid();

    private:
        Api     *m_api;
        QString m_huid;
};

#endif

