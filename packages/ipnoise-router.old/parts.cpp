#include "parts.h"

using namespace std;

void initParts(struct parts *ps, char * str, char delimiter)
{
    int i;
    int len=strlen(str);
    ps->count=0;
    ps->_parts=strdup(str);
    ps->part[ps->count]=ps->_parts;
    ps->count++;
    for (i = 0; i < len;i++){
        if (delimiter == ps->_parts[i] || '\n'== ps->_parts[i]){
            ps->_parts[i]='\0';
            ps->part[ps->count]=&ps->_parts[i+1];
            ps->count++;
            if (ps->count >= 1024){
                break;
            }
        }
    }
}

void initPartsMulti(struct parts *ps, char * str, char * delimiter)
{
    int i;
    int len      = strlen(str);
    int delimLen = strlen(delimiter);
    ps->count    = 0;
    ps->_parts   = strdup(str);
    ps->part[ps->count] = ps->_parts;
    ps->count++;
    for (i = 0; i < len; i++){
        for (int j = 0; j < delimLen; j++){
            if (delimiter[j] == ps->_parts[i]){
                ps->_parts[i]='\0';
                ps->part[ps->count] = &ps->_parts[i+1];
                ps->count++;
                break;
            }
        }
        if (ps->count >= 1024){
            break;
        }
    }
}

void initPartsMulti2(struct parts *ps, char * str, char * delimiter)
{
    int i;
    int len      = strlen(str);
    int delimLen = strlen(delimiter);
    ps->count    = 0;
    ps->_parts   = strdup(str);
    ps->part[ps->count] = ps->_parts;
    ps->count++;
    for (i = 0; i < len; i++){
        for (int j = 0; j < delimLen; j++){
            if (delimiter[j] == ps->_parts[i]){
                ps->_parts[i] = '\0';
                break;
            }
        }

        if (i && ps->_parts[i]   != '\0'
              && ps->_parts[i-1] == '\0' )
        {
            ps->part[ps->count] = &ps->_parts[i];
            ps->count++;
        }

        if (ps->count >= 1024){
            break;
        }
    }
}

void freeParts(struct parts *ps)
{
    if (NULL != ps->_parts){
        free(ps->_parts);
    }
}

