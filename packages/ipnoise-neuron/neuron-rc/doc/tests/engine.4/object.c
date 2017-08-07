#include "log.h"
#include "object.h"
#include "object_boolean.h"
#include "object_operator.h"
#include "object_statement.h"
#include "object_window.h"

#define object__destructor // for ctags

void void___object__destructor___Object_ptr(
    Object *a_object)
{
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem = NULL;
    }
    a_object->mem_size = 0;

    if (a_object->error){
        free(a_object->error);
        a_object->error = NULL;
    }

    free(a_object);
}

#define object__constructor // for ctags

Object * Object_ptr___object__constructor___const_uint32t(
    const uint32_t  a_size)
{
    Object      *ret        = NULL;
    uint32_t    min_size    = sizeof(*ret);

    if (min_size > a_size){
        TFATAL("object size: '%d'"
            " less than min size: '%d'\n",
            a_size,
            min_size
        );
    }

    ret = calloc(1, a_size);

    ret->ref_count = 1;

    // init mutex
    pthread_mutexattr_init(&ret->mutex_attr);
    pthread_mutexattr_settype(
        &ret->mutex_attr,
        PTHREAD_MUTEX_RECURSIVE
    );
    pthread_mutex_init(
        &ret->mutex,
        &ret->mutex_attr
    );

    // setup handlers
    ret->destructor = void___object__destructor___Object_ptr;

    return ret;
}

Object * Object_ptr___object__constructor()
{
    Object *ret = NULL;

    ret = Object_ptr___object__constructor___const_uint32t(
        sizeof(*ret)
    );

    return ret;
}

#define object__create // for ctags

int32_t  int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr(
    const char  *a_type,
    const char  *a_class,
    Object      **a_out)
{
    int32_t err = -1;

    if (!a_type){
        TFATAL("missing argument: 'a_type'\n");
    }
    if (!strcmp(a_type, "")){
        TFATAL("empty argument: 'a_type'\n");
    }

    TWARN("create object with"
        " type: '%s', class: '%s'\n",
        a_type,
        a_class
    );

    if (        !strcmp(a_type,     "object")
        &&      !strcmp(a_class,    ""))
    {
        *a_out = Object_ptr___object__constructor();
    } else if ( !strcmp(a_type,     "object")
        &&      !strcmp(a_class,    "Window"))
    {
        *a_out = (Object *)ObjectWindow_ptr___object_window__constructor();
    } else {
        TFATAL("unsupported case,"
            " type: '%s', class: '%s'"
            "\n",
            a_type,
            a_class
        );
    }

    // all ok
    err = 0;

    return err;
}

int32_t int32t___object__create___Object_ptr_ptr(
    Object **a_out)
{
    int32_t err = -1;

    err = int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr(
        "object",
        "",
        a_out
    );

    return err;
}

#define object__ref_dec // for ctags

void void___object__ref_dec___Object_ptr(
    Object *a_object)
{
    OBJECT_CHECK(a_object);
    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    a_object->ref_count--;
    if (!a_object->ref_count){
        a_object->destructor(a_object);
    }
}

void void___object__neigh_right_add___Object_ptr__Object_ptr(
    Object  *a_object,
    Object  *a_neigh)
{

}

Object * object__add_item(
    Object          *a_context,
    const char      *a_parser_type,
    Object          *a_object)
{
    Object *ret = NULL;

    TFATAL("here\n");

    OBJECT_CHECK(a_context);
    if (!a_parser_type){
        TFATAL("missing argument: 'a_parser_type'\n");
    }
    OBJECT_CHECK(a_object);

    a_object->parser_type = a_parser_type;

    void___object__neigh_right_add___Object_ptr__Object_ptr(
        a_context,
        a_object
    );

    //void___object__ref_dec___Object_ptr(a_object);

    return ret;
}

#define object__parse // for ctags

