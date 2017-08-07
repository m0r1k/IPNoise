#ifndef API_COMMAND_DEL_GROUP_HPP
#define API_COMMAND_DEL_GROUP_HPP

#include "apiCommand.hpp"

class ApiCommandDelGroup
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandDelGroup(Api *a_api);
        virtual ~ApiCommandDelGroup();

        virtual void process(const ApiCommandArgs &);
};

#endif

