#ifndef API_COMMAND_CONFERENCE_JOIN_HPP
#define API_COMMAND_CONFERENCE_JOIN_HPP

#include "apiCommand.hpp"

class ApiCommandConferenceJoin
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandConferenceJoin(Api *a_api);
        virtual ~ApiCommandConferenceJoin();

        virtual void process(const ApiCommandArgs &);
};

#endif

