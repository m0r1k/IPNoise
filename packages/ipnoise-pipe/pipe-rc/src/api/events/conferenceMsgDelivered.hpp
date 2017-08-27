#ifndef API_EVENT_CONFERENCE_MSG_DELIVERED
#define API_EVENT_CONFERENCE_MSG_DELIVERED

#include "apiEvent.hpp"

#include <QtCore/QString>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

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

