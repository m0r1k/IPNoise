#include <sys/time.h>
#include <unistd.h>

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#include <string>

#include <ipnoise-common/log_common.h>

using namespace std;

// Simple disk IO tester
// 20141130 Roman E. Chechnev morik@
//
// compilation:
// g++ -g3 -O0 -o disk doc/tests/disk.cpp -I /mnt/multimedia/ipnoise/IPNoise/packages
//
// g_files_count depend from inodes count on your fs
// use df -i to see inodes statistics
int32_t g_files_count = 300*1000;

string read_file(
    const string &a_fname)
{
    char    buffer[65535]   = { 0x00 };
    FILE    *file           = NULL;
    string  mode            = "r+";
    int32_t res;

    file = fopen(a_fname.c_str(), mode.c_str());
    if (!file){
        PFATAL("cannot open: '%s' for mode: '%s'\n",
            a_fname.c_str(),
            mode.c_str()
        );
    }

    res = fread(buffer, sizeof(buffer), 1, file);
    if (0 > res){
        PFATAL("cannot read file: '%d', res: '%d'\n",
            a_fname.c_str(),
            res
        );
    }

    fclose(file);

    return buffer;
}

void create_file(
    const string &a_fname,
    const string &a_data)
{
    FILE    *file = NULL;
    string  mode  = "w+";

    file = fopen(a_fname.c_str(), mode.c_str());
    if (!file){
        PFATAL("cannot open: '%s' for mode: '%s'\n",
            a_fname.c_str(),
            mode.c_str()
        );
    }

    fwrite(a_data.c_str(), 1, a_data.size(), file);

    fclose(file);
}

void modify_file(
    const string &a_fname,
    const string &a_data)
{
    FILE    *file = NULL;
    string  mode  = "w+";

    file = fopen(a_fname.c_str(), mode.c_str());
    if (!file){
        PFATAL("cannot open: '%s' for mode: '%s'\n",
            a_fname.c_str(),
            mode.c_str()
        );
    }

    fwrite(a_data.c_str(), 1, a_data.size(), file);

    fclose(file);
}

void append_file(
    const string &a_fname,
    const string &a_data)
{
    FILE    *file = NULL;
    string  mode  = "a+";

    file = fopen(a_fname.c_str(), mode.c_str());
    if (!file){
        PFATAL("cannot open: '%s' for mode: '%s'\n",
            a_fname.c_str(),
            mode.c_str()
        );
    }

    fwrite(a_data.c_str(), 1, a_data.size(), file);

    fclose(file);
}

void test_create()
{
    char            buffer[512] = { 0x00 };
    int32_t         i           = 0;
    struct timeval  tv;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer, sizeof(buffer),
            "/tmp/test/%d.txt",
            i
        );
        create_file(buffer, "hello world");
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

void test_read(
    const int32_t &a_was_renamed = 0)
{
    char            buffer[512] = { 0x00 };
    int32_t         i           = 0;
    struct timeval  tv;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;
    string          content;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer, sizeof(buffer),
            "/tmp/test/%d.txt%s",
            i,
            a_was_renamed ? ".old" : ""
        );
        if (content.empty()){
            content = read_file(buffer);
            PINFO("file: '%s', content: '%s'\n",
                buffer,
                content.c_str()
            );
        } else {
            read_file(buffer);
        }
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

void test_modify()
{
    char            buffer[512] = { 0x00 };
    int32_t         i           = 0;
    struct timeval  tv;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer, sizeof(buffer),
            "/tmp/test/%d.txt",
            i
        );
        modify_file(buffer, "morik has modify it");
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

void test_append()
{
    char            buffer[512] = { 0x00 };
    int32_t         i           = 0;
    struct timeval  tv;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer, sizeof(buffer),
            "/tmp/test/%d.txt",
            i
        );
        append_file(buffer, ", morik has append it");
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

void test_rename()
{
    char            buffer_src[512] = { 0x00 };
    char            buffer_dst[512] = { 0x00 };
    int32_t         res, i          = 0;
    struct timeval  tv;
    double          time_before     = 0.0f;
    double          time_after      = 0.0f;
    double          time_delta      = 0.0f;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer_src, sizeof(buffer_src),
            "/tmp/test/%d.txt",
            i
        );
        snprintf(buffer_dst, sizeof(buffer_dst),
            "/tmp/test/%d.txt.old",
            i
        );

        res = rename(buffer_src, buffer_dst);
        if (res){
            PFATAL("cannot rename: '%s' to '%s'\n",
                buffer_src,
                buffer_dst
            )
        }
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

void test_unlink()
{
    char            buffer[512] = { 0x00 };
    int32_t         res, i      = 0;
    struct timeval  tv;
    double          time_before = 0.0f;
    double          time_after  = 0.0f;
    double          time_delta  = 0.0f;

    gettimeofday(&tv, NULL);
    time_before = tv.tv_sec + tv.tv_usec / 1e6;

    PINFO("%s..\n", __FUNCTION__);

    for (i = 0; i < g_files_count; i++){
        snprintf(buffer, sizeof(buffer),
            "/tmp/test/%d.txt.old",
            i
        );

        res = unlink(buffer);
        if (res){
            PFATAL("cannot unlink: '%s'\n",
                buffer
            )
        }
    }

    gettimeofday(&tv, NULL);
    time_after = tv.tv_sec + tv.tv_usec / 1e6;

    time_delta = time_after - time_before;

    PINFO("%s count: %d,"
        " total: '%f' sec,"
        " per file: '%f' ms\n"
        "\n",
        __FUNCTION__,
        g_files_count,
        time_delta,
        (time_delta * 1e3) / g_files_count
    );
}

int32_t main(void)
{
    int32_t err = -1;

    // all ok
    err = 0;

    test_create();
    test_read();

    test_modify();
    test_read();

    test_append();
    test_read();

    test_rename();
    test_read(1);

    test_unlink();

    return err;
}