int32_t int32t___object__parse___Object_ptr__const_char_ptr(
    Object      *a_context,
    const char  *a_code)
{
    int32_t     res, err    = -1;
    int32_t     state       = 0;
    int32_t     parser_pos  = 0;
    int32_t     parser_line = 1;
    int32_t     parser_col  = 1;
    uint8_t     c           = '\0';
    uint8_t     c_prev      = '\0';
    uint8_t     c_next      = '\0';
    Object      *tmp_word   = NULL;
    Flags       flags       = 0;

    res = int32t___object__create___Object_ptr_ptr(&tmp_word);
    if (res){
        PERROR("Object_ptr___object__create() failed\n");
        goto fail;
    }

    do {
        c = a_code[parser_pos];
        if ('\0' == c){
            break;
        }

        c_next = a_code[parser_pos + 1];

        TWARN("state: '%d',"
            " c: '%c', c_prev: '%c', c_next: '%c'"
            "\n",
            state,
            c,
            c_prev,
            c_next
        );

        if ('\n' == c_prev){
            parser_line++;
            parser_col = 1;
        } else {
            parser_col++;
        }

again:

        switch (state){
/*
            case STATE_WORD_COLLECT_STRING_END:
                {
                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    object__add_item(
                        a_context,
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
            */

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
                    int32_t is_statement    = 0;
                    int32_t is_operator     = 0;

                    is_undefined = !strcmp(
                        object__mem_c_str(tmp_word),
                        "undefined"
                    );

                    if (!is_undefined){
                        is_statement = object__is_statement(
                            a_context,
                            tmp_word,
                            &flags,
                            object__mem_c_str(tmp_word)
                        );
                        is_operator = object__is_operator(
                            a_context,
                            tmp_word,
                            &flags,
                            object__mem_c_str(tmp_word)
                        );
                    }

                    // now it's object__add_item's care
                    // about tmp_word, so we don't need dec
                    // ref count here
                    if (is_statement){
                        object__add_item(
                            a_context,
                            "statement",
                            tmp_word
                        );
                    } else if (is_operator){
                        object__add_item(
                            a_context,
                            "operator",
                            tmp_word
                        );
                    } else if (is_undefined){
                        object__add_item(
                            a_context,
                            "constant",
                            NULL
                        );
                        void___object__ref_dec___Object_ptr(
                            tmp_word
                        );
                        tmp_word = NULL;
                    } else if (
                            !strcmp(
                                object__mem_c_str(tmp_word),
                                "true")
                        ||  !strcmp(
                                object__mem_c_str(tmp_word),
                                "false"))

                    {
                        Object *val = NULL;

                        res = int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr(
                            "boolean",
                            "",
                            &val
                        );
                        if (res){
                            PERROR("int32t___object__create___const_char_ptr__const_char_ptr__Object_ptr_ptr() failed\n");
                            goto fail;
                        }

                        res = int32_t___object_boolean__set_val___ObjectBoolean_ptr__const_double(
                            (ObjectBoolean *)val,
                            !strcmp(
                                object__mem_c_str(tmp_word),
                                "true"
                            ) ? 1 : 0
                        );
                        if (res){
                            PERROR("int32_t___object_boolean__set_val___ObjectBoolean_ptr__const_double() failed\n");
                            goto fail;
                        }

                        object__add_item(
                            a_context,
                            "constant",
                            val
                        );

                        void___object__ref_dec___Object_ptr(tmp_word);
                        tmp_word = NULL;
                    } else {
                        object__add_item(
                            a_context,
                            "variable",
                            tmp_word
                        );
                    }

                    // prepare new tmp_word
                    res = int32t___object__create___Object_ptr_ptr(
                        &tmp_word
                    );
                    if (res){
                        object__add_error(
                            a_context,
                            NULL,
                            "int32t___object__create___Object_ptr_ptr() failed\n"
                        );
                        goto fail;
                    }
                    state = STATE_PREPROCESSOR;
                    goto again;
                }
                break;

            /*
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
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

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
*/

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
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

                    object__mem_add_printf(
                        tmp_word,
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
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

                    object__mem_add_printf(
                        (Object *)tmp_word,
                        "%c",
                        c
                    );
                    state = STATE_WORD_COLLECT_NUMBER_INTEGER;
                } else if ('\'' == c){
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

                    state = STATE_WORD_COLLECT_STRING1;
                } else if ('"' == c){
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

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
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

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
                        "statement",
                        tmp_word
                    );

                    // prepare new tmp_word
                    res = int32t___object__create___Object_ptr_ptr(
                        &tmp_word
                    );
                    if (res){
                        object__add_error(
                            a_context,
                            NULL,
                            "int32t___object__create___Object_ptr_ptr() failed\n"
                        );
                        goto fail;
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
                    tmp_word->parser_pos    = parser_pos;
                    tmp_word->parser_line   = parser_line;
                    tmp_word->parser_col    = parser_col;

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
                        "operator",
                        tmp_word
                    );

                    // prepare new tmp_word
                    res = int32t___object__create___Object_ptr_ptr(
                        &tmp_word
                    );
                    if (res){
                        object__add_error(
                            a_context,
                            NULL,
                            "int32t___object__create___Object_ptr_ptr() failed\n"
                        );
                        goto fail;
                    }
                    state = STATE_PREPROCESSOR;
                } else {
                    object__add_error(
                        a_context,
                        NULL,
                        "unexpected char: '%c'\n",
                        c
                    );
                    goto fail;
                }
                break;

            default:
                object__add_error(
                    a_context,
                    NULL,
                    "unsupported state: '%d'\n",
                    state
                );
                goto fail;
        };

        c_prev = c;
        parser_pos++;
    } while (1);

    // all ok
    err = 0;

