#include <string.h>

#include "shttp.h"
#include "cookie.h"

cookie::cookie()
{
    buffer = NULL;
}

#define COOK_NAMESTART  1
#define COOK_NAME       2
#define COOK_VALUE      3

void cookie::parse(char * str){
    buffer=strdup(str);
    unsigned char chr;
    char *name=buffer;
    char *value;
    int state=COOK_NAMESTART;
    int len=strlen(buffer);
    for( int i=0;i<len;i++)
    {
        chr=buffer[i];
        switch(state)
        {
            case COOK_NAMESTART:
                if(chr != ' ' && chr != '\n' && chr != '\n' && chr != '\t') {
                    name=&buffer[i];
                    state=COOK_NAME;
                }
                break;
            case COOK_NAME:
                if(chr =='=') {
                    buffer[i]='\0';
                    value=&buffer[i+1];
                    state=COOK_VALUE;
                }
                break;
            case COOK_VALUE:
                if(chr ==';') {
                    buffer[i]='\0';
                    state=COOK_NAMESTART;
                    pairs[name]=value;
                }
                break;

        }

    }
    if(state==COOK_VALUE) pairs[name]=value;
}

void cookie::set(struct evhttp_request *req, char *name, char *value, const time_t MaxAge,char *path,char *domain){
    stringstream out;
    out <<  name << '=' << value;
    if(domain!=NULL) out << "; Domain=" << domain;
    if(0 != MaxAge) {
        char tmp[81];
        strftime(tmp, 80, "%a, %d-%b-%Y %H:%M:%S GMT",gmtime(&MaxAge));
        out << "; expires=" << tmp;
    }
    if(path!=NULL)   out << "; Path=" << path;
    evhttp_add_header(req->output_headers, "Set-Cookie", out.str().c_str());
}

char *cookie::get(char * name){
    if(pairs.find(name)!=pairs.end()) {
        return (char *) pairs[name].c_str();
    }
    return NULL;
}
cookie::~cookie(){
    if(buffer!=NULL){
        free(buffer);
        buffer=NULL;
    }
}

