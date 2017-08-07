#ifndef API_COMMAND_INIT_HPP
#define API_COMMAND_INIT_HPP

#include "apiCommand.hpp"

class ApiCommandInit
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandInit(Api *a_api);
        virtual ~ApiCommandInit();

        virtual void process(const ApiCommandArgs &);
};

#endif

