#ifndef API_EVENT_GETCONTACTLIST_SUCCESS
#define API_EVENT_GETCONTACTLIST_SUCCESS

#include "apiEvent.hpp"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

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

