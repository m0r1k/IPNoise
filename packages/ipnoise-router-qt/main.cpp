#include <QCoreApplication>

#include "server.h"
#include "main.h"

int debug_level = 9;

Instance *inst = NULL;

int main(int argc, char *argv[])
{
    int err;
    QCoreApplication app(argc, argv);

    inst            = new Instance();
    inst->server    = new Server();
    inst->cmd       = new Command();

    inst->server->init();
    inst->cmd->init();

    PINFO("IPNoise routing daemon: '%s'\n", VERSION);

    err = app.exec();

    if (inst->cmd != NULL){
        delete inst->cmd;
        inst->cmd = NULL;
    }
    if (inst->server != NULL){
        delete inst->server;
    }
    if (inst != NULL){
        delete inst;
        inst = NULL;
    }

    return err;
}


