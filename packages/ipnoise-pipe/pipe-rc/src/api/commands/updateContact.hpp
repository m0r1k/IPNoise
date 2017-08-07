#ifndef API_COMMAND_UPDATE_CONTACT_HPP
#define API_COMMAND_UPDATE_CONTACT_HPP

#include "apiCommand.hpp"

class ApiCommandUpdateContact
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandUpdateContact(Api *a_api);
        virtual ~ApiCommandUpdateContact();

        virtual void process(const ApiCommandArgs &);
};

#endif

