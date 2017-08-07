#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>

#define ELECTRON_RADIUS     2.8179403267e-15f
#define DEFAULT_DELIMITER   ": "
#define DEFAULT_DIR_MODE    0755

using namespace std;

class Utils
{
    public:
        static double rand(
            const double &a_min,
            const double &a_max
        );

        static void init_timeval(
            struct timeval  &a_out,
            const  double   &a_val = 0.0f
        );

        static string   dump(
            const char      *a_data,
            const uint64_t  &a_size
        );
        static string   dump(const string &a_data);
};

inline string getIdFromPath(
    const char *a_path)
{
    string  ret;
    int32_t i   = 0;
    char    c   = 0;

    // should return:
    // db/objects/static.thread.morik   => static.thread.morik
    // db/objects/static.thread.morik/  => static.thread.morik

    do {
        c = a_path[i++];
        if ('\0' == c){
            break;
        }

        if ('\0' == a_path[i]){
            // it's last symbol
            if ('/' != c){
                ret += c;
            }
        } else {
            // it's not last symbol
            if ('/' == c){
                ret = "";
            } else {
                ret += c;
            }
        }
    } while ('\0' != c);

    return ret;
}

//
// if a_len > length of a_str, a_symbol(s) will be added,
// otherwise a_str will be truncated to a_len length
//
inline string s_align_len(
    const char      *a_str,
    const uint32_t  a_len,
    const char      a_symbol = ' ')
{
    uint32_t    i, out_of_range = 0;
    string      ret;

    for (i = 0; i < a_len; i++){
        char c = a_symbol;
        if (!out_of_range){
            c = a_str[i];
            if ('\0' == c){
                out_of_range = 1;
                c = a_symbol;
            }
        }
        ret += c;
    }

    return ret;
}

inline void s_split(
    const char      *a_str,
    const char      *a_delim,
    vector<string>  &a_out,
    const int32_t   &a_strict = 0)
{
    const char  *cur    = a_str;
    const char  *begin  = a_str;
    string      part;

    if (!a_str){
        PFATAL("missing argument: 'a_str'\n");
    }
    if (!a_delim){
        PFATAL("missing argument: 'a_delim'\n");
    }

    while ('\0' != cur[0]){
        const char    *tmp_cur        = NULL;
        const char    *tmp_cur_delim  = NULL;
        const char    *cur_delim      = NULL;

        PDEBUG(100, "cur char: '%c'\n", *cur);

        part      += cur[0];
        cur_delim = a_delim;

        // check delim from current pos
        for (tmp_cur = cur, tmp_cur_delim = cur_delim;
            '\0' != tmp_cur[0] && '\0' != tmp_cur_delim[0];
            tmp_cur++, tmp_cur_delim++)
        {
            const char *next_delim_char = tmp_cur_delim + 1;

            if (tmp_cur[0] != tmp_cur_delim[0]){
                break;
            }

            PDEBUG(100, "search delim,"
                " tmp_cur: '%c', tmp_cur_delim: '%c'\n",
                tmp_cur[0],
                tmp_cur_delim[0]
            );

            if ('\0' == next_delim_char[0]){
                part.assign(begin, cur - begin);
                PDEBUG(100, "end of delim, part: '%s'\n",
                    part.c_str()
                );
                if (    !part.empty()
                    ||  a_strict)
                {
                    a_out.push_back(part);
                }
                part  = "";
                cur   = tmp_cur;
                begin = cur + 1;
                break;
            }
        }

        // check from next symbol
        cur++;
    }

    if (    !part.empty()
        ||  a_strict)
    {
        a_out.push_back(part);
    }
}

