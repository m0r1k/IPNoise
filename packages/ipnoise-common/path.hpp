class Path;

#ifndef PATH_HPP
#define PATH_HPP

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <wchar.h>

#include "utf.h"

#ifndef PATH_MAX
#define PATH_MAX 65535
#endif

using namespace std;

#include <ipnoise-common/strings.h>
#include <ipnoise-common/log_common.h>

class Path
{
    public:
        Path(const string &a_path = ""){
            m_path_absolute = 0;
            if (a_path.size()){
                setPath(a_path);
            }
        };

        ~Path(){
            clear();
        };

        enum PathOS {
            PATH_OS_UNKNOWN = 0,
            PATH_OS_WIN,
            PATH_OS_UNIX
        };

        enum PathType {
            PATH_TYPE_DIR_AND_FILE = 0,
            PATH_TYPE_ONLY_DIR
        };

        void clear(){
            m_path.clear();
            m_disk.clear();
            m_path_absolute = 0;
        }

        bool empty() const {
            return (m_disk.empty() && m_path.empty());
        }

        void addPath(
            const PathOS    &a_os_type,
            const string    &a_path)
        {
            string::size_type   pos = 0;
            string              l_path;
            Splitter            *parts = NULL;

            // skip if empty
            if (not a_path.size()){
                goto fail;
            }

            if (    PATH_OS_WIN     != a_os_type
                &&  PATH_OS_UNIX    != a_os_type)
            {
                PERROR("Unsupported path type: '%d'\n",
                    a_os_type
                );
                goto fail;
            }

            // skip if path is only delimeter
            if (PATH_OS_WIN == a_os_type){
                if ("\\" == a_path){
                    goto out;
                }
            }
            if (PATH_OS_UNIX == a_os_type){
                if ("/" == a_path){
                    goto out;
                }
            }

            if (PATH_OS_WIN == a_os_type){
                // search disk name
                if  (   3       <= a_path.size()
                    &&  ':'     == a_path.at(1)
                    &&  '\\'    == a_path.at(2))
                {
                    // disk name present
                    string cur_disk = getDisk();
                    string new_disk = string() + a_path.at(0);

                    if (cur_disk.size()){
                       // current disk exist
                        if (cur_disk != new_disk){
                            // attempt to add path to other disk :(
                            PERROR("Attempt to add path: '%s' to '%s'\n",
                                a_path.c_str(),
                                path().c_str()
                            );
                            goto fail;
                        }
                    } else {
                        // store current disk
                        string disk_name = string() + a_path.at(0);
                        setDisk(disk_name);
                    }

                    // skip disk name
                    pos += 2;
                }
            }

            // parse path
            for (; pos < a_path.size(); pos++){
                char cur = a_path.at(pos);
                if (pos > 0){
                    char prev = a_path.at(pos - 1);
                    if (PATH_OS_WIN == a_os_type){
                        if (    cur  == prev
                            &&  '\\' == cur)
                        {
                            // skip multiple '\\' in path
                            continue;
                        }
                    } else if (PATH_OS_UNIX == a_os_type){
                        if (    cur  == prev
                            &&  '/'  == cur)
                        {
                            // skip multiple '//' in path
                            continue;
                        }
                    } else {
                        PERROR("Unsupported path type: '%d'\n",
                            a_os_type
                        );
                        goto fail;
                    }
                }
                l_path += cur;
            }

            // remove last '/' or '\\'
            do {
                char c = '\0';
                if (not l_path.size()){
                    break;
                }
                // get last symbol
                c = l_path.at(l_path.size() - 1);
                if (PATH_OS_WIN == a_os_type){
                    if ('\\' == c){
                        // remove last symbol
                        l_path.resize(l_path.size() - 1);
                        break;
                    }
                } else if (PATH_OS_UNIX == a_os_type){
                    if ('/' == c){
                        // remove last symbol
                        l_path.resize(l_path.size() - 1);
                        break;
                    }
                } else {
                    PERROR("Unsupported path type: '%d'\n",
                        a_os_type
                    );
                    goto fail;
                }
            } while (0);

            if (PATH_OS_WIN == a_os_type){
                parts = new Splitter(l_path, "\\");
            } else if (PATH_OS_UNIX == a_os_type){
                parts = new Splitter(l_path, "/");
            } else {
                PERROR("Unsupported path type: '%d'\n",
                    a_os_type
                );
                goto fail;
            }

            if (not parts){
                PERROR("Cannot allocate memory\n");
                goto fail;
            }

            PDEBUG(5, "l_path: '%s', parts.size(): '%d'\n",
                l_path.c_str(),
                parts->size()
            );

            for (pos = 0; pos < parts->size(); pos++){
                string cur_path = (*parts)[pos];
                if (    0 == pos
                    &&  not m_path.size())
                {
                    // if path absolute, then parts[0] will be empty
                    if (not cur_path.size()){
                        setAbsolute(1);
                        continue;
                    }
                    setAbsolute(0);
                }
                m_path.push_back(cur_path);
            }

            out:
                if (parts){
                    delete parts;
                    parts = NULL;
                }
                return;
            fail:
                goto out;
        }

