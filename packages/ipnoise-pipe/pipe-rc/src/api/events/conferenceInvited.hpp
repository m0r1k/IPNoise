#ifndef API_EVENT_CONFERENCE_INVITED
#define API_EVENT_CONFERENCE_INVITED

#include "apiEvent.hpp"

#include <QtCore/QString>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventConferenceInvited
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventConferenceInvited(Api *a_api);
        virtual ~ApiEventConferenceInvited();

        virtual void process();

        QString     getConfId();
        QString     getHuid();
};


#endif

