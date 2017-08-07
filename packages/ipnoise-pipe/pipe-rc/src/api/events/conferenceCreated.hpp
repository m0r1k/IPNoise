#ifndef API_EVENT_CONFERENCE_CREATED
#define API_EVENT_CONFERENCE_CREATED

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

class ApiEventConferenceCreated
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventConferenceCreated(Api *a_api);
        virtual ~ApiEventConferenceCreated();

        virtual void process();

        QString getConfId();
        QString getConfName();
};


#endif

