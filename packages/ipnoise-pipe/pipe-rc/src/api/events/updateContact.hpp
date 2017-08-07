#ifndef API_EVENT_UPDATE_CONTACT
#define API_EVENT_UPDATE_CONTACT

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

class ApiEventUpdateContact
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventUpdateContact(Api *a_api);
        virtual ~ApiEventUpdateContact();

        virtual void process();
};


#endif

