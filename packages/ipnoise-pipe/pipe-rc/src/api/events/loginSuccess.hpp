#ifndef API_EVENT_LOGIN_SUCCESS
#define API_EVENT_LOGIN_SUCCESS

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

class ApiEventLoginSuccess
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventLoginSuccess(Api *a_api);
        virtual ~ApiEventLoginSuccess();

        virtual void process();
};

#endif

