#ifndef API_EVENT_CONFERENCE_MSG
#define API_EVENT_CONFERENCE_MSG

#include "apiEvent.hpp"

#include <QtCore/QString>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventConferenceMsg
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventConferenceMsg(Api *a_api);
        virtual ~ApiEventConferenceMsg();

        virtual void process();

        QString     getConfId();
        QString     getMsgId();
        QString     getMsgValue();
        QString     getMsgType();
};


#endif

