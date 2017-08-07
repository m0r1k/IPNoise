#ifndef API_COMMAND_UPDATE_ITEM_HPP
#define API_COMMAND_UPDATE_ITEM_HPP

#include "apiCommand.hpp"

class ApiCommandUpdateItem
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandUpdateItem(Api *a_api);
        virtual ~ApiCommandUpdateItem();

        virtual void process(const ApiCommandArgs &);
};

#endif

