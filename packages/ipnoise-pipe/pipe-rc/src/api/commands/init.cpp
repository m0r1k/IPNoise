#include "api.hpp"
#include "init.hpp"

ApiCommandInit::ApiCommandInit(
    Api *a_api)
    :   ApiCommand(a_api, "init")
{
}

ApiCommandInit::~ApiCommandInit()
{
}

void ApiCommandInit::process(
    const ApiCommandArgs &a_args)
{
    QString cmd;

    cmd += cmdStart(a_args, "flags=\"init\"");
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

