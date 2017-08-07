#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "log.h"
#include "object_code_define.h"
#include "object_code_call.h"
#include "object_action.h"
#include "object_number.h"
#include "object_string.h"
#include "object_array.h"
#include "object_statement.h"
#include "object_boolean.h"
#include "object_operator.h"
#include "object_connection.h"
#include "object_timer.h"

#include "object.h"

// ---------------- list_item ----------------

void list_item_free(
    ListItem *a_item)
{
    if (!a_item){
        TFATAL("missing argument: 'a_item'\n");
    }

    if (a_item->object){
        object__ref_dec(a_item->object);
        a_item->object = NULL;
    }

    free(a_item);
}

ListItem * list_item_alloc()
{
    ListItem    *ret    = NULL;
    uint32_t    size    = sizeof(*ret);

    ret = calloc(1, size);
    if (!ret){
        TFATAL("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            size
        );
    }

    return ret;
}

// do free list (list must have listhead)
// unsafe, because a_list_head doesn't have mutex for lock
void list_items_free(
    ListItem    *a_list_head)
{
    struct list_head    *first  = NULL;
    ListItem            *item   = NULL;

    if (!a_list_head){
        TFATAL("missing argument: 'a_list_head'\n");
    }

    while (!list_empty(&a_list_head->list)){
        // ok, list is not empty
        first   = a_list_head->list.next;
        item    = list_entry(first, ListItem, list);

        // remove from queue
        list_del(first);

        // free header
        list_item_free(item);
    }
}

void list_items_copy(
    ListItem    *a_list_head_dst,
    ListItem    *a_list_head_src)
{
    struct list_head    *tmp, *cur = NULL;

    if (!a_list_head_dst){
        TFATAL("missing argument: 'a_list_head_dst'\n");
    }
    if (!a_list_head_src){
        TFATAL("missing argument: 'a_list_head_src'\n");
    }

    for (cur = a_list_head_src->list.next, tmp = cur->next;
        cur != &a_list_head_src->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;
        ListItem    *new_item   = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        new_item = list_item_alloc();
        if (!new_item){
            TFATAL("list_item_alloc() failed\n");
        }

        if (cur_object){
            object__ref_inc(cur_object);
        }
        new_item->object = cur_object;

        list_add_tail(&new_item->list, &a_list_head_dst->list);
    }
}

int32_t list_item_left(
    ListItem        *a_list_head,
    ListItem        *a_pos,
    ListItem        **a_out_item,
    Object          **a_out_object)
{
    int32_t     err     = -1;
    ListItem    *item   = NULL;

    if (!a_list_head){
        TFATAL("missing argument: 'a_list_head'\n");
    }
    if (!a_pos){
        TFATAL("missing argument: 'a_pos'\n");
    }

    *a_out_item     = NULL;
    *a_out_object   = NULL;

    if (a_pos == a_list_head){
        // skip list head
        goto fail;
    }

    if (a_pos->list.prev == &a_list_head->list){
        // skip list head
        goto fail;
    }

    // get left part
    item        = list_entry(a_pos->list.prev, ListItem, list);
    *a_out_item = item;
    if (item){
        *a_out_object = item->object;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t list_item_right(
    ListItem        *a_list_head,
    ListItem        *a_pos,
    ListItem        **a_out_item,
    Object          **a_out_object)
{
    int32_t     err     = -1;
    ListItem    *item   = NULL;

    if (!a_list_head){
        TFATAL("missing argument: 'a_list_head'\n");
    }
    if (!a_pos){
        TFATAL("missing argument: 'a_pos'\n");
    }

    *a_out_item     = NULL;
    *a_out_object   = NULL;

    if (a_pos == a_list_head){
        // skip list head
        goto fail;
    }

    if (a_pos->list.next == &a_list_head->list){
        // skip list head
        goto fail;
    }

    // get left part
    item        = list_entry(a_pos->list.next, ListItem, list);
    *a_out_item = item;
    if (item){
        *a_out_object = item->object;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

// ---------------- object key ----------------

void object_key__destructor_cb(
    Object *a_object)
{
    ObjectKey *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectKey, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (ukey_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectKey * object_key__constructor()
{
    ObjectKey   *object = NULL;
    uint32_t    size    = sizeof(*object);

    object = (ObjectKey *)object__constructor_uint32_t(
        "object_key",   // type
        "",             // class name
        size,
        NULL            // prototype
    );
    if (!object){
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            size
        );
        goto fail;
    }

    object->super.destructor_cb = object_key__destructor_cb;
    object->super.serialize_cb  = object_key__serialize_cb;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

out:
    return object;
fail:
    if (object){
        object__ref_dec((Object *)object);
        object = NULL;
    }
    goto out;
}

ObjectKey * object_key__constructor_str(
    const char  *a_str,
    Object      *a_prototype)
{
    ObjectKey *object_key = NULL;

    object_key = object_key__constructor(a_prototype);
    if (!object_key){
        TFATAL("object_key__constructor() failed\n");
    }

    object__mem_set_copy_str((Object *)object_key, a_str);

    return object_key;
}

ObjectKey * object_key__constructor_double(
    const double    a_str,
    Object          *a_prototype)
{
    ObjectKey   *object_key = NULL;
    char        buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%g",
        a_str
    );

    object_key = object_key__constructor(a_prototype);
    if (!object_key){
        TFATAL("object_key__constructor() failed\n");
    }

    object__mem_set_copy_str((Object *)object_key, buffer);

    return object_key;
}

void object_key__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object__mem_add_printf(
        a_out,
        "\"%s\"",
        object__mem_c_str(a_object)
    );
}

const char * object_key__mem_c_str(
    ObjectKey *a_object)
{
    const char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = object__mem_c_str((Object *)a_object);

    return ret;
}

// ---------------- object value ----------------

void object_value__destructor_cb(
    Object *a_object)
{
    ObjectValue *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectValue, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uval_t)object);
#endif

    if (object->object){
        object__ref_dec(object->object);
        object->object = NULL;
    }

    object__destructor_cb((Object *)object);
}

ObjectValue * object_value__constructor()
{
    ObjectValue  *object = NULL;
    //Object      *obj    = NULL;
    uint32_t    size    = sizeof(*object);

    object = (ObjectValue *)object__constructor_uint32_t(
        "object_value", // type
        "",             // class name
        size,
        NULL            // prototype
    );
    if (!object){
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            size
        );
        goto fail;
    }
    //obj = (Object *)object;

    object->super.destructor_cb = object_value__destructor_cb;
    object->super.serialize_cb  = object_value__serialize_cb;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

out:
    return object;
fail:
    if (object){
        object__ref_dec((Object *)object);
        object = NULL;
    }
    goto out;
}

void object_value__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object__mem_add_printf(
        a_out,
        "\"%s\"",
        (char *)a_object->mem
    );
}

// ---------------- object ----------------

void object__destructor_cb(
    Object *a_object)
{
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx), type: '%s'\n",
        (uint64_t)a_object,
        a_object->type
    );
#else
    PDEBUG(100, "object: (0x%x), type: '%s'\n",
        (uint32_t)a_object,
        a_object->type
    );
#endif

    if (a_object->timers){
        PWARN("a_object->timers->super.ref_count: '%d'\n",
            ((ObjectArray *)a_object->timers)->super.ref_count
        );
        object__ref_dec((Object *)a_object->timers);
        a_object->timers = NULL;
    }

    if (a_object->code_queue){
        PWARN("a_object->code_queue->super.ref_count: '%d'\n",
            ((ObjectArray *)a_object->code_queue)->super.ref_count
        );
        object__ref_dec((Object *)a_object->code_queue);
        a_object->code_queue = NULL;
    }

    if (a_object->dns_base){
        evdns_base_free(
            a_object->dns_base,
            0 // fail_requests
        );
        a_object->dns_base = NULL;
    }
    if (a_object->event_base){
        event_base_free(a_object->event_base);
        a_object->event_base = NULL;
    }

    // low level
    if (a_object->this){
        g_hash_table_destroy(a_object->this);
        a_object->this = NULL;
    }

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem = NULL;
    }
    a_object->mem_size = 0;

    free(a_object);
}

Object * object__constructor_uint32_t(
    const char      *a_type,
    const char      *a_class_name,
    const uint32_t  a_size,
    Object          *a_prototype)
{
    Object      *object     = NULL;
    uint32_t    min_size    = sizeof(*object);

    if (    !a_type
        ||  !strlen(a_type))
    {
        TFATAL("missing argument: 'a_type'\n");
    }

    if (min_size > a_size){
        TFATAL("assertion failed,"
            " sizeof(*object) > a_size,"
            " (%u > %u)\n",
            min_size,
            a_size
        );
    }

    object = (Object *)calloc(1, a_size);
    if (!object){
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            a_size
        );
        goto fail;
    }

    // init mutex
    pthread_mutexattr_init(&object->mutex_attr);
    pthread_mutexattr_settype(
        &object->mutex_attr,
        PTHREAD_MUTEX_RECURSIVE
    );
    pthread_mutex_init(
        &object->mutex,
        &object->mutex_attr
    );

    object->type            = a_type;
    object->class_name      = a_class_name;
    object->ref_count       = 1;
    object->destructor_cb   = object__destructor_cb;
    object->serialize_cb    = object__serialize_cb;
    object->to_string_cb    = object__to_string_cb;

    object->this = g_hash_table_new_full(
        g_str_hash,
        object__key_equal_func, //g_str_equal,
        object__key_destroy_cb,
        object__value_destroy_cb
    );

    if (a_prototype){
        object__ref_inc(a_prototype);
        object__prop_set_str_obj(
            object,
            "prototype",
            a_prototype
        );
    }

    if (    strcmp(a_type, "object_key")
        &&  strcmp(a_type, "object_value")
        &&  strcmp(a_type, "code_define")
        &&  strcmp(a_type, "code_call"))
    {
        object__code_define(object,
            "addEventListener", object__api_addEventListener
        );
        object__code_define(object,
            "dispatchEvent",    object__api_dispatchEvent
        );
    }

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

out:
    return object;
fail:
    if (object){
        object__destructor_cb(object);
        object = NULL;
    }
    goto out;
}

Object * object__constructor_window()
{
    Object      *object = NULL;
    uint32_t    size    = sizeof(*object);

    object = object__constructor_uint32_t(
        "object",   // type
        "Window",   // class_name
        size,
        NULL        // prototype
    );

    object__var_set(
        object,
        NULL,   // position
        "statements",
        object__constructor(object)
    );
    object__var_set(
        object,
        NULL,   // position
        "operators",
        object__constructor(object)
    );

    // init code queue
    object->code_queue = (struct obect_array *)
        object_array__constructor(object);
    if (!object->code_queue){
        PERROR("object_array__constructor() failed\n");
        goto fail;
    }

    // init event base
    object->event_base = event_base_new();
    if (!object->event_base){
        PERROR("event_base_new() failed\n");
        goto fail;
    }

    // event dns
    object->dns_base = evdns_base_new(
        object->event_base,
        1   // initialize_nameservers
    );
    if (!object->dns_base){
        PERROR("object->dns_base failed\n");
        goto fail;
    }

    // prepare stack's vars
    //object_object__stack_create(object);
    //vars = object_object__stack_vars_get(object);

    // FYI:
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence
    // http://www.w3schools.com/jsref/jsref_operators.asp
    // http://www.w3schools.com/jsref/jsref_statements.asp
    // http://en.wikipedia.org/wiki/Operators_in_C_and_C++

    // populate statements
    object__statement_define(
        object, "new",      object__statement_new
    );
    object__statement_define(
        object, "var",      object__statement_var
    );
    object__statement_define(
        object, "if",       object__statement_if
    );
    object__statement_define(
        object, "for",      object__statement_for
    );
    object__statement_define(
        object, "function", object__statement_function
    );
    object__statement_define(
        object, "=",        object__statement_assign
    );
    object__statement_define(
        object, "{",        object__statement_brace
    );

    // populate operators, order is important
    // first operators will have the lowest priority
    // 0
    object__operator_define(
        object, "return", object__operator_return
    );
    object__operator_define(
        object, ",",    object__operator_comma
    );
    // 3
    object__operator_define(
        object, "=",    object__operator_assign
    );
    object__operator_define(
        object, "+=",   object__operator_addition_assign
    );
    object__operator_define(
        object, "-=",   object__operator_subtraction_assign
    );
    object__operator_define(
        object, ":",    object__operator_colon
    );
    // 5
    object__operator_define(
        object, "||",   object__operator_logical_or
    );
    // 6
    object__operator_define(
        object, "&&",   object__operator_logical_and
    );
    // 10
    object__operator_define(
        object, "!=",   object__operator_compare_not_equal
    );
    object__operator_define(
        object, "==",   object__operator_compare_equal
    );
    // 11
    object__operator_define(
        object, "<=",   object__operator_greater_eq_than
    );
    object__operator_define(
        object, ">=",   object__operator_less_eq_than
    );
    object__operator_define(
        object, "<",    object__operator_greater_than
    );
    object__operator_define(
        object, ">",    object__operator_less_than
    );
    // 13
    object__operator_define(
        object, "-",    object__operator_subtraction
    );
    object__operator_define(
        object, "+",    object__operator_addition
    );
    // 14
    object__operator_define(
        object, "/",    object__operator_division
    );
    object__operator_define(
        object, "*",    object__operator_multiplication
    );
    // 16
    object__operator_define(
        object, "--",   object__operator_dec
    );
    object__operator_define(
        object, "++",   object__operator_inc
    );
    // 19
    object__operator_define(
        object, "(",    object__operator_bracket
    );
    // 20 (max priority)
    object__operator_define(
        object, ";",    object__operator_semicolon
    );

    // populate global methods
    object__code_define(object,
        "alert",        object__api_alert
    );
    object__code_define(object,
        "setTimeout",   object__api_setTimeout
    );
    object__code_define(object,
        "Action",       object__api_Action
    );

    // populate object's methods
    object__code_define(object,
        "listen",       object__api_listen
    );
    object__code_define(object,
        "exit",         object__api_exit
    );

    // setup ipnoise support
    //{
    //    ObjectIPNoise *ipnoise = NULL;
    //    ipnoise = object_ipnoise__constructor();
    //    object__prop_set_str_obj(
    //        object,
    //        "ipnoise",
    //        (Object *)ipnoise
    //    );
    //}

out:
    return object;

fail:
    goto out;
}

Object * object__constructor(
    Object *a_prototype)
{
    Object      *ret = NULL;
    uint32_t    size = sizeof(*ret);

    ret = object__constructor_uint32_t(
        "object",   // type
        "Object",   // class_name
        size,
        a_prototype
    );

    return ret;
}

// ---------------- g_hash_table ----------------

gboolean object__key_equal_func(
    gconstpointer   a_object1,
    gconstpointer   a_object2)
{
    Object      *object1 = (Object *)a_object1;
    Object      *object2 = (Object *)a_object2;
    gboolean    ret      = FALSE;

    object__lock(object1);
    object__lock(object2);

    if (    object1->mem_size
        &&  object1->mem_size == object2->mem_size
        &&  !memcmp(
                object1->mem,
                object2->mem,
                object1->mem_size
            )
        )
    {
        ret = TRUE;
    }

#ifdef __x86_64__
    PDEBUG(100,
    //PWARN(
        "(0x%lx"
        " '%s'"
        " size: '%lu')"
        "  %s "
        " (0x%lx"
        " '%s'"
        " size: '%lu')"
        "\n",
        (uint64_t)object1,
        (const char *)object1->mem,
        object1->mem_size,
        ret ? "==" : "!=",
        (uint64_t)object2,
        (const char *)object2->mem,
        object2->mem_size
    );
#else
    PDEBUG(100,
        "(0x%lx"
        " '%s'"
        " size: '%llu')"
        "  %s "
        " (0x%x"
        " '%s'"
        " size: '%llu')"
        "\n",
        (uint32_t)object1,
        (const char *)object1->mem,
        object1->mem_size,
        ret ? "==" : "!=",
        (uint32_t)object2,
        (const char *)object2->mem,
        object2->mem_size
    );
#endif

    object__unlock(object2);
    object__unlock(object1);

#ifdef __x86_64__
    PDEBUG(100, "compare object: 0x%lx with 0x%lx,"
        " ret: '%d'\n",
        (uint64_t)object1,
        (uint64_t)object2,
        ret
    );
#else
    PDEBUG(100, "compare object: 0x%x with 0x%x\n,"
        " ret: '%d'\n",
        (uint32_t)object1,
        (uint32_t)object2,
        ret
    );
#endif

    return ret;
}

void object__key_destroy_cb(
    gpointer a_data)
{
    Object *object = (Object *)a_data;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__ref_dec(object);
}

void object__value_destroy_cb(
    gpointer a_data)
{
    Object *object = (Object *)a_data;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    if (object){
        object__ref_dec(object);
    }
}

// ---------------- synchronize ----------------

void object__lock(
    Object *a_object)
{
    int32_t res;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    res = pthread_mutex_lock(&a_object->mutex);
    if (res){
        TFATAL("pthread_mutex_lock failed,"
            " res: '%d'\n",
            res
        );
    }
}

void object__unlock(
    Object *a_object)
{
    int32_t res;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    res = pthread_mutex_unlock(&a_object->mutex);
    if (res){
        TFATAL("pthread_mutex_unlock failed\n");
    }
}

// ---------------- reference count ----------------

void object__ref_dec(
    Object *a_object)
{
    OBJECT_CHECK(a_object);

    object__lock(a_object);
    a_object->ref_count--;
    object__unlock(a_object);

    if (0 > a_object->ref_count){
        TFATAL("0 > a_object->ref_count (%d)\n",
            a_object->ref_count
        );
    }

    if (!a_object->ref_count){
#ifdef __x86_64__
        PDEBUG(100, "object: (0x%lx), refcount: '%u'\n",
            (uint64_t)a_object,
            a_object->ref_count
        );
#else
        PDEBUG(100, "object: (0x%x), refcount: '%u'\n",
            (uint32_t)a_object,
            a_object->ref_count
        );
#endif
        a_object->destructor_cb(a_object);
    }
}

void object__ref_inc(
    Object *a_object)
{
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object__lock(a_object);
    a_object->ref_count++;
    object__unlock(a_object);
}

// ---------------- code ----------------

