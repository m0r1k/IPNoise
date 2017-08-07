#ifndef API_COMMAND_CONFERENCE_MSG_HPP
#define API_COMMAND_CONFERENCE_MSG_HPP

#include "apiCommand.hpp"

class ApiCommandConferenceMsg
    :   public ApiCommand
{
    Q_OBJECT

    public:
        ApiCommandConferenceMsg(Api *a_api);
        virtual ~ApiCommandConferenceMsg();

        virtual void process(const ApiCommandArgs &);
};

#endif

