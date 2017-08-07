#ifndef API_COMMAND_SEARCH_USERS_HPP
#define API_COMMAND_SEARCH_USERS_HPP

#include "apiCommand.hpp"

class ApiCommandSearchUsers
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandSearchUsers(Api *a_api);
        virtual ~ApiCommandSearchUsers();

        virtual void process(const ApiCommandArgs &);
};

#endif

