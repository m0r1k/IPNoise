#ifndef API_COMMAND_LOGIN_HPP
#define API_COMMAND_LOGIN_HPP

#include "apiCommand.hpp"

class ApiCommandLogin
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandLogin(Api *a_api);
        virtual ~ApiCommandLogin();

        virtual void process(const ApiCommandArgs &);
};

#endif

