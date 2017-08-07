#include "api.hpp"
#include "login.hpp"

ApiCommandLogin::ApiCommandLogin(Api *a_api)
    :   ApiCommand(a_api, "login")
{
}

ApiCommandLogin::~ApiCommandLogin()
{
}

void ApiCommandLogin::process(
    const ApiCommandArgs &a_args)
{
    QString login;
    QString password;
    QString cmd;

    if (a_args.contains("login")){
        login = a_args["login"];
    }
    if (a_args.contains("password")){
        password = a_args["password"];
    }

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"login\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=             " login=\""    + login    + "\"";
    cmd +=             " password=\"" + password + "\"";

    if (a_args.contains("register")){
        cmd += " register=\"" + a_args["register"] + "\"";
    }

    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