        void addPath(const string &a_path){
#if defined(WIN32) || defined(WIN64)
            addPath(PATH_OS_WIN, a_path);
#else
            addPath(PATH_OS_UNIX, a_path);
#endif
        }

        void addPathWin(const string &a_path){
            addPath(PATH_OS_WIN, a_path);
        }

        void addPathUnix(const string &a_path){
            addPath(PATH_OS_UNIX, a_path);
        }

        void setPath(const string &a_path){
            // clear current value
            clear();
            addPath(a_path);
        }

        void setPathWin(const string &a_path){
            // clear current value
            clear();
            addPathWin(a_path);
        }

        void setPathUnix(const string &a_path){
            // clear current value
            clear();
            addPathUnix(a_path);
        }

        int isExist() const {
            int res;
            struct stat st;
            res = stat(path().c_str(), &st);
            return res ? 0 : 1;
        }

        int isDir() const {
            int res, err = -1;
            struct stat st;

            res = stat(path().c_str(), &st);
            if (not res){
                if (S_ISDIR(st.st_mode)){
                    err = 1;
                } else {
                    err = 0;
                }
            }
            return err;
        }

        int isFile() const {
            int res, err = 0;
            struct stat st;

            res = stat(path().c_str(), &st);
            if (res){
                goto out;
            }

#if defined(WIN32) || defined(WIN64)
            {
                Path old_dir = Path::getCwd();
                res = cd();
                if (res){
                    // change dir failed
                    err = 0;
                } else {
                    // change dir success
                    err = 1;
                    // go back
                    old_dir.cd();
                }
            }
#else
            {
                if (S_ISREG(st.st_mode)){
                    err = 1;
                } else {
                    err = 0;
                }
            }
#endif

            out:
                return err;
        }

        int chmod(const string &a_perm){
            int res, err = -1;
            string  cur_path    = path();
            mode_t  mode        = parseMode(a_perm);

            res = ::chmod(cur_path.c_str(), mode);
            if (res){
                goto fail;
            }

            // all ok
            err = 0;

            out:
                return err;
            fail:
                if (err >= 0){
                    err = -1;
                }
                goto out;
        }

        string getPermStr() const {
            string          perm;
            int             res;
            struct stat     st;

            res = stat(path().c_str(), &st);
            if (res){
                goto fail;
            }

            // get perm as string
            perm = parseMode(st.st_mode);

            out:
                return perm;
            fail:
                goto out;
        }

        int isAbsolute() const {
            return m_path_absolute;
        }

        int getAbsolute() const {
            return m_path_absolute;
        }

        void setAbsolute(const int &a_val){
            m_path_absolute = a_val;
        }

        string getDisk(){
            return m_disk;
        }

        void setDisk(const string &a_disk){
            m_disk = a_disk;
        }

