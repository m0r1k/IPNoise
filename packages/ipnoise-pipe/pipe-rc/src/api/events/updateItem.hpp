#ifndef API_EVENT_UPDATE_ITEM
#define API_EVENT_UPDATE_ITEM

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

class ApiEventUpdateItem
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventUpdateItem(Api *a_api);
        virtual ~ApiEventUpdateItem();

        virtual void process();
};


#endif