void object__code_define(
    Object      *a_object,
    const char  *a_name,
    Object *    (*a_func)(
        Object          *a_object,
        Object          *a_position, // file position or NULL
        ObjectArray     *a_args
    ))
{
    ObjectKey           *key            = NULL;
    ObjectValue         *val            = NULL;
    ObjectCodeDefine    *code_define    = NULL;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        TFATAL("empty argument: 'a_name'\n");
    }

    // key
    key = object_key__constructor(a_object);
    if (!key){
        PERROR("object_key_constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)key, a_name);

    // code_define
    code_define = object_code_define__constructor(a_object);
    if (!code_define){
        PERROR("object_code_define_constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)code_define, a_name);
    code_define->func = a_func;

    // val
    val = object_value__constructor(a_object);
    if (!val){
        PERROR("object_value_constructor() failed\n");
        goto fail;
    }
    object__ref_inc((Object *)code_define);
    val->object = (Object *)code_define;

    // insert
    object__ref_inc((Object *)key);
    object__ref_inc((Object *)val);
    object__prop_set_obj_obj(
        a_object,
        key,
        val
    );

out:
    if (key){
        object__ref_dec((Object *)key);
    }
    if (val){
        object__ref_dec((Object *)val);
    }
    if (code_define){
        object__ref_dec((Object *)code_define);
    }
    return;
fail:
    goto out;
}

// ---------------- prop get val ----------------

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_ptr_get_obj_obj(
    Object          *a_object,
    ObjectKey       *a_key,
    ObjectValue     **a_out)
{
    ObjectValue *value      = NULL;
    Object      *val        = NULL;
    int32_t     res, err    = -1;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_key);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    // search
    object__lock(a_object);

    val = g_hash_table_lookup(
        a_object->this,
        a_key
    );
    if (!val){
        // not found or empty
        res = g_hash_table_contains(
            a_object->this,
            a_key
        );
        if (res){
            // empty
            object__unlock(a_object);
            TFATAL("object's key exist,"
                " but object's value is NULL\n"
            );
        }

        // not found
        *a_out = NULL;
        goto out;
    }

    value = dynamic_pointer_cast(
        ObjectValue,
        val,
        "object_value",
        ""
    );
    if (!value){
        object__unlock(a_object);
        TFATAL("object's key exist,"
            " but object's value type"
            " is not 'object_value' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
    }

    // MORIK
    //if (value->object){
    //    object__ref_inc(value->object);
    //}
    object__ref_inc((Object *)value);
    *a_out = value;

    // mark as exist
    err = 0;

out:
    object__unlock(a_object);

    return err;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_obj_obj(
    Object      *a_object,
    ObjectKey   *a_key,
    Object      **a_out)
{
    ObjectValue *value  = NULL;
    int32_t     err     = -1;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_key);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    // search
    object__lock(a_object);

    object__prop_ptr_get_obj_obj(
        a_object,
        a_key,
        &value
    );

    if (value){
        if (value->object){
            object__ref_inc(value->object);
        }
        *a_out = value->object;
        object__ref_dec((Object *)value);
        // mark as exist
        err = 0;
    } else {
        *a_out = NULL;
    }

    object__unlock(a_object);

    return err;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_str_obj(
    Object      *a_object,
    const char  *a_name,
    Object      **a_out)
{
    ObjectKey   *key    = NULL;
    int32_t     err     = -1;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        TFATAL("empty argument: 'a_name'\n");
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    // create temporary key
    key = object_key__constructor_str(a_name, a_object);
    if (!key){
        TFATAL("object_key_constructor_str() failed\n");
    }

    // search
    err = object__prop_get_obj_obj(
        a_object,
        key,
        a_out
    );

    // free key
    object__ref_dec((Object *)key);

    return err;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_ptr_get_str_obj(
    Object      *a_object,
    const char  *a_name,
    ObjectValue **a_out)
{
    ObjectKey   *key    = NULL;
    int32_t     err     = -1;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        TFATAL("empty argument: 'a_name'\n");
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    // create temporary key
    key = object_key__constructor_str(a_name, a_object);
    if (!key){
        TFATAL("object_key_constructor_str() failed\n");
    }

    // search
    err = object__prop_ptr_get_obj_obj(
        a_object,
        key,
        a_out
    );

    // free key
    object__ref_dec((Object *)key);

    return err;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_double_obj(
    Object          *a_object,
    const double    a_name,
    Object          **a_out)
{
    ObjectKey   *key    = NULL;
    int32_t     err     = -1;

    OBJECT_CHECK(a_object);

    // create temporary key
    key = object_key__constructor_double(a_name, a_object);
    if (!key){
        TFATAL("object_key_constructor_double() failed\n");
    }

    // search
    err = object__prop_get_obj_obj(
        a_object,
        key,
        a_out
    );

    // free key
    object__ref_dec((Object *)key);

    return err;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_str_number(
    Object          *a_object,
    const char      *a_name,
    ObjectNumber    **a_out)
{
    Object          *val        = NULL;
    ObjectNumber    *prop       = NULL;
    int32_t         res, err    = -1;

    OBJECT_CHECK(a_object);

    res = object__prop_get_str_obj(
        a_object,
        a_name,
        &val
    );
    if (res){
        err = res;
        goto fail;
    }

    prop = dynamic_pointer_cast(
        ObjectNumber,
        val,
        "number",
        ""
    );
    if (!prop){
        PWARN("prop: '%s' was found,"
            " but prop's type is not int32 (%s)\n",
            a_name,
            val->type
        );
        goto fail;
    }

    *a_out = prop;

    // all ok, value was found
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_str_double(
    Object      *a_object,
    const char  *a_name,
    double      *a_out)
{
    ObjectNumber    *val        = NULL;
    int32_t         res, err    = -1;

    OBJECT_CHECK(a_object);

    res = object__prop_get_str_number(
        a_object,
        a_name,
        &val
    );
    if (res){
        err = res;
        goto fail;
    }

    *a_out = *((double *)val->super.mem);

    // all ok, value was found
    err = 0;

out:
    if (val){
        object__ref_dec((Object *)val);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_str_str(
    Object          *a_object,
    const char      *a_name,
    ObjectString    **a_out)
{
    Object       *val       = NULL;
    ObjectString *prop      = NULL;
    int32_t      res, err   = -1;

    OBJECT_CHECK(a_object);

    res = object__prop_get_str_obj(
        a_object,
        a_name,
        &val
    );
    if (res){
        err = res;
        goto fail;
    }

    prop = dynamic_pointer_cast(
        ObjectString,
        val,
        "string",
        ""
    );
    if (!prop){
        PWARN("prop: '%s' was found,"
            " but prop's type is not string (%s)\n",
            a_name,
            val->type
        );
        goto out;
    }

    *a_out = prop;

    // all ok, value was found
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_str_array(
    Object          *a_object,
    const char      *a_name,
    ObjectArray     **a_out)
{
    Object      *val     = NULL;
    ObjectArray *prop    = NULL;
    int32_t     res, err = -1;

    OBJECT_CHECK(a_object);

    res = object__prop_get_str_obj(
        a_object,
        a_name,
        &val
    );
    if (res){
        err = res;
        goto fail;
    }

    prop = dynamic_pointer_cast(
        ObjectArray,
        val,
        "object",
        "Array"
    );
    if (!prop){
        PWARN("prop: '%s' was found,"
            " but prop's type is not 'array' (%s)\n",
            a_name,
            val->type
        );
        goto out;
    }

    *a_out = prop;

    // all ok, value was found
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

//
// return: 0 - if prop was placed to a_out
//
int32_t object__prop_get_obj_array(
    Object          *a_object,
    ObjectKey       *a_name,
    ObjectArray     **a_out)
{
    Object          *val        = NULL;
    ObjectArray     *prop       = NULL;
    int32_t         res, err    = -1;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_name);

    res = object__prop_get_obj_obj(
        a_object,
        a_name,
        &val
    );
    if (res){
        err = res;
        goto fail;
    }

    prop = dynamic_pointer_cast(
        ObjectArray,
        val,
        "object",
        "Array"
    );
    if (!prop){
        PWARN("prop: '%s' was found,"
            " but prop's type is not array (%s)\n",
            object_key__mem_c_str(a_name),
            val->type
        );
        goto out;
    }

    *a_out = prop;

    // all ok, value was found
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

// ---------------- prop set val ----------------

void object__prop_set_obj_obj(
    Object      *a_object,
    ObjectKey   *a_name,
    ObjectValue *a_val)
{
    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_name);

    // insert
    object__lock(a_object);
    g_hash_table_insert(
        a_object->this,
        a_name,
        a_val
    );
    object__unlock(a_object);
}

void object__prop_set_str_double(
    Object          *a_object,
    const char      *a_name,
    const double    a_val)
{
    ObjectKey       *key    = NULL;
    ObjectValue     *value  = NULL;
    ObjectNumber    *val    = NULL;
    Object          *window = NULL;
    uint64_t        size    = 0;
    int32_t         res     = 0;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    window = object__get_window_context(a_object);
    OBJECT_CHECK(window);

    // key
    key = object_key__constructor(a_object);
    if (!key){
        PERROR("object_key_constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)key, a_name);

    // val
    val = object_number__constructor(a_object);
    if (!val){
        PERROR("object_number__constructor() failed\n");
        goto fail;
    }

    size = sizeof(a_val);
    res  = object__mem_malloc(window, (Object *)val, size);
    if (res){
#ifdef __x86_64__
        PERROR("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            size
        );
#else
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            size
        );
#endif
        goto fail;
    }
    memcpy(val->super.mem, &a_val, val->super.mem_size);

    // value
    value = object_value__constructor(a_object);
    if (!value){
        PERROR("object_value_constructor() failed\n");
        goto fail;
    }
    object__ref_inc((Object *)val);
    value->object = (Object *)val;

    // insert
    object__ref_inc((Object *)key);
    object__ref_inc((Object *)value);
    object__prop_set_obj_obj(
        a_object,
        key,
        value
    );

out:
    if (window){
        object__ref_dec(window);
    }
    if (key){
        object__ref_dec((Object *)key);
    }
    if (val){
        object__ref_dec((Object *)val);
    }
    if (value){
        object__ref_dec((Object *)value);
    }
    return;
fail:
    goto out;
}

void object__prop_set_obj_str(
    Object      *a_object,
    ObjectKey   *a_key,
    const char  *a_val)
{
    ObjectValue     *value  = NULL;
    ObjectString    *val    = NULL;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_key);
    if (!a_val){
        TFATAL("missing argument: 'a_val'\n");
    }

    // val
    val = object_string__constructor(a_object);
    if (!val){
        PERROR("object_string__constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)val, a_val);

    // value
    value = object_value__constructor(a_object);
    if (!value){
        PERROR("object_value__constructor() failed\n");
        goto fail;
    }

    object__ref_inc((Object *)val);
    value->object = (Object *)val;

    // insert
    object__ref_inc((Object *)value);
    object__prop_set_obj_obj(
        a_object,
        a_key,
        value
    );

out:
    if (val){
        object__ref_dec((Object *)val);
    }
    if (value){
        object__ref_dec((Object *)value);
    }

    return;
fail:
    goto out;
}

void object__prop_set_str_str(
    Object      *a_object,
    const char  *a_name,
    const char  *a_val)
{
    ObjectKey       *key    = NULL;
    ObjectString    *val    = NULL;
    ObjectValue     *value  = NULL;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    // key
    key = object_key__constructor(a_object);
    if (!key){
        PERROR("object_key__constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)key, a_name);

    // val
    val = object_string__constructor(a_object);
    if (!val){
        PERROR("object_string__constructor() failed\n");
        goto fail;
    }

    object__mem_set_copy_str((Object *)val, a_val);

    // value
    value = object_value__constructor(a_object);
    if (!value){
        PERROR("object_value__constructor() failed\n");
        goto fail;
    }

    object__ref_inc((Object *)val);
    value->object = (Object *)val;

    // insert
    object__ref_inc((Object *)key);
    object__ref_inc((Object *)value);
    object__prop_set_obj_obj(
        a_object,
        key,
        value
    );

out:
    if (key){
        object__ref_dec((Object *)key);
    }
    if (val){
        object__ref_dec((Object *)val);
    }
    if (value){
        object__ref_dec((Object *)value);
    }
    return;
fail:
    goto out;
}

void object__prop_set_double_obj(
    Object          *a_object,
    const double    a_name,
    Object          *a_val)
{
    ObjectKey   *key    = NULL;
    ObjectValue *value  = NULL;

    OBJECT_CHECK(a_object);

    // key
    key = object_key__constructor_double(a_name, a_object);
    if (!key){
        PERROR("object_key__constructor_double() failed\n");
        goto fail;
    }

    // value
    value = object_value__constructor(a_object);
    if (!value){
        PERROR("object_value__constructor() failed\n");
        goto fail;
    }
    value->object = a_val;

    // insert
    object__ref_inc((Object *)key);
    object__prop_set_obj_obj(
        a_object,
        key,
        value
    );

out:
    if (key){
        object__ref_dec((Object *)key);
    }
    return;
fail:
    goto out;
}

void object__prop_set_str_obj(
    Object      *a_object,
    const char  *a_name,
    Object      *a_val)
{
    ObjectKey   *key    = NULL;
    ObjectValue *value  = NULL;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    // key
    key = object_key__constructor(a_object);
    if (!key){
        PERROR("object_key_constructor() failed\n");
    }

    object__mem_set_copy_str((Object *)key, a_name);

    // value
    value = object_value__constructor(a_object);
    if (!value){
        PERROR("object_value__constructor()\n");
        goto fail;
    }

    value->object = a_val;

    // insert
    object__ref_inc((Object *)key);
    object__ref_inc((Object *)value);
    object__prop_set_obj_obj(
        a_object,
        key,
        value
    );

out:
    if (key){
        object__ref_dec((Object *)key);
    }
    if (value){
        object__ref_dec((Object *)value);
    }

    return;
fail:
    goto out;
}

// ---------------- works with memory ----------------

void object__mem_free(
    Object      *a_window,
    Object      *a_object)
{
    OBJECT_CHECK(a_window);
    OBJECT_CHECK(a_object);

    if (a_object->mem){
        a_window->mem_total_allocated -= a_object->mem_size;
        free(a_object->mem);
    }

    a_object->mem_size = 0;
}

int32_t object__mem_malloc(
    Object      *a_window,
    Object      *a_object,
    uint64_t    a_size)
{
    int32_t err = -1;

    OBJECT_CHECK(a_window);
    OBJECT_CHECK(a_object);

    object__mem_free(a_window, a_object);

    a_object->mem = malloc(a_size);
    if (!a_object->mem){
        object__add_error(
            a_object,
            a_object,   // position
            "cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            a_size
        );
        goto fail;
    }

    a_window->mem_total_allocated   += a_size;
    a_object->mem_size              = a_size;

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

const char * object__mem_c_str(
    Object *a_object)
{
    const char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = (const char *)a_object->mem;

    return ret;
}

uint32_t object__mem_add_printf_va(
    Object      *a_object,
    const char  *a_format,
    va_list     *a_va)
{
    double      len     = 0;
    char        *tmp    = NULL;
    int32_t     res;

    OBJECT_CHECK(a_object);
    if (!a_format){
        TFATAL("missing argument: 'a_format'\n");
    }
    if (!a_va){
        TFATAL("missing argument: 'a_va'\n");
    }

    object__lock(a_object);

    // get cur value length
    if (a_object->mem){
        len = strlen(a_object->mem);
    }

    //PWARN("old size: '%lu', old len: '%g', old val: '%s'\n",
    //    a_object->mem_size,
    //    len,
    //    a_object->mem ? (const char *)a_object->mem : ""
    //);

    res = vasprintf(&tmp, a_format, *a_va);
    if (!tmp){
        TFATAL("vasprintf failed\n");
    }
    if (0 >= res){
        free(tmp);
        TFATAL("vasprintf failed, res: '%d'\n", res);
    }

    //PWARN("tmp: '%s', res: '%d'\n", tmp, res);

    a_object->mem_size = len + res + 1;
    a_object->mem      = (char *)(realloc(
        a_object->mem,
        a_object->mem_size
    ));

    // copy new data
    strcpy(&((char *)a_object->mem)[(size_t)len], tmp);

    //PWARN("new size: '%lu', new len: '%g', new val: '%s'\n",
    //    a_object->mem_size,
    //    len + res,
    //    a_object->mem ? (const char *)a_object->mem : ""
    //);

    free(tmp);

    // update length prop
    object__length_set(
        a_object,
        len + res
    );

    object__unlock(a_object);

    return res;
}

uint32_t object__prop_mem_add_printf(
    Object      *a_object,
    const char  *a_name,
    const char  *a_format,
    ...)
{
    uint32_t    ret     = 0;
    Object      *val    = NULL;
    va_list     ap;
    int32_t     res;

    OBJECT_CHECK(a_object);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }
    if (!a_format){
        TFATAL("missing argument: 'a_format'\n");
    }

    object__lock(a_object);

    res = object__prop_get_str_obj(
        a_object,
        a_name,
        &val
    );
    if (res){
        TFATAL("cannot get prop: '%s'\n", a_name);
    }

    va_start(ap, a_format);
    ret = object__mem_add_printf_va(
        val,
        a_format,
        &ap
    );
    va_end(ap);

    object__ref_dec(val);

    object__unlock(a_object);

    return ret;
}

uint32_t object__mem_add_printf(
    Object      *a_object,
    const char  *a_format,
    ...)
{
    uint32_t ret = 0;
    va_list  ap;

    OBJECT_CHECK(a_object);

    object__lock(a_object);

    va_start(ap, a_format);
    ret = object__mem_add_printf_va(
        a_object,
        a_format,
        &ap
    );
    va_end(ap);

    object__unlock(a_object);

    return ret;
}

void object__mem_set(
    Object      *a_object,
    char        *a_data,
    uint64_t    a_data_size)
{
    OBJECT_CHECK(a_object);
    if (!a_data){
        TFATAL("missing argument: 'a_data'\n");
    }

    object__lock(a_object);

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem       = NULL;
        a_object->mem_size  = 0;
    }

    a_object->mem       = a_data,
    a_object->mem_size  = a_data_size;

    object__unlock(a_object);
}

void object__mem_set_copy_raw(
    Object      *a_object,
    const char  *a_data,
    uint64_t    a_data_size)
{
    int32_t res;
    Object  *window = NULL;

    OBJECT_CHECK(a_object);
    if (!a_data){
        TFATAL("missing argument: 'a_data'\n");
    }

    object__lock(a_object);

    window = object__get_window_context(a_object);
    OBJECT_CHECK(window);

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem       = NULL;
        a_object->mem_size  = 0;
    }

    res = object__mem_malloc(
        window,
        a_object,
        a_data_size
    );
    if (res){
        TFATAL("malloc failed,"
#ifdef __x86_64__
            " was needed: '%ld' bytes"
#else
            " was needed: '%d' bytes"
#endif
            "\n",
            a_data_size
        );
    }

    memcpy(a_object->mem, a_data, a_data_size);

    if (window){
        object__ref_dec(window);
    }

    object__unlock(a_object);
}

void object__mem_set_copy_str(
    Object      *a_object,
    const char  *a_data)
{
    OBJECT_CHECK(a_object);
    if (!a_data){
        TFATAL("missing argument: 'a_data'\n");
    }

    object__lock(a_object);

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem      = NULL;
        a_object->mem_size = 0;
    }

    a_object->mem      = strdup(a_data);
    a_object->mem_size = strlen(a_data) + 1;

    if (!a_object->mem){
#ifdef __x86_64__
        TFATAL("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            a_object->mem_size
        );
#else
        TFATAL("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            a_object->mem_size
        );
#endif
    }

    object__unlock(a_object);
}

void object__mem_copy_obj(
    Object      *a_object,
    Object      *a_data)
{
    int32_t res;
    Object  *window = NULL;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_data);

    object__lock(a_object);
    object__lock(a_data);

    window = object__get_window_context(a_object);
    OBJECT_CHECK(window);

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem_size = 0;
    }

    if (a_data->mem_size){
        res = object__mem_malloc(
            window,
            a_object,
            a_data->mem_size
        );
        if (res){
#ifdef __x86_64__
            TFATAL("cannot allocate memory,"
                " was needed: '%lu' byte(s)\n",
                a_data->mem_size
            );
#else
            TFATAL("cannot allocate memory,"
                " was needed: '%u' byte(s)\n",
                a_data->mem_size
            );
#endif
        }
        memcpy(a_object->mem, a_data->mem, a_data->mem_size);
    }

    if (window){
        object__ref_dec(window);
    }

    object__unlock(a_object);
    object__unlock(a_data);
}

// ---------------- prop del val ----------------

void object__prop_del_obj(
    Object  *a_object,
    Object  *a_name)
{
    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_name);

    object__lock(a_object);
    g_hash_table_remove(
        a_object->this,
        a_name
    );
    object__unlock(a_object);
}

void object__prop_del_str(
    Object      *a_object,
    const char  *a_name)
{
    ObjectKey  *key = NULL;

    OBJECT_CHECK(a_object);
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    key = object_key__constructor(a_object);
    if (!key){
        TFATAL("object_key__constructor() failed\n");
    }

    object__mem_set_copy_str((Object *)key, a_name);

    object__prop_del_obj(
        a_object,
        (Object *)key
    );

    object__ref_dec((Object *)key);
}

void object__prop_del_double(
    Object          *a_object,
    const double    a_name)
{
    ObjectKey   *key    = NULL;
    Object      *window = NULL;
    uint64_t    size    = 0;
    int32_t     res;

    OBJECT_CHECK(a_object);

    window = object__get_window_context(a_object);
    OBJECT_CHECK(window);

    key = object_key__constructor(a_object);
    if (!key){
        object__add_error(
            a_object,
            a_object,   // position
            "object_key__constructor() failed\n"
        );
        goto fail;
    }

    size = sizeof(a_name);
    res = object__mem_malloc(
        window,
        (Object *)key,
        size
    );
    if (res){
#ifdef __x86_64__
        object__add_error(
            a_object,
            a_object, // position
            "canot allocate memory,"
            " was needed '%lu' byte(s)\n",
            size
        );
#else
        object__add_error(
            a_object,
            a_object, // position
            "canot allocate memory,"
            " was needed '%llu' byte(s)\n",
            size
        );
#endif
        goto fail;
    }
    memcpy(key->super.mem, &a_name, size);

    object__prop_del_obj(
        a_object,
        (Object *)key
    );

out:
    if (key){
        object__ref_dec((Object *)key);
    }
    if (window){
        object__ref_dec(window);
    }

    return;

fail:
    goto out;
}

// ---------------- prop exist ----------------

int32_t object__prop_exist_obj(
    Object  *a_object,
    Object  *a_name)
{
    int32_t exist = 0;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_name);

    object__lock(a_object);
    exist = g_hash_table_contains(
        a_object->this,
        a_name
    );
    object__unlock(a_object);

    return exist;
}

int32_t object__prop_exist_str(
    Object      *a_object,
    const char  *a_name)
{
    ObjectKey   *key    = NULL;
    int32_t     found   = 0;

    OBJECT_CHECK(a_object);
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    // create temporary key
    key = object_key__constructor(a_object);
    if (!key){
        TFATAL("object_key_constructor() failed\n");
    }

    object__mem_set_copy_str((Object *)key, a_name);

    found = object__prop_exist_obj(
        a_object,
        (Object *)key
    );

    object__ref_dec((Object *)key);

    return found;
}

// ---------------- prop call val ----------------

//
// object__call_with_args will decrease a_args ref count
//
Object *    object__call_with_args(
    Object          *a_context,
    const char      *a_method,
    ObjectArray     *a_args)
{
    Object              *ret    = NULL;
    Object              *val    = NULL;
    ObjectCodeDefine    *code   = NULL;
    int32_t             res;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);
    if (!a_method){
        object__ref_dec((Object *)a_args);
        TFATAL("missing argument: 'a_method'\n");
    }

    // search
    //res = object__prop_get_str_obj(
    //    a_context,
    //    a_method,
    //    &val
    //);
    res = object__var_get(
        a_context,
        NULL,
        a_method,
        &val,
        1   // not exist is error
    );
    if (res){
        object__ref_dec((Object *)a_args);
        OBJECT__DUMP((Object *)a_context, "object dump: ");
#ifdef __x86_64__
        TFATAL("object: (0x%lx) (type: '%s'),"
            " method: '%s' not found\n",
            (uint64_t)a_context,
            a_context->type,
            a_method
        );
#else
        TFATAL("object: (0x%x) (type: '%s'),"
            " method: '%s' not found\n",
            (uint32_t)a_context,
            a_context->type,
            a_method
        );
#endif
    }

    code = dynamic_pointer_cast(
        ObjectCodeDefine,
        val,
        "code_define",
        ""
    );
    if (!code){
        object__ref_dec(val);
        object__ref_dec((Object *)a_args);
#ifdef __x86_64__
        TFATAL("object: (0x%lx),"
            " key type: '%s' is not 'code' (%s)\n",
            (uint64_t)a_context,
            a_method,
            val->type
        );
#else
        TFATAL("object: (0x%x),"
            " key: '%s' is not 'code' (%s)\n",
            (uint32_t)a_context,
            a_method,
            val->type
        );
#endif
    }

    ret = code->func(a_context, a_context, a_args);

    object__ref_dec((Object *)code);
    object__ref_dec((Object *)a_args);

    return ret;
}

//
// object__call_with_args1_obj will decrease 'a_arg' ref count
//
Object *    object__call_with_args1_obj(
    Object      *a_object,
    const char  *a_method,
    Object      *a_arg)
{
    Object      *ret    = NULL;
    ObjectArray *args   = NULL;

    if (!a_arg){
        TFATAL("missing argument: 'a_arg'\n");
    }
    if (!a_object){
        object__ref_dec(a_arg);
        TFATAL("missing argument: 'a_object'\n");
    }
    if (!a_method){
        object__ref_dec(a_arg);
        TFATAL("missing argument: 'a_method'\n");
    }

    args = object_array__constructor(a_object);
    if (!args){
        object__ref_dec(a_arg);
        TFATAL("object_array_constructor() failed\n");
    }

    // add arg
    object_array__push_obj(args, a_arg);

    // object__call_with_args will decrease a_args ref count
    // (a_args destructor will decrease a_arg ref count)
    ret = object__call_with_args(
        a_object,
        a_method,
        args
    );

    return ret;
}

// it will decrease a_arg ref count
ObjectString * object__call_with_args1_obj_str(
    Object          *a_object,
    const char      *a_method,
    Object          *a_arg)
{
    ObjectString    *ret    = NULL;
    Object          *val    = NULL;

    OBJECT_CHECK(a_object);

    val = object__call_with_args1_obj(
        a_object,
        a_method,
        a_arg
    );
    if (!val){
        goto out;
    }

    ret = dynamic_pointer_cast(
        ObjectString,
        val,
        "string",
        ""
    );
    if (!ret){
        // free return value
        object__ref_dec(val);
        val = NULL;

#ifdef __x86_64__
        PWARN("object: 0x%lx, method: '%s',"
            " return value type is not 'string' (%s)\n",
            (uint64_t)val,
            a_method,
            val->type
        );
#else
        PWARN("object: 0x%x, method: '%s',"
            " return value type is not 'string' (%s)\n",
            (uint32_t)val,
            a_method,
            val->type
        );
#endif
        goto fail;
    }

out:
    return ret;
fail:
    goto out;
}

Object *    object__call_with_args1_double(
    Object          *a_object,
    const char      *a_method,
    const double    a_arg)
{
    Object      *ret    = NULL;
    ObjectArray *args   = NULL;

    OBJECT_CHECK(a_object);
    if (!a_method){
        TFATAL("missing argument: 'a_method'\n");
    }

    args = object_array__constructor(a_object);
    if (!args){
        TFATAL("object_array_constructor() failed\n");
    }

    // add arg
    object_array__push_double(args, a_arg);

    // object__call_with_args will decrease a_args ref count
    ret = object__call_with_args(
        a_object,
        a_method,
        args
    );

    return ret;
}

Object *    object__call(
    Object      *a_object,
    const char  *a_method)
{
    Object      *ret    = NULL;
    ObjectArray *args   = NULL;

    OBJECT_CHECK(a_object);
    if (!a_method){
        TFATAL("missing argument: 'a_method'\n");
    }

    args = object_array__constructor(a_object);
    if (!args){
        TFATAL("object_array__constructor() failed\n");
    }

    // object__call_with_args will decrease 'args' ref count
    ret = object__call_with_args(
        a_object,
        a_method,
        args
    );

    return ret;
}

// ---------------- to string ----------------

void object__to_string_cb(
    Object  *a_object,
    Object  *a_out)
{
    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_out);

    object__mem_add_printf(
        a_out,
        "[object %s]",
        a_object->class_name
    );
}

// ---------------- serialize ----------------

void object__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    GList   *keys       = NULL;
    GList   *cur_key    = NULL;
    int32_t res;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_out);
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)a_object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)a_object);
#endif

    // lock
    object__lock(a_object);
    object__lock(a_out);

    PDEBUG(100, "---"
        " There are '%d' key(s)"
        " in the hash table"
        " ---\n",
        g_hash_table_size(a_object->this)
    );

    object__mem_add_printf(a_out, "%s", "{");

    keys = g_hash_table_get_keys(a_object->this);
    for (cur_key = keys; cur_key; cur_key = cur_key->next){
        Object      *tmp = (Object *)cur_key->data;
        ObjectKey   *key = NULL;
        Object      *val = NULL;

        key = dynamic_pointer_cast(
            ObjectKey,
            tmp,
            "object_key",
            ""
        );
        if (!key){
            TFATAL("key is not object_key\n");
        }

        res = object__prop_get_obj_obj(
            a_object,
            key,
            &val
        );
        if (res){
            OBJECT__DUMP(val, "val: ");
            TFATAL("cannot get prop by key: '%s',"
                " val: 0x%lx\n",
                (const char *)key->super.mem,
                (uint64_t)val
            );
        }

        if (cur_key != keys){
            object__mem_add_printf(a_out, "%s", ", ");
        }

        object__serialize((Object *)key, a_out, a_delimeter);
        object__mem_add_printf(a_out, "%s", a_delimeter);

        if (!strcmp(object_key__mem_c_str(key), "prototype")){
            object__mem_add_printf(a_out, "undefined");
        } else if (val){
            if ((void *)val == (void *)a_object){
                object__mem_add_printf(a_out, "this");
            } else {
                object__serialize(val, a_out, a_delimeter);
            }
            object__ref_dec(val);
        } else {
            object__mem_add_printf(a_out, "undefined");
        }
    }

    object__mem_add_printf(a_out, "%s", "}");

    if (keys){
        g_list_free(keys);
        keys = NULL;
    }

    // unlock
    object__unlock(a_out);
    object__unlock(a_object);
}

void object__serialize(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_out);
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    if (!a_object->serialize_cb){
#ifdef __x86_64__
        TFATAL("object: (0x%lx), type: '%s',"
            " object doesn't support serialize\n",
            (uint64_t)a_object,
            a_object->type
        );
#else
        TFATAL("object: (0x%x), type: '%s',"
            " object doesn't support serialize\n",
            (uint32_t)a_object,
            a_object->type
        );
#endif
    }

    a_object->serialize_cb(
        a_object,
        a_out,
        a_delimeter
    );
}

