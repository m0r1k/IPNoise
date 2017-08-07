#include <iostream>
#include <fstream>

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

struct parts
{
    char    *part[1024];
    char    *_parts;
    int     count;
};

void initParts(struct parts *ps, char * str, char delimiter);
void initPartsMulti(struct parts *ps, char * str, char * delimiter);
void initPartsMulti2(struct parts *ps, char * str, char * delimiter);
void freeParts(struct parts *ps);