        string getPath(
            const PathOS    &a_os_type,
            const PathType  &a_path_type) const
        {
            int     i;
            string  path;

            if (    PATH_OS_WIN     != a_os_type
                &&  PATH_OS_UNIX    != a_os_type)
            {
                PERROR("Unsupported path type: '%d'\n",
                    a_os_type
                );
                goto fail;
            }

            // add disk name
            if (PATH_OS_WIN == a_os_type){
                if (m_disk.size()){
                    path += m_disk + ":";
                }
            }

            // process absolute path
            if (m_path_absolute){
                if (PATH_OS_WIN == a_os_type){
                    path += "\\";
                } else if (PATH_OS_UNIX == a_os_type){
                    path += "/";
                } else {
                    PERROR("Unsupported path type: '%d'\n",
                        a_os_type
                    );
                    goto fail;
                }
            }

            // process path parts
            for (i = 0; i < int(m_path.size()); i++){
                string cur_path = m_path[i];
                if (    PATH_TYPE_ONLY_DIR == a_path_type
                    &&  (i == (int(m_path.size()) - 1)))
                {
                    // only dir path
                    break;
                }
                if (i > 0){
                    if (PATH_OS_WIN == a_os_type){
                        path += "\\";
                    } else if (PATH_OS_UNIX == a_os_type){
                        path += "/";
                    } else {
                        PERROR("Unsupported path type: '%d'\n",
                            a_os_type
                        );
                        goto fail;
                    }
                }
                path += cur_path;
            }

            out:
                return path;
            fail:
                goto out;
        }
        string getDirPath() const {
            string ret;
#if defined(WIN32) || defined(WIN64)
            ret = getPath(PATH_OS_WIN,  PATH_TYPE_ONLY_DIR);
#else
            ret = getPath(PATH_OS_UNIX, PATH_TYPE_ONLY_DIR);
#endif
            return ret;
        }
        wstring path16() const {
            return Utf8ToUtf16(path());
        }
        string path() const {
            string ret;
#if defined(WIN32) || defined(WIN64)
            ret = pathWin();
#else
            ret = pathUnix();
#endif
            return ret;
        }
        wstring path16Win(){
            return Utf8ToUtf16(pathWin());
        }
        string pathWin() const {
            string ret;
            ret = getPath(PATH_OS_WIN,  PATH_TYPE_DIR_AND_FILE);
            return ret;
        }
        wstring path16Unix(){
            return Utf8ToUtf16(pathUnix());
        }
        string pathUnix() const {
            string ret;
            ret = getPath(PATH_OS_UNIX,  PATH_TYPE_DIR_AND_FILE);
            return ret;
        }
        wstring absolutePath16(){
            wstring ret;
            string  path = absolutePath();
            ret = Utf8ToUtf16(path);
            return ret;
        }
        string absolutePath() const {
            string ret;
#if defined(WIN32) || defined(WIN64)
            ret = absolutePathWin();
#else
            ret = absolutePathUnix();
#endif
            return ret;
        }
        wstring absolutePath16Win(){
            return Utf8ToUtf16(absolutePathWin());
        }
        string absolutePathWin() const {
            Path res, ret;
            res = getPath(PATH_OS_WIN,  PATH_TYPE_DIR_AND_FILE);
            if (not res.isAbsolute()){
                ret = Path::getCwd();
            }
            ret += res;
            return ret.path();
        }
        wstring absolutePath16Unix(){
            return Utf8ToUtf16(absolutePathUnix()).c_str();
        }
        string absolutePathUnix() const {
            Path res, ret;
            res = getPath(PATH_OS_UNIX,  PATH_TYPE_DIR_AND_FILE);
            if (not res.isAbsolute()){
                ret = Path::getCwd();
            }
            ret += res;
            return ret.path();
        }
        string getBaseName() const {
            string ret;

            if (not m_path.size()){
                goto out;
            }

            ret = m_path[m_path.size() - 1];

            out:
                return ret;
        }

        int cd() const {
            int res, err = -1;
            string l_path;

            if (isDir()){
                l_path = path();
            } else {
                l_path = getDirPath();
            };

            if (not l_path.size()){
                goto fail;
            }

#if defined(WIN32) || defined(WIN64)
            res = _wchdir(Utf8ToUtf16(l_path).c_str());
#else
            res = chdir(l_path.c_str());
#endif
            if (res){
                goto fail;
            }

            // all ok
            err = 0;

            out:
                return err;
            fail:
                if (err >= 0){
                    err = -1;
                }
                goto out;
        }

        FILE * fopen(const string &a_mode) const {
            FILE *file = NULL;
#if defined(WIN32) || defined(WIN64)
            int i;
            wstring mode;
            for (i = 0;
                i < int(a_mode.size());
                i++)
            {
                mode += a_mode.at(i);
            }
            file = _wfopen(
                Utf8ToUtf16(path()).c_str(),
                mode.c_str()
            );
#else
            file = ::fopen(
                path().c_str(),
                a_mode.c_str()
            );
#endif
            return file;
        }