// ---------------- length ----------------

void object__length_get(
    Object  *a_object,
    double  *a_out)
{
    int32_t res;

    OBJECT_CHECK(a_object);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    res = object__prop_get_str_double(
        (Object *)a_object,
        "length",
        a_out
    );
    if (res){
        TFATAL("cannot get object's length\n");
    }
}

void object__length_set(
    Object          *a_object,
    const double    a_val)
{
    OBJECT_CHECK(a_object);

    object__prop_set_str_double(
        (Object *)a_object,
        "length",
        a_val
    );
}

// ---------------- dump ----------------

void object__dump(
    const char      *a_file,
    const int32_t   a_line,
    const char      *a_function,
    Object          *a_object,
    const char      *a_format,
    ...)
{
    char        *descr  = NULL;
    Object      *dump   = NULL;
    va_list     ap;
    int32_t     res;

    dump = object__constructor(a_object);
    if (!dump){
        TFATAL_POS(
            a_file,
            a_line,
            a_function,
            "object__constructor() failed\n"
        );
    }

    // add descr
    {
        va_start(ap, a_format);
        res = vasprintf(&descr, a_format, ap);
        va_end(ap);

        if (!descr){
            object__ref_dec(dump);
            free(descr);
            TFATAL_POS(
                a_file,
                a_line,
                a_function,
                "vasprintf failed\n"
            );
        }
        if (0 >= res){
            object__ref_dec(dump);
            free(descr);
            TFATAL_POS(
                a_file,
                a_line,
                a_function,
                "vasprintf failed, res: '%d'\n",
                res
            );
        }

        object__mem_add_printf(dump, "%s", descr);
    }

    if (a_object){
        OBJECT_CHECK(a_object);
    }

    object__mem_add_printf(
        dump,
#ifdef __x86_64__
        "dump of object 0x%lx\n"
#else
        "dump of object 0x%x\n"
#endif
        "type:          '%s'\n"
        "class:         '%s'\n"
        "ref_count:     '%d'\n"
        "parser_type:   '%s'\n",
#ifdef __x86_64__
        (uint64_t)a_object,
#else
        (uint32_t)a_object,
#endif
        a_object ? a_object->type           : "undefined",
        a_object ? a_object->class_name     : "undefined",
        a_object ? a_object->ref_count      : -1,
        a_object ? a_object->parser_type    : "undefined"
    );

    if (a_object){
        a_object->serialize_cb(
            a_object,
            dump,
            DEFAULT_DELIMETER
        );
    } else {
        object__mem_add_printf(dump, "undefined");
    }

    PWARN_POS(
         a_file,
         a_line,
         a_function,
        "%s\n\n",
        (const char *)dump->mem
    );

    object__ref_dec(dump);
    free(descr);
}

void object__addEventListener(
    Object              *a_object,
    const char          *a_action_name,
    ObjectCodeDefine    *a_code_define)
{
    char    buffer[64]          = { 0x00 };
    Object  *handlers           = NULL;
    Object  *action_handlers    = NULL;
    int32_t res                 = 0;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_code_define);
    if (    !a_action_name
        ||  !strlen(a_action_name))
    {
        TFATAL("missing argument: 'a_action_name'\n");
    }

    //PWARN("add action: '%s'\n", a_action_name);
    //OBJECT__DUMP((Object *)a_code_define, "a_code_define: ");

#ifdef __x86_64__
    snprintf(buffer, sizeof(buffer),
        "0x%lx",
        (uint64_t)a_code_define
    );
#else
    snprintf(buffer, sizeof(buffer),
        "0x%x",
        (uint32_t)a_code_define
    );
#endif

    res = object__prop_get_str_obj(
        a_object,
        "handlers",
        &handlers
    );
    if (res){
        // handlers not found, create new
        handlers = object__constructor(a_object);
        if (!handlers){
            TFATAL("object__constructor() failed\n");
        }
        object__ref_inc((Object *)handlers);
        object__prop_set_str_obj(
            a_object,
            "handlers",
            handlers
        );
    }

    res = object__prop_get_str_obj(
        handlers,
        a_action_name,
        &action_handlers
    );
    if (res){
        // action handlers not found, create new
        action_handlers = object__constructor(a_object);
        if (!action_handlers){
            TFATAL("object__constructor() failed\n");
        }
        object__ref_inc((Object *)action_handlers);
        object__prop_set_str_obj(
            handlers,
            a_action_name,
            action_handlers
        );
    }

    // add handler
    object__ref_inc((Object *)a_code_define);
    object__prop_set_str_obj(
        action_handlers,
        buffer, // code_define addr
        (Object *)a_code_define
    );

    if (handlers){
        object__ref_dec((Object *)handlers);
    }
    if (action_handlers){
        object__ref_dec((Object *)action_handlers);
    }
}

// ---------------- events ----------------

Object *    object__api_addEventListener(
    Object          *a_object,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object              *ret            = NULL;
    Object              *val            = NULL;
    ObjectString        *action_name    = NULL;
    ObjectCodeDefine    *code_define    = NULL;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_args);

    object__lock((Object *)a_object);

    // get action name
    object_array__get_by_index(
        a_args,
        0,
        &val
    );
    action_name = dynamic_pointer_cast(
        ObjectString,
        val,
        "string",
        ""
    );
    if (!action_name){
        TFATAL("first argument (action name)"
            " is not 'string' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }

    // now works with timeout_obj instead val
    val = NULL;

    // get code
    object_array__get_by_index(
        a_args,
        1,
        &val
    );
    code_define = dynamic_pointer_cast(
        ObjectCodeDefine,
        val,
        "code_define",
        ""
    );
    if (!code_define){
        TFATAL("second argument is not 'code_define' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }

    // now works with code_define instead val
    val = NULL;

    //OBJECT__DUMP((Object *)action_name, "action_name: ");
    //OBJECT__DUMP((Object *)code_define, "code_define: ");

    object__addEventListener(
        a_object,
        object_string__mem_c_str(action_name),
        code_define
    );

out:
    if (val){
        object__ref_dec(val);
    }
    if (action_name){
        object__ref_dec((Object *)action_name);
    }
    if (code_define){
        object__ref_dec((Object *)code_define);
    }

    object__unlock((Object *)a_object);

    return ret;

fail:
    goto out;
}

// it will dec a_action ref count
void object__dispatchEvent(
    Object          *a_object,
    ObjectAction    *a_action)
{
    Object  *handlers           = NULL;
    Object  *action_handlers    = NULL;
    GList   *keys               = NULL;
    GList   *cur_key            = NULL;
    int32_t res;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_action);

    object__lock((Object *)a_object);

    // get handlers
    object__prop_get_str_obj(
        a_object,
        "handlers",
        &handlers
    );
    if (!handlers){
        goto out;
    }

    // get handlers for action
    object__prop_get_str_obj(
        handlers,
        object__mem_c_str((Object *)a_action),
        &action_handlers
    );
    if (!action_handlers){
        goto out;
    }

    keys = g_hash_table_get_keys(action_handlers->this);
    for (cur_key = keys; cur_key; cur_key = cur_key->next){
        Object              *tmp            = (Object *)cur_key->data;
        ObjectKey           *key            = NULL;
        Object              *val            = NULL;
        ObjectCodeDefine    *code_define    = NULL;

        key = dynamic_pointer_cast(
            ObjectKey,
            tmp,
            "object_key",
            ""
        );
        if (!key){
            TFATAL("key is not object_key\n");
        }

        res = object__prop_get_obj_obj(
            action_handlers,
            key,
            &val
        );
        if (res){
            OBJECT__DUMP(action_handlers, "action_handlers: ");
            OBJECT__DUMP(val, "val: ");
            TFATAL("cannot get prop by key: '%s',"
                " val: 0x%lx\n",
                object_key__mem_c_str(key),
                (uint64_t)val
            );
        }

        code_define = dynamic_pointer_cast(
            ObjectCodeDefine,
            val,
            "code_define",
            ""
        );
        if (!code_define){
            PWARN("skip: '%s'\n",
                object_key__mem_c_str(key)
            );
            object__ref_dec(val);
            continue;
        }

#ifdef __x86_64__
        {
            uint64_t func_addr = 0;
            res = sscanf(
                object_key__mem_c_str(key),
                "0x%lx",
                &func_addr
            );
        }
#else
        {
            uint32_t func_addr = 0;
            res = sscanf(
                object_key__mem_c_str(key),
                "0x%x",
                &func_addr
            );
        }
#endif
        if (res){
            Object          *val            = NULL;
            ObjectArray     *args           = NULL;

            args = object_array__constructor(a_object);

            object__ref_inc((Object *)a_action);
            object_array__push_obj(args, (Object *)a_action);

            val = object_code_call__call_with_args(
                code_define,
                a_object,
                a_object,   // position
                args
            );
            if (val){
                object__ref_dec(val);
            }
            object__ref_dec((Object *)args);
        }

        object__ref_dec((Object *)code_define);
    }

    if (keys){
        g_list_free(keys);
        keys = NULL;
    }

out:
    if (a_action){
        object__ref_dec((Object *)a_action);
    }
    if (handlers){
        object__ref_dec(handlers);
    }
    if (action_handlers){
        object__ref_dec(action_handlers);
    }

    object__unlock((Object *)a_object);

    return;
}

Object *    object__api_dispatchEvent(
    Object          *a_object,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object          *ret                = NULL;
    Object          *val                = NULL;
    ObjectAction    *action             = NULL;
    int32_t         res;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_args);

    object__lock((Object *)a_object);

    //OBJECT__DUMP((Object *)a_object,    "a_object: ");
    //OBJECT__DUMP((Object *)a_args,      "a_args: ");

    // get action
    res = object_array__shift(a_args, &val);
    if (res){
        PERROR("missing first argument\n");
        goto fail;
    }

    action = dynamic_pointer_cast(
        ObjectAction,
        val,
        "object",
        "Action"
    );
    if (!action){
        PERROR("first argument"
            " is not 'object Action'"
            " (type: '%s', class: '%s')"
            "\n",
            val ? val->type       : "undefined",
            val ? val->class_name : "undefined"
        );
        goto fail;
    }

    // now works with timeout_obj instead val
    val = NULL;

    object__dispatchEvent(
        a_object,
        action
    );

    // don't free action it will do 'object__dispatchEvent'
    action = NULL;

out:
    if (val){
        object__ref_dec(val);
    }
    if (action){
        object__ref_dec((Object *)action);
    }

    object__unlock((Object *)a_object);

    return ret;

fail:
    goto out;
}

int32_t object__var_exist(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name)
{
    Object      *val        = NULL;
    int32_t     res, ret    = 0;

    OBJECT_CHECK(a_context);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    object__lock(a_context);

    res = object__var_get(
        a_context,
        a_position,
        a_name,
        &val,
        0   // not exist is error
    );
    if (val){
        object__ref_dec(val);
    }
    ret = res ? 0 : 1;

    object__unlock(a_context);

    return ret;
}

Object *    object__get_parent_context(
    Object  *a_context)
{
    Object *ret = NULL;

    OBJECT_CHECK(a_context);

    object__prop_get_str_obj(
        a_context,
        "prototype",
        &ret
    );

    return ret;
}

Object *    object__get_window_context(
    Object  *a_context)
{
    Object  *window = NULL;
    Object  *cur    = NULL;

    OBJECT_CHECK(a_context);

    cur = a_context;
    object__ref_inc(cur);
    while (cur){
        if (    !strcmp(cur->type,       "object")
            &&  !strcmp(cur->class_name, "Window"))
        {
            window = cur;
            object__ref_inc(window);
            break;
        }
        object__ref_dec(cur);
        cur = object__get_parent_context(cur);
    }
    if (cur){
        object__ref_dec(cur);
    }

    return window;
}

int32_t object__item_resolve(
    Object      *a_context,
    Object      *a_position,
    ListItem    *a_item,
    Flags       *a_flags)
{
    int32_t res, err        = -1;
    Object  *val            = NULL;
    Object  *object         = NULL;
    Object  *new_context    = NULL;

    OBJECT_CHECK(a_context);
    if (!a_item){
        TFATAL("missing argument: 'a_item'\n");
    }

    object__lock(a_context);

    object = a_item->object;
    if (object){
        OBJECT_CHECK(object);
    }

    if (    object
        &&  object->parser_type
        &&  !strcmp(object->parser_type, "variable"))
    {
        if (*a_flags & FLAG_RESOLVER_DONT_RESOLVE_VARS){
            object__ref_inc(object);
            val = object;
        } else {
            if (*a_flags & FLAG_RESOLVER_CREATE_VAR){
                // create empty var if not exist
                res = object__var_exist(
                    a_context,
                    a_position,
                    object__mem_c_str(object)
                );
                if (!res){
                    res = object__var_set(
                        a_context,
                        a_position,
                        object__mem_c_str(object),
                        NULL
                    );
                }
            }

            res = object__var_get(
                a_context,
                a_position,
                object__mem_c_str(object),
                &val,
                1   // not exist is error
            );
            if (res){
                char *tmp = NULL;
                // next code doesn't work:
                // (looks like it's bug in libc 2.18)
                //
                // #0  vfprintf () from /lib64/libc.so.6
                // #1  vasprintf () from /lib64/libc.so.6
                // #2  object__mem_add_printf_va
                // #3  object__prop_mem_add_printf
                // #4  object__item_resolve
                // #5  object__process_code
                // #6  object__operator_semicolon
                // #7  object__process_operator
                //
                //object__add_error(
                //    a_context,
                //    object,
                //    "variable '%s' not found\n",
                //    (const char *)object->mem
                //);
                //
                // so do workaround

                tmp = strdup(object__mem_c_str(object));
                if (!tmp){
                    object__add_error(
                        a_context,
                        a_position,
                        "strdup() failed\n"
                    );
                    goto fail;
                }

                object__add_error(
                    a_context,
                    a_position,
                    "variable: '%s' not found\n",
                    tmp
                );

                free(tmp);
                goto fail;
            }
        }
        //PWARN("resolve var: '%s', flags: 0x%x, val: 0x%lx\n",
        //    object__mem_c_str(object),
        //    *a_flags,
        //    (uint64_t)val
        //);
    } else if ( object
        &&      object->type
        &&      !strcmp(object->type, "object_value"))
    {
        if (*a_flags & FLAG_RESOLVER_DONT_RESOLVE_PTRS){
            object__ref_inc(object);
            val = object;
        } else {
            ObjectValue *value = NULL;

            value = (ObjectValue *)object;
            if (value->object){
                object__ref_inc(value->object);
            }
            val = value->object;
        }
    } else if ( object
        &&      object->type
        &&      !strcmp(object->type, "code_call"))
    {
        ObjectCodeCall  *code_call      = NULL;

        code_call = (ObjectCodeCall *)object;

        OBJECT_CHECK(code_call->context);

        if (!code_call->code_define){
            TFATAL(
                "empty function's code\n"
            );
            goto fail;
        }

        if (!code_call->args){
            object__add_error(
                a_context,
                a_position,
                "empty function call args\n"
            );
            goto fail;
        }

        new_context = object__stack_create(
            a_context,
            a_position
        );

        //PWARN("function call: '%s', flags: 0x%x\n",
        //    object__mem_c_str(
        //        (Object *)code_call->code_define
        //    ),
        //    *a_flags
        //);

        // call handler
        if (code_call->code_define->func){
            // native function
            val = code_call->code_define->func(
                new_context,
                a_position,
                code_call->args
            );
        } else {
            // setup variables
            ListItem    instructions;
            double      pos, length = 0;

            if (!code_call->code_define->args_names){
                object__add_error(
                    a_context,
                    a_position,
                    "cannot get function's args names\n"
                );
                goto fail;
            }

            memset(&instructions, 0x00, sizeof(instructions));
            INIT_LIST_HEAD(&instructions.list);

            object__length_get(
                (Object *)code_call->code_define->args_names,
                &length
            );

            for (pos = 0; pos < length; pos++){
                Object *arg_name = NULL;
                Object *arg_val  = NULL;

                // get awaiting argument name
                object_array__get_by_index(
                    code_call->code_define->args_names,
                    pos,
                    &arg_name
                );
                if (!arg_name){
                    OBJECT__DUMP(
                        (Object *)code_call->code_define->args_names,
                        "code_call->code_define->args:\n"
                    );
                    //object__add_error(
                    //    a_context,
                    //    object,
                    //    "cannot get arg name for pos: '%g'\n",
                    //    pos
                    //);
                    goto fail;
                }

                // get val
                object_array__get_by_index(
                    code_call->args,
                    pos,
                    &arg_val
                );
                object__var_set(
                    new_context,
                    a_position,
                    object__mem_c_str(arg_name),
                    arg_val
                );

                object__ref_dec(arg_name);
            }

            // copy instructions
            res = object_array__copy_to_list(
                code_call->code_define->instructions,
                &instructions
            );
            if (res){
                object__add_error(
                    a_context,
                    a_position,
                    "object_array__copy_to_list() failed\n"
                );
                goto fail;
            }

            // process code
            if (!list_empty(&instructions.list)){
                Flags flags = 0;
                val = object__process_code(
                    new_context,
                    a_position,
                    &instructions,
                    &flags
                );
            }

            // free code
            list_items_free(&instructions);
        }

        // check on errors
        res = object__is_error(new_context);
        if (res){
            goto fail;
        }
    } else {
        if (object){
            object__ref_inc(object);
        }
        val = object;
    }

    // replace object on resolved object
    if (a_item->object){
        object__ref_dec(a_item->object);
    }
    a_item->object = val;

    // all ok
    err = 0;

out:
    if (new_context){
        object__ref_dec(new_context);
    }
    object__unlock(a_context);
    return err;
fail:
    goto out;
}

int32_t object__var_get(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name,
    Object          **a_out,
    uint8_t         a_not_exist_is_error)
{
    Object      *cur_context    = NULL;
    Object      *prototype      = NULL;
    int32_t     res, err        = -1;
    uint8_t     *name           = NULL;

    OBJECT_CHECK(a_context);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    object__lock(a_context);

    // process dots
    if (strchr(a_name, '.')){
        uint8_t     *c              = NULL;
        uint32_t    pos             = 0;
        uint8_t     *ptr            = NULL;
        uint8_t     *left           = NULL;
        uint8_t     loop            = 1;
        Object      *left_object    = NULL;
        Object      *cur_object     = NULL;

        name = (uint8_t *)strdup(a_name);
        ptr  = name;
        if (!name){
            object__unlock(a_context);
            TFATAL("strdup() failed\n");
        }

        do {
            c = &name[pos++];
            if ('\0' == *c){
                loop = 0;
            }
            if (    '.' == *c
                ||  !loop)
            {
                *c = '\0';

                if (left){
                    if (!left_object){
                        object__add_error(
                            a_context,
                            a_position,
                            "attempt to get: '%s'"
                            " from undefined"
                            "\n",
                            (const char *)ptr
                        );
                        goto fail;
                    }
                    res = object__prop_get_str_obj(
                        left_object,
                        (const char *)ptr,
                        &cur_object
                    );
                    if (res){
                        if (a_not_exist_is_error){
                            object__add_error(
                                a_context,
                                a_position,
                                "'%s' not found\n",
                                ptr
                            );
                        }
                        goto fail;
                    }
                } else {
                    res = object__var_get(
                        a_context,
                        a_position,
                        (const char *)ptr,
                        &cur_object,
                        1   // not exist is error
                    );
                    if (res){
                        if (a_not_exist_is_error){
                            object__add_error(
                                a_context,
                                a_position,
                                "'%s' not found\n",
                                ptr
                            );
                        }
                        goto fail;
                    }
                }

                if (left_object){
                    object__ref_dec(left_object);
                    left_object = NULL;
                }

                left_object = cur_object;
                left        = ptr;
                ptr         = c + 1;
            }
        } while (loop);

        err     = 0;
        *a_out  = cur_object;

        goto out;
    }

    if (!strcmp(a_name, "this")){
        object__ref_inc(a_context);
        *a_out = a_context;
        err    = 0;
        goto out;
    }

    cur_context = a_context;
    while (cur_context){
        // maybe var in context?
        res = object__prop_get_str_obj(
            cur_context,
            a_name,
            a_out
        );
        if (!res){
            // found
            err = 0;
            break;
        }

        // maybe we have super context and var there?
        if (prototype){
            object__ref_dec(prototype);
        }
        object__prop_get_str_obj(
            cur_context,
            "prototype",
            &prototype
        );

        cur_context = prototype;
    }

    if (prototype){
        object__ref_dec(prototype);
    }

out:
    if (name){
        free(name);
    }
    object__unlock(a_context);
    return err;
fail:
    goto out;
}

int32_t object__var_set(
    Object          *a_context,
    Object          *a_position,
    const char      *a_name,
    Object          *a_val)
{
    char    *name           = NULL;
    Object  *cur_object     = NULL;
    Object  *left_object    = NULL;
    int32_t res, err        = -1;

    OBJECT_CHECK(a_context);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    object__lock(a_context);

    // process dots
    if (strchr(a_name, '.')){
        char        *c      = NULL;
        uint32_t    pos     = 0;
        char        *ptr    = NULL;
        char        *left   = NULL;
        uint8_t     loop    = 1;

        name = strdup(a_name);
        ptr  = name;
        if (!name){
            object__unlock(a_context);
            TFATAL("strdup() failed\n");
        }

        do {
            c = &name[pos++];

            if ('\0' == *c){
                loop = 0;
            } else if ('.' == *c){
                *c = '\0';
            } else {
                continue;
            }

            if (!strlen(ptr)){
                object__add_error(
                    a_context,
                    a_position,
                    "cannot get '%s'"
                    " from undefined"
                    "\n",
                    ptr
                );
                goto fail;
            }

            if (left){
                // our behavior:
                //
                // var two = { a: { } };
                //
                // two.a.z   = 4; // it's ok
                // two.a.z.x = 4; // it's error
                //

                if (!left_object){
                    object__add_error(
                        a_context,
                        a_position,
                        "cannot get '%s'"
                        " from undefined"
                        "\n",
                        ptr
                    );
                    goto fail;
                }

                if (!loop){
                    // cur symbol is '\0',
                    // so it's last element,
                    // just set new val
                    object__prop_set_str_obj(
                        left_object,
                        ptr,
                        a_val
                    );
                    err = 0;
                    goto out;
                } else {
                    object__prop_get_str_obj(
                        left_object,
                        ptr,
                        &cur_object
                    );
                    if (!cur_object){
                        // we have more symbols in name,
                        // so it's error
                        object__add_error(
                            a_context,
                            a_position,
                            "key '%s' not found\n",
                            ptr
                        );
                        goto fail;
                    }
                }
            } else {
                // get first part
                res = object__var_get(
                    a_context,
                    a_position,
                    (const char *)ptr,
                    &cur_object,
                    1   // not exist is error
                );
                if (res){
                    object__add_error(
                        a_context,
                        a_position,
                        "variable '%s' not found\n",
                        ptr
                    );
                    goto fail;
                }
            }

            if (left_object){
                object__ref_dec(left_object);
                left_object = NULL;
            }

            left_object = cur_object;
            cur_object  = NULL;
            left        = ptr;
            ptr         = c   + 1;
        } while (loop);
    } else {
        // store variable
        object__prop_set_str_obj(
            a_context,
            a_name,
            a_val
        );
    }

    // all ok
    err = 0;

out:
    if (left_object){
        object__ref_dec(left_object);
    }
    if (cur_object){
        object__ref_dec(cur_object);
    }
    if (name){
        free(name);
    }
    object__unlock(a_context);
    return err;
fail:
    goto out;
}

int32_t object__is_error(
    Object    *a_context)
{
    int32_t res, ret = 0;

    OBJECT_CHECK(a_context);

    res = object__prop_exist_str(
        a_context,
        "error"
    );
    if (res){
        ret = 1;
    }

    return ret;
}

void object__add_error(
    Object      *a_context,
    Object      *a_position,
    const char  *a_format,
    ...)
{
    ObjectString    *error  = NULL;
    int32_t         res;
    va_list         ap;

    res = object__prop_exist_str(
        a_context,
        "error"
    );
    if (!res){
        object__prop_set_str_str(
            a_context,
            "error",
            ""
        );
    }

    object__prop_mem_add_printf(
        a_context,
        "error",
        "--- near of"
        " line: '%d', col: '%d', file pos: '%d' "
        "---"
        "\n",
        a_position ? a_position->parser_line : 0,
        a_position ? a_position->parser_col  : 0,
        a_position ? a_position->parser_pos  : 0
    );

    res = object__prop_get_str_str(
        a_context,
        "error",
        &error
    );
    if (res){
        TFATAL("cannot get prop: 'error'\n");
    }

    va_start(ap, a_format);
    res = object__mem_add_printf_va(
        (Object *)error,
        a_format,
        &ap
    );
    va_end(ap);

    PERROR("%s", object_string__mem_c_str(error));
    //PFATAL("%s", object_string__mem_c_str(error));

    object__ref_dec((Object *)error);

    object__shutdown(a_context);
}

void object__shutdown(
    Object *a_context)
{
    OBJECT_CHECK(a_context);

    if (a_context->event_base){
        PWARN("loop_break\n");
        event_base_loopbreak(a_context->event_base);
    }
}

void object__run(
    Object  *a_context)
{
    int32_t res;

    OBJECT_CHECK(a_context);

    // maybe we already have error
    res = object__is_error(a_context);
    if (!res){
        // start main loop
        //PWARN("start\n");
        event_base_loop(
            a_context->event_base,
            EVLOOP_NO_EXIT_ON_EMPTY
        );
        //PWARN("shutdown\n");
    }
}

void object__parser_cb(
    int32_t a_fd,
    short   a_event,
    void    *a_arg)
{
    Object  *context = NULL;
    int32_t res;

    context = (Object *)a_arg;
    if (!context){
        TFATAL("missing argument: 'context'\n");
    }

    object__lock((Object *)context);

    do {
        Object          *val    = NULL;
        ObjectString    *buffer = NULL;
        double          length  = 0;

        object__length_get(
            (Object *)context->code_queue,
            &length
        );

        res = object_array__shift(
            (ObjectArray *)context->code_queue,
            &val
        );
        if (res){
            // value not found
            break;
        }

        object__length_get(
            (Object *)context->code_queue,
            &length
        );

        buffer = dynamic_pointer_cast(
            ObjectString,
            val,
            "string",
            ""
        );
        if (!buffer){
            object__add_error(
                context,
                NULL,
                "code buffer by index 0"
                " is not 'string' (%s)\n",
                val->type
            );
            object__ref_dec(val);
            goto fail;
        }

        val = object__parse_buffer(
            context,
            NULL, // position
            buffer
        );
        if (val){
            object__ref_dec(val);
        }

        object__ref_dec((Object *)buffer);
    } while (1);

out:
    object__unlock((Object *)context);
    return;

fail:
    goto out;
}

// ignition!
Object *    object__parse_file(
    Object          *a_context,
    ObjectString    *a_path)
{
    Object          *ret    = NULL;
    ObjectString    *buffer = NULL;
    struct event    *ev     = NULL;
    struct timeval  tv;
    int32_t         res;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_path);

    object__lock(a_context);

    buffer = object_string__constructor(a_context);
    if (!buffer){
        object__add_error(
            a_context,
            NULL,
            "object_string__constructor() failed\n"
        );
        goto fail;
    }

    res = object_string__load_from_file(
        buffer,
        a_path
    );
    if (res){
        object__add_error(
            a_context,
            NULL,
            "cannot load data from: '%s'\n",
            object_string__mem_c_str(a_path)
        );
        goto fail;
    }

    // queue code for parse
    object__ref_inc((Object *)buffer);
    object_array__push_obj(
        (ObjectArray *)a_context->code_queue,
        (Object *)buffer
    );

    if (!a_context->event_base){
        object__add_error(
            a_context,
            NULL,
            "a_context->event_base is NULL\n"
        );
        goto fail;
    }

    // init event
    ev = event_new(
        a_context->event_base,
        -1,     // fd
        0,      // EV_PERSIST,
        object__parser_cb,
        a_context
    );

    // timeout in microseconds
    tv.tv_usec = 0;
    tv.tv_sec  = 0;

    event_add(ev, &tv);

    // fire!
    object__run(a_context);

    event_free(ev);

out:
    if (buffer){
        object__ref_dec((Object *)buffer);
    }

    return ret;

fail:
    goto out;
}

