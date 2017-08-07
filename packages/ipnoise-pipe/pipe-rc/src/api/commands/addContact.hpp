#ifndef API_COMMAND_ADD_CONTACT_HPP
#define API_COMMAND_ADD_CONTACT_HPP

#include "apiCommand.hpp"

class ApiCommandAddContact
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandAddContact(Api *a_api);
        virtual ~ApiCommandAddContact();

        virtual void process(const ApiCommandArgs &);
};

#endif

