#ifndef API_EVENT_SEARCH_USERS_ANSWER
#define API_EVENT_SEARCH_USERS_ANSWER

#include "apiEvent.hpp"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventSearchUsersAnswer
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventSearchUsersAnswer(Api *a_api);
        virtual ~ApiEventSearchUsersAnswer();

        virtual void process();
};


#endif