// add new stack level
Object * object__stack_create(
    Object *a_context,
    Object *a_position)
{
    Object  *new_context   = NULL;

    new_context = object__constructor(a_context);
    if (!new_context){
        object__add_error(
            a_context,
            a_position,
            "object__constructor() failed\n"
        );
        goto fail;
    }

out:
    return new_context;

fail:
    goto out;
}

/*
Object * object__stack_vars_get(
    Object *a_context)
{
    Object      *ret        = NULL;
    ListItem    *last_item  = NULL;

    OBJECT_CHECK(a_context);

    object__lock(a_context);

    if (list_empty(&a_context->stack.list)){
        object__unlock(a_context);
        TFATAL("stack empty"
            " (maybe too many '}' operators?)\n"
        );
    }

    last_item = list_entry(
        a_context->stack.list.prev,
        ListItem,
        list
    );

    ret = last_item->object;

    object__ref_inc(ret);

    object__unlock(a_context);

    return ret;
}

// remove stack level
void object__stack_vars_pop(
    Object *a_context)
{
    ListItem            *last_item  = NULL;
    struct list_head    *last       = NULL;

    OBJECT_CHECK(a_context);

    object__lock(a_context);

    if (list_empty(&a_context->stack.list)){
        object__unlock(a_context);
        TFATAL("stack empty"
            " (maybe too many '}' operators?)\n"
        );
    }

    last      = a_context->stack.list.prev;
    last_item = list_entry(last, ListItem, list);

    list_del(last);
    list_item_free(last_item);

    if (list_empty(&a_context->stack.list)){
        object__unlock(a_context);
        TFATAL("after stack vars pop, stack empty"
            " (maybe too many '}' operators?)\n"
        );
    }

    object__unlock(a_context);
}

int32_t object__vars_get(
    Object  *a_context,
    Object  **a_out)
{
    int32_t             err         = -1;
    struct list_head    *last       = NULL;
    ListItem            *last_item  = NULL;

    OBJECT_CHECK(a_context);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    object__lock(a_context);

    if (list_empty(&a_context->stack.list)){
        object__unlock(a_context);
        TFATAL("cannot get stack vars\n");
    }

    last        = a_context->stack.list.prev;
    last_item   = list_entry(last, ListItem, list);

    if (!last_item){
        object__unlock(a_context);
        TFATAL("cannot get stack's last item\n");
    };

    if (last_item->object){
        object__ref_inc(last_item->object);
    }
    *a_out = last_item->object;
    if (!*a_out){
        object__unlock(a_context);
        TFATAL("cannot get stack's last vars\n");
    };

    // all ok
    err = 0;

    object__unlock(a_context);

    return err;
}
*/

ListItem * object__add_item(
    Object          *a_context,
    ListItem        *a_code,
    const char      *a_context_type,
    Object          *a_object)
{
    ListItem *item = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_context_type){
        TFATAL("missing argument: 'a_context_type'\n");
    }

    object__lock(a_context);

    if (a_object){
        // store parser_type to object
        a_object->parser_type = a_context_type;
    }

    // create list item
    item = list_item_alloc();
    if (!item){
        object__unlock(a_context);
        TFATAL("list_item_alloc() failed\n");
    }
    item->object = a_object;

    list_add_tail(
        &item->list,
        &a_code->list
    );

    object__unlock(a_context);

    return item;
}

// unsafe (because a_code doesn't contain mutex,
// what we can lock)
void object__items_dump(
    const char      *a_file,
    const int32_t   a_line,
    const char      *a_function,
    Object          *a_window,
    ListItem        *a_code,
    const char      *a_format,
    ...)
{
    struct list_head    *cur            = NULL;
    struct list_head    *tmp            = NULL;
    ListItem            *cur_item       = NULL;
    Object              *cur_object     = NULL;
    char                *descr          = NULL;
    Object              *dump           = NULL;
    va_list             ap;
    int32_t             res;

    if (!a_code){
        TFATAL_POS(
            a_file,
            a_line,
            a_function,
            "missing argument: 'a_code'\n"
        );
    }

    dump = object__constructor(a_window);
    if (!dump){
        TFATAL_POS(
            a_file,
            a_line,
            a_function,
            "object__constructor() failed\n"
        );
    }

    // add descr
    {
        va_start(ap, a_format);
        res = vasprintf(&descr, a_format, ap);
        va_end(ap);

        if (!descr){
            object__ref_dec(dump);
            TFATAL(
                a_file,
                a_line,
                a_function,
                "vasprintf failed\n"
            );
        }
        if (0 >= res){
            free(descr);
            object__ref_dec(dump);
            TFATAL(
                a_file,
                a_line,
                a_function,
                "vasprintf failed, res: '%d'\n",
                res
            );
        }

        object__mem_add_printf(dump, "%s\n", descr);
    }

    object__mem_add_printf(dump, "{\n");

    // go through list
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        Object  *serialized = NULL;

        cur_item   = list_entry(cur, ListItem, list);
        cur_object = cur_item->object;

        if (cur_object){
            serialized = object__constructor(a_window);
            if (!serialized){
                free(descr);
                object__ref_dec(dump);
                TFATAL(
                    a_file,
                    a_line,
                    a_function,
                    "object__constructor() failed\n"
                );
            }
            cur_object->serialize_cb(
                cur_object,
                serialized,
                DEFAULT_DELIMETER
            );
        }

#ifdef __x86_64__
        object__mem_add_printf(dump,
            "   item (0x%lx): %s"
            " type: '%s',"
            " parser_type: '%s',"
            " size: '%lu'"
            "\n",
            (uint64_t)cur_item,
            serialized
                ?   (const char *)serialized->mem
                :   "undefined",
            cur_object
                ?   cur_object->type
                :   "undefined",
            cur_object
                ?   cur_object->parser_type
                :   "undefined",
            serialized
                ?   serialized->mem_size
                :   0
        );
#else
        object__mem_add_printf(dump,
            "   item (0x%x): %s,"
            " type: '%s',"
            " parser_type: '%s',"
            " size: '%u'"
            "\n",
            (uint32_t)cur_item,
            serialized
                ?   (const char *)serialized->mem
                :   "undefined",
            cur_object
                ?   cur_object->type
                :   "undefined",
            cur_object
                ?   cur_object->parser_type
                :   "undefined",
            serialized
                ?   serialized->mem_size
                :   0
        );
#endif
        if (serialized){
            object__ref_dec(serialized);
        }
    }

    object__mem_add_printf(dump, "}\n");

    PWARN_POS(
        a_file,
        a_line,
        a_function,
        "%s",
        (const char *)dump->mem
    );

    object__ref_dec(dump);
    free(descr);
}

// ---------------- statements ----------------

void    object__statement_define(
    Object          *a_context,
    const char      *a_statement,
    Object *        (*a_func)(
        Object          *a_context,
        ListItem        *a_code,
        ListItem        *a_operator,
        Flags           *a_flags
    ))
{
    ObjectStatement     *statement  = NULL;
    Object              *tmp        = NULL;

    OBJECT_CHECK(a_context);
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }
    if (!a_func){
        TFATAL("missing argument: 'a_func'\n");
    }

    object__lock(a_context);

    statement = object_statement__constructor(a_context);
    if (!statement){
        object__add_error(
            a_context,
            NULL,
            "object_statement__constructor() failed\n"
        );
        goto fail;
    }

    statement->func = a_func;

    tmp = object__constructor((Object *)statement);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    object__mem_add_printf(
        tmp,
        "statements.%s",
        a_statement
    );

    object__var_set(
        a_context,
        NULL,   // position
        object__mem_c_str(tmp),
        (Object *)statement
    );

out:
    if (tmp){
        object__ref_dec(tmp);
    }
    object__unlock(a_context);
    return;
fail:
    goto out;
}

// ---------------- operators ----------------

void    object__operator_define(
    Object          *a_context,
    const char      *a_operator,
    Object *        (*a_func)(
        Object          *a_context,
        ListItem        *a_code,
        ListItem        *a_operator,
        Flags           *a_flags
    ))
{
    ObjectOperator  *operator   = NULL;
    Object          *tmp        = NULL;

    OBJECT_CHECK(a_context);
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }
    if (!a_func){
        TFATAL("missing argument: 'a_func'\n");
    }

    object__lock(a_context);

    operator = object_operator__constructor(a_context);
    if (!operator){
        object__add_error(
            a_context,
            NULL,
            "object_operator__constructor() failed\n"
        );
        goto fail;
    }

    a_context->max_operator_weight += 10;

    operator->func      = a_func;
    operator->weight    = a_context->max_operator_weight;
    object__mem_add_printf(
        (Object *)operator,
        "%s",
        a_operator
    );

    tmp = object__constructor((Object *)operator);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    object__mem_add_printf(
        tmp,
        "operators.%s",
        a_operator
    );

    object__var_set(
        a_context,
        NULL,   // position
        object__mem_c_str(tmp),
        (Object *)operator
    );

out:
    if (tmp){
        object__ref_dec(tmp);
    }
    object__unlock(a_context);
    return;
fail:
    goto out;
}

int32_t object__is_statement(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_statement)
{
    Object          *val        = NULL;
    ObjectStatement *statement  = NULL;
    Object          *tmp        = NULL;
    int32_t         res, ret    = 0;

    OBJECT_CHECK(a_context);
    if (!a_flags){
        TFATAL("missing argument: 'a_flags'");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'");
    }

    object__lock(a_context);

    tmp = object__constructor(a_context);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    object__mem_add_printf(
        tmp,
        "statements.%s",
        a_statement
    );

    // search statement
    //res = object__prop_get_str_obj(
    //    a_context,
    //    (const char *)tmp->mem,
    //    &val
    //);
    res = object__var_get(
        a_context,
        a_position,
        object__mem_c_str(tmp),
        &val,
        0   // not exist is error
    );
    if (res){
        goto out;
    }
    statement = dynamic_pointer_cast(
        ObjectStatement,
        val,
        "statement",
        ""
    );
    if (!statement){
        goto out;
    }

    // all ok
    ret = 1;

out:
    if (val){
        object__ref_dec(val);
    }
    if (tmp){
        object__ref_dec(tmp);
    }
    object__unlock(a_context);
    return ret;
fail:
    goto out;
}

// statement 'if'
Object *    object__statement_if(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object          *ret                = NULL;
    const char      *statement          = NULL;
    ListItem        *right_item         = NULL;
    Object          *right_object       = NULL;
    Object          *val                = NULL;
    Object          *condition          = NULL;
    ObjectArray     *code_true          = NULL;
    ObjectArray     *code_false         = NULL;
    Object          *window             = NULL;
    ListItem        code;
    int32_t         res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    statement = object__mem_c_str(a_statement->object);

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_statement->object,
            "cannot get window context\n"
        );
        goto fail;
    }

    *a_flags |= FLAG_BRACE_CODE;
    *a_flags |= FLAG_COMMA_NO_RETURN;

    // ------------ process brackets (condition) ------------

    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing ()\n",
            statement
        );
        goto fail;
    }

    if (    !right_object
        ||  !right_object->parser_type
        ||  strcmp(right_object->parser_type, "operator")
        ||  strcmp((const char *)right_object->mem, "("))
    {
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing ()\n",
            statement
        );

        goto fail;
    }

    condition = object__operator_bracket(
        a_context,
        a_code,
        right_item,
        a_flags
    );

    // remove condition
    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing ()\n",
            statement
        );
        goto fail;
    }
    list_del(&right_item->list);
    list_item_free(right_item);

    // maybe error(s)?
    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    // ------------ collect true code ------------

    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing {}\n",
            statement
        );
        goto fail;
    }

    if (    !right_object
        ||  !right_object->parser_type
        ||  strcmp(right_object->parser_type, "statement")
        ||  strcmp((const char *)right_object->mem, "{"))
    {
        if (val){
            object__ref_dec(val);
            val = NULL;
        }
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing {}\n",
            statement
        );

        goto fail;
    }

    val = object__statement_brace_code(
        a_context,
        a_code,
        right_item,
        a_flags
    );
    code_true = dynamic_pointer_cast(
        ObjectArray,
        val,
        "object",
        "Array"
    );
    if (!code_true){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', return value from"
            " object__statement_brace_code"
            " is not array (%s)\n",
            statement,
            val->type
        );
        if (val){
            object__ref_dec(val);
        }
        goto fail;
    }

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    // ------------ check else ------------

    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        goto process;
    }

    if (    !right_object
        ||  !right_object->parser_type
        ||  strcmp(right_object->parser_type, "variable")
        ||  strcmp((const char *)right_object->mem, "else"))
    {
        goto process;
    }

    // remove 'else'
    list_del(&right_item->list);
    list_item_free(right_item);

    // ------------ collect false code ------------

    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        if (val){
            object__ref_dec(val);
            val = NULL;
        }
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing {} after 'else'\n",
            statement
        );
        goto fail;
    }

    if (    !right_object
        ||  !right_object->parser_type
        ||  strcmp(right_object->parser_type, "statement")
        ||  strcmp((const char *)right_object->mem, "{"))
    {
        if (val){
            object__ref_dec(val);
            val = NULL;
        }
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing {} after 'else'\n",
            statement
        );
        goto fail;
    }

    val = object__statement_brace_code(
        a_context,
        a_code,
        right_item,
        a_flags
    );
    code_false = dynamic_pointer_cast(
        ObjectArray,
        val,
        "object",
        "Array"
    );
    if (!code_false){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', return value from"
            " object__statement_brace_code"
            " is not array (%s)\n",
            statement,
            val->type
        );
        if (val){
            object__ref_dec(val);
            val = NULL;
        }
        goto fail;
    }

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    // ------------ process condition ------------

process:

    memset(&code, 0x00, sizeof(code));
    INIT_LIST_HEAD(&code.list);

    if (!condition){
        // condition == undefined
        if (code_false){
            object_array__copy_to_list(
                code_false,
                &code
            );
        }
    } else if (!strcmp(condition->type, "boolean")){
        if (((ObjectBoolean *)condition)->val){
            object_array__copy_to_list(
                code_true,
                &code
            );
        } else if (code_false){
            object_array__copy_to_list(
                code_false,
                &code
            );
        }
    } else if (!strcmp(condition->type, "number")){
        if (*((double *)condition->mem)){
            object_array__copy_to_list(
                code_true,
                &code
            );
        } else if (code_false){
            object_array__copy_to_list(
                code_false,
                &code
            );
        }
    } else if (!strcmp(condition->type, "string")){
        double length = 0;

        object__length_get(condition, &length);

        if (length){
            object_array__copy_to_list(
                code_true,
                &code
            );
        } else if (code_false){
            object_array__copy_to_list(
                code_false,
                &code
            );
        }
    } else if (!strcmp(condition->type, "object")){
        object_array__copy_to_list(
            code_true,
            &code
        );
    } else {
        OBJECT__ITEMS_DUMP(window, a_code, "a_code: ");
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s',"
            " unsupported condition type: '%s',"
            " flags: 0x%x"
            "\n",
            statement,
            condition->type,
            *a_flags
        );
        goto fail;
    }

    // process code
    if (!list_empty(&code.list)){
        ret = object__process_code(
            a_context,
            (Object *)a_statement,
            &code,
            a_flags // &flags
        );
        if (ret){
            object__ref_inc(ret);
        }

        list_items_free(&code);
    }

out:
    if (window){
        object__ref_dec(window);
    }
    if (condition){
        object__ref_dec(condition);
    }
    if (code_true){
        object__ref_dec((Object *)code_true);
    }
    if (code_false){
        object__ref_dec((Object *)code_false);
    }
    // remove statement
    list_del(&a_statement->list);
    list_item_free(a_statement);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// statement 'for'
Object *    object__statement_for(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object              *ret                = NULL;
    const char          *statement          = NULL;
    struct list_head    *tmp, *cur          = NULL;
    int32_t             state               = 0;
    int32_t             deep                = 1;
    ObjectArray         *code               = NULL;
    Object              *new_context        = NULL;
    ListItem            code_bracket;
    ListItem            for_initial;
    ListItem            for_condition;
    ListItem            for_next;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    statement = object__mem_c_str(
        a_statement->object
    );

    *a_flags |= FLAG_COMMA_NO_RETURN;

    memset(&code_bracket,   0x00, sizeof(code_bracket));
    memset(&for_initial,    0x00, sizeof(for_initial));
    memset(&for_condition,  0x00, sizeof(for_condition));
    memset(&for_next,       0x00, sizeof(for_next));

    INIT_LIST_HEAD(&code_bracket.list);
    INIT_LIST_HEAD(&for_initial.list);
    INIT_LIST_HEAD(&for_condition.list);
    INIT_LIST_HEAD(&for_next.list);

    // check right item
    {
        ListItem *right_item   = NULL;
        Object   *right_object = NULL;

        res = list_item_right(
            a_code,
            a_statement,
            &right_item,
            &right_object
        );
        if (res){
            object__add_error(
                a_context,
                a_statement->object,
                "statement: '%s', missing right part\n",
                statement
            );
            goto fail;
        }

        if (    !right_object
            ||  !right_object->parser_type
            ||  strcmp(right_object->parser_type, "operator")
            ||  strcmp((const char *)right_object->mem, "("))
        {
            object__add_error(
                a_context,
                a_statement->object,
                "statement: '%s', right part is not '('\n",
                statement
            );
            goto fail;
        }

        // remove right part
        list_del(&right_item->list);
        list_item_free(right_item);
    }

    for (cur = a_statement->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        // remove from a_code list
        list_del(cur);

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type, "operator"))
        {
            const char *val = NULL;

            val = (const char *)cur_object->mem;

            if (!strcmp(val, "(")){
                deep++;
            } else if (!strcmp(val, ")")){
                deep--;
            }

            if (!deep){
                // remove last ')' symbol
                list_item_free(cur_item);
                break;
            }
        }

        list_add_tail(cur, &code_bracket.list);
    }

    for (cur = code_bracket.list.next, tmp = cur->next;
        cur != &code_bracket.list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        switch (state){
            case STATE_FOR_COLLECT_NEXT:
                if (    cur_object
                    &&  cur_object->parser_type
                    &&  !strcmp(cur_object->parser_type,
                        "operator")
                    &&  !strcmp((const char *)cur_object->mem,
                        ";"))
                {
                    break;
                } else {
                    list_del(cur);
                    list_add_tail(cur, &for_next.list);
                }
                break;

            case STATE_FOR_COLLECT_COND:
                if (    cur_object
                    &&  cur_object->parser_type
                    &&  !strcmp(cur_object->parser_type,
                        "operator")
                    &&  !strcmp((const char *)cur_object->mem,
                        ";"))
                {
                    state = STATE_FOR_COLLECT_NEXT;
                } else {
                    list_del(cur);
                    list_add_tail(cur, &for_condition.list);
                }
                break;

            case STATE_FOR_COLLECT_INITIAL:
                if (    cur_object
                    &&  cur_object->parser_type
                    &&  !strcmp(cur_object->parser_type,
                        "operator")
                    &&  !strcmp((const char *)cur_object->mem,
                        ";"))
                {
                    state = STATE_FOR_COLLECT_COND;
                } else {
                    list_del(cur);
                    list_add_tail(cur, &for_initial.list);
                }
                break;

            default:
                object__add_error(
                    a_context,
                    a_statement->object,
                    "statement: '%s',"
                    " unsupported state: '%d'\n",
                    statement,
                    state
                );
                goto fail;
        }
    }

    // collect code
    {
        ListItem *right_item    = NULL;
        Object   *right_object  = NULL;
        Object   *val           = NULL;

        res = list_item_right(
            a_code,
            a_statement,
            &right_item,
            &right_object
        );
        if (res){
            object__add_error(
                a_context,
                a_statement->object,
                "statement: '%s', missing right part\n",
                statement
            );
            goto fail;
        }

        if (    !right_object
            ||  !right_object->parser_type
            ||  strcmp(right_object->parser_type, "statement")
            ||  strcmp((const char *)right_object->mem, "{"))
        {
            object__add_error(
                a_context,
                a_statement->object,
                "statement: '%s', missing '{}'\n",
                statement
            );
            goto fail;
        }

        val = object__statement_brace_code(
            a_context,
            a_code,
            right_item,
            a_flags
        );
        code = dynamic_pointer_cast(
            ObjectArray,
            val,
            "object",
            "Array"
        );
        if (!code){
            object__add_error(
                a_context,
                a_statement->object,
                "statement: '%s', return value from"
                " object__statement_brace_code"
                " is not array (%s)\n",
                statement,
                val->type
            );
            if (val){
                object__ref_dec(val);
            }
            goto fail;
        }

        res = object__is_error(a_context);
        if (res){
            goto fail;
        }
    }

    //OBJECT__ITEMS_DUMP(window, &code_bracket,  "code_bracket:");
    //OBJECT__ITEMS_DUMP(window, &for_initial,   "initial:   ");
    //OBJECT__ITEMS_DUMP(window, &for_condition, "condition: ");
    //OBJECT__ITEMS_DUMP(window, &for_next,      "next:      ");
    //OBJECT__DUMP((Object *)code, "code: ");

    // ------------  start 'for' ------------

    // run initial code
    {
        Flags   flags   = FLAG_RESOLVER_CREATE_VAR;
        Object  *val    = NULL;

        val = object__process_code(
            a_context,
            (Object *)a_statement,
            &for_initial,
            &flags
        );
        if (val){
            object__ref_dec(val);
        }
        res = object__is_error(a_context);
        if (res){
            goto fail;
        }
    }

    // dive into stack
    new_context = object__stack_create(
        a_context,
        a_statement->object
    );

