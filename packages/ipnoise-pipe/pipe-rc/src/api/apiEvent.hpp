//class ApiEvent;
//#include "api.hpp"

#ifndef API_EVENT_HPP
#define API_EVENT_HPP

class Api;

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QDomElement>

class ApiEvent
    :   public QObject
{
    Q_OBJECT

    public:
        ApiEvent(
            Api             *a_api,
            const QString   &a_name
        );
        virtual ~ApiEvent();

        virtual void process() = 0;

        QString         getName();
        Api *           getApi();

        void        process(QDomElement &a_event);
        QDomElement getElement();

    protected:
        QDomElement m_element;
        Api         *m_api;

    private:
        QString     m_name;
};

#endif

