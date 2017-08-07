#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Logging
extern int debug_level;

#define PDEBUG(level, args...)                          \
    if (debug_level >= level){                          \
        fprintf(stderr, "[DEBUG: %2.2d] ", level);      \
        fprintf(stderr, args);                          \
    }

#define PINFO(args...) do { fprintf(stdout, args); } while(0);
#define PWARN(args...) do { fprintf(stderr, args); } while(0);
#define PERROR(args...) do {                                            \
    fprintf(stderr, "--------- ERROR --------- {\n");                   \
    fprintf(stderr, "%s:%d (%s)\n", __FILE__, __LINE__, __FUNCTION__);  \
    fprintf(stderr, COLOR_RED);                                         \
    fprintf(stderr, args);                                              \
    fprintf(stderr, COLOR_GRAY);                                        \
    perror("last error no: ");                                          \
    fprintf(stderr, "--------- ERROR --------- }\n");                   \
} while(0);

#define COLOR_RED   "\033[1;31m"
#define COLOR_GRAY  "\033[0;39m"

/*
 * Roman E. Chechnev Jan, 2009
 */

inline void hexdump (unsigned char *data, int len)
{
    int i, m = 0;
    unsigned char *buffer = NULL;
    unsigned char line[20];
    if (len <= 0){
        return;
    }
    buffer = (unsigned char *)malloc(len*10);
    if (buffer == NULL){
        PERROR("malloc() failed, was needed: '%d' byte(s)", len*10);
        return;
    }
    memset(buffer, 0x00, len*10);
    for (i = 0, m = 0; i < len + (16 - (len % 16)); i++, m++){
        if (i < len){
            sprintf((char *)(buffer + strlen((const char*)buffer)),
                "%2.2x ", (unsigned int)data[i]);
            if (data[i] > 32 && data[i] < 127){
                line[m] = data[i];
            } else {
                line[m] = '.';
            }
        } else {
            sprintf((char *)(buffer + strlen((const char*)buffer)),
                "   ");
            line[m] = ' ';
        }
        if (i && ((i + 1) % 8 == 0)){
            sprintf((char *)(buffer + strlen((const char*)buffer)),
                " ");
        }
        if (i && ((i + 1) % 16 == 0)){
            line[m + 1] = '\00';
            m = -1;
            sprintf((char *)(buffer + strlen((const char*)buffer)),
                " %s\n", line);
        }
    }
    sprintf((char *)(buffer + strlen((const char*)buffer)), "\n");
    fprintf (stderr, "%s\n", buffer);
    free(buffer);
};

#endif