again:

    // run condition code
    {
        //Flags       flags           = 0;
        Object      *val            = NULL;
        uint8_t     boolean_result  = 0;
        ListItem    code;

        memset(&code, 0x00, sizeof(code));
        INIT_LIST_HEAD(&code.list);

        list_items_copy(
            &code,
            &for_condition
        );
        val = object__process_code(
            new_context,
            (Object *)a_statement,
            &code,
            a_flags //&flags
        );
        list_items_free(&code);
        res = object__is_error(new_context);
        if (res){
            if (val){
                object__ref_dec(val);
            }
            goto fail;
        }

        res = object_boolean__get_bool_result(
            val,
            &boolean_result
        );
        if (res){
            object__add_error(
                a_context,
                a_statement->object,
                " statement: '%s',"
                " cannot convert for conditions to boolean,"
                " type: '%s',"
                " parser_type: '%s'"
                "\n",
                statement,
                val ? val->type         : "undefined",
                val ? val->parser_type  : "undefined"
            );
            if (val){
                object__ref_dec(val);
            }
            goto fail;
        }

        if (val){
            object__ref_dec(val);
        }

        if (!boolean_result){
            goto out;
        }
    }

    // process iteration
    {
        Flags       flags           = 0;
        Object      *val            = NULL;
        ListItem    iteration_code;

        memset(&iteration_code, 0x00, sizeof(iteration_code));
        INIT_LIST_HEAD(&iteration_code.list);

        object_array__copy_to_list(
            code,
            &iteration_code
        );

        if (!list_empty(&iteration_code.list)){
            val = object__process_code(
                a_context,
                (Object *)a_statement,
                &iteration_code,
                &flags
            );
        }

        list_items_free(&iteration_code);
        if (val){
            object__ref_dec(val);
        }

        res = object__is_error(a_context);
        if (res){
            goto fail;
        }
    }

    // process next
    {
        Flags       flags   = 0;
        Object      *val    = NULL;
        ListItem    code;

        memset(&code, 0x00, sizeof(code));
        INIT_LIST_HEAD(&code.list);

        list_items_copy(
            &code,
            &for_next
        );
        val = object__process_code(
            a_context,
            (Object *)a_statement,
            &code,
            &flags
        );
        list_items_free(&code);
        if (val){
            object__ref_dec(val);
        }
        res = object__is_error(a_context);
        if (res){
            goto fail;
        }
    }

    // iteration complited, go to next iteration
    goto again;

out:
    if (new_context){
        object__ref_dec(new_context);
    }

    list_items_free(&code_bracket);
    list_items_free(&for_initial);
    list_items_free(&for_condition);
    list_items_free(&for_next);

    if (code){
        object__ref_dec((Object *)code);
    }

    // remove statement
    list_del(&a_statement->list);
    list_item_free(a_statement);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// statement 'function'
Object *    object__statement_function(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    const char          *statement      = NULL;
    ListItem            *right_item     = NULL;
    Object              *right_object   = NULL;
    ObjectCodeDefine    *code_define    = NULL;
    Object              *val            = NULL;
    Flags               flags           = 0;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    statement = (const char *)a_statement->object->mem;

    flags |= FLAG_RESOLVER_DONT_RESOLVE_VARS;
    flags |= FLAG_RESOLVER_DONT_RESOLVE_PTRS;
    flags |= FLAG_BRACE_CODE;
    flags |= FLAG_COMMA_RETURN_ARRAY;
    flags |= FLAG_BRACKET_RETURN_ARRAY;

    res = list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_statement->object,
            "statement: '%s', missing right part\n",
            statement
        );
        goto fail;
    }

    code_define = object_code_define__constructor(
        a_context
    );
    if (!code_define){
        object__add_error(
            a_context,
            a_statement->object,
            "object_code__constructor() failed\n"
        );
        goto fail;
    }

    if (    right_object
        &&  right_object->parser_type
        &&  !strcmp(right_object->parser_type, "variable"))
    {
        ListItem *function_name = NULL;

        // right part - function name
        object__mem_copy_obj(
            (Object *)code_define,
            (Object *)right_object
        );

        // get function name
        function_name = right_item;

        // get next part
        list_item_right(
            a_code,
            function_name,
            &right_item,
            &right_object
        );

        // remove function's name
        list_del(&function_name->list);
        list_item_free(function_name);
    }

    // ------------ collect args ------------

    if (    !right_object
        ||  !right_object->parser_type
        ||  strcmp(right_object->parser_type, "operator")
        ||  strcmp((const char *)right_object->mem, "("))
    {
        object__add_error(
            a_context,
            a_statement->object,
            "missing function's args"
        );
        goto fail;
    }

    // process brackets
    val = object__operator_bracket(
        a_context,
        a_code,
        right_item,
        &flags
    );

    code_define->args_names = dynamic_pointer_cast(
        ObjectArray,
        val,
        "object",
        "Array"
    );
    if (!code_define->args_names){
        object__add_error(
            a_context,
            a_statement->object,
            "function's args are not array (%s)"
            " flags: 0x%x"
            "\n",
            val ? val->type : "undefined",
            flags
        );
        if (val){
            object__ref_dec(val);
            val = NULL;
        }
        goto fail;
    }

    // ------------ collect code ------------

    // re-read right part
    list_item_right(
        a_code,
        a_statement,
        &right_item,
        &right_object
    );

    if (    right_object
        &&  right_object->parser_type
        &&  !strcmp(right_object->parser_type, "statement")
        &&  !strcmp((const char *)right_object->mem, "{"))
    {
        Object *val = NULL;

        val = object__statement_brace(
            a_context,
            a_code,
            right_item,
            &flags
        );
        code_define->instructions = dynamic_pointer_cast(
            ObjectArray,
            val,
            "object",
            "Array"
        );
        if (!code_define->instructions){
            object__add_error(
                a_context,
                a_statement->object,
                "function's code are not array (%s)\n",
                val ? val->type : "undefined"
            );
            if (val){
                object__ref_dec(val);
            }
            goto fail;
        }
    }

    // if code's name exist, setup var
    if (code_define->super.mem){
        object__ref_inc((Object *)code_define);
        object__var_set(
            a_context,
            a_statement->object,
            object_code_define__mem_c_str(code_define),
            (Object *)code_define
        );
    }

out:
    // replace statement on code define
//    if (*a_flags & FLAG_ASSIGN){
        if (a_statement->object){
            object__ref_dec(a_statement->object);
        }
        object__ref_inc((Object *)code_define);
        a_statement->object = (Object *)code_define;
//    } else {
//        // remove statement
//        list_del(&a_statement->list);
//        list_item_free(a_statement);
//    }

    object__unlock(a_context);

    if (code_define){
        OBJECT_CHECK(code_define);
    }

    return (Object *)code_define;

fail:
    goto out;
}

// statement '='
Object * object__statement_assign(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object      *ret        = NULL;
    //const char  *statement    = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    *a_flags |= FLAG_BRACE_ARRAY;

    //statement = (const char *)a_statement->object->mem;

    // replace statement on operator
    a_statement->object->parser_type = "operator";

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// statement '{'
Object *    object__statement_brace(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object  *ret    = NULL;
    Flags   flags   = 0;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    PWARN("statement: '%s',"
        " flags: 0x%x,"
        " line: %d,"
        " col: %d"
        "\n",
        object__mem_c_str(a_statement->object),
        *a_flags,
        a_statement->object->parser_line,
        a_statement->object->parser_col
    );

    if (*a_flags & FLAG_BRACE_CODE){
        ret = object__statement_brace_code(
            a_context,
            a_code,
            a_statement,
            &flags
        );
    } else if (*a_flags & FLAG_BRACE_ARRAY){
        flags |= FLAG_COMMA_NO_RETURN;
        ret = object__statement_brace_array(
            a_context,
            a_code,
            a_statement,
            &flags
        );
    } else {
        ret = object__statement_brace_stack(
            a_context,
            a_code,
            a_statement,
            &flags
        );
    }

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// statement '{}' for function's code,
// case: 'function test(a, b){ return a + b; }'
Object *    object__statement_brace_code(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    struct list_head    *tmp, *cur  = NULL;
    ObjectArray         *code       = NULL;
    int32_t             deep        = 1;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }

    object__lock(a_context);

    code = object_array__constructor(a_context);
    if (!code){
        object__add_error(
            a_context,
            a_operator->object,
            "object_array__constructor() failed"
        );
        goto fail;
    }

    // collect code
    for (cur = a_operator->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        if (cur_object){
            object__ref_inc(cur_object);
        }

        // remove from a_code list
        list_del(cur);
        list_item_free(cur_item);

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type, "statement"))
        {
            const char *val = NULL;

            val = (const char *)cur_object->mem;

            if (!strcmp(val, "{")){
                deep++;
            } else if (!strcmp(val, "}")){
                deep--;
            }

            if (!deep){
                // remove last '}' symbol
                object__ref_dec(cur_object);
                break;
            }
        }

        // add to code list
        // object__ref_inc(cur_object);
        object_array__push_obj(code, cur_object);
    }

    // free operator
    list_del(&a_operator->list);
    list_item_free(a_operator);

out:

    if ((Object *)code){
        OBJECT_CHECK((Object *)code);
    }

    object__unlock(a_context);
    return (Object *)code;
fail:
    goto out;
}

// operator '{}' in case: tigra = { "some_key": some_val }
// (associative array)
Object *    object__statement_brace_array(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object                  *ret            = NULL;
    ListItem                *result_item    = NULL;
    struct list_head        *tmp, *cur      = NULL;
    int32_t                 deep            = 1;
    ListItem                code;
    int32_t                 res;

    //PWARN("brace array\n");

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    result_item = list_item_alloc();
    if (!result_item){
        object__add_error(
            a_context,
            a_operator->object,
            "list_item_alloc() failed\n"
        );
        goto fail;
    }

    // init code list
    memset(&code, 0x00, sizeof(code));
    INIT_LIST_HEAD(&code.list);

    for (cur = a_operator->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        // remove from a_code list
        list_del(cur);

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type,
                "statement")
            &&  !strcmp((const char *)cur_object->mem,
                "{"))
        {
            // maybe value is associative array too
            deep++;
        } else if (cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type,
                "statement")
            &&  !strcmp((const char *)cur_object->mem,
                "}"))
        {
            deep--;
        }

        if (!deep){
            // del last symbol '}'
            list_item_free(cur_item);

            // process value as code if exist
            // (because it can contain any code)
            {
                Object  *val            = NULL;
                Object  *new_context    = NULL;

                // insert stack level (results will be there)
                new_context = object__stack_create(
                    a_context,
                    a_operator->object
                );
                if (!new_context){
                    object__add_error(
                        a_context,
                        a_operator->object,
                        "object__stack_create() failed\n"
                    );
                    goto fail;
                }

                OBJECT__DUMP(new_context, "new_context: ");

                if (!list_empty(&code.list)){
                    val = object__process_code(
                        new_context,
                        (Object *)a_operator,
                        &code,
                        a_flags
                    );
                }

                // remove process code result
                // (our results in vars)
                if (val){
                    object__ref_dec(val);
                }

                // remove code
                list_items_free(&code);

                // maybe error?
                res = object__is_error(a_context);
                if (res){
                    object__ref_dec(new_context);
                    goto fail;
                }

                // store result
                result_item->object = new_context;

                // store to ret
                object__ref_inc(new_context);
                ret = new_context;
            }
            break;
        }

        // add to code list
        list_add_tail(cur, &code.list);
    }

out:
    list_replace(
        &a_operator->list,
        &result_item->list
    );
    list_item_free(a_operator);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// operator '{}' in case: var a = 10; { var a = 7; alert(a); }
Object *    object__statement_brace_stack(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object              *ret            = NULL;
    Object              *new_context    = NULL;
    struct list_head    *tmp, *cur      = NULL;
    int32_t             deep            = 1;
    ListItem            code;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }

    object__lock(a_context);

    // init list for code
    memset(&code, 0x00, sizeof(code));
    INIT_LIST_HEAD(&code.list);

    // collect code
    for (cur = a_statement->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        // remove from a_code list
        list_del(cur);

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type, "statement"))
        {
            const char *val = NULL;

            val = (const char *)cur_object->mem;

            if (!strcmp(val, "{")){
                deep++;
            } else if (!strcmp(val, "}")){
                deep--;
            }

            if (!deep){
                list_item_free(cur_item);
                break;
            }
        }

        // add to code list
        list_add_tail(cur, &code.list);
    }

    // dive into stack
    new_context = object__stack_create(
        a_context,
        a_statement->object
    );

    // process code
    if (!list_empty(&code.list)){
        //Flags   flags   = 0;
        Object  *val    = NULL;

        val = object__process_code(
            new_context,
            (Object *)a_statement,
            &code,
            a_flags //&flags
        );

        if (val){
            object__ref_dec(val);
        }
    }

    // return from stack
    if (new_context){
        object__ref_dec(new_context);
    }

    list_items_free(&code);

    // free statement
    list_del(&a_statement->list);
    list_item_free(a_statement);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// return -1 if it's not operator
int32_t object__get_operator_weight(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_operator)
{
    Object          *val        = NULL;
    ObjectOperator  *operator   = NULL;
    Object          *tmp        = NULL;
    int32_t         res, ret    = -1;

    OBJECT_CHECK(a_context);
    if (!a_flags){
        TFATAL("missing argument: 'a_flags'\n");
    }
    if (    !a_operator
        ||  !strlen(a_operator))
    {
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    tmp = object__constructor(a_context);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    object__mem_add_printf(
        tmp,
        "operators.%s",
        a_operator
    );

    // search operator
    //res = object__prop_get_str_obj(
    //    a_context,
    //    (const char *)tmp->mem,
    //    &val
    //);
    res = object__var_get(
        a_context,
        a_position,
        object__mem_c_str(tmp),
        &val,
        0   // not exist is error
    );
    if (res){
        PWARN("operator: '%s' not found\n",
            object__mem_c_str(tmp)
        );
        goto fail;
    }

    operator = dynamic_pointer_cast(
        ObjectOperator,
        val,
        "operator",
        ""
    );
    if (!operator){
        goto fail;
    }

    // avoid double memory free,
    // now we should free 'operator' instead 'val'
    val = NULL;

    // all ok
    ret = operator->weight;

out:
    if (val){
        object__ref_dec(val);
    }
    if (operator){
        object__ref_dec((Object *)operator);
    }
    if (tmp){
        object__ref_dec(tmp);
    }

    object__unlock(a_context);

    PWARN("operator: '%s', weight: '%d'\n",
       a_operator,
        ret
    );

    return ret;

fail:
    goto out;
}

int32_t object__is_operator(
    Object          *a_context,
    Object          *a_position,
    Flags           *a_flags,
    const char      *a_operator)
{
    int32_t weight  = -1;
    int32_t ret     = 0;

    OBJECT_CHECK(a_context);
    if (!a_flags){
        TFATAL("missing argument: 'a_flags'");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'");
    }

    weight = object__get_operator_weight(
        a_context,
        a_position,
        a_flags,
        a_operator
    );

    if (0 <= weight){
        ret = 1;
    }

    return ret;
}

// operator 'return'
Object *    object__operator_return(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret            = NULL;
    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    *a_flags |= FLAG_RETURN;

    // get right part
    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );

    if (right_item){
        object__item_resolve(
            a_context,
            a_operator->object,
            right_item,
            a_flags
        );

        // get right part
        list_item_right(
            a_code,
            a_operator,
            &right_item,
            &right_object
        );
        if (right_item->object){
            object__ref_inc(right_item->object);
        }
        ret = right_item->object;
    }

    // remove operator
    list_del(&a_operator->list);
    list_item_free(a_operator);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator: '='
Object * object__operator_assign(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret        = NULL;
    const char  *operator   = NULL;

    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;

    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;

    int32_t     res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = object__mem_c_str(a_operator->object);

    // get left part
    res = list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing left part\n",
            operator
        );
        goto fail;
    }

    // get right part
    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing right part\n",
            operator
        );
        goto fail;
    }

    // resolve right part
    object__item_resolve(
        a_context,
        a_operator->object,
        right_item,
        a_flags
    );
    // reread right part
    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing right part\n",
            operator
        );
        goto fail;
    }

    // process vars for left part
    if (    left_object
        &&  left_object->parser_type
        &&  !strcmp(left_object->parser_type, "variable"))
    {
        if (right_object){
            object__ref_inc(right_object);
        }
        res = object__var_set(
            a_context,
            a_operator->object,
            object__mem_c_str(left_object),
            right_object
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "cannot set var: '%s'\n",
                (const char *)left_object->mem
            );
            goto fail;
        }
        // add to ret
        if (right_object){
            object__ref_inc(right_object);
        }
        ret = right_object;
    } else if ( left_object
        &&      left_object->type
        &&      !strcmp(left_object->type, "object_value"))
    {
        ObjectValue *value = (ObjectValue *)left_object;
        if (value->object){
            object__ref_dec(value->object);
        }
        if (right_object){
            object__ref_inc(right_object);
        }
        value->object = right_object;
        // add to ret
        if (right_object){
            object__ref_inc(right_object);
        }
        ret = right_object;
    }

out:
    // remove left part
    list_del(&left_item->list);
    list_item_free(left_item);

    // remove right part
    list_del(&right_item->list);
    list_item_free(right_item);

    // replace operator on result
    object__ref_dec(a_operator->object);
    if (ret){
        object__ref_inc(ret);
    }
    a_operator->object = ret;

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// operator: '+='
Object * object__operator_addition_assign(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    TFATAL("TODO\n");

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator: '-='
Object * object__operator_subtraction_assign(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    TFATAL("TODO\n");

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '<'
Object * object__operator_less_than(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '>'
Object * object__operator_greater_than(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '<='
Object * object__operator_less_eq_than(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '>='
Object * object__operator_greater_eq_than(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator ':'
Object *    object__operator_colon(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret            = NULL;
    const char  *operator       = NULL;

    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;

    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;
    Object      *right_var      = NULL;

    int32_t     res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    // get left part
    res = list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing left part"
        );
    }

    // get right part
    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing right part"
        );
    }

    res = object__item_resolve(
        a_context,
        a_operator->object,
        right_item,
        a_flags
    );

    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );

    if (right_object){
        object__ref_inc(right_object);
    }
    right_var = right_object;

    // store cur value

    if (    left_object
        &&  !strcmp(left_object->type, "string"))
    {
        // key like: "some_key" or 'some_key'
        if (right_var){
            object__ref_inc(right_var);
        }
        object__var_set(
            a_context,
            a_operator->object,
            object__mem_c_str(left_object),
            right_var
        );
    } else if ( left_object
        &&      !strcmp(left_object->type, "variable"))
    {
        // key like: some_key
        if (right_var){
            object__ref_inc(right_var);
        }
        object__var_set(
            a_context,
            a_operator->object,
            object__mem_c_str(left_object),
            right_var
        );
    } else {
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " unsupported left object type: '%s',"
            " parser_type: '%s'"
            "\n",
            operator,
            left_object ? left_object->type        : "undefined",
            left_object ? left_object->parser_type : "undefined"
        );
        goto fail;
    }

out:
    // remove right var
    if (right_var){
        object__ref_dec(right_var);
    }

    // remove left part
    list_del(&left_item->list);
    list_item_free(left_item);

    // remove right part
    list_del(&right_item->list);
    list_item_free(right_item);

    // remove operator
    list_del(&a_operator->list);
    list_item_free(a_operator);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
fail:
    goto out;
}

// operator '&&'
Object *    object__operator_logical_and(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '||'
Object *    object__operator_logical_or(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    const char      *operator       = NULL;
    Object          *ret            = NULL;
    ObjectBoolean   *result         = NULL;
    ListItem        *left_item      = NULL;
    Object          *left           = NULL;
    ListItem        *right_item     = NULL;
    Object          *right          = NULL;
    uint8_t         left_result     = 0;
    uint8_t         right_result    = 0;
    int32_t         res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    //OBJECT__ITEMS_DUMP(window, a_code, "||");

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    // get left and right parts
    list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left
    );

    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right
    );

    // resolve left
    object__item_resolve(
        a_context,
        a_operator->object,
        left_item,
        a_flags
    );
    list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left
    );
    res = object_boolean__get_bool_result(
        left,
        &left_result
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " cannot get boolean result for left part"
            "\n",
            operator
        );
        goto fail;
    }

    //PWARN("left_result: '%d'\n", left_result);

    if (left_result){
        result = object_boolean__constructor_double(
            1,
            a_context
        );
        if (!result){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " object_boolean__constructor_double() failed"
                "\n",
                operator
            );
            goto fail;
        }
        goto out;
    }

    // resolve right
    object__item_resolve(
        a_context,
        a_operator->object,
        right_item,
        a_flags
    );
    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right
    );
    res = object_boolean__get_bool_result(
        right,
        &right_result
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " cannot get boolean result for right part"
            "\n",
            operator
        );
        goto fail;
    }
    if (right_result){
        result = object_boolean__constructor_double(
            1,
            a_context
        );
        if (!result){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " object_boolean__constructor_double() failed"
                "\n",
                operator
            );
            goto fail;
        }
    }

out:
    if (!result){
        result = object_boolean__constructor_double(
            0,
            a_context
        );
        if (!result){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " object_boolean__constructor_double() failed"
                "\n",
                operator
            );
        }
    }

    // remove left part
    list_del(&left_item->list);
    list_item_free(left_item);

    // remove right part
    list_del(&right_item->list);
    list_item_free(right_item);

    // replace operator on result
    object__ref_dec(a_operator->object);
    a_operator->object = (Object *)result;

    // prepare ret
    if (result){
        object__ref_inc((Object *)result);
    }
    ret = (Object *)result;

    object__unlock(a_context);

    return ret;

fail:
    goto out;
}

