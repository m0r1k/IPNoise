#ifndef CONFERENCE_ITEM_WIDGET
#define CONFERENCE_ITEM_WIDGET

#include <QtCore/QString>

#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>

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

