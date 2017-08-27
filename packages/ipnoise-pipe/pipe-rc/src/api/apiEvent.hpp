//class ApiEvent;
//#include "api.hpp"

#ifndef API_EVENT_HPP
#define API_EVENT_HPP

class Api;

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

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

