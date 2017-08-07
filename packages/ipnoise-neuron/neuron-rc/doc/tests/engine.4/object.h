#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <pthread.h>
#include <glib.h>

#include <ipnoise-common/log_common.h>
#include "libevent.h"

#define OBJECT_CHECK(a_var)                                 \
    if (!a_var){                                            \
        TFATAL("missing argument: '" #a_var "'\n");         \
    } else if (0 >= ((Object *)a_var)->ref_count){          \
        TFATAL("invalid refcount: '%d'"                     \
            " for argument: '" #a_var "'\n",                \
            ((Object *)a_var)->ref_count                    \
        );                                                  \
    }

#define dynamic_pointer_cast(                               \
    a_class, a_object, a_type, a_class_name)                \
    (a_object                                               \
        && !strcmp(a_object->type,       a_type)            \
        && !strcmp(a_object->class_name, a_class_name))     \
            ? (a_class *)a_object                           \
            : NULL

typedef enum
{
    STATE_PREPROCESSOR          = 0,
    STATE_COMMENT_LINE,
    STATE_COMMENT_BLOCK,
    STATE_COMMENT_LINE_SKIP_ONE,
    STATE_COMMENT_BLOCK_SKIP_ONE,
    STATE_WORD_COLLECT,
    STATE_WORD_COLLECT_STRING1,
    STATE_WORD_COLLECT_STRING1_ESC,
    STATE_WORD_COLLECT_STRING2,
    STATE_WORD_COLLECT_STRING2_ESC,
    STATE_WORD_COLLECT_STRING_END,
    STATE_WORD_COLLECT_NUMBER_INTEGER,
    STATE_WORD_COLLECT_NUMBER_FLOAT,
    STATE_OPERATOR_TWO_SYMBOLS
} ParserState;

typedef enum
{
    FLAG_RESOLVER_CREATE_VAR        = (1 << 0), //  1
    FLAG_RESOLVER_DONT_RESOLVE_VARS = (1 << 1), //  2
    FLAG_RESOLVER_DONT_RESOLVE_PTRS = (1 << 2), //  4
    FLAG_BRACE_CODE                 = (1 << 3), //  8
    FLAG_BRACE_ARRAY                = (1 << 4), //  16
    FLAG_COMMA_NO_RETURN            = (1 << 5), //  32
    FLAG_COMMA_RETURN_ARRAY         = (1 << 6), //  64
    FLAG_BRACKET_RETURN_ARRAY       = (1 << 7), //  128
    FLAG_RETURN                     = (1 << 8), //  256
    FLAG_FUNCTION_CALL_NEW          = (1 << 9), //  512
} Flags;

typedef enum
{
    LINK_TYPE_PARENT,
    LINK_TYPE_NEIGH_LEFT,
    LINK_TYPE_NEIGH_RIGHT,
    LINK_TYPE_CHILD
} LinkType;

struct object
{
    // synchronization
    pthread_mutex_t         mutex;
    pthread_mutexattr_t     mutex_attr;

    // object info
    const char              *type;
    const char              *class_name;
    uint32_t                ref_count;

    // mem
    char                    *mem;
    int64_t                 mem_size;

    // parser info
    const char              *parser_type;
    int32_t                 parser_pos;
    int32_t                 parser_line;
    int32_t                 parser_col;

    // errors
    char                    *error;

    // links
    GHashTable              *parents;
    GHashTable              *neighs_left;
    GHashTable              *neighs_right;
    GHashTable              *children;

    void    (*destructor)(struct object *);
};
typedef struct object Object;

void        void___object__destructor___Object_ptr(
    Object      *a_object
);
Object *    Object_ptr___object__constructor___const_uint32t(
    const uint32_t a_size
);
Object *    Object_ptr___object__constructor();

int32_t     int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr(
    const char  *a_type,
    const char  *a_class,
    Object      **a_out
);
int32_t     int32t___object__create___Object_ptr_ptr(
    Object  **a_out
);
void        void___object__ref_dec___Object_ptr(Object *);
int32_t     int32t___object__parse___Object_ptr__const_char_ptr(
    Object      *a_object,
    const char  *a_code
);

int32_t         object__is_error(
    Object      *a_object
);
void            object__add_error(
    Object      *a_object,
    Object      *a_position,
    const char  *a_format,
    ...
);

int32_t mem_add_printf_va(
    char        **a_mem,
    const char  *a_format,
    va_list     *a_va
);

int32_t mem_add_printf(
    char        **a_mem,
    const char  *a_format,
    ...
);

int32_t object__mem_add_printf_va(
    Object      *a_object,
    const char  *a_format,
    va_list     *a_va
);

int32_t object__mem_add_printf(
    Object      *a_object,
    const char  *a_format,
    ...
);

void object__lock(Object *a_object);
void object__unlock(Object *a_object);

void    void___object__mem_set___Object_ptr__char_ptr__const_int64t(
    Object          *a_object,
    char            *a_data,
    const int64_t   a_size
);
int32_t int32t___object__mem_set___Object_ptr__const_double(
    Object          *a_object,
    const double    a_val
);
void    object__mem_free(Object  *a_object);
char *  object__mem_c_str(Object *a_object);

int32_t object__is_statement(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_statement
);

int32_t object__get_operator_weight(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_operator
);

int32_t object__is_operator(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_operator
);

int32_t object__var_get(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name,
    Object          **a_out,
    uint8_t         a_not_exist_is_error
);

#endif