//done:
out:
    if (tmp_word){
        void___object__ref_dec___Object_ptr(tmp_word);
    }
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t object__is_error(
    Object    *a_object)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_object);

    if (a_object->error){
        ret = 1;
    }

    return ret;
}

void object__add_error(
    Object      *a_object,
    Object      *a_position,
    const char  *a_format,
    ...)
{
    va_list ap;

    mem_add_printf(
        &a_object->error,
        "--- near of"
        " line: '%d', col: '%d', file pos: '%d' "
        "---"
        "\n",
        a_position ? a_position->parser_line : 0,
        a_position ? a_position->parser_col  : 0,
        a_position ? a_position->parser_pos  : 0
    );

    va_start(ap, a_format);
    mem_add_printf_va(
        &a_object->error,
        a_format,
        &ap
    );
    va_end(ap);

    PERROR("%s", a_object->error);

    //object__shutdown(a_object);
}

int32_t mem_add_printf_va(
    char        **a_mem,
    const char  *a_format,
    va_list     *a_va)
{
    int32_t     len         = 0;
    int32_t     new_size    = 0;
    char        *tmp        = NULL;
    int32_t     res;

    if (!a_format){
        TFATAL("missing argument: 'a_format'\n");
    }
    if (!a_va){
        TFATAL("missing argument: 'a_va'\n");
    }

    // get cur value length
    if (*a_mem){
        len = strlen(*a_mem);
    }

    res = vasprintf(&tmp, a_format, *a_va);
    if (!tmp){
        TFATAL("vasprintf failed\n");
    }
    if (0 >= res){
        free(tmp);
        TFATAL("vasprintf failed, res: '%d'\n", res);
    }

    new_size    = len + res + 1;
    *a_mem      = (char *)realloc(*a_mem, new_size);

    // copy new data
    strcpy(*a_mem + len, tmp);

    free(tmp);

    return res;
}

int32_t mem_add_printf(
    char        **a_mem,
    const char  *a_format,
    ...)
{
    int32_t ret = 0;
    va_list ap;

    va_start(ap, a_format);
    ret = mem_add_printf_va(
        a_mem,
        a_format,
        &ap
    );
    va_end(ap);

    return ret;
}

int32_t object__mem_add_printf_va(
    Object      *a_object,
    const char  *a_format,
    va_list     *a_va)
{
    int32_t res;

    OBJECT_CHECK(a_object);
    if (!a_format){
        TFATAL("missing argument: 'a_format'\n");
    }
    if (!a_va){
        TFATAL("missing argument: 'a_va'\n");
    }

    object__lock(a_object);

    res = mem_add_printf_va(
        &a_object->mem,
        a_format,
        a_va
    );
    if (0 > res){
        TERROR("mem_add_printf_va() failed\n");
    }
    a_object->mem_size = res;

    object__unlock(a_object);

    return res;
}

