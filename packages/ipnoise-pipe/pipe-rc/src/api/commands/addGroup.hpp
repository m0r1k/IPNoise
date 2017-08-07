#ifndef API_COMMAND_ADD_GROUP_HPP
#define API_COMMAND_ADD_GROUP_HPP

#include "apiCommand.hpp"

class ApiCommandAddGroup
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandAddGroup(Api *a_api);
        virtual ~ApiCommandAddGroup();

        virtual void process(const ApiCommandArgs &);
};

#endif

