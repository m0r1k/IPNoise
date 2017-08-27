#ifndef API_EVENT_INIT
#define API_EVENT_INIT

#include "apiEvent.hpp"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class ApiEventInit
    :   public ApiEvent
{
    Q_OBJECT

    public:
        ApiEventInit(Api *a_api);
        virtual ~ApiEventInit();

        virtual void process();

        bool    isNew();

    private:
        bool    m_is_new;
};


#endif

