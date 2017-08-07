#ifndef MAIN_H
#define MAIN_H

int32_t constructor(Object *a_object);
void    destructor(Object *a_object);
void    action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size
);

#endif

