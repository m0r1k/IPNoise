#ifndef API_COMMAND_GETCONTACTLIST_HPP
#define API_COMMAND_GETCONTACTLIST_HPP

#include "apiCommand.hpp"

class ApiCommandGetContactList
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandGetContactList(Api *a_api);
        virtual ~ApiCommandGetContactList();

        virtual void process(const ApiCommandArgs &);
};

#endif

