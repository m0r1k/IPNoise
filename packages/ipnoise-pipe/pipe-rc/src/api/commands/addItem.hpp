#ifndef API_COMMAND_ADD_ITEM_HPP
#define API_COMMAND_ADD_ITEM_HPP

#include "apiCommand.hpp"

class ApiCommandAddItem
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandAddItem(Api *a_api);
        virtual ~ApiCommandAddItem();

        virtual void process(const ApiCommandArgs &);
};

#endif

