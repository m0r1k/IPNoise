#ifndef API_EVENT_UPDATE_CONFERENCE
#define API_EVENT_UPDATE_CONFERENCE

#include "apiEvent.hpp"

#include <QtCore/QString>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventUpdateConference
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventUpdateConference(Api *a_api);
        virtual ~ApiEventUpdateConference();

        virtual void process();

        QString     getConfId();
        QString     getConfTopic();
        QString     getConfFlags();
};


#endif

