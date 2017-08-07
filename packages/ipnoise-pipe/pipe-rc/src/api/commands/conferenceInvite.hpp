#ifndef API_COMMAND_CONFERENCE_INVITE_HPP
#define API_COMMAND_CONFERENCE_INVITE_HPP

#include "apiCommand.hpp"

class ApiCommandConferenceInvite
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandConferenceInvite(Api *a_api);
        virtual ~ApiCommandConferenceInvite();

        virtual void process(const ApiCommandArgs &);
};

#endif

