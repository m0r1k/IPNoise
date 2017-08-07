#ifndef MAIN_H
#define MAIN_H

#define VERSION "0.01"

#include "command.h"
#include "server.h"

class Instance
{
    public:
        Command *cmd;
        Server  *server;
};

#endif