        int mkdirr(mode_t a_mode = 0755) const {
            int res, err = -1;
            Splitter    *parts  = NULL;
            Path        old_dir = Path::getCwd();

            vector <string>::const_iterator path_it;
            struct stat st;

            if (m_path.empty()){
                goto out;
            }

            // if path m_path_absolute, change dir to 'root'
#if defined(WIN32) || defined(WIN64)
            if (m_path_absolute){
                string path;
                if (m_disk.size()){
                    path += m_disk + ":\\";
                }
                _wchdir(Utf8ToUtf16(path).c_str());
            }
#else
            if (m_path_absolute){
                chdir("/");
            }
#endif

            for (path_it = m_path.begin();
                path_it != m_path.end();
                path_it++)
            {
                string part = *path_it;

                PDEBUG(5, "part: '%s'\n", part.c_str());
                if (not part.size()){
                    PERROR("Empty 'part' for path: '%s'\n",
                        path().c_str()
                    );
                    goto fail;
                }

                res = stat(part.c_str(), &st);
                PDEBUG(5, "stat: '%s', res: '%d'\n",
                    part.c_str(),
                    res
                );
                if (res){
#if defined(WIN32) || defined(WIN64)
                    // TODO perm?
                    res = _wmkdir(Utf8ToUtf16(part).c_str());
#else
                    if ((path_it + 1) == m_path.end()){
                        res = mkdir(part.c_str(), a_mode);
                    } else {
                        res = mkdir(part.c_str(), 0755);
                    }
#endif
                    if (res){
                        PERROR("Cannot create dir: '%s'\n",
                            part.c_str()
                        );
                        goto fail;
                    }
                }
                chdir(part.c_str());
            }

            // all ok
            err = 0;

        out:
            // goto old dir
            old_dir.cd();

            // remove parts
            if (parts){
                delete parts;
                parts = NULL;
            }
            return err;

        fail:
            if (err >= 0){
                err = -1;
            }
            goto out;
        }

        Path & operator+ (const string &a_path){
            addPath(a_path);
            return *this;
        }

        Path & operator+= (const string &a_path){
            addPath(a_path);
            return *this;
        }

        Path & operator+= (const Path &a_right){
            addPath(a_right.path());
            return *this;
        }

        Path & operator= (const Path &a_right){
            const string path = a_right.path();
            this->setPath(path);
            return *this;
        }

        static Path getCwd(){
            Path path;

#if defined(WIN32) || defined(WIN64)
            wchar_t *wcur_dir = NULL;
            wcur_dir = _wgetcwd(NULL, 0);
            if (not wcur_dir){
                PERROR("_wgetcwd failed\n");
                goto fail;
            }
            path.setPath(Utf16ToUtf8(wcur_dir));
            free(wcur_dir);
#else
            char *cur_dir = NULL;
            cur_dir = getcwd(NULL, 0);
            if (not cur_dir){
                PERROR("getcwd failed\n");
                goto fail;
            }
            path.setPath(cur_dir);
            free(cur_dir);
#endif

            out:
                return path;
            fail:
                goto out;
        }

        static string parseMode(mode_t a_mode){
            string perm = "";

#if defined(WIN32) || defined(WIN64)
            // TODO
#else
            // owner
            perm += (S_IRUSR & a_mode) ? "r" : "-";
            perm += (S_IWUSR & a_mode) ? "w" : "-";
            perm += (S_IXUSR & a_mode) ? "x" : "-";

            // group
            perm += (S_IRGRP & a_mode) ? "r" : "-";
            perm += (S_IWGRP & a_mode) ? "w" : "-";
            perm += (S_IXGRP & a_mode) ? "x" : "-";

            // others
            perm += (S_IROTH & a_mode) ? "r" : "-";
            perm += (S_IWOTH & a_mode) ? "w" : "-";
            perm += (S_IXOTH & a_mode) ? "x" : "-";
#endif

            return perm;
        }

        static mode_t parseMode(
            const string &a_mode)
        {
            mode_t mode = 0;

            if (9 != a_mode.size()){
                PERROR("Invalid mode line: '%s'\n",
                    a_mode.c_str());
                goto fail;
            }

#if defined(WIN32) || defined(WIN64)
            // TODO
#else
            // owner
            if ('r' == a_mode.at(0)){
                mode |= S_IRUSR;
            }
            if ('w' == a_mode.at(1)){
                mode |= S_IWUSR;
            }
            if ('x' == a_mode.at(2)){
                mode |= S_IXUSR;
            }

            // group
            if ('r' == a_mode.at(3)){
                mode |= S_IRGRP;
            }
            if ('w' == a_mode.at(4)){
                mode |= S_IWGRP;
            }
            if ('x' == a_mode.at(5)){
                mode |= S_IXGRP;
            }

            // others
            if ('r' == a_mode.at(6)){
                mode |= S_IROTH;
            }
            if ('w' == a_mode.at(7)){
                mode |= S_IWOTH;
            }
            if ('x' == a_mode.at(8)){
                mode |= S_IXOTH;
            }
#endif

        out:
            return mode;

        fail:
            if (mode > 0){
                mode = 0;
            }
            goto out;
        }

    private:
        string          m_disk;
        vector<string>  m_path;
        int             m_path_absolute;
};

#endif

