#ifndef API_EVENT_INIT
#define API_EVENT_INIT

#include "apiEvent.hpp"

#include <QDomDocument>
#include <QDomElement>

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