int32_t object__mem_add_printf(
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

#define object__mem_set // for ctags

void void___object__mem_set___Object_ptr__char_ptr__const_int64t(
    Object          *a_object,
    char            *a_data,
    const int64_t   a_size)
{
    OBJECT_CHECK(a_object);

    object__mem_free((Object *)a_object);

    a_object->mem       = a_data;
    a_object->mem_size  = a_size;
}

int32_t int32t___object__mem_set___Object_ptr__const_double(
    Object          *a_object,
    const double    a_val)
{
    int32_t err     = -1;
    int64_t size    = sizeof(a_val);
    char    *mem    = NULL;

    OBJECT_CHECK(a_object);

    mem = calloc(1, size);
    if (!mem){
        object__add_error(
            a_object,
            NULL,
            "cannot allocate memory,"
            " was needed: '%ld' byte(s)\n",
            size
        );
        goto fail;
    }

    memcpy(mem, &a_val, size);

    void___object__mem_set___Object_ptr__char_ptr__const_int64t(
        a_object,
        mem,
        size
    );

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

void object__mem_free(
    Object  *a_object)
{
    OBJECT_CHECK(a_object);

    if (a_object->mem){
        free(a_object->mem);
        a_object->mem = NULL;
    }
    a_object->mem_size = 0;
}

char * object__mem_c_str(
    Object *a_object)
{
    char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = a_object->mem;

    return ret;
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

    res = int32t___object__create___Object_ptr_ptr(
        &tmp
    );
    if (!res){
        object__add_error(
            a_context,
            NULL,
            "int32t___object__create___Object_ptr_ptr() failed\n"
        );
        goto fail;
    }

    object__mem_add_printf(
        tmp,
        "statements.%s",
        a_statement
    );

    TFATAL("FIXME\n");
    // search statement
    //res = object__var_get(
    //    a_context,
    //    a_position,
    //    object__mem_c_str(tmp),
    //    &val,
    //    0   // not exist is error
    //);
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
        void___object__ref_dec___Object_ptr(val);
    }
    if (tmp){
        void___object__ref_dec___Object_ptr(tmp);
    }
    object__unlock(a_context);
    return ret;
fail:
    goto out;
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
    TFATAL("FIXME\n");
    //res = object__var_get(
    //    a_context,
    //    a_position,
    //    object__mem_c_str(tmp),
    //    &val,
    //    0   // not exist is error
    //);
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
        void___object__ref_dec___Object_ptr(val);
    }
    if (operator){
        void___object__ref_dec___Object_ptr(
            (Object *)operator
        );
    }
    if (tmp){
        void___object__ref_dec___Object_ptr(tmp);
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

int32_t object__link_get(
    Object          *a_context,
    Object          a_position,
    LinkType        a_link_type,
    const char      *a_name,
    Object          **a_out,
    uint8_t         a_not_exist_is_error)
{
    GHashTable  *table          = NULL;
    int32_t     err             = -1;
    uint8_t     *name           = NULL;

    OBJECT_CHECK(a_context);
    if (!a_name){
        TFATAL("missing argument: 'a_name'\n");
    }

    switch (a_link_type){
        case LINK_TYPE_PARENT:
            table = a_context->parents;
            break;

        case LINK_TYPE_NEIGH_LEFT:
            table = a_context->neighs_left;
            break;

        case LINK_TYPE_NEIGH_RIGHT:
            table = a_context->neighs_right;
            break;

        case LINK_TYPE_CHILD:
            table = a_context->children;
            break;

        default:
            TFATAL("unsupported link type: '%d'\n",
                a_link_type
            );
    };

    PWARN("table: 0x'%lx'\n",
        (uint64_t)table
    );

/*
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
            object__add_error(
                a_context,
                NULL,
                "strdup() failed\n"
            );
            goto fail;
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
                    res = object__link_get(
                        left_object,
                        a_position,
                        a_link_type,
                        (const char *)ptr,
                        &cur_object,
                        a_not_exist_is_error
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
                    res = object__link_get(
                        a_context,
                        a_position,
                        a_link_type,
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
                    void___object__ref_dec___Object_ptr(
                        left_object
                    );
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
        res = object__prop_get(
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
            void___object__ref_dec___Object_ptr(prototype);
        }
        object__prop_get(
            cur_context,
            "prototype",
            &prototype
        );

        cur_context = prototype;
    }

    if (prototype){
        void___object__ref_dec___Object_ptr(prototype);
    }
*/

//out:
    if (name){
        free(name);
    }
    object__unlock(a_context);
    return err;
//fail:
//    goto out;
}

