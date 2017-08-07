#ifndef API_EVENT_GETCONTACTLIST_SUCCESS
#define API_EVENT_GETCONTACTLIST_SUCCESS

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

class ApiEventGetContactListSuccess
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventGetContactListSuccess(Api *a_api);
        virtual ~ApiEventGetContactListSuccess();

        virtual void process();
};


#endif

