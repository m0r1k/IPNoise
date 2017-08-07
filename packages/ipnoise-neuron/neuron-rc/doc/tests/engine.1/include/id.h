#ifndef ID_H
#define ID_H

#include <stdint.h>

#define OBJECT_ID_BYTES 16

char * generate_id(const uint32_t a_len_bytes);
char * generate_id2(const uint32_t a_len_bytes);

#endif

