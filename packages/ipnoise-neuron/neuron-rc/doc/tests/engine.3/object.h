#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <pthread.h>
#include <glib.h>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/list.h>
#include "libevent.h"

#define DEFAULT_DELIMETER ": "

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
    STATE_KEY_WAIT              = 0,
    STATE_DELIMETER_WAIT,
    STATE_VAL_COLLECT
} AssociativeArrayState;

typedef enum
{
    STATE_FOR_COLLECT_INITIAL   = 0,
    STATE_FOR_COLLECT_COND,
    STATE_FOR_COLLECT_NEXT
} ForState;


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

#define OBJECT__DUMP(a_object, a_format, ...)

#define OBJECT__DUMP1(a_object, a_format, ...)          \
    object__dump(                                       \
        __FILE__,                                       \
        __LINE__,                                       \
        __FUNCTION__,                                   \
        a_object,                                       \
        a_format,                                       \
        ## __VA_ARGS__                                  \
    );

#define OBJECT_CHECK(a_var)                             \
    if (!a_var){                                        \
        TFATAL("missing argument: '" #a_var "'\n");     \
    } else if (0 >= ((Object *)a_var)->ref_count){      \
        TFATAL("invalid refcount: '%d'"                 \
            " for argument: '" #a_var "'\n",            \
            ((Object *)a_var)->ref_count                \
        );                                              \
    }

struct object
{
    // object info
    const char              *type;
    const char              *parser_type;
    const char              *class_name;
    int32_t                 parser_pos;
    int32_t                 parser_line;
    int32_t                 parser_col;

    // synchronization
    pthread_mutex_t         mutex;
    pthread_mutexattr_t     mutex_attr;

    // virtual methods
    void                    (*destructor_cb)();
    void                    (*to_string_cb)(
        struct object   *a_object,
        struct object   *a_out
    );
    void                    (*serialize_cb)(
        struct object   *a_object,
        struct object   *a_out,
        const char      *a_delimeter
    );

    // context
    //struct object           *context;

    // memory
    GHashTable              *this;
    int32_t                 ref_count;
    void                    *mem;
    uint64_t                mem_size;
    uint64_t                mem_total_allocated;

    // parser
    int32_t                 max_operator_weight;
    struct event_base       *event_base;
    struct evdns_base       *dns_base;
    struct obect_array      *timers;
    struct obect_array      *code_queue;
};
typedef struct object Object;

struct list_item
{
    Object              *object;
    struct list_head    list;
};
typedef struct list_item ListItem;

void            list_item_free(ListItem *a_item);
ListItem *      list_item_alloc();
void            list_items_free(ListItem *a_list_head);
void            list_items_copy(
    ListItem    *a_list_head_dst,
    ListItem    *a_list_head_src
);
int32_t         list_item_left(
    ListItem        *a_list_head,
    ListItem        *a_pos,
    ListItem        **a_out_item,
    Object          **a_out_object
);
int32_t         list_item_right(
    ListItem        *a_list_head,
    ListItem        *a_pos,
    ListItem        **a_out_item,
    Object          **a_out_object
);

#define OBJECT__ITEMS_DUMP(                                 \
    a_window, a_code, a_format, ...)

#define OBJECT__ITEMS_DUMP1(                                \
    a_window, a_code, a_format, ...)                        \
    do {                                                    \
        object__items_dump(                                 \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            a_window,                                       \
            a_code,                                         \
            a_format,                                       \
            ## __VA_ARGS__                                  \
        );                                                  \
    } while (0);

#define dynamic_pointer_cast(                               \
    a_class, a_object, a_type, a_class_name)                \
    (a_object                                               \
        && !strcmp(a_object->type,       a_type)            \
        && !strcmp(a_object->class_name, a_class_name))     \
            ? (a_class *)a_object                           \
            : NULL

#include "object_code_define.h"
#include "object_action.h"
#include "object_string.h"
#include "object_array.h"
#include "object_number.h"

// ---------------- object key ----------------

struct object_key
{
    Object  super;
};
typedef struct object_key ObjectKey;

void        object_key__destructor_cb(Object *);
ObjectKey * object_key__constructor();
void        object_key__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
ObjectKey * object_key__constructor_str(
    const char      *a_str,
    Object          *a_prototype
);
ObjectKey * object_key__constructor_double(
    const double    a_str,
    Object          *a_prototype
);
const char * object_keyword__mem_c_str(
    ObjectKey *a_object
);

// ---------------- object value ----------------

struct object_value
{
    Object  super;
    Object  *object;
};
typedef struct object_value ObjectValue;

void            object_value__destructor_cb(Object *);
ObjectValue *   object_value__constructor();
void            object_value__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);

// ---------------- object ----------------