// operator handler for common compare cases
Object *    object__operator__compare(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret            = NULL;
    const char  *operator       = NULL;
    double      result          = 0;
    ListItem    *left_item      = NULL;
    Object      *left           = NULL;
    ListItem    *right_item     = NULL;
    Object      *right          = NULL;
    Object      *window         = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_operator->object,
            "cannot get window context\n"
        );
        goto fail;
    }

    // resolve left
    list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left
    );
    if (!left_item){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing left part,"
            " right type: '%s', parser_type: '%s'"
            "\n",
            operator,
            right   ? right->type          : "undefined",
            right   ? right->parser_type   : "undefined"
        );
        goto fail;
    }

    object__item_resolve(
        a_context,
        a_operator->object,
        left_item,
        a_flags
    );
    list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left
    );

    // resolve right
    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right
    );
    if (!right_item){
        OBJECT__ITEMS_DUMP(window, a_code, "a_code: ");
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing right part,"
            " left type: '%s', parser_type: '%s'"
            "\n",
            operator,
            left  ? left->type          : "undefined",
            left  ? left->parser_type   : "undefined"
        );
        goto fail;
    }
    object__item_resolve(
        a_context,
        a_operator->object,
        right_item,
        a_flags
    );
    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right
    );

    // process
    if (!left && !right){
        // undefined == undefined
        result = 1;
    } else if ( (left  && !strcmp(left->type,  "number"))
        &&      (right && !strcmp(right->type, "number")))
    {
        double left_val  = *((double *)left->mem);
        double right_val = *((double *)right->mem);

        if (!strcmp(operator, "==")){
            result = (left_val == right_val) ? 1 : 0;
        } else if (!strcmp(operator, "!=")){
            result = (left_val != right_val) ? 1 : 0;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " unsupported case,"
                " left type: '%s', parser_type: '%s',"
                " right type: '%s', parser_type: '%s'"
                "\n",
                operator,
                left  ? left->type          : "undefined",
                left  ? left->parser_type   : "undefined",
                right ? right->type         : "undefined",
                right ? right->parser_type  : "undefined"
            );
            goto fail;
        }
    } else if ( (left  && !strcmp(left->type,  "string"))
        &&      (right && !strcmp(right->type, "string")))
    {
        result = object_string__cmp_obj(
            (ObjectString *)left,
            (ObjectString *)right
        ) ? 0 : 1;
    } else if ( (left  && !strcmp(left->type,  "boolean"))
        &&      (right && !strcmp(right->type, "boolean")))
    {
        ObjectBoolean   *boolean_object_left    = NULL;
        ObjectBoolean   *boolean_object_right   = NULL;
        uint8_t         boolean_left            = 0;
        uint8_t         boolean_right           = 0;

        boolean_object_left  = (ObjectBoolean *)left;
        boolean_object_right = (ObjectBoolean *)right;

        boolean_left = object_boolean__to_uint8_t(
            boolean_object_left
        );
        boolean_right = object_boolean__to_uint8_t(
            boolean_object_right
        );

        if (!strcmp(operator, "==")){
            result = (boolean_left == boolean_right) ? 1 : 0;
        } else if (!strcmp(operator, "!=")){
            result = (boolean_left != boolean_right) ? 1 : 0;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " unsupported case,"
                " left type: '%s', parser_type: '%s',"
                " right type: '%s', parser_type: '%s'"
                "\n",
                operator,
                left  ? left->type          : "undefined",
                left  ? left->parser_type   : "undefined",
                right ? right->type         : "undefined",
                right ? right->parser_type  : "undefined"
            );
            goto fail;
        }

    } else if ((left  && !strcmp(left->type,  "string"))){
        // at least 1 part is string
        if (right){
            ObjectString *str = NULL;

            str = object_string__constructor(a_context);
            if (!str){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_string__constructor() failed"
                    "\n"
                );
                goto fail;
            }

            right->to_string_cb(right, (Object *)str);

            result = object_string__cmp_obj(
                (ObjectString *)left,
                str
            ) ? 0 : 1;

            object__ref_dec((Object *)str);
        }
    } else if ((right && !strcmp(right->type, "string"))){
        // at least 1 part is string
        if (left){
            ObjectString *str = NULL;

            str = object_string__constructor(a_context);
            if (!str){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_string__constructor() failed"
                    "\n"
                );
                goto fail;
            }

            left->to_string_cb(left, (Object *)str);

            result = object_string__cmp_obj(
                str,
                (ObjectString *)right
            ) ? 0 : 1;

            object__ref_dec((Object *)str);
        }
    } else {
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " unsupported case,"
            " left type: '%s', parser_type: '%s',"
            " right type: '%s', parser_type: '%s'"
            "\n",
            operator,
            left  ? left->type          : "undefined",
            left  ? left->parser_type   : "undefined",
            right ? right->type         : "undefined",
            right ? right->parser_type  : "undefined"
        );
        goto fail;
    }

    ret = (Object *)object_boolean__constructor_double(
        result,
        a_context
    );
    if (!ret){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " object_boolean__constructor_double() failed\n",
            operator
        );
        goto fail;
    }

out:
    if (window){
        object__ref_dec(window);
    }

    // remove left part
    list_del(&left_item->list);
    list_item_free(left_item);

    // remove right part
    list_del(&right_item->list);
    list_item_free(right_item);

    // replace operator on result
    object__ref_dec(a_operator->object);
    a_operator->object = ret;

    if (ret){
        object__ref_inc(ret);
    }

    object__unlock(a_context);

    return ret;

fail:
    goto out;
}

// operator '=='
Object *    object__operator_compare_equal(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__compare(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '!='
Object *    object__operator_compare_not_equal(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__compare(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator handler for common arithmetic cases
Object *    object__operator__arithmetic(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret            = NULL;
    const char  *operator       = NULL;

    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;
    Object      *left_var       = NULL;

    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;
    Object      *right_var      = NULL;

    Object      *result         = NULL;
    int32_t     res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    // get left part
    res = list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );

    // get right part
    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );

    // process vars for left part
    if (    left_object
        &&  left_object->parser_type
        &&  !strcmp(left_object->parser_type, "variable"))
    {
        res = object__var_get(
            a_context,
            a_operator->object,
            (const char *)left_object->mem,
            &left_var,
            1   // not exist is error
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "operator '%s',"
                " variable '%s' not found\n",
                operator,
                (const char *)left_object->mem
            );
            goto fail;
        }
    } else if (    left_object
        &&  left_object->type
        &&  !strcmp(left_object->type, "object_value"))
    {
        ObjectValue *value = NULL;

        value = (ObjectValue *)left_object;
        if (value->object){
            object__ref_inc(value->object);
        }
        left_var = value->object;
    } else {
        if (left_object){
            object__ref_inc(left_object);
        }
        left_var = left_object;
    }

    // process vars for right part
    if (    right_object
        &&  right_object->parser_type
        &&  !strcmp(right_object->parser_type, "variable"))
    {
        res = object__var_get(
            a_context,
            a_operator->object,
            (const char *)right_object->mem,
            &right_var,
            1   // not exist is error
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " variable '%s' not found\n",
                operator,
                (const char *)right_object->mem
            );
            goto fail;
        }
    } else if (    right_object
        &&  right_object->type
        &&  !strcmp(right_object->type, "object_value"))
    {
        ObjectValue *value = (ObjectValue *)right_object;
        if (value->object){
            object__ref_inc(value->object);
        }
        right_var = value->object;
    } else {
        if (right_object){
            object__ref_inc(right_object);
        }
        right_var = right_object;
    }

    //OBJECT__ITEMS_DUMP(window, a_code,
    //    "operator: '%s'\n"
    //    "left_var: 0x%lx (%s),"
    //    " right_var: 0x%lx (%s),"
    //    " a_code:",
    //    operator,
    //    (uint64_t)left_var,
    //    left_var ? left_var->type : "undefined",
    //    (uint64_t)right_var,
    //    right_var ? right_var->type : "undefined"
    //);

    if (!left_var && !right_var){
        if (!strcmp(operator, "+")){
            // both parts are undefined
            ObjectNumber *val = NULL;

            val = object_number__constructor(a_context);
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "object_number__constructor() failed\n"
                );
                goto fail;
            }

            val->nan    = 1;
            result      = (Object *)val;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s', unsupported case\n",
                operator
            );
            goto fail;
        }
    } else if ( (left_var  && !strcmp(left_var->type,  "number"))
        &&      (right_var && !strcmp(right_var->type, "number")))
    {
        // both parts are numbers
        if (!strcmp(operator, "+")){
            double          res     = 0;
            ObjectNumber    *val    = NULL;

            res  = *((double *)left_var->mem);
            res += *((double *)right_var->mem);

            val = object_number__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_number__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "-")){
            double          res     = 0;
            ObjectNumber    *val    = NULL;

            res  = *((double *)left_var->mem);
            res -= *((double *)right_var->mem);

            val = object_number__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_number__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "*")){
            double          res     = 0;
            ObjectNumber    *val    = NULL;

            res  = *((double *)left_var->mem);
            res *= *((double *)right_var->mem);

            val = object_number__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_number__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "/")){
            double          res     = 0;
            ObjectNumber    *val    = NULL;

            res  = *((double *)left_var->mem);
            res /= *((double *)right_var->mem);

            val = object_number__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_number__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "<")){
            double          res     = 0;
            ObjectBoolean   *val    = NULL;

            if (*((double *)left_var->mem)
                <   *((double *)right_var->mem))
            {
                res = 1;
            }

            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, ">")){
            double          res     = 0;
            ObjectBoolean   *val    = NULL;

            if (*((double *)left_var->mem)
                >   *((double *)right_var->mem))
            {
                res = 1;
            }

            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "<=")){
            double          res     = 0;
            ObjectBoolean   *val    = NULL;

            if (*((double *)left_var->mem)
                <=   *((double *)right_var->mem))
            {
                res = 1;
            }

            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, ">=")){
            double          res     = 0;
            ObjectBoolean   *val    = NULL;

            if (*((double *)left_var->mem)
                >=  *((double *)right_var->mem))
            {
                res = 1;
            }

            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else if (!strcmp(operator, "&&")){
            double          res             = 0;
            ObjectBoolean   *val            = NULL;
            uint8_t         left_result     = 0;
            uint8_t         right_result    = 0;

            res = object_boolean__get_bool_result(
                left_var,
                &left_result
            );
            if (res){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " cannot get boolean result"
                    " for left part\n",
                    operator
                );
                goto fail;
            }

            res = object_boolean__get_bool_result(
                right_var,
                &right_result
            );
            if (res){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " cannot get boolean result"
                    " for right part\n",
                    operator
                );
                goto fail;
            }

            if (left_result && right_result){
                res = 1;
            }
            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor() failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s', unsupported case\n",
                operator
            );
            goto fail;
        }
    } else {
        if (!strcmp(operator, "+")){
            // process as strings
            result = (Object *)object_string__constructor(
                a_context
            );
            if (!result){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "object_string__constructor() failed\n"
                );
                goto fail;
            }

            if (left_var){
                left_var->to_string_cb(
                    left_var,
                    result
                );
            } else {
                object__mem_add_printf(result, "undefined");
            }

            if (right_var){
                right_var->to_string_cb(
                    right_var,
                    result
                );
            } else {
                object__mem_add_printf(result, "undefined");
            }
        } else if ( !strcmp(operator, "*")
            ||      !strcmp(operator, "/"))
        {
            // at least one part is not number
            ObjectNumber *val = NULL;

            val = object_number__constructor_double(
                0,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "object_number__constructor_double()"
                    " failed\n"
                );
                goto fail;
            }

            val->nan    = 1;
            result      = (Object *)val;
        } else if (!strcmp(operator, "&&")){
            ObjectBoolean  *val             = NULL;
            uint8_t         left_boolean    = 0;
            uint8_t         right_boolean   = 0;

            if (    left_var
                &&  !strcmp(left_var->type, "boolean"))
            {
                left_boolean = object_boolean__to_uint8_t(
                    (ObjectBoolean *)left_var
                );
            } else {
                res = object_boolean__get_bool_result(
                    left_var,
                    &left_boolean
                );
                if (res){
                    object__add_error(
                        a_context,
                        a_operator->object,
                        "operator: '%s',"
                        " cannot get boolean result"
                        " for left part"
                        "\n",
                        operator
                    );
                    goto fail;
                }
            }

            if (    right_var
                &&  !strcmp(right_var->type, "boolean"))
            {
                right_boolean = object_boolean__to_uint8_t(
                    (ObjectBoolean *)right_var
                );
            } else {
                res = object_boolean__get_bool_result(
                    right_var,
                    &right_boolean
                );
                if (res){
                    object__add_error(
                        a_context,
                        a_operator->object,
                        "operator: '%s',"
                        " cannot get boolean result"
                        " for right part"
                        "\n",
                        operator
                    );
                    goto fail;
                }
            }

            if (left_boolean && right_boolean){
                res = 1;
            }
            val = object_boolean__constructor_double(
                res,
                a_context
            );
            if (!val){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "operator: '%s',"
                    " object_boolean__constructor_double()"
                    " failed\n",
                    operator
                );
                goto fail;
            }
            result = (Object *)val;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " unsupported case,"
                " left_var type: '%s', parser_type: '%s',"
                " right_var type: '%s', parser_type: '%s'"
                "\n",
                operator,
                left_var  ? left_var->type          : "undefined",
                left_var  ? left_var->parser_type   : "undefined",
                right_var ? right_var->type         : "undefined",
                right_var ? right_var->parser_type  : "undefined"
            );
            goto fail;
        }
    }

out:
    // remove left part
    list_del(&left_item->list);
    list_item_free(left_item);

    // replace operator on result
    object__ref_dec(a_operator->object);
    a_operator->object = result;

    // setup ret
    if (result){
        object__ref_inc(result);
    }
    ret = (Object *)result;

    // remove right part
    list_del(&right_item->list);
    list_item_free(right_item);

    if (left_var){
        object__ref_dec(left_var);
    }
    if (right_var){
        object__ref_dec(right_var);
    }

    object__unlock(a_context);

    return ret;
fail:
    goto out;
}

// operator '+'
Object *    object__operator_addition(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '-'
Object *    object__operator_subtraction(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator handler for common increment, decrement cases
Object *    object__operator__inc_dec(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    const char  *operator       = NULL;
    Object      *ret            = NULL;

    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;

    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;

    int32_t     res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );

    list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );

    if (    left_object
        &&  left_object->parser_type
        &&  !strcmp(left_object->parser_type, "variable"))
    {
        Object          *val    = NULL;
        ObjectNumber    *number = NULL;

        res = object__var_get(
            a_context,
            a_operator->object,
            (const char *)left_object->mem,
            &val,
            1   // not exist is error
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',",
                " left part,"
                " variable: '%s' not found",
                "\n",
                operator,
                (const char *)left_object->mem
            );
            goto fail;
        }

        number = dynamic_pointer_cast(
            ObjectNumber,
            val,
            "number",
            ""
        );
        if (!number){
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " left part,"
                " variable: '%s' is not number (%s)"
                "\n",
                operator,
                (const char *)left_object->mem,
                val ? val->type : "undefined"
            );
            if (val){
                object__ref_dec(val);
            }
            goto fail;
        }

        if (!strcmp(operator, "++")){
            // increment
            object_number__inc(number, 1);

            // remove left part
            list_del(&left_item->list);
            list_item_free(left_item);

            // replace operator on result
            object__ref_dec(a_operator->object);
            object__ref_inc((Object *)number);
            a_operator->object = (Object *)number;
        } else if (!strcmp(operator, "--")){
            // increment
            object_number__dec(number, 1);

            // remove left part
            list_del(&left_item->list);
            list_item_free(left_item);

            // replace operator on result
            object__ref_dec(a_operator->object);
            object__ref_inc((Object *)number);
            a_operator->object = (Object *)number;
        } else {
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " unsupported case,"
                " left object type: '%s',"
                " right object type: '%s'"
                "\n",
                operator,
                left_object  ? left_object->type  : "undefined",
                right_object ? right_object->type : "undefined"
            );
            object__ref_dec((Object *)number);
            goto fail;
        }
        object__ref_dec((Object *)number);
    } else {
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',",
            " unsupported case,"
            " left object type: '%s',"
            " right object type: '%s'"
            "\n",
            operator,
            left_object  ? left_object->type  : "undefined",
            right_object ? right_object->type : "undefined"
        );
        goto fail;
    }

out:
    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
fail:
    goto out;
}

// operator '++'
Object *    object__operator_inc(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__inc_dec(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '--'
Object *    object__operator_dec(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__inc_dec(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '*'
Object *    object__operator_multiplication(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator '/'
Object *    object__operator_division(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object *ret = NULL;

    ret = object__operator__arithmetic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// operator ',' convert '(a, b, c, d)' to [a, b, c, d]
// (without resolve vars)
Object *    object__operator_comma_to_array(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object      *ret            = NULL;
    const char  *operator       = NULL;

    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;

    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;

    int32_t     res;

    ObjectArray *left_comma_vars    = NULL;
    ObjectArray *right_comma_vars   = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = (const char *)a_operator->object->mem;

    res = list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing left part\n",
            operator
        );
        goto fail;
    }

    res = list_item_right(
        a_code,
        a_operator,
        &right_item,
        &right_object
    );
    if (res){
        object__add_error(
            a_context,
            a_operator->object,
            "operator: '%s',"
            " missing right part\n",
            operator
        );
        goto fail;
    }

    if (    left_object
        &&  left_object->parser_type
        &&  !strcmp(left_object->parser_type, "comma_vars"))
    {
        left_comma_vars = (ObjectArray *)left_object;
    } else {
        // left part is not 'comma_vars', convert
        left_comma_vars = object_array__constructor(
            a_context
        );
        if (!left_comma_vars){
            object__add_error(
                a_context,
                a_operator->object,
                "object_array_constructor() failed\n"
            );
            goto fail;
        }
        left_comma_vars->super.parser_type = "comma_vars";
        object_array__push_obj(left_comma_vars, left_object);

        // replace left object
        left_item->object = (Object *)left_comma_vars;
    }

    if (    right_object
        &&  right_object->parser_type
        &&  !strcmp(right_object->parser_type, "comma_vars"))
    {
        // right size - comma_vars
        double pos, length = 0;

        right_comma_vars = dynamic_pointer_cast(
            ObjectArray,
            right_object,
            "object",
            "Array"
        );
        if (!right_comma_vars){
            object__add_error(
                a_context,
                a_operator->object,
                "right part of comma,"
                " have parser_type: '%s',"
                " and type: '%s',"
                " but type must be array",
                right_object->parser_type,
                right_object->type
            );
            goto fail;
        }
        object__length_get(
            (Object *)right_comma_vars,
            &length
        );
        for (pos = 0; pos < length; pos++){
            Object *val = NULL;
            res = object_array__get_by_index(
                right_comma_vars,
                pos,
                &val
            );
            if (res){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "cannot get right object"
                    " by index: '%g'\n",
                    pos
                );
                goto fail;
            }
            object_array__push_obj(
                left_comma_vars,
                val
            );
        }
    } else {
        if (right_object){
            object__ref_inc(right_object);
        }
        object_array__push_obj(
            left_comma_vars,
            right_object
        );
    }

    list_del(&a_operator->list);
    list_item_free(a_operator);

    list_del(&right_item->list);
    list_item_free(right_item);

out:
    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// operator ',' run code as:
// 1. 'a = (1 + c + 2)*3, b = 3'
//   result will be: 'a, b', so don't free operator here
// 2. 'a = { b: 4 + 2, c: function(a, b){ return a + b; } }
//   result will be: ''
void   object__operator_comma_process(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    struct list_head    *tmp, *cur  = NULL;
    ListItem            *left_item  = NULL;
    ListItem            *right_item = NULL;
    ListItem            code_left;
    ListItem            code_right;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }
    if (list_empty(&a_code->list)){
        TFATAL("empty list\n");
    }

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    left_item = list_item_alloc();
    if (!left_item){
        object__add_error(
            a_context,
            a_operator->object,
            "list_item_alloc() failed\n"
        );
        goto fail;
    }

    right_item = list_item_alloc();
    if (!right_item){
        list_item_free(left_item);
        object__add_error(
            a_context,
            a_operator->object,
            "list_item_alloc() failed\n"
        );
        goto fail;
    }

    memset(&code_left.list, 0x00, sizeof(code_left.list));
    INIT_LIST_HEAD(&code_left.list);

    memset(&code_right.list, 0x00, sizeof(code_right.list));
    INIT_LIST_HEAD(&code_right.list);

    object__lock(a_context);

    // process left part
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_operator->list;
        cur = tmp, tmp = tmp->next)
    {
        // move to left list
        list_del(cur);
        list_add_tail(cur, &code_left.list);
    }

    // process right part
    for (cur = a_operator->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        // move to right list
        list_del(cur);
        list_add_tail(cur, &code_right.list);
    }

    left_item->object = object__process_code(
        a_context,
        (Object *)a_operator,
        &code_left,
        a_flags
    );

    res = object__is_error(a_context);
    if (!res){
        right_item->object = object__process_code(
            a_context,
            (Object *)a_operator,
            &code_right,
            a_flags
        );
    }

    list_items_free(&code_left);
    list_items_free(&code_right);

    if (*a_flags & FLAG_COMMA_NO_RETURN){
        list_item_free(left_item);
        list_item_free(right_item);
        list_del(&a_operator->list);
        list_item_free(a_operator);
    } else {
        // insert left and right result
        left_item->list.prev        = a_operator->list.prev;
        a_operator->list.prev->next = &left_item->list;

        left_item->list.next        = &a_operator->list;
        a_operator->list.prev       = &left_item->list;

        right_item->list.next       = a_operator->list.next;
        a_operator->list.next->prev = &right_item->list;

        a_operator->list.next       = &right_item->list;
    }

    object__unlock(a_context);

out:
    return;
fail:
    goto out;
}

// operator ','
Object *    object__operator_comma(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    const char  *operator       = NULL;
    Object      *ret            = NULL;
    ListItem    *left_item      = NULL;
    Object      *left_object    = NULL;
    ListItem    *right_item     = NULL;
    Object      *right_object   = NULL;
    Object      *window         = NULL;
    int32_t     res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = object__mem_c_str(a_operator->object);
    window   = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_operator->object,
            "cannot get window context\n"
        );
        goto fail;
    }

    //OBJECT__ITEMS_DUMP(window, a_code,
    //    "--------------- before ---------------"
    //    " flags: 0x%x"
    //    "\n",
    //    *a_flags
    //);

    if (*a_flags & FLAG_COMMA_NO_RETURN){
        // remove operator
        list_del(&a_operator->list);
        list_item_free(a_operator);
        goto out;
    }

    // resolve left var
    {
        // get left part
        res = list_item_left(
            a_code,
            a_operator,
            &left_item,
            &left_object
        );
        if (res){
            OBJECT__ITEMS_DUMP(window, a_code, "a_code: ");
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " missing left part,"
                " flags: 0x%x"
                "\n",
                operator,
                *a_flags
            );
            goto fail;
        }

        res = object__item_resolve(
            a_context,
            a_operator->object,
            left_item,
            a_flags
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "cannot resolve var\n"
            );
            goto fail;
        }
    }

    // resolve right var
    {
        // get right part
        res = list_item_right(
            a_code,
            a_operator,
            &right_item,
            &right_object
        );
        if (res){
            OBJECT__ITEMS_DUMP(window, a_code, "a_code: ");
            object__add_error(
                a_context,
                a_operator->object,
                "operator: '%s',"
                " missing right part,"
                " flags: 0x%x"
                "\n",
                operator,
                *a_flags
            );
            goto fail;
        }

        res = object__item_resolve(
            a_context,
            a_operator->object,
            right_item,
            a_flags
        );
        if (res){
            object__add_error(
                a_context,
                a_operator->object,
                "cannot resolve var\n"
            );
            goto fail;
        }
    }

    if (*a_flags & FLAG_COMMA_RETURN_ARRAY){
        // convert values to array
        ret = object__operator_comma_to_array(
            a_context,
            a_code,
            a_operator,
            a_flags
        );
    } else {
        // remove operator
        list_del(&a_operator->list);
        list_item_free(a_operator);
    }

out:
    if (window){
        object__ref_dec(window);
    }

    //OBJECT__ITEMS_DUMP(window, a_code,
    //    "--------------- after ---------------\n"
    //);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// operator ';'
