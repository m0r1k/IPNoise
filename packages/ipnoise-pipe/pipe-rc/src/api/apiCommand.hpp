#ifndef API_COMMAND_HPP
#define API_COMMAND_HPP

class Api;

#include <QObject>
#include <QString>
#include <QMap>

typedef QMap<QString, QString>          ApiCommandArgs;
typedef ApiCommandArgs::iterator        ApiCommandArgsIt;
typedef ApiCommandArgs::const_iterator  ApiCommandArgsConstIt;

class ApiCommand
    :   public QObject
{
    Q_OBJECT

    public:
        ApiCommand(
            Api             *a_api,
            const QString   &a_name
        );
        virtual ~ApiCommand();

        virtual void process(const ApiCommandArgs &a_args) = 0;

        QString     getName();
        Api *       getApi();
        QString     cmdStart(
            const ApiCommandArgs    &,
            const QString           &a_attrs = ""
        );
        QString     cmdEnd(const ApiCommandArgs &);

    private:
        Api         *m_api;
        QString     m_name;
};

#endif

