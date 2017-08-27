#ifndef API_EVENT_CONFERENCE_JOIN
#define API_EVENT_CONFERENCE_JOIN

#include "apiEvent.hpp"

#include <QtCore/QString>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

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

