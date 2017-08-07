#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include "object.h"

struct object_window
{
    Object  super;
};
typedef struct object_window ObjectWindow;

void            void___object_window__destructor__Object_ptr(
    Object *a_object
);
ObjectWindow *  ObjectWindow_ptr___object_window__constructor();

#endif

