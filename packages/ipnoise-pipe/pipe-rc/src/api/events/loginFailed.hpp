#ifndef API_EVENT_LOGIN_FAILED
#define API_EVENT_LOGIN_FAILED

#include "apiEvent.hpp"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventLoginFailed
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventLoginFailed(Api *a_api);
        virtual ~ApiEventLoginFailed();

        virtual void process();
};


#endif

