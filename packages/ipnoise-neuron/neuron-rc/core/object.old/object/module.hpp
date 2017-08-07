#ifndef MODULE_HPP
#define MODULE_HPP

using namespace std;

// don't include objects here

#define INIT_PROP(a_object, a_name, a_val)                  \
    do {                                                    \
        a_val->setDirty(0);                                 \
        a_object->set##a_name(a_val);                       \
    } while (0);

#define SAVE_PROP(a_name, a_method)                         \
    do {                                                    \
        ObjectMapIt it;                                     \
        it = a_props->find(a_name);                         \
        if (a_props->end() != it){                          \
            /* already set */                               \
            break;                                          \
        }                                                   \
        a_props->add(a_name, a_method());                   \
    } while (0);

#endif

