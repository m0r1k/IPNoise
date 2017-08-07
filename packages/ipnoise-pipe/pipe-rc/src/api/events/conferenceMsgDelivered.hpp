#ifndef API_EVENT_CONFERENCE_MSG_DELIVERED
#define API_EVENT_CONFERENCE_MSG_DELIVERED

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

class ApiEventConferenceMsgDelivered
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventConferenceMsgDelivered(Api *a_api);
        virtual ~ApiEventConferenceMsgDelivered();

        virtual void process();

        QString     getConfId();
        QString     getMsgId();
};


#endif