inline int32_t s_mkdir(
    const char      *a_path,
    const int32_t   &a_recursive = 0)
{
    int32_t                     res, err = -1;
    vector<string>              parts;
    vector<string>::iterator    parts_it;
    string                      path;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    if (a_recursive){
        ::s_split(a_path, "/", parts);
    } else {
        parts.push_back(a_path);
    }

    if ('/' == a_path[0]){
        path += "/";
    }

    for (parts_it = parts.begin();
        parts_it != parts.end();
        parts_it++)
    {
        struct stat st;
        path += *parts_it + "/";
        res = stat(path.c_str(), &st);
        if (    !res
            &&  S_ISDIR(st.st_mode))
        {
            // already exist
            continue;
        }
        res = ::mkdir(
            path.c_str(),
            DEFAULT_DIR_MODE
        );
        PDEBUG(100, "after mkdir: '%s', res: '%d'\n",
            path.c_str(),
            res
        );
        if (res){
            err = res;
            goto fail;
        }
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

inline int32_t s_rm(
    const char      *a_path,
    const int32_t   &a_recursive      = 0,
    const int32_t   &a_remove_top_dir = 1)
{
    char        buffer[65535]   = { 0x00 };
    int32_t     res, err        = -1;
    int32_t     is_link         = 0;
    struct stat st;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    res = stat(a_path, &st);
    if (res){
        // not exist
        err = 0;
        goto out;
    }

    // looks like S_ISLNK don't work
    // at 'Fedora release 19 (Schrödinger’s Cat)',
    // so work around is readdir
    res = readlink(a_path, buffer, sizeof(buffer));
    if (0 < res){
        is_link = 1;
    }

    PDEBUG(100, "s_rm, a_path: '%s':\n"
        "  a_recursive:         '%d'\n"
        "  a_remove_top_dir:    '%d'\n"
        "  S_ISDIR:             '%d'\n"
        "  S_ISLNK:             '%d'\n"
        "  is_link:             '%d'\n",
        a_path,
        a_recursive,
        a_remove_top_dir,
        S_ISDIR(st.st_mode),
        S_ISLNK(st.st_mode),
        is_link
    );

    if (    S_ISDIR(st.st_mode)
        &&  !is_link)
    {
        DIR             *dirp       = NULL;
        struct dirent   *dir_ent    = NULL;
        string          dir         = a_path;

        if ('/' != dir.at(dir.size() - 1)){
            dir += "/";
        }

        if (!a_recursive){
            PDEBUG(100, "attempt to remove dir: '%s',"
                " but recursive flag is not set\n",
                dir.c_str()
            );
            goto fail;
        }

        dirp = opendir(dir.c_str());
        if (!dirp){
            PERROR("cannot open dir: '%s'\n", dir.c_str());
            goto fail;
        }

        do {
            string  cur_path;

            dir_ent = readdir(dirp);
            if (!dir_ent){
                break;
            }

            if (    !strcmp(dir_ent->d_name, ".")
                ||  !strcmp(dir_ent->d_name, ".."))
            {
                continue;
            }

            cur_path = dir + dir_ent->d_name;

            // it is dir, call recursive
            ::s_rm(
                cur_path.c_str(),
                1,  // recursive
                1   // remove top dir
            );
        } while (dir_ent);

        closedir(dirp);

        if (a_remove_top_dir){
            res = rmdir(dir.c_str());
            if (res){
                PFATAL("rmdir: '%s' failed, res: '%d'\n",
                    dir.c_str(),
                    res
                );
                goto fail;
            }
        }
    } else {
        res = unlink(a_path);
        if (res){
            PERROR("cannot unlink: '%s',"
                " res: '%d'\n",
                a_path,
                res
            );
        }
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    goto out;
}

inline int32_t s_recreate_dir(
    const char *a_path)
{
    int32_t err = -1;

    // remove if exist
    ::s_rm(a_path, 1, 1);

    err = ::s_mkdir(a_path, 1);

    return err;
}

enum ChompState {
    CHOMP_STATE_ERROR   = 0,
    CHOMP_STATE_FIRST_N,
    CHOMP_STATE_FIRST_R,
    CHOMP_STATE_DONE
};

inline void s_chomp(
    string &a_str)
{
    while (a_str.size()){
        char c = a_str.at(a_str.size() - 1);
        if (    '\r' == c
            ||  '\n' == c)
        {
            a_str = a_str.substr(0, a_str.size() - 1);
            continue;
        }
        break;
    }
}

inline void s_chomp2(
    string &a_str)
{
    ChompState state = CHOMP_STATE_FIRST_N;

    while (a_str.size()){
        char c = a_str.at(a_str.size() - 1);
        switch (state){
            case CHOMP_STATE_DONE:
                break;

            case CHOMP_STATE_FIRST_R:
                if ('\r' == c){
                    a_str.erase(a_str.size() - 1);
                }
                state = CHOMP_STATE_DONE;
                break;

            case CHOMP_STATE_FIRST_N:
                if ('\n' == c){
                    a_str.erase(a_str.size() - 1);
                    state = CHOMP_STATE_FIRST_R;
                } else {
                    state = CHOMP_STATE_DONE;
                }
                break;

            default:
                PERROR("invalid state: '%d'\n", state);
                break;
        }
        if (CHOMP_STATE_DONE == state){
            break;
        }
    }
}

inline void s_removeSpaces(
    string          &a_data,
    const int32_t   &a_from_start   = 1,
    const int32_t   &a_from_end     = 1)
{
    string::size_type size = 0;

    if (a_from_start){
        while (1){
            size = a_data.size();
            if (!size){
                break;
            }
            if (' ' != a_data.at(0)){
                break;
            }
            a_data = a_data.substr(1);
        }
    }
    if (a_from_end){
        while (1){
            size = a_data.size();
            if (!size){
                break;
            }
            if (' ' != a_data.at(size - 1)){
                break;
            }
            a_data = a_data.substr(0, size - 1);
        }
    }
}

inline void s_removeSpacesFromStart(
    string  &a_data)
{
    ::s_removeSpaces(a_data, 1, 0);
}

inline void s_removeSpacesFromEnd(
    string  &a_data)
{
    ::s_removeSpaces(a_data, 0, 1);
}

#endif

