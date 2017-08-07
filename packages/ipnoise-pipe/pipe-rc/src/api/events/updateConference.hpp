#ifndef API_EVENT_UPDATE_CONFERENCE
#define API_EVENT_UPDATE_CONFERENCE

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

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

