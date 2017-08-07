#ifndef COOKIE_H
#define COOKIE_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <sys/queue.h>
#include <map>
#include <string>

using namespace std;

class cookie
{
    private:
        char *buffer;
        map<string,string> pairs;

    public:
        cookie();
        void parse(char * str);
        void set(struct evhttp_request * req,
            char * name,
            char * value,
            const time_t MaxAge=0,
            char * path   = NULL,
            char * domain = NULL
        );
        char *get(char * name);
        ~cookie();
};

#endif