Object *    object__operator_semicolon(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object              *ret        = NULL;
    struct list_head    *tmp, *cur  = NULL;
    ListItem            left;
    ListItem            right;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }
    if (list_empty(&a_code->list)){
        TFATAL("empty list\n");
    }

    object__lock(a_context);

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    memset(&left.list, 0x00, sizeof(left.list));
    INIT_LIST_HEAD(&left.list);

    memset(&right.list, 0x00, sizeof(right.list));
    INIT_LIST_HEAD(&right.list);

    // process left part
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_operator->list;
        cur = tmp, tmp = tmp->next)
    {
        // move to left list
        list_del(cur);
        list_add_tail(cur, &left.list);
    }

    // process right part
    for (cur = a_operator->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        // move to right list
        list_del(cur);
        list_add_tail(cur, &right.list);
    }

    if (!list_empty(&left.list)){
        // left part must use current flags
        ret = object__process_code(
            a_context,
            (Object *)a_operator,
            &left,
            a_flags
        );
        list_items_free(&left);
    }

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    if (!list_empty(&right.list)){
        Flags flags = 0;
        // right part must start with own flags
        if (ret){
            // free old answer
            object__ref_dec(ret);
        }
        ret = object__process_code(
            a_context,
            (Object *)a_operator,
            &right,
            &flags
        );
        list_items_free(&right);
    }

out:
    // remove operator
    list_del(&a_operator->list);
    list_item_free(a_operator);

    // reset flags
    //*a_flags = 0;

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
fail:
    goto out;
}

// operator '()' in generic case
Object *    object__operator_bracket_generic(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object              *ret        = NULL;
    struct list_head    *tmp, *cur  = NULL;
    int32_t             deep        = 1;
    int32_t             was_called  = 0;
    ListItem            code;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    memset(&code, 0x00, sizeof(code));
    INIT_LIST_HEAD(&code.list);

    for (cur = a_operator->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        ListItem    *cur_item   = NULL;
        Object      *cur_object = NULL;

        cur_item    = list_entry(cur, ListItem, list);
        cur_object  = cur_item->object;

        // remove from a_code list
        list_del(cur);

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type,
                "operator"))
        {
            const char *val = NULL;

            val = (const char *)cur_object->mem;

            if (!strcmp(val, "(")){
                deep++;
            } else if (!strcmp(val, ")")){
                deep--;
            }

            if (!deep){
                list_item_free(cur_item);
                break;
            }
        }

        // add to code list
        list_add_tail(cur, &code.list);
    }

    // process code inside brackets
    if (!list_empty(&code.list)){
        //Flags flags = 0; //*a_flags;

        // for avoid 'empty code list' error in cases:
        // alert(two.a.c(1, (5 + two.a.b)));
        //                  ^
        // here ------------/
        //
        // if (1 != (3 + 2)){ alert('ok'); };
        //          ^
        // here-----/
        //
        // because prev '(' set flag FLAG_FUNCTION_CALL
        // we will thinking what it's function-call context
        // and will search code on the left part of '(',
        // so reset it flag

        was_called = 1;

        //flags &= ~FLAG_FUNCTION_CALL;
        //flags &= ~FLAG_IF;
        //flags &= ~FLAG_DONT_RESOLVE_VARS;
        //flags &= ~FLAG_DONT_RESOLVE_VARS_PTRS;

        ret = object__process_code(
            a_context,
            (Object *)a_operator,
            &code,
            a_flags
        );
    }

    PDEBUG(50, "flags: 0x%x\n", *a_flags);

    if (*a_flags & FLAG_BRACKET_RETURN_ARRAY){
        // example: some.method(some code here, some code)
        // we need answer 'array' here
        // even if was only one argument for function
        ObjectArray *args = NULL;

        // convert to array if need
        args = dynamic_pointer_cast(
            ObjectArray,
            ret,
            "object",
            "Array"
        );
        if (!args){
            args = object_array__constructor(
                a_context
            );
            if (!args){
                object__add_error(
                    a_context,
                    a_operator->object,
                    "object_array__constructor() failed\n"
                );
                if (ret){
                    object__ref_dec(ret);
                    ret = NULL;
                }
                goto fail;
            }
            if (was_called){
                object_array__push_obj(args, ret);
            }
            ret = (Object *)args;
        }

        // replace operator on result
        //object__ref_dec(a_operator->object);
        //if (ret){
        //    object__ref_inc(ret);
        //}
        //a_operator->object = ret;

        // remove operator
        list_del(&a_operator->list);
        list_item_free(a_operator);

        // remove code
        list_items_free(&code);

        goto out;
    }

    if (list_empty(&code.list)){
        // there are no result here
        list_del(&a_operator->list);
        list_item_free(a_operator);
    } else {
        // insert result code instead operator
        a_operator->list.prev->next = code.list.next;
        a_operator->list.next->prev = code.list.prev;
        code.list.next->prev        = a_operator->list.prev;
        code.list.prev->next        = a_operator->list.next;

        list_item_free(a_operator);
    }

out:
    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
fail:
    goto out;
}

Object * object__detect_context(
    Object      *a_context,
    Object      *a_position,
    Object      *a_object)
{
    Object *ret = NULL;

    OBJECT_CHECK(a_context);

    if (    a_object
        &&  !strcmp(a_object->parser_type, "variable"))
    {
        // it's some_array.somekey.someval variable
        // get some_array as context
        char *var_name = strdup(object__mem_c_str(a_object));
        char *dot_pos  = strchr(var_name, '.');

        if (dot_pos){
            PWARN("using dots object context\n");
            // get by first part of name
            *dot_pos = '\0';
            object__var_get(
                a_context,
                a_position,
                var_name,
                &ret,
                1   // not exist is error
            );
        } else {
            // get by full name
            object__var_get(
                a_context,
                a_position,
                var_name,
                &ret,
                1   // not exist is error
            );
        }
        if (    ret
            &&  strcmp(ret->type, "object"))
        {
            // variable value is not object
            object__ref_dec(ret);
            ret = NULL;
        }

        free(var_name);
    }

    if (!ret){
        PWARN("using old context\n");
        // use current context
        if (a_context){
            object__ref_inc(a_context);
        }
        ret = a_context;
    }

    return ret;
}

// operator '()'
Object *    object__operator_bracket(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object              *ret                = NULL;
    const char          *operator           = NULL;
    ListItem            *left_item          = NULL;
    Object              *left_object        = NULL;
    ObjectCodeDefine    *code_define        = NULL;
    Object              *operator_object    = NULL;
    Object              *window             = NULL;
    int32_t             res;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    object__lock(a_context);

    operator = object__mem_c_str(a_operator->object);

    // maybe errors?
    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_operator->object,
            "cannot get window context\n"
        );
        goto fail;
    }

    // remove FLAG_RESOLVER_CREATE_VAR flag for case:
    // var a = setTimeout(
    //     function(a_context){
    //         alert(a_context);
    //         alert('timeout after 2 sec');
    //     },
    //     2000,
    //     a_context
    // );
    // because of 'var' will set FLAG_RESOLVER_CREATE_VAR
    // all new vars will be re/created (and will rewrite
    // old values if exist), so third argument: 'a_context'
    // will be 'undefined' (because will be re/created),
    // so remove flags
    *a_flags &= ~FLAG_RESOLVER_CREATE_VAR;

    // we need a_operator->object after call function:
    // 'object__operator_bracket_generic'
    // but it function will dec ref count,
    // so store it until end of this function
    object__ref_inc(a_operator->object);
    operator_object = a_operator->object;

    // get left part for case: function_ptr (a, b)
    res = list_item_left(
        a_code,
        a_operator,
        &left_item,
        &left_object
    );

    if  (   left_object
        &&  left_object->parser_type
        &&  !strcmp(left_object->parser_type, "variable"))
    {
        // left part - variable
        Object *val = NULL;

        object__var_get(
            a_context,
            operator_object,
            object__mem_c_str(left_object),
            &val,
            1   // not exist is error
        );

        if (!val){
            OBJECT__ITEMS_DUMP(window, a_code, "a_code: ");
            object__add_error(
                a_context,
                operator_object,
                "operator: '%s',"
                " left part, unknown var: '%s',"
                " flags: 0x%x"
                "\n",
                operator,
                (const char *)left_object->mem,
                val ? val->type : "undefined",
                *a_flags
            );
            goto fail;
        }

        // check what it's code
        code_define = dynamic_pointer_cast(
            ObjectCodeDefine,
            val,
            "code_define",
            ""
        );
        if (!code_define){
            OBJECT__DUMP(val, "val: ");
            object__add_error(
                a_context,
                operator_object,
                "operator: '%s',"
                " left part is not code (%s),"
                " flags: 0x%x"
                "\n",
                operator,
                val ? val->type : "undefined",
                *a_flags
            );
            object__ref_dec(val);
            goto fail;
        }
        *a_flags |= FLAG_COMMA_RETURN_ARRAY;
        *a_flags |= FLAG_BRACKET_RETURN_ARRAY;
        *a_flags |= FLAG_BRACE_CODE;
    } else if ( left_object
        &&      !strcmp(left_object->type, "object_value"))
    {
        ObjectValue *value  = NULL;
        Object      *val    = NULL;

        value       = (ObjectValue *)left_object;
        val         = value->object;

        code_define = dynamic_pointer_cast(
            ObjectCodeDefine,
            val,
            "code_define",
            ""
        );
        if (!code_define){
            object__add_error(
                a_context,
                operator_object,
                "operator: '%s',"
                " left part is not code (%s),"
                " flags: 0x%x"
                "\n",
                operator,
                val ? val->type : "undefined",
                *a_flags
            );
            goto fail;
        }
        object__ref_inc((Object *)code_define);
        *a_flags |= FLAG_COMMA_RETURN_ARRAY;
        *a_flags |= FLAG_BRACKET_RETURN_ARRAY;
        *a_flags |= FLAG_BRACE_CODE;
    } else if ( left_object
        &&      !strcmp(left_object->type, "code_define"))
    {
        // left part - code
        code_define = (ObjectCodeDefine *)left_object;
        object__ref_inc((Object *)code_define);
        *a_flags |= FLAG_COMMA_RETURN_ARRAY;
        *a_flags |= FLAG_BRACKET_RETURN_ARRAY;
        *a_flags |= FLAG_BRACE_CODE;
    }

    PDEBUG(50, "before call brackets generic,"
        " flags: 0x%x\n",
        *a_flags
    );

    // ok, now process brackets
    ret = object__operator_bracket_generic(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

    PDEBUG(50, "after call brackets generic,"
        " flags: 0x%x\n",
        *a_flags
    );

    res = object__is_error(a_context);
    if (res){
        goto fail;
    }

    if (code_define){
        ObjectCodeCall  *code_call  = NULL;
        ObjectArray     *args       = NULL;

        args = dynamic_pointer_cast(
            ObjectArray,
            ret,
            "object",
            "Array"
        );
        if (!args){
            object__add_error(
                a_context,
                operator_object,
                "operator: '%s',"
                " function args is not array (%s)"
                "\n",
                operator,
                ret->type
            );
            goto fail;
        }

        // now work with args instead ret
        ret = NULL;

        code_call = object_code_call__constructor(
            a_context
        );
        if (!code_call){
            object__ref_dec((Object *)args);
            object__add_error(
                a_context,
                operator_object,
                "operator: '%s',"
                " object_code_call__constructor() failed\n",
                operator
            );
            goto fail;
        }

        object__ref_inc((Object *)code_define);
        code_call->code_define = code_define;

        code_call->context = object__detect_context(
            a_context,
            operator_object,
            left_object
        );

        // because args will be returned as 'ret'
        code_call->args = args;

        // insert code_call item instead left part
        object__ref_dec(left_item->object);
        left_item->object = (Object *)code_call;

        //PWARN("code_call ref_count: '%d'\n",
        //    code_call->super.ref_count
        //);
    }

out:
    if (window){
        object__ref_dec(window);
    }
    if (code_define){
        object__ref_dec((Object *)code_define);
    }
    if (operator_object){
        object__ref_dec(operator_object);
    }

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// statement 'new'
Object *    object__statement_new(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object *ret = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    *a_flags |= FLAG_FUNCTION_CALL_NEW;

    // remove statement
    list_del(&a_statement->list);
    list_item_free(a_statement);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

// statement 'var'
Object *    object__statement_var(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object *ret = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    *a_flags |= FLAG_RESOLVER_CREATE_VAR;

    // remove statement
    list_del(&a_statement->list);
    list_item_free(a_statement);

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}

/*
// statement 'return'
Object *    object__statement_return(
    ObjectParser    *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object *ret = NULL;

    OBJECT_CHECK(a_context);
    if (!a_code){
        TFATAL("missing argument: 'a_code'\n");
    }
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    object__lock(a_context);

    a_statement->object->parser_type = "operator";

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
}
*/

Object *    object__process_statement(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_statement,
    Flags           *a_flags)
{
    Object          *ret                = NULL;
    Object          *val                = NULL;
    ObjectStatement *object_statement   = NULL;
    Object          *tmp                = NULL;
    Object          *window             = NULL;
    int32_t         res                 = NULL;

    OBJECT_CHECK(a_context);
    if (!a_statement){
        TFATAL("missing argument: 'a_statement'\n");
    }

    if (list_empty(&a_code->list)){
        TFATAL("list empty()\n");
    }

    object__lock(a_context);

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_statement->object,
            "cannot get window context\n"
        );
        goto fail;
    }

    tmp = object__constructor(a_context);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    // copy statement name
    object__mem_add_printf(
        tmp,
        "statements.%s",
        object__mem_c_str(a_statement->object)
    );

    OBJECT__DUMP(a_context, "a_context:\n");

    // search statement
    //res = object__prop_get_str_obj(
    //    a_context,
    //    (const char *)tmp->mem,
    //    &val
    //);
    res = object__var_get(
        a_context,
        NULL,   // position
        object__mem_c_str(tmp),
        &val,
        1       // not exist is error
    );
    if (res){
        object__add_error(
            a_context,
            NULL,
            "statement handler not found for: '%s'\n",
            object__mem_c_str(tmp)
        );
        goto fail;
    }

    object_statement = dynamic_pointer_cast(
        ObjectStatement,
        val,
        "statement",
        ""
    );
    if (!object_statement){
        object__add_error(
            a_context,
            NULL,
            "statement handler for: '%s' found,"
            " but have wrong type: '%s'"
            " (must be: 'statement')\n",
            object__mem_c_str(tmp),
            val->type
        );
        goto fail;
    }

    // for avoid free
    // (we should free 'object_statement' instead 'val')
    val = NULL;

    if (!object_statement->func){
        object__add_error(
            a_context,
            NULL,
            "statement handler func is NULL\n"
        );
        goto fail;
    }

    PDEBUG(50, "before process statement: '%s',"
        " flags: 0x%x\n",
        object__mem_c_str(tmp),
        *a_flags
    );

    ret = object_statement->func(
        a_context,
        a_code,
        a_statement,
        a_flags
    );

    PDEBUG(50, "after process statement: '%s',"
        " flags: 0x%x\n",
        object__mem_c_str(tmp),
        *a_flags
    );

out:
    if (window){
        object__ref_dec(window);
    }
    if (val){
        object__ref_dec(val);
    }
    if (object_statement){
        object__ref_dec((Object *)object_statement);
    }
    if (tmp){
        object__ref_dec(tmp);
    }

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;
fail:
    goto out;
}

Object *    object__process_operator(
    Object          *a_context,
    ListItem        *a_code,
    ListItem        *a_operator,
    Flags           *a_flags)
{
    Object          *ret                = NULL;
    Object          *val                = NULL;
    ObjectOperator  *object_operator    = NULL;
    Object          *tmp                = NULL;
    int32_t         res;

    OBJECT_CHECK(a_context);
    if (!a_operator){
        TFATAL("missing argument: 'a_operator'\n");
    }

    if (list_empty(&a_code->list)){
        TFATAL("list empty()\n");
    }

    object__lock(a_context);

    tmp = object__constructor(a_context);
    if (!tmp){
        object__add_error(
            a_context,
            NULL,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    // copy operator name
    object__mem_add_printf(
        tmp,
        "operators.%s",
        object__mem_c_str(a_operator->object)
    );

    // search operator
    //res = object__prop_get_str_obj(
    //    a_context,
    //    object__mem_c_str(tmp),
    //    &val
    //);
    res = object__var_get(
        a_context,
        a_operator->object,
        object__mem_c_str(tmp),
        &val,
        1   // not exist is error
    );
    if (res){
        object__add_error(
            a_context,
            NULL,
            "operator handler not found for: '%s'\n",
            object__mem_c_str(tmp)
        );
        goto fail;
    }

    object_operator = dynamic_pointer_cast(
        ObjectOperator,
        val,
        "operator",
        ""
    );
    if (!object_operator){
        object__add_error(
            a_context,
            NULL,
            "operator handler for: '%s' found,"
            " but have wrong type: '%s'"
            " (must be: 'operator')\n",
            (const char *)tmp->mem,
            val->type
        );
        goto fail;
    }

    // for avoid double memory free
    val = NULL;

    if (!object_operator->func){
        object__add_error(
            a_context,
            NULL,
            "operator handler func is NULL\n"
        );
        goto fail;
    }

    ret = object_operator->func(
        a_context,
        a_code,
        a_operator,
        a_flags
    );

out:
    if (tmp){
        object__ref_dec(tmp);
    }
    if (val){
        object__ref_dec(val);
    }
    if (object_operator){
        object__ref_dec((Object *)object_operator);
    }

    object__unlock(a_context);
    return ret;

fail:
    goto out;
}

Object * object__process_code(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ListItem        *a_code,
    Flags           *a_flags)
{
    Object              *ret                = NULL;
    struct list_head    *cur                = NULL;
    struct list_head    *tmp                = NULL;
    ListItem            *cur_item           = NULL;
    Object              *cur_object         = NULL;
    ListItem            *max_operator       = NULL;
    Object              *window             = NULL;
    int32_t             max_operator_weight = 0;
    int32_t             weight              = 0;
    int32_t             res;

    OBJECT_CHECK(a_context);

    if (list_empty(&a_code->list)){
        TFATAL("empty code list\n");
    }

    object__lock(a_context);

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_position,
            "cannot get window context\n"
        );
        goto fail;
    }

    OBJECT__ITEMS_DUMP(window, a_code,
        "process code (flags: 0x%x): ",
        *a_flags
    );

again:
    max_operator        = NULL;
    max_operator_weight = 0;

    if (list_empty(&a_code->list)){
        object__unlock(a_context);
        goto out;
    }

    res = object__is_error(a_context);
    if (res){
        object__unlock(a_context);
        goto fail;
    }

    if (50 <= g_debug_level){
        OBJECT__ITEMS_DUMP(window, a_code,
            "before process statements:"
        );
    }

    // process statement(s)
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        cur_item   = list_entry(cur, ListItem, list);
        cur_object = cur_item->object;

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type, "operator")
            &&  !strcmp((const char *)cur_object->mem, ";"))
        {
            // search statement only until first ';', example:
            // var res = test(); return res;
            // we have two statements here, and should process
            // only the first of these
            break;
        }

        if (    cur_object
            &&  cur_object->parser_type
            &&  !strcmp(cur_object->parser_type, "statement"))
        {
            if (ret){
                object__ref_dec(ret);
                ret = NULL;
            }
            PWARN("statement: '%s', line: '%d', col: '%d'\n",
                object__mem_c_str(cur_object),
                cur_object->parser_line,
                cur_object->parser_col
            );

            OBJECT_CHECK(a_context);
            ret = object__process_statement(
                a_context,
                a_code,
                cur_item,
                a_flags
            );
            OBJECT_CHECK(a_context);
            if (ret){
                OBJECT_CHECK(ret);
            }

            res = object__is_error(a_context);
            if (res){
                object__unlock(a_context);
                goto fail;
            } else {
                // because object__process_statement
                // can modify list
                goto again;
            }
        }
    }

    if (50 <= g_debug_level){
        OBJECT__ITEMS_DUMP(window, a_code,
            "after process statements,"
            " flags: 0x%x, a_code:\n",
            *a_flags
        );
    }

    // search top operator
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        cur_item   = list_entry(cur, ListItem, list);
        cur_object = cur_item->object;

        if (    !cur_object
            ||  !cur_object->parser_type
            ||  strcmp(cur_object->parser_type, "operator"))
        {
            continue;
        }

        //PWARN("search top operator,"
        //   " parser_type: '%s', val: '%s'\n",
        //    cur_object->parser_type,
        //    object__mem_c_str(cur_object)
        //);

        weight = object__get_operator_weight(
            a_context,
            cur_object,
            a_flags,
            object__mem_c_str(cur_object)
        );

        PWARN("weight: '%d'\n", weight);

        if (max_operator_weight < weight){
            max_operator_weight = weight;
            max_operator        = cur_item;
        }
    }

    if (max_operator){
        //PWARN("max_operator: '%s', line: '%d', col: '%d'\n",
        //    object__mem_c_str(max_operator->object),
        //    max_operator->object->parser_line,
        //    max_operator->object->parser_col
        //);

        if (ret){
            object__ref_dec(ret);
            ret = NULL;
        }

        ret = object__process_operator(
            a_context,
            a_code,
            max_operator, // handler will remove max_operator
            a_flags
        );

        res = object__is_error(a_context);
        if (res){
            object__unlock(a_context);
            goto fail;
        }

        if (*a_flags & FLAG_RETURN){
            *a_flags &= ~FLAG_RETURN;
            object__unlock(a_context);
            goto out;
        } else {
            goto again;
        }
    }

    // OBJECT__ITEMS_DUMP(window, a_code,
    //     "after process operators: \n"
    // );

    if (list_empty(&a_code->list)){
        object__unlock(a_context);
        goto out;
    }

    //OBJECT__ITEMS_DUMP(window, a_code,
    //    "before resolve a_code:"
    //);

    // resolve vars
    for (cur = a_code->list.next, tmp = cur->next;
        cur != &a_code->list;
        cur = tmp, tmp = tmp->next)
    {
        cur_item   = list_entry(cur, ListItem, list);
        cur_object = cur_item->object;

        if (cur_object){
            OBJECT_CHECK(cur_object);
        }

        //PWARN("resolve: '%s' type: '%s', ref_count: '%d'\n",
        //    (const char *)cur_item->object->mem,
        //    cur_item->object->type,
        //    cur_item->object->ref_count
        //);

        object__item_resolve(
            a_context,
            cur_object,
            cur_item,
            a_flags
        );

        //OBJECT__DUMP(cur_item->object, "after resolve: ");

        // replace return value
        {
            if (ret){
                object__ref_dec(ret);
                ret = NULL;
            }
            if (cur_item->object){
                object__ref_inc(cur_item->object);
            }
            ret = cur_item->object;
            if (ret){
                OBJECT_CHECK(ret);
            }
        }
    }

    //OBJECT__ITEMS_DUMP(window, a_code,
    //    "after resolve a_code:"
    //);

    object__unlock(a_context);

out:
    if (window){
        object__ref_dec(window);
    }
    if (ret){
        OBJECT_CHECK(ret);
    }
    //OBJECT__DUMP(ret, "--> ret: ");
    return ret;
fail:
    goto out;
}

void object__store_pos_info(
    Object          *a_context,
    ObjectString    *a_out)
{
    OBJECT_CHECK(a_context);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    a_out->super.parser_pos     = a_context->parser_pos;
    a_out->super.parser_line    = a_context->parser_line;
    a_out->super.parser_col     = a_context->parser_col;
}

