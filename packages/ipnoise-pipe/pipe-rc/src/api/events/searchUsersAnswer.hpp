#ifndef API_EVENT_SEARCH_USERS_ANSWER
#define API_EVENT_SEARCH_USERS_ANSWER

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

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

