#ifndef API_COMMAND_LOGOUT_HPP
#define API_COMMAND_LOGOUT_HPP

#include "apiCommand.hpp"

class ApiCommandLogout
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandLogout(Api *a_api);
        virtual ~ApiCommandLogout();

        virtual void process(const ApiCommandArgs &);
};

#endif