Object * object__parse_buffer(
    Object          *a_context,
    Object          *a_position,
    ObjectString    *a_buffer)
{
    Object          *ret        = NULL;
    Object          *parser     = a_context;
    ObjectString    *tmp_word   = NULL;
    int32_t         state       = 0;
    double          length      = 0;
    Flags           flags       = 0;
    uint8_t         c           = 0;
    uint8_t         c_prev      = 0;
    uint8_t         c_next      = 0;
    ListItem        code;
    int32_t         res;

    OBJECT_CHECK(a_context);
    if (!a_buffer){
        TFATAL("missing argument: 'a_buffer'\n");
    }

    object__lock(parser);

    // init code list
    memset(&code, 0x00, sizeof(code));
    INIT_LIST_HEAD(&code.list);

    // init tmp word
    tmp_word = object_string__constructor(
        a_context
    );
    if (!tmp_word){
        object__add_error(
            a_context,
            NULL,
            "object_string__constructor() failed\n"
        );
        goto fail;
    }

    // get code length
    object__length_get((Object *)a_buffer, &length);

    parser->parser_pos  = 0;
    parser->parser_line = 1;
    parser->parser_col  = 1;

    for (parser->parser_pos = 0;
        parser->parser_pos < length;
        parser->parser_pos++)
    {
        // maybe we have errors?
        res = object__is_error(a_context);
        if (res){
            break;
        }

        // read cur char
        res = object_string__charAt(
            a_buffer,
            parser->parser_pos,
            &c
        );
        if (res){
            object__add_error(
                a_context,
                NULL,
                "cannot read char from pos: '%d'\n",
                parser->parser_pos
            );
            break;
        }

        // read next char
        object_string__charAt(
            a_buffer,
            parser->parser_pos + 1,
            &c_next
        );

        if ('\n' == c_prev){
            parser->parser_line++;
            parser->parser_col = 1;
        } else {
            parser->parser_col++;
        }

again:
        if (c_prev){
            PDEBUG(100, "pos: '%d',"
                " state: '%d',"
                " c: '%c'"
                " c_prev: '%c'"
                "\n",
                parser->parser_pos,
                state,
                c,
                c_prev
            );
        } else {
            PDEBUG(100, "pos: '%d',"
                " state: '%d',"
                " c: '%c'"
                " c_prev: ''"
                "\n",
                parser->parser_pos,
                state,
                c
            );
        }

        switch (state){
            case STATE_WORD_COLLECT_STRING_END:
                {
                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    object__add_item(
                        a_context,
                        &code,
                        "constant",
                        (Object *)tmp_word
                    );

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                }
                goto again;

            // string "..."
            case STATE_WORD_COLLECT_STRING2_ESC:
                if ('"' == c){
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                } else {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "\\%c",
                        c
                    );
                }
                state = STATE_WORD_COLLECT_STRING2;
                break;

            case STATE_WORD_COLLECT_STRING2:
                if ('\\' == c){
                    state = STATE_WORD_COLLECT_STRING2_ESC;
                } else if ('"' == c){
                    state = STATE_WORD_COLLECT_STRING_END;
                } else {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                }
                break;

            // string '...'
            case STATE_WORD_COLLECT_STRING1_ESC:
                object__mem_add_printf(
                    (Object *)tmp_word,
                    "%c",
                    c
                );
                state = STATE_WORD_COLLECT_STRING1;
                break;

            case STATE_WORD_COLLECT_STRING1:
                if ('\\' == c){
                    state = STATE_WORD_COLLECT_STRING1_ESC;
                } else if ('\'' == c){
                    state = STATE_WORD_COLLECT_STRING_END;
                } else {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                }
                break;

            case STATE_WORD_COLLECT:
                if (    ('a' <= c && 'z' >= c)
                    ||  ('A' <= c && 'Z' >= c)
                    ||  ('0' <= c && '9' >= c)
                    ||  '.' == c
                    ||  '_' == c)
                {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                } else {
                    int32_t is_undefined    = 0;
                    int32_t is_statement      = 0;
                    int32_t is_operator     = 0;

                    is_undefined = !strcmp(
                        (const char *)tmp_word->super.mem,
                        "undefined"
                    );

                    if (!is_undefined){
                        is_statement = object__is_statement(
                            a_context,
                            (Object *)tmp_word,
                            &flags,
                            object_string__mem_c_str(tmp_word)
                        );
                        is_operator = object__is_operator(
                            a_context,
                            (Object *)tmp_word,
                            &flags,
                            object_string__mem_c_str(tmp_word)
                        );
                    }

                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    if (is_statement){
                        object__add_item(
                            a_context,
                            &code,
                            "statement",
                            (Object *)tmp_word
                        );
                    } else if (is_operator){
                        object__add_item(
                            a_context,
                            &code,
                            "operator",
                            (Object *)tmp_word
                        );
                    } else if (is_undefined){
                        object__add_item(
                            a_context,
                            &code,
                            "constant",
                            NULL
                        );
                        object__ref_dec((Object *)tmp_word);
                        tmp_word = NULL;
                    } else if (
                            !strcmp(
                                (const char *)tmp_word->super.mem,
                                "true")
                        ||  !strcmp(
                                (const char *)tmp_word->super.mem,
                                "false"))

                    {
                        ObjectBoolean *val = NULL;

                        val = object_boolean__constructor_double(
                            !strcmp(
                                (const char *)tmp_word->super.mem,
                                "true"
                            ) ? 1 : 0,
                            a_context
                        );
                        object__add_item(
                            a_context,
                            &code,
                            "constant",
                            (Object *)val
                        );

                        object__ref_dec((Object *)tmp_word);
                        tmp_word = NULL;
                    } else {
                        object__add_item(
                            a_context,
                            &code,
                            "variable",
                            (Object *)tmp_word
                        );
                    }

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                    goto again;
                }
                break;

            case STATE_WORD_COLLECT_NUMBER_FLOAT:
                if (    '0' <= c
                    &&  '9' >= c)
                {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                } else if ( ';' == c
                    ||      ' ' == c
                    ||      '}' == c
                    ||      ')' == c)
                {
                    ObjectNumber *val = NULL;

                    val = object_number__constructor_double(
                        object_string__to_double(
                            tmp_word
                        ),
                        a_context
                    );

                    object__add_item(
                        a_context,
                        &code,
                        "constant",
                        (Object *)val
                    );

                    object__ref_dec((Object*)tmp_word);

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                    goto again;
                } else {
                    object__add_error(
                        a_context,
                        NULL,
                        "unexpected char: '%c'\n",
                        c
                    );
                    goto done;
                }
                break;

            case STATE_WORD_COLLECT_NUMBER_INTEGER:
                if (    '0' <= c
                    &&  '9' >= c)
                {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                } else if ('.' == c){
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_WORD_COLLECT_NUMBER_FLOAT;
                } else {
                    ObjectNumber *val = NULL;

                    val = object_number__constructor_double(
                        object_string__to_double(
                            tmp_word
                        ),
                        a_context
                    );

                    object__add_item(
                        a_context,
                        &code,
                        "constant",
                        (Object *)val
                    );

                    object__ref_dec((Object*)tmp_word);

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                    goto again;
                }
                break;

            // comment line
            case STATE_COMMENT_LINE_SKIP_ONE:
                state = STATE_COMMENT_LINE;
                break;

            case STATE_COMMENT_LINE:
                if ('\n' == c){
                    state = STATE_PREPROCESSOR;
                }
                break;

            // comment block
            case STATE_COMMENT_BLOCK_SKIP_ONE:
                state = STATE_COMMENT_BLOCK;
                break;

            case STATE_COMMENT_BLOCK:
                if (    '*' == c_prev
                    &&  '/' == c)
                {
                    state = STATE_PREPROCESSOR;
                }
                break;

            case STATE_OPERATOR_TWO_SYMBOLS:
                {
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );

                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    object__add_item(
                        a_context,
                        &code,
                        "operator",
                        (Object *)tmp_word
                    );

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                }
                break;

            // preprocessor
            case STATE_PREPROCESSOR:
                if (    '/' == c
                    &&  '/' == c_next)
                {
                    state = STATE_COMMENT_LINE_SKIP_ONE;
                } else if ( '/' == c
                    &&      '*' == c_next)
                {
                    state = STATE_COMMENT_BLOCK_SKIP_ONE;
                } else if (0x20 >= c){
                    // skip spaces, special codes, etc
                } else if ( ('a' <= c && 'z' >= c)
                    ||      ('A' <= c && 'Z' >= c))
                {
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_WORD_COLLECT;
                } else if ( '-' == c
                    &&     ('0' <= c_next && '9' >= c_next))
                {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_WORD_COLLECT_NUMBER_INTEGER;
                } else if ( '0' <= c
                    &&      '9' >= c)
                {
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_WORD_COLLECT_NUMBER_INTEGER;
                } else if ('\'' == c){
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    state = STATE_WORD_COLLECT_STRING1;
                } else if ('"' == c){
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    state = STATE_WORD_COLLECT_STRING2;
                } else if ( ('&' == c && '&' == c_next)
                    ||      ('|' == c && '|' == c_next)
                    ||      ('=' == c && '=' == c_next)
                    ||      ('+' == c && '+' == c_next)
                    ||      ('+' == c && '=' == c_next)
                    ||      ('-' == c && '=' == c_next)
                    ||      ('-' == c && '-' == c_next)
                    ||      ('!' == c && '=' == c_next)
                    ||      ('<' == c && '=' == c_next)
                    ||      ('>' == c && '=' == c_next))
                {
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_OPERATOR_TWO_SYMBOLS;
                } else if ( '=' == c
                    ||      '{' == c
                    ||      '}' == c)
                {
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );

                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    object__add_item(
                        a_context,
                        &code,
                        "statement",
                        (Object *)tmp_word
                    );

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                } else if ( 0
                    ||      '<' == c
                    ||      '>' == c
                    ||      '-' == c
                    ||      '+' == c
                    ||      '%' == c
                    ||      '/' == c
                    ||      '*' == c
                    ||      '[' == c
                    ||      ']' == c
                    ||      '(' == c
                    ||      ')' == c
                    ||      ':' == c
                    ||      ',' == c
                    ||      ';' == c)
                {
                    object__store_pos_info(
                        a_context,
                        tmp_word
                    );
                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );

                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    object__add_item(
                        a_context,
                        &code,
                        "operator",
                        (Object *)tmp_word
                    );

                    // prepare new tmp_word
                    tmp_word = object_string__constructor(
                        a_context
                    );
                    if (!tmp_word){
                        object__add_error(
                            a_context,
                            NULL,
                            "object_string__constructor()"
                            " failed\n"
                        );
                        goto done;
                    }
                    state = STATE_PREPROCESSOR;
                } else {
                    object__add_error(
                        a_context,
                        NULL,
                        "unexpected char: '%c'\n",
                        c
                    );
                    goto done;
                }
                break;

            default:
                object__add_error(
                    a_context,
                    NULL,
                    "unsupported state: '%d'\n",
                    state
                );
                goto done;
        };

        c_prev = c;
    }

done:
    res = object__is_error(a_context);
    if (    !res
        &&  !list_empty(&code.list))
    {
        ret = object__process_code(
            a_context,
            a_position,
            &code,
            &flags
        );
    }

out:
    list_items_free(&code);

    if (tmp_word){
        object__ref_dec((Object *)tmp_word);
    }

    object__unlock(parser);

    return ret;
fail:
    goto out;
}

// api alert
Object *    object__api_alert(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object  *ret        = NULL;
    Object  *serialized = NULL;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    object__lock(a_context);

    //OBJECT__DUMP((Object *)a_args, "a_args: ");

    serialized = object__constructor(a_context);
    if (!serialized){
        object__add_error(
            a_context,
            a_position,
            "object__constructor() failed\n"
        );
        goto fail;
    }

    ((Object *)a_args)->serialize_cb(
        (Object *)a_args,
        serialized,
        DEFAULT_DELIMETER
    );

    PALERT("%s\n", (const char *)serialized->mem);

    object__ref_dec(serialized);

    // read press key
    //object_terminal__getchar_hidden();

out:
    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

void object__read_cb(
    struct bufferevent      *a_bev,
    void                    *a_ctx)
{
    ObjectConnection    *connection     = NULL;
    Object              *context        = NULL;
    ObjectAction        *action         = NULL;
    ObjectString        *data           = NULL;
    struct evbuffer     *in             = NULL;
    uint64_t            len             = 0;

    connection = (ObjectConnection *)a_ctx;
    if (!connection){
        TFATAL("missing argument: 'connection'\n");
    }

    context = connection->context;

    object__lock((Object *)context);

    // send event about 'read'
    action = object_action__constructor(
        "read",
        context
    );
    if (!action){
        object__add_error(
            context,
            NULL,
            "object_action__constructor() failed\n"
        );
        goto fail;
    }

    object__prop_set_str_obj(
        (Object *)action,
        "target",
        (Object *)connection
    );

    // get input
    in   = bufferevent_get_input(a_bev);
    len  = EVBUFFER_LENGTH(in);

    // create new string
    data = object_string__constructor_raw(
        (const char *)EVBUFFER_DATA(in),
        len - 1,
        (Object *)action
    );
    // add data to action
    object__prop_set_str_obj(
        (Object *)action,
        "data",
        (Object *)data
    );
    evbuffer_drain(in, len);

    object__dispatchEvent(context, action);

out:
    object__unlock((Object *)connection);

    return;

fail:
    goto out;
}

void object__event_cb(
    struct bufferevent      *a_bev,
    short                   a_what,
    void                    *a_ctx)
{
    ObjectConnection    *connection     = NULL;
    Object              *context        = NULL;
    ObjectAction        *action         = NULL;

    connection = (ObjectConnection *)a_ctx;
    if (!connection){
        TFATAL("missing argument: 'connection'\n");
    }

    context = connection->context;

    object__lock(context);

    // send event about 'event'
    action = object_action__constructor(
        "event",
        context
    );
    if (!action){
        object__add_error(
            context,
            NULL,
            "object_action__constructor() failed\n"
        );
        goto fail;
    }

    OBJECT__DUMP((Object *)connection, "connection:\n");

    object__prop_set_str_obj(
        (Object *)action,
        "target",
        (Object *)connection
    );


    object__dispatchEvent(context, action);

out:
    object__unlock(context);

    return;

fail:
    goto out;
}

void object__accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src,
    int32_t                 a_src_len,
    void                    *a_ctx)
{
    Object              *context    = (Object *)a_ctx;
    ObjectAction        *action     = NULL;
    ObjectConnection    *connection = NULL;

    if (!context){
        TFATAL("missing argument: a_context\n");
    }

    // create new connection
    connection = object_connection__constructor(context);
    connection->fd      = a_fd;
    connection->context = context;

    // wait IO
    connection->be = bufferevent_socket_new(
        context->event_base,
        a_fd,
        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS
    );
    bufferevent_setcb(
        connection->be,
        object__read_cb,
        NULL,
        object__event_cb,
        connection
    );

    OBJECT__DUMP((Object *)connection, "connection: ");

    // it must be before send event, because
    // event handler can close connection
    bufferevent_enable(connection->be, EV_READ | EV_WRITE);

    // send event about 'accept'
    action = object_action__constructor(
        "accept",
        context
    );
    object__ref_inc((Object *)connection);
    object__prop_set_str_obj(
        (Object *)action,
        "target",
        (Object *)connection
    );
    object__dispatchEvent((Object *)context, action);
}

// api listen
Object *    object__api_listen(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object          *ret         = NULL;
    Object          *val         = NULL;
    ObjectString    *addr        = NULL;
    int32_t         res;

    struct sockaddr_storage listen_addr;
    int32_t                 listen_addr_len = 0;
    struct evconnlistener   *listener       = NULL;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    object__lock(a_context);

    // get addrname
    object_array__get_by_index(
        a_args,
        0,
        &val
    );
    addr = dynamic_pointer_cast(
        ObjectString,
        val,
        "string",
        ""
    );
    if (!addr){
        object__add_error(
            a_context,
            a_position,
            "first argument is not 'string' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }

    // now works with addr instead val
    val = NULL;

    PWARN("listen at '%s'\n",
        object_string__mem_c_str(addr)
    );

    listen_addr_len = sizeof(listen_addr);
    memset(&listen_addr, 0x00, listen_addr_len);

    res = evutil_parse_sockaddr_port(
        object_string__mem_c_str(addr),
        (struct sockaddr*)&listen_addr,
        &listen_addr_len
    );
    if (0 > res){
        object__add_error(
            a_context,
            a_position,
            "cannot context addr: '%s'\n",
            object_string__mem_c_str(addr)
        );
        goto fail;
    }

    listener = evconnlistener_new_bind(
        a_context->event_base,
        object__accept_cb,
        a_context,
        LEV_OPT_CLOSE_ON_FREE
            | LEV_OPT_CLOSE_ON_EXEC
            | LEV_OPT_REUSEABLE,
        -1,
        (struct sockaddr*)&listen_addr,
        listen_addr_len
    );
    if (!listener){
        object__add_error(
            a_context,
            a_position,
            "evconnlistener_new_bind() failed"
            " for addr: '%s'\n",
            object_string__mem_c_str(addr)
        );
        goto fail;
    }

out:
    if (val){
        object__ref_dec(val);
    }
    if (addr){
        object__ref_dec((Object *)addr);
    }

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    return ret;

fail:
    goto out;
}

// api timer
void object__timer_cb(
    int32_t a_fd,
    short   a_event,
    void    *a_arg)
{
    ObjectTimer         *timer          = NULL;
    Flags               flags           = 0;
    ListItem            *code_item      = NULL;
    ObjectCodeCall      *code_call      = NULL;

    timer = (ObjectTimer *)a_arg;
    if (!timer){
        TFATAL("missing argument: 'timer'\n");
    }

    code_item = list_item_alloc();
    if (!code_item){
        object__add_error(
            timer->context,
            (Object *)timer->code_define,
            "list_item_alloc() failed\n"
        );
        goto fail;
    }

    code_call = object_code_call__constructor(timer->context);
    if (!code_call){
        object__add_error(
            timer->context,
            (Object *)timer->code_define,
            "object_code_call__constructor() failed\n"
        );
        goto fail;
    }
    object__ref_inc((Object *)timer->code_define);
    code_call->code_define  = timer->code_define;
    code_call->args         = timer->args;
    object__ref_inc(timer->context);
    code_call->context      = timer->context;
    code_item->object       = (Object *)code_call;

    object__item_resolve(
        timer->context,
        NULL,
        code_item,
        &flags
    );
out:
    // remove code (it will ref dec code_call)
    list_item_free(code_item);

    object__ref_dec((Object *)timer);

    return;

fail:
    goto out;
}

Object *    object__api_exit(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object  *ret    = NULL;
    Object  *window = NULL;

    PWARN("api exit\n");

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    window = object__get_window_context(a_context);
    if (!window){
        object__add_error(
            a_context,
            a_position,
            "cannot get window context\n"
        );
        goto fail;
    }

    OBJECT__DUMP(a_context, "a_context:\n");

    object__shutdown(window);

    object__ref_dec(window);

out:
    return ret;
fail:
    goto out;
}

Object *    object__api_setTimeout(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object              *ret            = NULL;
    Object              *val            = NULL;
    ObjectCodeDefine    *code_define    = NULL;
    ObjectNumber        *timeout_obj    = NULL;
    ObjectTimer         *timer          = NULL;
    Object              *window         = NULL;
    Object              *parent_context = NULL;
    double              timeout         = 0;
    double              integral        = 0;
    double              fractional      = 0;
    double              timers_length   = 0;
    struct timeval      tv;
    int32_t             res;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    object__lock(a_context);

    // get window's context
    window = object__get_window_context(a_context);
    if (!window->event_base){
        object__add_error(
            a_context,
            a_position,
            "window->event_base is NULL\n"
        );
        goto fail;
    }

/*
    parent_context = object__get_parent_context(a_context);
    if (!parent_context){
        object__add_error(
            a_context,
            a_position,
            "parent_context is NULL\n"
        );
        goto fail;
    }
*/
    object__ref_inc(window);
    parent_context = window;

    // get code
    res = object_array__shift(a_args, &val);
    if (res){
        object__add_error(
            a_context,
            a_position,
            "missing first argument (code)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }
    code_define = dynamic_pointer_cast(
        ObjectCodeDefine,
        val,
        "code_define",
        ""
    );
    if (!code_define){
        object__add_error(
            a_context,
            a_position,
            "first argument is not 'code_define' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }

    // now works with code_define instead val
    val = NULL;

    // get timeout
    res = object_array__shift(a_args, &val);
    if (res){
        object__add_error(
            a_context,
            a_position,
            "missing second argument (timeout)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }
    timeout_obj = dynamic_pointer_cast(
        ObjectNumber,
        val,
        "number",
        ""
    );
    if (!timeout_obj){
        object__add_error(
            a_context,
            a_position,
            "second argument is not 'number' (%s)"
            "\n",
            val ? val->type : "undefined"
        );
        goto fail;
    }

    // now works with timeout_obj instead val
    val = NULL;

    timeout = object_number__to_double(timeout_obj);

    //OBJECT__DUMP((Object *)code_define,   "code_define: ");
    //OBJECT__DUMP((Object *)timeout_obj,   "timeout: ");

    //PWARN("timeout: '%g'\n", timeout);

    // init timers queue
    if (!parent_context->timers){
        parent_context->timers = (struct obect_array *)
            object_array__constructor(
                (Object *)parent_context
            );
        if (!parent_context->timers){
            PERROR("object_array__constructor() failed\n");
            goto fail;
        }
    }

    // init timer
    timer = object_timer__constructor(parent_context);
    if (!timer){
        object__add_error(
            a_context,
            a_position,
            "object_timer__constructor() failed\n"
        );
        goto fail;
    }

    object__ref_inc(parent_context);
    timer->context = parent_context;

    object__ref_inc((Object *)code_define);
    timer->code_define = code_define;

    object__ref_inc((Object *)a_args);
    timer->args = a_args;

    object_array__push_obj(
        (ObjectArray *)parent_context->timers,
        (Object *)timer
    );
    object__length_get(
        (Object *)parent_context->timers,
        &timers_length
    );
    OBJECT__DUMP(parent_context,
        "timers_length: '%g'\n",
        timer->id
    );
    timer->id = timers_length;

    // init event
    timer->ev = event_new(
        window->event_base,
        -1, // fd
        0,  // EV_PERSIST
        object__timer_cb,
        timer
    );
    if (!timer->ev){
        object__add_error(
            a_context,
            a_position,
            "event_new() failed\n"
        );
        goto fail;
    }

    // timeout in microseconds
    fractional = modf(timeout/1000, &integral);
    tv.tv_usec = fractional * 1e6; // microseconds
    tv.tv_sec  = integral;

    // because we will do ref dec below
    object__ref_inc((Object *)timer);

    // schedule event
    event_add(timer->ev, &tv);

    ret = (Object *)object_number__constructor_double(
        timer->id,
        a_context
    );
    if (!ret){
        object__add_error(
            a_context,
            a_position,
            "object_number__constructor_double() failed\n"
        );
        goto fail;
    }

out:
    if (parent_context){
        object__ref_dec(parent_context);
    }
    if (window){
        object__ref_dec(window);
    }
    if (timer){
        object__ref_dec((Object *)timer);
    }
    if (val){
        object__ref_dec(val);
    }
    if (code_define){
        object__ref_dec((Object *)code_define);
    }
    if (timeout_obj){
        object__ref_dec((Object *)timeout_obj);
    }

    object__unlock(a_context);

    if (ret){
        OBJECT_CHECK(ret);
    }

    //OBJECT__DUMP(ret, "ret: ");

    return ret;

fail:
    if (timer){
        object__ref_dec((Object *)timer);
        timer = NULL;
    }
    goto out;
}

Object *    object__api_Action(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    ObjectAction    *ret        = NULL;
    Object          *val        = NULL;
    ObjectString    *name       = NULL;

    int32_t         res;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    //if (!(*a_flags & FLAG_FUNCTION_CALL_NEW)){
    //    goto fail;
    //}

    res = object_array__get_by_index(
        a_args,
        0,
        &val
    );
    if (res){
        object__add_error(
            a_context,
            a_position,
            "missing action name"
            "\n"
        );
        goto fail;
    }

    name = dynamic_pointer_cast(
        ObjectString,
        val,
        "string",
        ""
    );
    if (!name){
        object__add_error(
            a_context,
            a_position,
            "action name is not string (%s)"
            "\n",
            val->type
        );
        goto fail;
    }

    // now work with name instead val
    val = NULL;

    // create action
    ret = object_action__constructor(
        object_string__mem_c_str(name),
        a_context
    );
    if (!ret){
        object__add_error(
            a_context,
            a_position,
            "object_action__constructor() failed\n"
        );
        goto fail;
    }

out:
    if (val){
        object__ref_dec(val);
    }
    if (name){
        object__ref_dec((Object *)name);
    }

    return (Object *)ret;

fail:
    goto out;
}