Object *    object__constructor_uint32_t(
    const char      *a_type,
    const char      *a_class_name,
    const uint32_t  a_size,
    Object          *a_prototype
);
Object *    object__constructor_window();
Object *    object__constructor(Object *a_prototype);
void        object__destructor_cb(Object *);
void        object__key_destroy_cb(gpointer a_data);
void        object__value_destroy_cb(gpointer a_data);
gboolean    object__key_equal_func(
    gconstpointer   object_a,
    gconstpointer   object_b
);

void        object__lock(Object *a_object);
void        object__unlock(Object *a_object);

void        object__ref_dec(Object *a_object);
void        object__ref_inc(Object *a_object);
void        object__code_define(
    Object          *a_object,
    const char      *a_name,
    Object *        (*a_func)(
        Object          *a_context,   // parser
        Object          *a_position, // file position or NULL
        ObjectArray     *a_args
    )
);

// get
int32_t     object__prop_ptr_get_obj_obj(
    Object          *a_object,
    ObjectKey       *a_key,
    ObjectValue     **a_out
);
int32_t     object__prop_get_obj_obj(
    Object          *a_object,
    ObjectKey       *a_key,
    Object          **a_out
);
int32_t     object__prop_get_str_obj(
    Object          *a_object,
    const char      *a_name,
    Object          **a_out
);
int32_t     object__prop_ptr_get_str_obj(
    Object      *a_object,
    const char  *a_name,
    ObjectValue **a_out
);
int32_t     object__prop_get_double_obj(
    Object          *a_object,
    const double    a_name,
    Object          **a_out
);
int32_t     object__prop_get_str_number(
    Object          *a_object,
    const char      *a_name,
    ObjectNumber    **a_out
);
int32_t     object__prop_get_str_double(
    Object          *a_object,
    const char      *a_name,
    double          *a_out
);
int32_t     object__prop_get_str_str(
    Object          *a_object,
    const char      *a_name,
    ObjectString    **a_out
);
int32_t     object__prop_get_str_array(
    Object          *a_object,
    const char      *a_name,
    ObjectArray     **a_out
);
int32_t     object__prop_get_obj_array(
    Object          *a_object,
    ObjectKey       *a_name,
    ObjectArray     **a_out
);

// set
void        object__prop_set_obj_obj(
    Object          *a_object,
    ObjectKey       *a_name,
    ObjectValue     *a_val
);
void        object__prop_set_str_double(
    Object          *a_object,
    const char      *a_name,
    const double    a_val
);
void        object__prop_set_obj_str(
    Object          *a_object,
    ObjectKey       *a_name,
    const char      *a_val
);
void        object__prop_set_str_str(
    Object          *a_object,
    const char      *a_name,
    const char      *a_val
);
void        object__prop_set_double_obj(
    Object          *a_object,
    const double    a_name,
    Object          *a_val
);
void        object__prop_set_str_obj(
    Object          *a_object,
    const char      *a_name,
    Object          *a_val
);

// mem
void object__mem_free(
    Object          *a_window,
    Object          *a_object
);
int32_t object__mem_malloc(
    Object          *a_window,
    Object          *a_object,
    uint64_t        a_size
);
const char * object__mem_c_str(
    Object          *a_object
);
uint32_t    object__mem_add_printf_va(
    Object          *a_object,
    const char      *a_format,
    va_list         *a_va
);
void        object__mem_set(
    Object          *a_object,
    char            *a_data,
    uint64_t        a_data_size
);
void        object__mem_set_copy_raw(
    Object          *a_object,
    const char      *a_data,
    uint64_t        a_data_size
);
void        object__mem_set_copy_str(
    Object          *a_object,
    const char      *a_data
);
void        object__mem_copy_obj(
    Object          *a_object,
    Object          *a_data
);
uint32_t    object__prop_mem_add_printf(
    Object          *a_object,
    const char      *a_name,
    const char      *a_format,
    ...
);
uint32_t    object__mem_add_printf(
    Object          *a_object,
    const char      *a_format,
    ...
);

// del
void        object__prop_del(
    Object          *a_object,
    Object          *a_name
);
void        object__prop_del_str(
    Object          *a_object,
    const char      *a_name
);
void object__prop_del_double(
    Object          *a_object,
    const double    a_name
);

// exist
int32_t     object__prop_exist_obj(
    Object          *a_object,
    Object          *a_name
);
int32_t     object__prop_exist_str(
    Object          *a_object,
    const char      *a_name
);

