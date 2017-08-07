#include "http_header.h"

void http_header_free(
    HttpHeader  *a_header)
{
    if (!a_header){
        PFATAL("missing argument: 'a_header'\n");
    }

    if (a_header->name){
        evbuffer_free(a_header->name);
        a_header->name = NULL;
    }

    if (a_header->val){
        evbuffer_free(a_header->val);
        a_header->val = NULL;
    }

    free(a_header);
}

HttpHeader * http_header_alloc()
{
    HttpHeader  *ret = NULL;
    int32_t     size = sizeof(*ret);

    ret = (HttpHeader *)malloc(size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }

    ret->name = evbuffer_new();
    if (!ret->name){
        PERROR("cannot allocate memory\n");
        goto fail;
    }

    ret->val = evbuffer_new();
    if (!ret->val){
        PERROR("cannot allocate memory\n");
        goto fail;
    }

out:
    return ret;
fail:
    if (ret){
        http_header_free(ret);
        ret = NULL;
    }
    goto out;
}

HttpHeader * http_header_get(
    HttpHeader      *a_list,
    const char      *a_name)
{
    int32_t             res         = 0;
    HttpHeader          *ret        = NULL;
    struct list_head    *cur        = NULL;
    struct list_head    *tmp        = NULL;
    HttpHeader          *cur_header = NULL;

    if (!a_list){
        PFATAL("missing argument: 'a_list'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    if (list_empty(&a_list->list)){
        goto out;
    }

    list_for_each_safe(cur, tmp, &a_list->list){
        cur_header = list_entry(cur, HttpHeader, list);
        res = strcasecmp(
            (const char *)EVBUFFER_DATA(cur_header->name),
            a_name
        );
        if (!res){
            ret = cur_header;
            break;
        }
    }

out:
    return ret;
}

void http_headers_dump(
    const char  *a_prefix,
    HttpHeader  *a_list)
{
    struct list_head    *cur        = NULL;
    struct list_head    *tmp        = NULL;
    HttpHeader          *cur_header = NULL;

    if (!a_list){
        PFATAL("missing argument: 'a_list'\n");
    }

    PWARN("%s\n", a_prefix);
    if (list_empty(&a_list->list)){
        PWARN("  empty list\n");
    }
    list_for_each_safe(cur, tmp, &a_list->list){
        cur_header = list_entry(cur, HttpHeader, list);
        PWARN("  header: '%s' => '%s'\n",
            EVBUFFER_DATA(cur_header->name),
            EVBUFFER_DATA(cur_header->val)
        );
    }
}

HttpHeader * http_header_get_create(
    HttpHeader      *a_list,
    const char      *a_name)
{
    HttpHeader  *ret = NULL;
    int32_t     res;

    if (!a_list){
        PFATAL("missing argument: 'a_list'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }

    ret = http_header_get(a_list, a_name);
    if (!ret){
        ret = http_header_alloc();
        if (!ret){
            PERROR("http_header_alloc() failed\n");
            goto fail;
        }
        res = evbuffer_add_printf(
            ret->name,
            "%s",
            a_name
        );
        if (0 >= res){
            PERROR("evbuffer_add_printf failed\n");
            goto fail;
        }
        list_add_tail(
            &ret->list,
            &a_list->list
        );
    }

out:
    return ret;
fail:
    if (ret){
        http_header_free(ret);
        ret = NULL;
    }
    goto out;
}

void http_header_remove(
    HttpHeader  *a_list,
    const char  *a_name)
{
    HttpHeader  *header = NULL;

    if (!a_list){
        PFATAL("missing argument: 'a_list'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }

    header = http_header_get(a_list, a_name);
    if (header){
        http_header_free(header);
    }
}

HttpHeader * http_header_add(
    HttpHeader      *a_list,
    const char      *a_name,
    const char      *a_val)
{
    HttpHeader  *ret = NULL;
    int32_t     res;

    if (!a_list){
        PFATAL("missing argument: 'a_list'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }
    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    ret = http_header_get_create(a_list, a_name);
    if (!ret){
        PERROR("cannot add header: '%s',"
            " cannot get/create header\n",
            a_name
        );
        goto fail;
    }

    // clear old val
    if (ret->val){
        evbuffer_free(ret->val);
        ret->val = NULL;
    }

    // alloc for new val
    ret->val = evbuffer_new();
    if (!ret->val){
        PERROR("cannot add header: '%s',"
            " cannot cannot allocate memory\n",
            a_name
        );
        goto fail;
    }

    // add val
    res = evbuffer_add_printf(
        ret->val,
        "%s",
        a_val
    );
    if (0 >= res){
        PERROR("header: '%s',"
            " evbuffer_add_printf() failed,"
            " res: '%d'\n",
            a_name,
            res
        );
        http_header_free(ret);
        ret = NULL;
        goto fail;
    }

out:
    return ret;
fail:
    goto out;
}

void http_headers_free(
    HttpHeader *a_list)
{
    struct list_head    *first  = NULL;
    HttpHeader          *header = NULL;

    while (!list_empty(&a_list->list)){
        // ok, list is not empty
        first   = a_list->list.next;
        header  = list_entry(first, HttpHeader, list);

        // remove from queue
        list_del(first);

        // free header
        http_header_free(header);
    }
}

