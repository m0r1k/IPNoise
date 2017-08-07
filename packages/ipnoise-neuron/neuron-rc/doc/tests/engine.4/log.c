#include "log.h"

void get_function_name(
    const char      *a_func_name,
    char            *a_out,
    const uint32_t  a_out_size)
{
    char        *ptr        = NULL;
    uint32_t    out_size    = a_out_size;
    uint8_t     c;
    int32_t     i;

    ptr = strstr(a_func_name, "___");
    if (ptr){
        ptr += 3;
        i   =  0;
        c   = ptr[i];
        while (c != '\0'
            && (0 < (out_size - 1)))
        {
            a_out[i]        = c;
            a_out[i + 1]    = '\0';
            i++;
            out_size--;
            c = ptr[i];
        }

        // search method name
        ptr = strstr(a_out, "__");
        if (ptr){
            char *method_name   = ptr + 2;
            char *end_of_method = strstr(method_name, "__");
            if (end_of_method){
                *end_of_method = '\0';
            }
        }
    }
}

