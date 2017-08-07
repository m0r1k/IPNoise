#ifndef API_EVENT_CONFERENCE_JOIN
#define API_EVENT_CONFERENCE_JOIN

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

class ApiEventConferenceJoin
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventConferenceJoin(Api *a_api);
        virtual ~ApiEventConferenceJoin();

        virtual void process();

        QString     getConfId();
        QString     getHuid();
};


#endif