// call
Object *    object__call_with_args(
    Object          *a_object,
    const char      *a_method,
    ObjectArray     *a_args
);
Object *    object__call(
    Object          *a_object,
    const char      *a_method
);
// 1 argument
Object *        object__call_with_args1_obj(
    Object          *a_object,
    const char      *a_method,
    Object          *a_arg
);
ObjectString *  object__call_with_args1_obj_str(
    Object          *a_object,
    const char      *a_method,
    Object          *a_arg
);
Object *        object__call_with_args1_double(
    Object          *a_object,
    const char      *a_method,
    const double    a_arg
);

// 2 arguments
Object *    object__call_with_args2_obj(
    Object          *a_object,
    const char      *a_method,
    Object          *a_arg
);
Object *    object__call_with_args2_int32(
    Object          *a_object,
    const char      *a_method,
    const int32_t   a_arg
);

// to string
void        object__to_string_cb(
    Object          *a_object,
    Object          *a_out
);

// serialize
void        object__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void        object__serialize(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);

// length
void        object__length_get(
    Object          *,
    double          *a_out
);
void        object__length_set(
    Object          *a_object,
    const double    a_val
);

// dump
void        object__dump(
    const char      *a_file,
    const int32_t   a_line,
    const char      *a_function,
    Object          *a_object,
    const char      *a_format,
    ...
);

// api
Object *    object__api_addEventListener(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
void        object__dispatchEvent(
    Object          *a_object,
    ObjectAction    *a_action
);
Object *    object__api_dispatchEvent(
    Object          *a_object,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
Object *    object__get_window_context(
    Object      *a_context
);
int32_t object__item_resolve(
    Object      *a_context,
    Object      *a_position,
    ListItem    *a_item,
    Flags       *a_flags
);
int32_t object__var_get(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name,
    Object          **a_out,
    uint8_t         a_not_exist_is_error
);
int32_t     object__var_exist(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name
);
int32_t     object__var_set(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name,
    Object          *a_val
);

int32_t         object__is_error(
    Object          *a_context
);
void            object__add_error(
    Object          *a_context,
    Object          *a_position,
    const char      *a_format,
    ...
);
void            object__shutdown(
    Object          *a_context
);

// statements
void    object__statement_define(
    Object          *a_context,
    const char      *a_statement,
    Object *        (*a_func)(
        Object          *a_context,
        ListItem        *a_code,
        ListItem        *a_operator,
        Flags           *a_flags
    )
);
Object *    object__statement_if(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__statement_for(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_function(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_return(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__statement_assign(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_brace(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_brace_code(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_brace_array(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_brace_stack(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_new(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);
Object *    object__statement_var(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags
);

// operators
Object *    object__operator_dot(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_less_than(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_greater_than(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_less_eq_than(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_greater_eq_than(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_colon(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
// logical
Object *    object__operator__logical(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_logical_and(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_logical_or(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
// compare
Object *    object__operator_compare_equal(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_compare_not_equal(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
void        object__operator_define(
    Object    *a_context,
    const char      *a_operator,
    Object *        (*a_func)(
        Object    *a_context,
        ListItem        *a_code,
        ListItem        *a_operator,
        Flags           *a_flags
    )
);
Object *    object__operator_assign(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_addition_assign(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_subtraction_assign(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator__arithmetic(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_addition(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_subtraction(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator__inc_dec(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_inc(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_dec(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_multiplication(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_division(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_comma(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_semicolon(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_bracket_generic(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_bracket_function_define(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_bracket_function_call(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__operator_bracket(
    Object    *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);

// stack
Object *    object__stack_create(
    Object  *a_context,
    Object  *a_position
);
//Object *    object__stack_vars_get(
//    Object  *a_context
//);
//void        object__stack_vars_pop(
//    Object  *a_context
//);
void        object__state_set(
    Object  *a_context,
    const int32_t   a_val
);
int32_t     object__state_get(
    Object  *a_context
);

// item
ListItem *  object__add_item(
    Object    *a_context,
    ListItem        *a_code,
    const char      *a_context_type,
    Object          *a_object
);
void        object__items_dump(
    const char      *a_file,
    const int32_t   a_line,
    const char      *a_function,
    Object          *a_window,
    ListItem        *a_items,
    const char      *a_format,
    ...
);
int32_t     object__var_get_resolve(
    Object          *a_context,
    Object          *a_position, // file position or NULL
    Flags           *a_flags,
    Object          *a_object,
    Object          **a_out
);

Object *    object__process_operator(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags
);
Object *    object__process_code(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ListItem        *a_code,
    Flags           *a_flags
);
Object *    object__parse_file(
    Object          *a_context,
    ObjectString    *a_path
);
Object *    object__parse_buffer(
    Object          *a_context,
    Object          *a_position,
    ObjectString    *a_buffer
);

// api
Object *    object__api_alert(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
Object *    object__api_exit(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
Object *    object__api_setTimeout(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
Object *    object__api_Action(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);
Object *    object__api_listen(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);


#endif

